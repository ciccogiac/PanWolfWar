#include "Components/TransformationComponent.h"

#include "PanWolfWar/PanWolfWarCharacter.h"

#include "UserWidgets/TransformationWidget.h"

#include "PanWolfWar/DebugHelper.h"

#include "NiagaraComponent.h"

#include "Components/AttributeComponent.h"
#include "Components/InteractComponent.h"

#include "Components/PandolfoComponent.h"
#include "Components/PanWolfComponent.h"
#include "Components/PandolFlowerComponent.h"

UTransformationComponent::UTransformationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	PanWolfWarCharacter = Cast<APanWolfWarCharacter>(GetOwner());

}


void UTransformationComponent::SelectRightTransformation()
{
	DesiredTransformationState_ID = (DesiredTransformationState_ID+1) % PossibleTransformationState.Num();
	TransformationWidget->SetTransformation(PossibleTransformationState[DesiredTransformationState_ID]);
}

void UTransformationComponent::SelectLeftTransformation()
{
	DesiredTransformationState_ID -= 1;
	if (DesiredTransformationState_ID < 0) { DesiredTransformationState_ID = PossibleTransformationState.Num() - 1; }

	TransformationWidget->SetTransformation(PossibleTransformationState[DesiredTransformationState_ID]);
}

void UTransformationComponent::SelectDesiredTransformation(int32 TransformationState_ID)
{
	DesiredTransformationState_ID = TransformationState_ID;
	ApplyTrasformation();
}

void UTransformationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PanWolfWarCharacter)
	{
		Attributes = PanWolfWarCharacter->GetAttributeComponent();
		Attributes->SetTransformationComponent(this);
		InteractComponent = PanWolfWarCharacter->GetInteractComponent();
	}


	TransformationWidget = CreateWidget< UTransformationWidget>(GetWorld(), TransformationWidgetClass, FName("TransformationWidget"));
	if (TransformationWidget)
	{
		TransformationWidget->AddToViewport();
	}

	TransformationWidget->SetTransformation(ETransformationState::ETS_PanFlower);
	
}

bool UTransformationComponent::CanTrasform(const int32 NewTransformation_ID)
{
	if (NewTransformation_ID >= PossibleTransformationState.Num() || NewTransformation_ID < 0) return false;
	if (CurrentTransformationState == ETransformationState::ETS_Transforming) return false;
	if (Attributes->IsInConsumingState()) return false;
	if(PanWolfWarCharacter->GetPandolfoComponent()->IsClimbing()) return false;
	if (PossibleTransformationState[NewTransformation_ID] == CurrentTransformationState) return false;

	return true;
}

void UTransformationComponent::ApplyTrasformation()
{
	
	const int32 LocalTransformation_ID = DesiredTransformationState_ID;
	if (!CanTrasform(LocalTransformation_ID)) return;

	ETransformationState PreviousTransformationState = CurrentTransformationState;
	CurrentTransformationState = ETransformationState::ETS_Transforming;
	ETransformationState NewTransformationState = PossibleTransformationState[LocalTransformation_ID];

	

	switch (NewTransformationState)
	{
	case ETransformationState::ETS_Pandolfo:

		ExecuteTransformation(NewTransformationState, Pandolfo_Material1, Pandolfo_Material2);
		HandleComponentActivation(NewTransformationState, PreviousTransformationState);
		break;

	case ETransformationState::ETS_Transforming:
		break;

	case ETransformationState::ETS_PanWolf:

		if (!Attributes->ConsumeBeer()) { CurrentTransformationState = PreviousTransformationState; break; }
		ExecuteTransformation(NewTransformationState, Panwolf_Material1, Panwolf_Material1);
		HandleComponentActivation(NewTransformationState, PreviousTransformationState);
		break;

	case ETransformationState::ETS_PanFlower:
		
		if (!Attributes->ConsumeFlowerStamina()) { CurrentTransformationState = PreviousTransformationState; break; }
		ExecuteTransformation(NewTransformationState, Pandolflower_Material1, Pandolflower_Material2, Pandolflower_Niagara);
		HandleComponentActivation(NewTransformationState, PreviousTransformationState);
		break;

	default:

		ExecuteTransformation(NewTransformationState, Pandolfo_Material1, Pandolfo_Material2);
		break;
	}
	


}

void UTransformationComponent::ExecuteTransformation(ETransformationState NewTransformationState, UMaterialInterface* Material1, UMaterialInterface* Material2, UNiagaraSystem* NiagaraTransformation)
{
	PanWolfWarCharacter->GetMesh()->SetMaterial(0, Material1);
	PanWolfWarCharacter->GetMesh()->SetMaterial(1, Material2);
	PanWolfWarCharacter->GetNiagaraTransformation()->SetAsset(NiagaraTransformation);
	PanWolfWarCharacter->GetNiagaraTransformationEffect()->Activate(true);
	CurrentTransformationState = NewTransformationState;
	TransformationWidget->SetTransformation(CurrentTransformationState);

	InteractComponent->ResetOverlappingObject();
}

void UTransformationComponent::HandleComponentActivation(ETransformationState NewTransformationState, ETransformationState PreviousTransformationState)
{
	switch (PreviousTransformationState)
	{
	case ETransformationState::ETS_Pandolfo:
		PanWolfWarCharacter->GetPandolfoComponent()->Deactivate();
		break;

	case ETransformationState::ETS_Transforming:
		break;

	case ETransformationState::ETS_PanWolf:
		PanWolfWarCharacter->GetPanWolfComponent()->Deactivate();
		break;

	case ETransformationState::ETS_PanFlower:
		PanWolfWarCharacter->GetPandolFlowerComponent()->Deactivate();
		break;

	default:
		break;
	}

	switch (NewTransformationState)
	{
	case ETransformationState::ETS_Pandolfo:
		PanWolfWarCharacter->GetPandolfoComponent()->Activate();
		break;

	case ETransformationState::ETS_Transforming:
		break;

	case ETransformationState::ETS_PanWolf:
		PanWolfWarCharacter->GetPanWolfComponent()->Activate();
		break;

	case ETransformationState::ETS_PanFlower:
		PanWolfWarCharacter->GetPandolFlowerComponent()->Activate();
		break;

	default:
		break;
	}
}



