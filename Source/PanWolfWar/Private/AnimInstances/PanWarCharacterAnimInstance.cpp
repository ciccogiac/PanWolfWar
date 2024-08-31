// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstances/PanWarCharacterAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

void UPanWarCharacterAnimInstance::NativeInitializeAnimation()
{
	OwningCharacter = Cast<ACharacter>(TryGetPawnOwner());

	if (OwningCharacter)
	{
		OwningMovementComponent = OwningCharacter->GetCharacterMovement();
	}
}

void UPanWarCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (!OwningCharacter || !OwningMovementComponent)
	{
		return;
	}

	GroundSpeed = OwningCharacter->GetVelocity().Size2D();

	bHasAcceleration = OwningMovementComponent->GetCurrentAcceleration().SizeSquared2D() > 0.f;

	LocomotionDirection = UKismetAnimationLibrary::CalculateDirection(OwningCharacter->GetVelocity(), OwningCharacter->GetActorRotation());
}