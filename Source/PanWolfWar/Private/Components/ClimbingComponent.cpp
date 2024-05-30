// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ClimbingComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
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
	CapsuleComponent = CharacterOwner->GetCapsuleComponent();

	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

	if (OwningPlayerAnimInstance)
	{
		OwningPlayerAnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UClimbingComponent::OnClimbMontageStartedHanging);
		OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UClimbingComponent::OnClimbMontageEnded);
	}

	
}

void UClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsClimbing && bCanClimb && MovementComponent->IsFalling()) { TryClimbing(); }

}



#pragma endregion

#pragma region SetClimbState

void UClimbingComponent::ToggleClimbing()
{
	if (bIsClimbing) { StopClimbing(); bCanClimb = false; }
	else { bCanClimb = true; }
	
}

void UClimbingComponent::StartClimbing()
{
	bIsClimbing = true;


	MovementComponent->StopMovementImmediately();

	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MovementComponent->MaxFlySpeed = 0.f;
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying, 0);

	OnEnterClimbStateDelegate.ExecuteIfBound();

	MovementComponent->StopMovementImmediately();
}

void UClimbingComponent::StopClimbing()
{
	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);
	OnExitClimbStateDelegate.ExecuteIfBound();

	bIsClimbing = false;

}



#pragma endregion

#pragma region CalculateClimbingCondition

bool UClimbingComponent::TryClimbing()
{
	bool bClimbableSurface = FindCLimbableObjectLocation();
	if (bClimbableSurface)
	{
		Debug::Print(TEXT("Can Climb"), FColor::Emerald, 1);
		//DrawDebugSphere(GetWorld(), LedgeLocation, 8.f, 12, FColor::Emerald, false, 20.f);
		//DrawDebugLine(GetWorld(),   FVector(CurrentClimbableSurfaceLocation.X, CurrentClimbableSurfaceLocation.Y,LedgeLocation.Z), LedgeLocation, FColor::Blue, false, 3.f);
		GrabLedge();
	}	
	return bClimbableSurface;
}

bool UClimbingComponent::FindCLimbableObjectLocation()
{
	const FVector CharacterLocation = ActorOwner->GetActorLocation();
	const FVector ForwardVector = ActorOwner->GetActorForwardVector();
	const FVector UpVector = ActorOwner->GetActorUpVector() * CharacterOwner->BaseEyeHeight * BaseEyeHeightOffset;

	const FVector Start = CharacterLocation + UpVector;
	const FVector End = Start + ForwardVector * ForwardOffset;

	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	FHitResult outClimbableObjectHit;

	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, Radius_FirstTrace, ClimbableObjectTypes, false, TArray<AActor*>(), DebugTraceType, outClimbableObjectHit, true);

	if (outClimbableObjectHit.bBlockingHit)
	{
		CurrentClimbableSurfaceLocation = outClimbableObjectHit.ImpactPoint;
		CurrentClimbableSurfaceNormal = outClimbableObjectHit.ImpactNormal;
		ClimbRotation = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::MakeRotFromX(CurrentClimbableSurfaceNormal), FRotator(0.f,  180.f , 0.f));

		return FindCLimbablePointLocation(outClimbableObjectHit);
	}

	return false;

}

bool UClimbingComponent::FindCLimbablePointLocation(FHitResult ClimbableObjectHit)
{
	const FVector Start = ClimbableObjectHit.ImpactPoint + FVector(0.f, 0.f, ClimbingTraceHeight);
	const FVector End = ClimbableObjectHit.ImpactPoint;

	FHitResult outClimbingPointHit;
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, Radius_SecondTrace, ClimbableObjectTypes, false, TArray<AActor*>(), DebugTraceType, outClimbingPointHit, true);

	if (outClimbingPointHit.bBlockingHit )
	{

		FVector LedgeLocationXY = CurrentClimbableSurfaceLocation - UKismetMathLibrary::GetForwardVector(ClimbRotation) * LedgeHeightLocationXY;
		float heightLedgeLocation = outClimbingPointHit.ImpactPoint.Z - LedgeHeightLocationZ;
		LedgeLocation = FVector(LedgeLocationXY.X, LedgeLocationXY.Y, heightLedgeLocation);

		return !CheckClimbableCondition(outClimbingPointHit);
	}

	return false;

}

bool UClimbingComponent::CheckClimbableCondition(FHitResult ClimbablePointHit)
{
	const FVector Start = ClimbablePointHit.ImpactPoint + FVector(0.f, 0.f, CheckingClimbable_Z_Offset);
	const FVector End = Start + ActorOwner->GetActorForwardVector() * CheckingClimbable_Forward_Offset;

	FHitResult outClimbableConditionHit;
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::SphereTraceSingle(this, Start, End, Radius_ThirdTrace, TraceType, false , TArray<AActor*>() , DebugTraceType, outClimbableConditionHit , true,FLinearColor::Yellow);
	
	return outClimbableConditionHit.bBlockingHit;

}

void UClimbingComponent::GrabLedge()
{

	PlayClimbMontage(MovementComponent->IsFalling() ? JumpToHang : IdleToHang);
	
}

void UClimbingComponent::MoveToLedgeLocation()
{
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	FRotator Rotator = FRotator(0.f, ClimbRotation.Yaw, 0.f);
	float OverTime = FVector::Distance(LedgeLocation, ActorOwner->GetActorLocation()) / 250.f;
	UKismetSystemLibrary::MoveComponentTo(CapsuleComponent, LedgeLocation, Rotator, true, false, OverTime, true, EMoveComponentAction::Move, LatentInfo);
}

#pragma endregion

#pragma region MontageSection

void UClimbingComponent::OnClimbMontageStartedHanging(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == FName("StartHanging"))
	{

		CharacterOwner->Jump();

		StartClimbing();
		MoveToLedgeLocation();
	}

}

void UClimbingComponent::OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	MovementComponent->StopMovementImmediately();
}

void UClimbingComponent::PlayClimbMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) return;
	if (!OwningPlayerAnimInstance) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
}

#pragma endregion












