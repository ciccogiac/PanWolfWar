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

bool UInteractComponent::IsTransformedInObjectTypes()
{
	ETransformationObjectTypes TransformationObjectType = OverlappingObject->GetTransformationObjectType();

	switch (TransformationObjectType)
	{
	case ETransformationObjectTypes::ETOT_Pandolfo_Object:
		if (!PandolfoComponent->IsActive()) return false;
		break;
	case ETransformationObjectTypes::ETOT_PanWolf_Object:
		if (!PanWolfCharacter->GetPanWolfComponent()->IsActive()) return false;
		break;
	case ETransformationObjectTypes::ETOT_PandolFlower_Object:
		if (!PanWolfCharacter->GetPandolFlowerComponent()->IsActive()) return false;
		break;
	case ETransformationObjectTypes::ETOT_PanBird_Object:
		if (!PanWolfCharacter->GetPanBirdComponent()->IsActive()) return false;
		break;
	default:
		break;
	}

	return true;
}

void UInteractComponent::SetInteractState()
{
	if (OverlappingObject)
	{
		EInteractableObjectTypes InteractableObjectTypes = OverlappingObject->GetInteractableObjectType();

		if (InteractableObjectTypes == EInteractableObjectTypes::EIOT_MovableObject)
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

		if (!IsTransformedInObjectTypes()) return;

		EInteractableObjectTypes InteractableObjectTypes = OverlappingObject->GetInteractableObjectType();
		InteractState = EInteractState::EIS_Interacting;

		if (InteractableObjectTypes == EInteractableObjectTypes::EIOT_MovableObject) { CurrentMovableObject = Cast<AMovableObject>(OverlappingObject); }
		if (OverlappingObject->Interact(CharacterOwner))
		{
			SetInteractState();			
		}

		else
		{
			InteractState = EInteractState::EIS_NOTinteracting;
			PandolfoComponent->PandolfoState = EPandolfoState::EPS_Pandolfo;

			if (InteractableObjectTypes == EInteractableObjectTypes::EIOT_MovableObject)
				PanWolfCharacter->RemoveMappingContext(MovableObjectMappingContext);
		}

	}

	else if(InteractState != EInteractState::EIS_NOTinteracting)
	{
		InteractState = EInteractState::EIS_NOTinteracting;
		PandolfoComponent->PandolfoState = EPandolfoState::EPS_Pandolfo;

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
		if (OverlappingObject &&  (OverlappingObject->GetInteractableObjectType() == EInteractableObjectTypes::EIOT_MovableObject))
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
		if (!IsTransformedInObjectTypes()) return;

		OverlappingObject->SetInteractWidgetVisibility(true);
	}

}

void UInteractComponent::ResetOverlappingObject()
{

	if (OverlappingObject) {

		InteractState = EInteractState::EIS_NOTinteracting;
		EInteractableObjectTypes InteractableObjectTypes = OverlappingObject->GetInteractableObjectType();
		if (InteractableObjectTypes == EInteractableObjectTypes::EIOT_MovableObject)
			PanWolfCharacter->RemoveMappingContext(MovableObjectMappingContext);

		OverlappingObject->SetInteractWidgetVisibility(false);
		SetOverlappingObject(OverlappingObject,true);
	}

}


#pragma endregion





