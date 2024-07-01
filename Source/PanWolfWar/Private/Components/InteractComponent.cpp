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
#include <PanWolfWar/PanWolfWarCharacter.h>

#include "PanWolfWar/DebugHelper.h"

#pragma region EngineFunctions

UInteractComponent::UInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
}

void UInteractComponent::BeginPlay()
{
	Super::BeginPlay();
}

#pragma endregion

#pragma region InteractSection

void UInteractComponent::SetInteractState()
{
	if (OverlappingObject)
	{
		if (OverlappingObject->ActorHasTag(FName("Movable_Object")))
		{
			InteractState = EInteractState::EIS_MovingObject;
			PanWolfCharacter->AddMappingContext(MovableObjectMappingContext, 2);
		}
			
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
				
		}

		else
		{
			InteractState = EInteractState::EIS_NOTinteracting;
			//OnExitInteractStateDelegate.ExecuteIfBound();
			if (OverlappingObject->ActorHasTag(FName("Movable_Object")))
				PanWolfCharacter->RemoveMappingContext(MovableObjectMappingContext);
		}

	}

}

void UInteractComponent::InteractMove(const FInputActionValue& Value)
{
	if (OverlappingObject)
		OverlappingObject->Move(Value);
}


#pragma endregion

#pragma region OverlappingSection

bool UInteractComponent::SetOverlappingObject(AInteractableObject* InteractableObject, bool bEnter)
{
	//Sto uscendo da oggetto overlappato prima e ho interazioni
	if (InteractableObject && InteractableObject == OverlappingObject && InteractState != EInteractState::EIS_NOTinteracting)
	{

		OverlappingObject->Interact(nullptr);
		InteractState = EInteractState::EIS_NOTinteracting;
		//OnExitInteractStateDelegate.ExecuteIfBound();
		if (OverlappingObject->ActorHasTag(FName("Movable_Object")))
			PanWolfCharacter->RemoveMappingContext(MovableObjectMappingContext);
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

void UInteractComponent::ResetOverlappingObject()
{
	if (OverlappingObject) {
		OverlappingObject->Interact(nullptr);
		InteractState = EInteractState::EIS_NOTinteracting;
		//OnExitInteractStateDelegate.ExecuteIfBound();
		if (OverlappingObject->ActorHasTag(FName("Movable_Object")))
			PanWolfCharacter->RemoveMappingContext(MovableObjectMappingContext);

		OverlappingObject->SetInteractWidgetVisibility(false);
		OverlappingObject->ResetBox();
	}
	OverlappingObject = nullptr;
}

#pragma endregion





