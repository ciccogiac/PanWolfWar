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

// Sets default values for this component's properties
UInteractComponent::UInteractComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	ActorOwner = GetOwner();
	CharacterOwner = Cast<ACharacter>(ActorOwner);
}


// Called when the game starts
void UInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = CharacterOwner->GetCharacterMovement();
	CapsuleComponent = CharacterOwner->GetCapsuleComponent();
	
}


// Called every frame
void UInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInteractComponent::Interact()
{


	if (!OverlappingObject) return;

	if (!bIsMovingObject)
	{
		bIsMovingObject = true;
		OnEnterInteractStateDelegate.ExecuteIfBound();
		//AddMappingContext(InteractableMappingContext, 1);
		CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 60.f;
		CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = false;
		OverlappingObject->Interact(true);

		AMovableObject* MovableObject = Cast<AMovableObject>(OverlappingObject);
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		FRotator Rotator1 = UKismetMathLibrary::MakeRotFromX(MovableObject->GetBoxForward());
		FRotator Rotator = FRotator(0.f, Rotator1.Yaw, 0.f);
		float OverTime = FVector::Distance(MovableObject->GetBoxPosition(), ActorOwner->GetActorLocation()) / 250.f;
		const FVector Position = FVector(MovableObject->GetBoxPosition().X, MovableObject->GetBoxPosition().Y, CapsuleComponent->GetComponentLocation().Z);
		UKismetSystemLibrary::MoveComponentTo(CapsuleComponent, Position, Rotator, true, false, OverTime, true, EMoveComponentAction::Move, LatentInfo);
	}
	else
	{
		bIsMovingObject = false;
		//RemoveMappingContext(InteractableMappingContext);
		OnExitInteractStateDelegate.ExecuteIfBound();
		CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 500.f;
		CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = true;
		OverlappingObject->Interact(false);
	}
}

void UInteractComponent::MoveObject(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (CharacterOwner->Controller != nullptr)
	{

		const FVector ForwardDirection = ActorOwner->GetActorForwardVector();
		const FVector RightDirection = ActorOwner->GetActorRightVector();

		CharacterOwner->AddMovementInput(ForwardDirection, MovementVector.Y);
		CharacterOwner->AddMovementInput(RightDirection, MovementVector.X);

		if (bIsMovingObject)
		{
			AMovableObject* MovableObject = Cast<AMovableObject>(OverlappingObject);
			if (!MovableObject) return;



			if (MovementVector.Y >= 0.1f)
			{
				MovableObject->SetObjectlocation(true, 15.f);
			}

			else if (MovementVector.Y <= -0.1f)
			{
				MovableObject->SetObjectlocation(true, -30.f);
			}

			if (MovementVector.X >= 0.1f)
			{
				MovableObject->SetObjectlocation(false, 15.f);
			}

			else if (MovementVector.X <= -0.1f)
			{
				MovableObject->SetObjectlocation(false, -15.f);
			}

			/*FLatentActionInfo LatentInfo;
			LatentInfo.CallbackTarget = this;
			FRotator Rotator1 = UKismetMathLibrary::MakeRotFromX(MovableObject->GetBoxForward());
			FRotator Rotator = FRotator(0.f, Rotator1.Yaw, 0.f);
			float OverTime = FVector::Distance(MovableObject->GetBoxPosition(), ActorOwner->GetActorLocation()) / 250.f;
			const FVector Position = FVector(MovableObject->GetBoxPosition().X, MovableObject->GetBoxPosition().Y, CapsuleComponent->GetComponentLocation().Z);
			UKismetSystemLibrary::MoveComponentTo(CapsuleComponent, Position, Rotator, true, false, OverTime, true, EMoveComponentAction::Move, LatentInfo);*/
		}
	}
}

void UInteractComponent::SetOverlappingObject(AInteractableObject* InteractableObject)
{
	if (InteractableObject == nullptr && bIsMovingObject)
	{
		bIsMovingObject = false;
		OnExitInteractStateDelegate.ExecuteIfBound();
		CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 500.f;
		CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = true;
		OverlappingObject->Interact(false);
	}

	OverlappingObject = InteractableObject;

}
