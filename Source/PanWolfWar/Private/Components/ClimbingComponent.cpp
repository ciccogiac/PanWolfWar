// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ClimbingComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "PanWolfWar/DebugHelper.h"


#pragma region EngineFunctions

UClimbingComponent::UClimbingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	ActorOwner = GetOwner();
	CharacterOwner = Cast<ACharacter>(ActorOwner);

}

void UClimbingComponent::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = CharacterOwner->GetCharacterMovement();
	
}

void UClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsClimbing) { SnapMovementToClimbableSurface(DeltaTime); }
}

#pragma endregion


#pragma region SetClimbState

bool UClimbingComponent::ToggleClimbing()
{
	if (bIsClimbing) { StopClimbing(); return false; }
	else if (TryClimbing()) { StartClimbing(); return true; }
	
	return false;
}

void UClimbingComponent::StartClimbing()
{
	bIsClimbing = true;

	MovementComponent->StopMovementImmediately();
	MovementComponent->MaxFlySpeed = 0.f;
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying, 0);


	MovementComponent->bOrientRotationToMovement = false;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(48.f);

	MovementComponent->StopMovementImmediately();
}

void UClimbingComponent::StopClimbing()
{

	MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);

	MovementComponent->bOrientRotationToMovement = true;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);

	const FRotator DirtyRotation = MovementComponent->UpdatedComponent->GetComponentRotation();
	const FRotator CleanStandRotation = FRotator(0.f, DirtyRotation.Yaw, 0.f);
	MovementComponent->UpdatedComponent->SetRelativeRotation(CleanStandRotation);

	MovementComponent->StopMovementImmediately();
	bIsClimbing = false;

}

#pragma endregion


#pragma region CalculateClimbingCondition

bool UClimbingComponent::TryClimbing()
{
	FHitResult hit;
	ClimbCapsuleTrace(hit);

	if (hit.bBlockingHit && CheckNormalContact(hit.ImpactNormal))
	{
			//Debug::Print(TEXT("Hit A climbable surface with Normal:") + hit.ImpactNormal.ToString());

			return FindClimbablePoint(hit);
		
	}

	return false;
}

bool UClimbingComponent::FindClimbablePoint(FHitResult hit)
{
	FVector Start = hit.ImpactPoint;
	FHitResult PreviousSphereHit;
	FHitResult SphereHit;

	FVector ImpactPoint;
	FVector ImpactNormal;

	for (size_t i = 0; i < 4; i++)
	{

		EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

		UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, Start, 10.f, ClimbableObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, SphereHit, true);


		if (!SphereHit.bBlockingHit && i>0 && CheckNormalContact(ImpactNormal))
		{
			Debug::Print(TEXT("Goood"));

			DrawDebugSphere(GetWorld(), ImpactPoint, 10.f, 12, FColor::Emerald, false , 3.f);
			DrawDebugLine(GetWorld(), ImpactPoint, ImpactPoint + ImpactNormal * 10.f, FColor::Blue, false, 3.f);

			CurrentClimbableSurfaceLocation = ImpactPoint;
			CurrentClimbableSurfaceNormal = ImpactNormal;

			return true;
		}

		ImpactPoint  = SphereHit.ImpactPoint;
		ImpactNormal = SphereHit.ImpactNormal;

		Start = Start + FVector(0.f, 0.f, 20.f);
	}


	return false;
}

void UClimbingComponent::ClimbCapsuleTrace(FHitResult& outHit)
{
	const FVector CharacterLocation = ActorOwner->GetActorLocation();
	const FVector ForwardVector = ActorOwner->GetActorForwardVector() * ForwardOffset;
	const FVector UpVector = ActorOwner->GetActorUpVector() * CharacterOwner->BaseEyeHeight;

	const FVector Start = CharacterLocation + UpVector + ForwardVector;
	const FVector End = CharacterLocation + ForwardVector;

	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	UKismetSystemLibrary::CapsuleTraceSingleForObjects(this, Start, End, Radius, HalfHeight, ClimbableObjectTypes, false, TArray<AActor*>(), DebugTraceType, outHit, true);

}

bool UClimbingComponent::CheckNormalContact(FVector_NetQuantizeNormal ImpactNormal)
{
	if (ImpactNormal.Z < -0.1f) return false;
	if (ImpactNormal.Z > 0.3f) return false;
	return true;
}

void UClimbingComponent::SnapMovementToClimbableSurface(float DeltaTime)
{
	const FVector ComponentForward = MovementComponent->UpdatedComponent->GetForwardVector();
	const FVector ComponentLocation = MovementComponent->UpdatedComponent->GetComponentLocation();

	const FVector ProjectedCharacterToSurface = (CurrentClimbableSurfaceLocation - ComponentLocation).ProjectOnTo(ComponentForward);

	const FVector SnapVector = -CurrentClimbableSurfaceNormal * ProjectedCharacterToSurface.Length();

	MovementComponent->UpdatedComponent->MoveComponent(SnapVector * DeltaTime * MaxClimbSpeed, MovementComponent->UpdatedComponent->GetComponentQuat(), true);
}

#pragma endregion







