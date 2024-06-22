#include "Components/TransformationComponent.h"

#include "PanWolfWar/PanWolfWarCharacter.h"
#include "Components/ClimbingComponent.h"

#include "UserWidgets/TransformationWidget.h"

#include "PanWolfWar/DebugHelper.h"

#include "NiagaraComponent.h"

#include "Components/AttributeComponent.h"

#include "TimerManager.h"

UTransformationComponent::UTransformationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	PanWolfWarCharacter = Cast<APanWolfWarCharacter>(GetOwner());

}

void UTransformationComponent::SelectRightTransformation()
{
	if (Attributes->IsInConsumingState()) return;

	DesiredTransformationState_ID = (DesiredTransformationState_ID+1) % PossibleTransformationState.Num();

	TransformationWidget->SetTransformation(PossibleTransformationState[DesiredTransformationState_ID]);
}

void UTransformationComponent::SelectLeftTransformation()
{
	if (Attributes->IsInConsumingState()) return;

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

	//TransformationWidget = Cast< UTransformationWidget> ( CreateWidget( GetWorld(), TransformationWidgetClass, FName("TransformationWidget")));
	if (PanWolfWarCharacter)
	{
		Attributes = PanWolfWarCharacter->GetAttributeComponent();
		Attributes->SetTransformationComponent(this);
	}


	TransformationWidget = CreateWidget< UTransformationWidget>(GetWorld(), TransformationWidgetClass, FName("TransformationWidget"));
	if (TransformationWidget)
	{
		TransformationWidget->AddToViewport();
	}

	TransformationWidget->SetTransformation(ETransformationState::ETS_Pandolfo);

	
}


bool UTransformationComponent::CanTrasform()
{
	if (CurrentTransformationState == ETransformationState::ETS_Transforming) return false;
	if (Attributes->IsInConsumingState()) return false;
	if(PanWolfWarCharacter->GetClimbingComponent()->IsClimbing()) return false;

	if (PossibleTransformationState[DesiredTransformationState_ID] == CurrentTransformationState) return false;

	return true;
}

void UTransformationComponent::ApplyTrasformation()
{
	
	if (!CanTrasform()) return;

	ETransformationState PreviousTransformationState = CurrentTransformationState;

	CurrentTransformationState = ETransformationState::ETS_Transforming;

	ETransformationState NewTransformationState = PossibleTransformationState[DesiredTransformationState_ID];

	if(NewTransformationState != ETransformationState::ETS_Pandolfo) PanWolfWarCharacter->GetClimbingComponent()->SetCanClimb(false);
	else { PanWolfWarCharacter->GetClimbingComponent()->SetCanClimb(true); }

	switch (NewTransformationState)
	{
	case ETransformationState::ETS_Pandolfo:
		PanWolfWarCharacter->GetMesh()->SetMaterial(0, Pandolfo_Material1);
		PanWolfWarCharacter->GetMesh()->SetMaterial(1, Pandolfo_Material2);
		PanWolfWarCharacter->GetNiagaraTransformation()->SetAsset(nullptr);
		PanWolfWarCharacter->GetNiagaraTransformationEffect()->Activate(true);
		CurrentTransformationState = NewTransformationState;
		TransformationWidget->SetTransformation(CurrentTransformationState);
		break;
	case ETransformationState::ETS_Transforming:
		break;
	case ETransformationState::ETS_PanWolf:

		if (!Attributes->ConsumeBeer()) { CurrentTransformationState = PreviousTransformationState; break; }
		
		PanWolfWarCharacter->GetMesh()->SetMaterial(0, Panwolf_Material1);
		PanWolfWarCharacter->GetMesh()->SetMaterial(1, Panwolf_Material1);
		PanWolfWarCharacter->GetNiagaraTransformation()->SetAsset(nullptr);
		PanWolfWarCharacter->GetNiagaraTransformationEffect()->Activate(true);
		CurrentTransformationState = NewTransformationState;
		TransformationWidget->SetTransformation(CurrentTransformationState);

		break;

	case ETransformationState::ETS_PanFlower:
		
		if (!Attributes->ConsumeFlowerStamina()) { CurrentTransformationState = PreviousTransformationState; break; }

		PanWolfWarCharacter->GetMesh()->SetMaterial(0, Pandolflower_Material1);
		PanWolfWarCharacter->GetMesh()->SetMaterial(1, Pandolflower_Material2);
		PanWolfWarCharacter->GetNiagaraTransformation()->SetAsset(Pandolflower_Niagara);
		PanWolfWarCharacter->GetNiagaraTransformationEffect()->Activate(true);
		CurrentTransformationState = NewTransformationState;
		TransformationWidget->SetTransformation(CurrentTransformationState);

		break;
	default:
		PanWolfWarCharacter->GetMesh()->SetMaterial(0, Pandolfo_Material1);
		PanWolfWarCharacter->GetMesh()->SetMaterial(1, Pandolfo_Material2);
		PanWolfWarCharacter->GetNiagaraTransformationEffect()->Activate(true);
		CurrentTransformationState = NewTransformationState;
		TransformationWidget->SetTransformation(CurrentTransformationState);

		break;
	}
	

}




