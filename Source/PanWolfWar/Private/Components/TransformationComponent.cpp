#include "Components/TransformationComponent.h"

#include "PanWolfWar/PanWolfWarCharacter.h"
#include "NiagaraComponent.h"

#include "Components/AttributeComponent.h"
#include "Components/InteractComponent.h"

#include "Components/PandolfoComponent.h"
#include "Components/PanWolfComponent.h"
#include "Components/PandolFlowerComponent.h"
#include "Components/PanBirdComponent.h"

#include "TimerManager.h"

#include "Components/UI/PandoUIComponent.h"

#include "PanWolfWar/DebugHelper.h"

#pragma region EngineFunctions

UTransformationComponent::UTransformationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	PanWolfWarCharacter = Cast<APanWolfWarCharacter>(GetOwner());

}

void UTransformationComponent::ClearAllTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(Transformation_TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(RegenFlower_TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(RegenBird_TimerHandle);
}

void UTransformationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PanWolfWarCharacter)
	{
		Attributes = PanWolfWarCharacter->GetAttributeComponent();
		InteractComponent = PanWolfWarCharacter->GetInteractComponent();
		PandolfoComponent = PanWolfWarCharacter->GetPandolfoComponent();
		PanWolfComponent = PanWolfWarCharacter->GetPanWolfComponent();
	}


}

#pragma endregion


void UTransformationComponent::InitializeTransformationUI(UPandoUIComponent* _PandoUIComponent)
{
	if (!_PandoUIComponent) return;

	PandoUIComponent = _PandoUIComponent;

	if (PandoUIComponent)
	{
		PandoUIComponent->OnFlowerIconVisibilityChanged.Broadcast(false);
		PandoUIComponent->OnBirdIconVisibilityChanged.Broadcast(false);
		SetTransformationWidgetVisibility();
	}

}

void UTransformationComponent::SetPossibleTransformationState(TArray<ETransformationState> _PossibleTransformationState)
{
	PossibleTransformationState = _PossibleTransformationState;
	SetTransformationWidgetVisibility();
}

void UTransformationComponent::SetTransformationWidgetVisibility()
{
	if (PandoUIComponent)
	{
		if (PossibleTransformationState.Contains(ETransformationState::ETS_PanFlower))
			PandoUIComponent->OnFlowerWidgetVisibilityChanged.Broadcast(true);
		else
			PandoUIComponent->OnFlowerWidgetVisibilityChanged.Broadcast(false);

		if (PossibleTransformationState.Contains(ETransformationState::ETS_PanWolf))
			PandoUIComponent->OnWolfWidgetVisibilityChanged.Broadcast(true);
		else
			PandoUIComponent->OnWolfWidgetVisibilityChanged.Broadcast(false);
	}
}

#pragma region SelectingTransformation

void UTransformationComponent::SelectPandolfoTransformation()
{
	SelectDesiredTransformation(ETransformationState::ETS_Pandolfo);
}

void UTransformationComponent::SelectFlowerTransformation()
{

	SelectDesiredTransformation(ETransformationState::ETS_PanFlower);
}

void UTransformationComponent::SelectWolfTransformation()
{
	SelectDesiredTransformation(ETransformationState::ETS_PanWolf);
}

void UTransformationComponent::SelectBirdTransformation()
{
	SelectDesiredTransformation(ETransformationState::ETS_PanBird);
}

void UTransformationComponent::SelectDesiredTransformation(const ETransformationState DesiredTransformationState)
{
	if (!CanTrasform(DesiredTransformationState)) return;

	const ETransformationState PreviousTransformationState = CurrentTransformationState;
	CurrentTransformationState = ETransformationState::ETS_Transforming;

	SetTransformation(DesiredTransformationState, PreviousTransformationState);
}

#pragma endregion

#pragma region HandleTransformation

bool UTransformationComponent::CanTrasform(const ETransformationState NewDesiredTransformationState)
{
	if (!PossibleTransformationState.Contains(NewDesiredTransformationState)) return false;
	if (NewDesiredTransformationState == CurrentTransformationState) return false;
	if (CurrentTransformationState == ETransformationState::ETS_Transforming) return false;

	if (CurrentTransformationState == ETransformationState::ETS_Pandolfo && PandolfoComponent && PandolfoComponent->PandolfoState != EPandolfoState::EPS_Pandolfo) return false;

	return true;
}

void UTransformationComponent::SetTransformation(const ETransformationState NewTransformationState, const ETransformationState PreviousTransformationState)
{
	switch (NewTransformationState)
	{
	case ETransformationState::ETS_Pandolfo:
		GetWorld()->GetTimerManager().ClearTimer(Transformation_TimerHandle);
		ExecuteTransformation(NewTransformationState);
		HandleComponentActivation(NewTransformationState, PreviousTransformationState);
		break;

	case ETransformationState::ETS_Transforming:
		break;

	case ETransformationState::ETS_PanWolf:

		if (!Attributes->ConsumeBeer() || !PanWolfComponent->CheckCapsuleSpace()) { CurrentTransformationState = PreviousTransformationState; break; }
		ExecuteTransformation(NewTransformationState);
		HandleComponentActivation(NewTransformationState, PreviousTransformationState);
		GetWorld()->GetTimerManager().SetTimer(Transformation_TimerHandle, [this, NewTransformationState]() {this->ConsumingTransformation(NewTransformationState); }, 0.05f, true);
		break;

	case ETransformationState::ETS_PanFlower:

		//if (!bCanRegenFlower || !Attributes->ConsumeFlowerStamina()) { CurrentTransformationState = PreviousTransformationState; break; }
		if (!Attributes->ConsumeFlowerStamina()) { CurrentTransformationState = PreviousTransformationState; break; }
		ExecuteTransformation(NewTransformationState);
		HandleComponentActivation(NewTransformationState, PreviousTransformationState);
		GetWorld()->GetTimerManager().SetTimer(Transformation_TimerHandle, [this, NewTransformationState]() {this->ConsumingTransformation(NewTransformationState); }, 0.05f, true);
		break;

	case ETransformationState::ETS_PanBird:
		if (!bBirdConsuptionStopped || !Attributes->ConsumeBirdStamina()) { CurrentTransformationState = PreviousTransformationState; break; }
		//if (!Attributes->ConsumeBirdStamina()) { CurrentTransformationState = PreviousTransformationState; break; }
		ExecuteTransformation(NewTransformationState);
		HandleComponentActivation(NewTransformationState, PreviousTransformationState);
		GetWorld()->GetTimerManager().SetTimer(Transformation_TimerHandle, [this, NewTransformationState]() {this->ConsumingTransformation(NewTransformationState); }, 0.05f, true);
		break;

	}

}

void UTransformationComponent::ConsumingTransformation(ETransformationState TransfomingState)
{
	if (CurrentTransformationState == TransfomingState)
	{
		bool bEndTrasformation = false;

		switch (TransfomingState)
		{
		case ETransformationState::ETS_PanWolf:
			bEndTrasformation = !Attributes->ConsumingBeer();
			break;

		case ETransformationState::ETS_PanFlower:
			bEndTrasformation = bFlowerConsuptionStopped ? false : !Attributes->ConsumingFlowerStamina();
			break;

		case ETransformationState::ETS_PanBird:
			bEndTrasformation = bBirdConsuptionStopped ? false : !Attributes->ConsumingBirdStamina();
			break;
		}

		if (bEndTrasformation)
		{
			GetWorld()->GetTimerManager().ClearTimer(Transformation_TimerHandle);
			SelectDesiredTransformation(ETransformationState::ETS_Pandolfo);

		}

	}

}

void UTransformationComponent::ExecuteTransformation(ETransformationState NewTransformationState)
{
	CurrentTransformationState = NewTransformationState;

	OnTransformationStateChanged.Broadcast(NewTransformationState);

	PanWolfWarCharacter->HandleTransformationChangedState();

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

	case ETransformationState::ETS_PanBird:
		PanWolfWarCharacter->GetPanBirdComponent()->Deactivate();
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

	case ETransformationState::ETS_PanBird:
		PanWolfWarCharacter->GetPanBirdComponent()->Activate();
		break;

	default:
		break;
	}

	InteractComponent->ResetOverlappingObject();
}


#pragma endregion

#pragma region FlowerRegeneration

void UTransformationComponent::SetCanRegenFlower(bool CanRegenFlower)
{
	/*Attributes->SetTransformationIcon(CanRegenFlower, false);*/
	bFlowerConsuptionStopped = CanRegenFlower;

	if(PandoUIComponent)
		PandoUIComponent->OnFlowerIconVisibilityChanged.Broadcast(CanRegenFlower);

	if (CanRegenFlower)
		GetWorld()->GetTimerManager().SetTimer(RegenFlower_TimerHandle, Attributes, &UAttributeComponent::RegenFlowerStamina, 0.05f, true);

	else
		GetWorld()->GetTimerManager().ClearTimer(RegenFlower_TimerHandle);
	
}

#pragma endregion

#pragma region BirdRegeneration

void UTransformationComponent::SetCanRegenBird(bool CanRegenBird)
{
	bBirdConsuptionStopped = CanRegenBird;

	if (PandoUIComponent)
		PandoUIComponent->OnBirdIconVisibilityChanged.Broadcast(CanRegenBird);

	if (CanRegenBird)
		GetWorld()->GetTimerManager().SetTimer(RegenBird_TimerHandle, Attributes, &UAttributeComponent::RegenBirdStamina, 0.05f, true);
	else
		GetWorld()->GetTimerManager().ClearTimer(RegenBird_TimerHandle);

}


#pragma endregion


bool UTransformationComponent::AddItemStamina(ETransformationObjectTypes TransformationItemType,  float Value)
{
	switch (TransformationItemType) 
	{
	case ETransformationObjectTypes::ETOT_PanWolf_Object :
		if (CurrentTransformationState != ETransformationState::ETS_PanWolf) return false;
		Attributes->AddBeerStamina(Value);
		return true;

	case ETransformationObjectTypes::ETOT_PandolFlower_Object :
		if (CurrentTransformationState != ETransformationState::ETS_PanFlower) return false;
		Attributes->AddFlowerStamina(Value);
		return true;

	case ETransformationObjectTypes::ETOT_PanBird_Object :
		if (CurrentTransformationState != ETransformationState::ETS_PanBird) return false;
		Attributes->AddBirdStamina(Value);
		return true;

	}
	return false;
}

