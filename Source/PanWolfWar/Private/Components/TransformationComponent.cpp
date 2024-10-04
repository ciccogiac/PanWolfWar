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

void UTransformationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PanWolfWarCharacter)
	{
		Attributes = PanWolfWarCharacter->GetAttributeComponent();
		InteractComponent = PanWolfWarCharacter->GetInteractComponent();
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
	}

}

#pragma region SelectingTransformation

void UTransformationComponent::SelectRightTransformation()
{
	SelectDesiredTransformation(3);
}

void UTransformationComponent::SelectLeftTransformation()
{

	SelectDesiredTransformation(2);
}

void UTransformationComponent::SelectUPTransformation()
{
	SelectDesiredTransformation(1);
}

void UTransformationComponent::SelectDesiredTransformation(int32 TransformationState_ID)
{
	DesiredTransformationState_ID = TransformationState_ID;
	ApplyTrasformation();
}

#pragma endregion

#pragma region HandleTransformation

bool UTransformationComponent::CanTrasform(const int32 NewTransformation_ID)
{
	if (NewTransformation_ID >= PossibleTransformationState.Num() || NewTransformation_ID < 0) return false;
	if (PossibleTransformationState[NewTransformation_ID] == CurrentTransformationState) return false;
	if (CurrentTransformationState == ETransformationState::ETS_Transforming) return false;
	//if (Attributes->IsInConsumingState()) return false;
	//if (PanWolfWarCharacter->GetPandolfoComponent()->IsClimbing()) return false;

	if (PanWolfWarCharacter->GetPandolfoComponent()->PandolfoState != EPandolfoState::EPS_Pandolfo) return false;

	//UAnimInstance* AnimInstance = PanWolfWarCharacter->GetMesh()->GetAnimInstance();
	//if (AnimInstance && AnimInstance->IsAnyMontagePlaying()) return false;

	return true;
}

void UTransformationComponent::ApplyTrasformation()
{

	const int32 LocalTransformation_ID = DesiredTransformationState_ID;
	if (!CanTrasform(LocalTransformation_ID)) return;

	ETransformationState PreviousTransformationState = CurrentTransformationState;
	CurrentTransformationState = ETransformationState::ETS_Transforming;
	ETransformationState NewTransformationState = PossibleTransformationState[LocalTransformation_ID];

	SetTransformation(NewTransformationState, PreviousTransformationState);
}

void UTransformationComponent::SetTransformation(ETransformationState NewTransformationState, ETransformationState PreviousTransformationState)
{
	switch (NewTransformationState)
	{
	case ETransformationState::ETS_Pandolfo:

		ExecuteTransformation(NewTransformationState);
		HandleComponentActivation(NewTransformationState, PreviousTransformationState);
		break;

	case ETransformationState::ETS_Transforming:
		break;

	case ETransformationState::ETS_PanWolf:

		if (!Attributes->ConsumeBeer()) { CurrentTransformationState = PreviousTransformationState; break; }
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
			SelectDesiredTransformation(0);

		}

	}

}

void UTransformationComponent::ExecuteTransformation(ETransformationState NewTransformationState)
{
	CurrentTransformationState = NewTransformationState;

	OnTransformationStateChanged.Broadcast(NewTransformationState);

	InteractComponent->ResetOverlappingObject();

	//PanWolfWarCharacter->GetNiagaraTransformationEffect()->Activate(true);

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
}

void UTransformationComponent::AnnulTrasnformation()
{
	if (CurrentTransformationState == ETransformationState::ETS_Pandolfo || CurrentTransformationState == ETransformationState::ETS_Transforming) return;

	GetWorld()->GetTimerManager().ClearTimer(Transformation_TimerHandle);
	SelectDesiredTransformation(0);
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

