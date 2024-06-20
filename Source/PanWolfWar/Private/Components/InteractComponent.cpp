// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InteractComponent.h"

#include "Actors/InteractableObject.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actors/MovableObject.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "InputActionValue.h"

#include "PanWolfWar/DebugHelper.h"


UInteractComponent::UInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
}

void UInteractComponent::SetInteractState()
{
	if (OverlappingObject)
	{
		if(Cast<AMovableObject>(OverlappingObject))
			InteractState = EInteractState::EIS_MovingObject;
		else
			InteractState = EInteractState::EIS_NOTinteracting;
	}
}

void UInteractComponent::Interact()
{
	if (OverlappingObject)
	{
		InteractState = EInteractState::EIS_Interacting;

		if (OverlappingObject->Interact(CharacterOwner))
		{
			SetInteractState();
			OnEnterInteractStateDelegate.ExecuteIfBound();
		}
			
		else
		{
			InteractState = EInteractState::EIS_NOTinteracting;
			OnExitInteractStateDelegate.ExecuteIfBound();		
		}
			
	}
		
}

void UInteractComponent::InteractMove(const FInputActionValue& Value)
{
	if (OverlappingObject)
		OverlappingObject->Move(Value);
}

bool UInteractComponent::SetOverlappingObject(AInteractableObject* InteractableObject , bool bEnter)
{
	//Sto uscendo da oggetto overlappato prima e ho interazioni
	if (InteractableObject && InteractableObject == OverlappingObject && InteractState != EInteractState::EIS_NOTinteracting)
	{
		
		OverlappingObject->Interact(nullptr);
		InteractState = EInteractState::EIS_NOTinteracting;
		OnExitInteractStateDelegate.ExecuteIfBound();
		OverlappingObject = nullptr;
		return true;
	}
	//Sto entrando in un oggetto  e non ho iterazioni
	else if (bEnter && InteractableObject && OverlappingObject != InteractableObject && InteractState == EInteractState::EIS_NOTinteracting)
	{
		if (OverlappingObject) { OverlappingObject->SetInteractWidgetVisibility(false); }
		OverlappingObject = InteractableObject;
		return true;
	}

	//Sto uscendo da un oggetto  e non ho iterazioni
	else if (!bEnter && InteractableObject && OverlappingObject == InteractableObject && InteractState == EInteractState::EIS_NOTinteracting)
	{
		OverlappingObject = nullptr;
		return true;
	}


	return false;

}


