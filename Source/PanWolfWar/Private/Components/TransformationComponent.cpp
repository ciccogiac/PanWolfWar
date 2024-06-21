#include "Components/TransformationComponent.h"

#include "PanWolfWar/PanWolfWarCharacter.h"
#include "Components/ClimbingComponent.h"

#include "UserWidgets/TransformationWidget.h"

#include "PanWolfWar/DebugHelper.h"

#include "NiagaraComponent.h"

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

void UTransformationComponent::SelectDesiredTransformation()
{

}


void UTransformationComponent::BeginPlay()
{
	Super::BeginPlay();

	//TransformationWidget = Cast< UTransformationWidget> ( CreateWidget( GetWorld(), TransformationWidgetClass, FName("TransformationWidget")));

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
	if(PanWolfWarCharacter->GetClimbingComponent()->IsClimbing()) return false;

	if (PossibleTransformationState[DesiredTransformationState_ID] == CurrentTransformationState) return false;

	return true;
}

void UTransformationComponent::ApplyTrasformation()
{
	

	if (!CanTrasform()) return;

	CurrentTransformationState = ETransformationState::ETS_Transforming;

	ETransformationState NewTransformationState = PossibleTransformationState[DesiredTransformationState_ID];

	//ActivateDeactivate CLimbing
	if(NewTransformationState != ETransformationState::ETS_Pandolfo) PanWolfWarCharacter->GetClimbingComponent()->SetCanClimb(false);
	else { PanWolfWarCharacter->GetClimbingComponent()->SetCanClimb(true); }

	switch (NewTransformationState)
	{
	case ETransformationState::ETS_Pandolfo:
		PanWolfWarCharacter->GetMesh()->SetMaterial(0, Pandolfo_Material1);
		PanWolfWarCharacter->GetMesh()->SetMaterial(1, Pandolfo_Material2);
		PanWolfWarCharacter->GetNiagaraTransformation()->SetAsset(nullptr);
		break;
	case ETransformationState::ETS_Transforming:
		break;
	case ETransformationState::ETS_PanWolf:
		PanWolfWarCharacter->GetMesh()->SetMaterial(0, Panwolf_Material1);
		PanWolfWarCharacter->GetMesh()->SetMaterial(1, Panwolf_Material1);
		PanWolfWarCharacter->GetNiagaraTransformation()->SetAsset(nullptr);
		break;
	case ETransformationState::ETS_PanFlower:
		PanWolfWarCharacter->GetMesh()->SetMaterial(0, Pandolflower_Material1);
		PanWolfWarCharacter->GetMesh()->SetMaterial(1, Pandolflower_Material2);
		PanWolfWarCharacter->GetNiagaraTransformation()->SetAsset(Pandolflower_Niagara);
		break;
	default:
		PanWolfWarCharacter->GetMesh()->SetMaterial(0, Pandolfo_Material1);
		PanWolfWarCharacter->GetMesh()->SetMaterial(1, Pandolfo_Material2);
		break;
	}
	

	////
	////

	CurrentTransformationState = NewTransformationState;
}




