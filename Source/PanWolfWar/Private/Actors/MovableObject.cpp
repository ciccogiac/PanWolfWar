// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/MovableObject.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "TimerManager.h"

#include "PanWolfWar/DebugHelper.h"

#include "Components/BoxComponent.h"


AMovableObject::AMovableObject()
{
	//PrimaryActorTick.bCanEverTick = true;

	PhysicsConstraintComponent = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraintComponent"));
	PhysicsConstraintComponent->SetupAttachment(StaticMesh);

	FConstrainComponentPropName a;
	a.ComponentName = StaticMesh->GetFName();
	PhysicsConstraintComponent->ComponentName1 = a;
	PhysicsConstraintComponent->SetLinearXLimit(ELinearConstraintMotion::LCM_Free,0.f);
	PhysicsConstraintComponent->SetLinearYLimit(ELinearConstraintMotion::LCM_Free, 0.f);
	PhysicsConstraintComponent->SetLinearZLimit(ELinearConstraintMotion::LCM_Free, 0.f);
	PhysicsConstraintComponent->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked,45.f);
	PhysicsConstraintComponent->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 45.f);
	PhysicsConstraintComponent->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 45.f);

	StaticMesh->SetSimulatePhysics(true);
	//StaticMesh->SetMassOverrideInKg(NAME_None, 10000.f);

	Tags.Add(FName("Movable_Object"));

	N_InteractBox = 4;
	InitializeBoxComponents();

}

void AMovableObject::BeginPlay()
{
	Super::BeginPlay();

	StaticMesh->SetMassOverrideInKg(NAME_None, 10000.f);
}

bool AMovableObject::Interact(ACharacter* _CharacterOwner  )
{

	if (!_CharacterOwner && CharacterOwner)
	{
		SetMovingState(false,500.f, 10000.f);
		return false;
	}

	Super::Interact(_CharacterOwner);

	if (!bIsMovingObject && CharacterOwner)
	{
		SetMovingState(true, 60.f, 2500.f);
		BoxComponent->GetChildComponent(1)->SetVisibility(false);
		SetCharacterPosition();
	}
	else if(bIsMovingObject && CharacterOwner)
	{
		SetMovingState(false, 500.f, 10000.f);
		BoxComponent->GetChildComponent(1)->SetVisibility(true);
	}

	return bIsMovingObject;
}

void AMovableObject::SetMovingState(const bool state, const float WalkSpeed, const float Mass)
{
	bIsMovingObject = state;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = !state;
	StaticMesh->SetMassOverrideInKg(NAME_None, Mass);
}

void AMovableObject::Move(const FInputActionValue& Value)
{
	if (!CharacterOwner) return;
	if (!bIsMovingObject) return;

	FVector2D MovementVector = Get8DirectionVector(Value.Get<FVector2D>());

	if (CharacterOwner->Controller != nullptr)
	{

		const FVector ForwardDirection = CharacterOwner->GetActorForwardVector();
		const FVector RightDirection = CharacterOwner->GetActorRightVector();

		CharacterOwner->AddMovementInput(ForwardDirection, MovementVector.Y);
		CharacterOwner->AddMovementInput(RightDirection, MovementVector.X);

		if (bIsMovingObject) {MoveObject(MovementVector);}
	}
}

void AMovableObject::MoveObject(FVector2D& MovementVector)
{
	if (MovementVector.Y != 0.f && MovementVector.X == 0.f)
	{
		SetObjectlocation(true, FWD_DirectionSpeed * FMath::Sign(MovementVector.Y));
	}

	else if (MovementVector.X != 0.f && MovementVector.Y == 0.f)
	{
		SetObjectlocation(false, RIGHT_DirectionSpeed * FMath::Sign(MovementVector.X));
	}

	else if (MovementVector.X != 0.f && MovementVector.Y != 0.f)
	{
		SetObjectlocation(MovementVector.Y >= 0.f, TURN_DirectionSpeed * MovementVector.X, true);
	}

	SetCharacterPosition();
}

void AMovableObject::SetObjectlocation(bool bForwardDirection, float DirectionSpeed, bool bRotate )
{
	if (!CharacterOwner || !BoxComponent) return;

	const FVector ForwardVector = CharacterOwner->GetActorForwardVector();
	const FVector CurrentLocation = StaticMesh->GetRelativeLocation();
	
	if (!bRotate)
	{
		const FVector RightVector = CharacterOwner->GetActorRightVector();
		const FVector DirectionVector = UKismetMathLibrary::SelectVector(ForwardVector, RightVector, bForwardDirection);
		const FVector TargetLocation = CurrentLocation + DirectionVector * DirectionSpeed;
		const FVector InterpolatedPosition = UKismetMathLibrary::VInterpTo(CurrentLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), InterpSpeed);
		StaticMesh->SetRelativeLocation(InterpolatedPosition);
	}
		
	else
	{
		float bForwardDir = bForwardDirection ? 1.f : -1.f;
		const FVector TargetLocation = CurrentLocation + ForwardVector * FMath::Abs(DirectionSpeed) * bForwardDir;
		const FVector InterpolatedPosition = UKismetMathLibrary::VInterpTo(CurrentLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), InterpSpeed);

		const float Rotation = (bForwardDirection ? FMath::Sign(DirectionSpeed) : -FMath::Sign(DirectionSpeed))* RotationMultiplier;
		FRotator Rotator = StaticMesh->GetComponentRotation() +  FRotator(0.f, Rotation, 0.f);
		FRotator NewRotation = UKismetMathLibrary::RInterpTo(StaticMesh->GetComponentRotation(), Rotator, GetWorld()->GetDeltaSeconds(), InterpSpeed);
		
		StaticMesh->SetRelativeLocationAndRotation(InterpolatedPosition, NewRotation);
	}

}

void AMovableObject::SetCharacterPosition()
{
	if (!CharacterOwner || !BoxComponent) return;
	
	FVector Direction = CharacterOwner->GetActorLocation() - GetBoxPosition();
	const float CurrentDistance = Direction.Size();

	if (FMath::Abs(CurrentDistance) > FWD_AcceptanceOffset_Character)
	{
		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		const FRotator RotX = UKismetMathLibrary::MakeRotFromX(GetBoxForward());
		const FRotator Rotator = FRotator(0.f, RotX.Yaw, 0.f);

		//FVector NewLocation = GetBoxPosition() + Direction.Normalize() * FWD_Offset_Character;
		FVector NewLocation = GetBoxPosition() - GetBoxForward() * FWD_Offset_Character;
		NewLocation.Z = CharacterOwner->GetActorLocation().Z;

		CharacterOwner->GetCapsuleComponent()->SetRelativeLocationAndRotation(NewLocation, Rotator, true);
	}

}



