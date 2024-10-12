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
#include "Components/PandolfoComponent.h"
#include "Components/PanWolfComponent.h"
#include "Components/PandolFlowerComponent.h"
#include "Components/PanBirdComponent.h"

#include "Actors/MovableObject.h"

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

	if (PanWolfCharacter)
	{
		PandolfoComponent = PanWolfCharacter->GetPandolfoComponent();
	}
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
			PandolfoComponent->PandolfoState = EPandolfoState::EPS_Interacting; 
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
		ETransformationObjectTypes TransformationObjectType = OverlappingObject->GetTransformationObjectType();
		if (TransformationObjectType == ETransformationObjectTypes::ETOT_Pandolfo_Object && !PandolfoComponent->IsActive()) return;
		if (TransformationObjectType == ETransformationObjectTypes::ETOT_PanWolf_Object && !PanWolfCharacter->GetPanWolfComponent()->IsActive()) return;
		if (TransformationObjectType == ETransformationObjectTypes::ETOT_PandolFlower_Object && !PanWolfCharacter->GetPandolFlowerComponent()->IsActive()) return;
		if (TransformationObjectType == ETransformationObjectTypes::ETOT_PanBird_Object && !PanWolfCharacter->GetPanBirdComponent()->IsActive()) return;

		InteractState = EInteractState::EIS_Interacting;

		if (OverlappingObject->ActorHasTag(FName("Movable_Object"))) { CurrentMovableObject = Cast<AMovableObject>(OverlappingObject); }

		if (OverlappingObject->Interact(CharacterOwner))
		{
			SetInteractState();
				
		}

		else
		{
			InteractState = EInteractState::EIS_NOTinteracting;
			PandolfoComponent->PandolfoState = EPandolfoState::EPS_Pandolfo;
			//OnExitInteractStateDelegate.ExecuteIfBound();
			if (OverlappingObject->ActorHasTag(FName("Movable_Object"))) 
				PanWolfCharacter->RemoveMappingContext(MovableObjectMappingContext);
		}

	}

	else if(InteractState != EInteractState::EIS_NOTinteracting)
	{
		InteractState = EInteractState::EIS_NOTinteracting;
		PandolfoComponent->PandolfoState = EPandolfoState::EPS_Pandolfo;
		//OnExitInteractStateDelegate.ExecuteIfBound();
		//PanWolfCharacter->RemoveMappingContext(MovableObjectMappingContext);

		if (CurrentMovableObject)
		{
			PanWolfCharacter->RemoveMappingContext(MovableObjectMappingContext);
			CurrentMovableObject->SetMovingState(false, 500.f, 10000.f);
			CurrentMovableObject = nullptr;
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

void UInteractComponent::SetOverlappingObject(AInteractableObject* InteractableObject, bool bEnter)
{
	if (InteractableObject && OverlappingObject && (InteractableObject != OverlappingObject) ) return; // Stai entrando o uscendo da oggetto diverso da quello overlappato

	//Stai uscendo da oggetto
	if (!bEnter) 
	{
		if (OverlappingObject && OverlappingObject->ActorHasTag(FName("Movable_Object")))
		{
			CurrentMovableObject = Cast<AMovableObject>(OverlappingObject);
			InteractState = EInteractState::EIS_NOTinteracting;
			PandolfoComponent->PandolfoState = EPandolfoState::EPS_Pandolfo;
			PanWolfCharacter->RemoveMappingContext(MovableObjectMappingContext);
			if (CurrentMovableObject)
				CurrentMovableObject->SetMovingState(false, 500.f, 10000.f);
			CurrentMovableObject = nullptr;
		}
		OverlappingObject = nullptr;
		return;
	}

	OverlappingObject = InteractableObject;

	if (OverlappingObject && bEnter)
	{
		ETransformationObjectTypes TransformationObjectType = OverlappingObject->GetTransformationObjectType();
		if (TransformationObjectType == ETransformationObjectTypes::ETOT_Pandolfo_Object && !PandolfoComponent->IsActive()) return;
		if (TransformationObjectType == ETransformationObjectTypes::ETOT_PanWolf_Object && !PanWolfCharacter->GetPanWolfComponent()->IsActive()) return;
		if (TransformationObjectType == ETransformationObjectTypes::ETOT_PandolFlower_Object && !PanWolfCharacter->GetPandolFlowerComponent()->IsActive()) return;
		if (TransformationObjectType == ETransformationObjectTypes::ETOT_PanBird_Object && !PanWolfCharacter->GetPanBirdComponent()->IsActive()) return;

		OverlappingObject->SetInteractWidgetVisibility(true);
	}

}

void UInteractComponent::ResetOverlappingObject()
{

	if (OverlappingObject) {

		InteractState = EInteractState::EIS_NOTinteracting;
		if (OverlappingObject->ActorHasTag(FName("Movable_Object")))
			PanWolfCharacter->RemoveMappingContext(MovableObjectMappingContext);

		OverlappingObject->SetInteractWidgetVisibility(false);
		SetOverlappingObject(OverlappingObject,true);
	}


}


#pragma endregion





