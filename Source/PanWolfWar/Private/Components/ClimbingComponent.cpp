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

	// Add Delegates of animation notify

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
	ClimbDirection = 0.f;

	MovementComponent->StopMovementImmediately();

	//CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MovementComponent->MaxFlySpeed = 0.f;
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying, 0);

	OnEnterClimbStateDelegate.ExecuteIfBound();

	MovementComponent->StopMovementImmediately();
}

void UClimbingComponent::StopClimbing()
{
	ClimbDirection = 0.f;

	CapsuleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);
	OnExitClimbStateDelegate.ExecuteIfBound();

	bIsClimbing = false;

}



#pragma endregion

#pragma region CalculateClimbingCondition

bool UClimbingComponent::TryClimbing()
{
	bool bClimbableSurface = FindClimbableObject();
	if (bClimbableSurface)
	{
		//DrawDebugSphere(GetWorld(), LedgeLocation, 8.f, 12, FColor::Emerald, false, 20.f);
		//DrawDebugLine(GetWorld(),   FVector(CurrentClimbableSurfaceLocation.X, CurrentClimbableSurfaceLocation.Y,LedgeLocation.Z), LedgeLocation, FColor::Blue, false, 3.f);
		GrabLedge();
	}	
	return bClimbableSurface;
}

bool UClimbingComponent::FindClimbableObject(const float BaseEyeHeightOffset_UP)
{
	const FVector CharacterLocation = ActorOwner->GetActorLocation();
	const FVector ForwardVector = ActorOwner->GetActorForwardVector();
	float BaseEyeHeightOffset = MovementComponent->IsFalling() ? BaseEyeHeightOffset_Jumping : BaseEyeHeightOffset_Idle;
	BaseEyeHeightOffset = bIsClimbing ? BaseEyeHeightOffset + BaseEyeHeightOffset_UP : BaseEyeHeightOffset; // debugging
	const FVector UpVector = ActorOwner->GetActorUpVector() * CharacterOwner->BaseEyeHeight * BaseEyeHeightOffset;

	const FVector Start = CharacterLocation + UpVector;
	const FVector End = Start + ForwardVector * ForwardOffset;

	const FHitResult outClimbableObjectHit = DoSphereTraceSingleForObjects(Start, End, Radius_FirstTrace);

	if (CheckClimbableObjectTrace(outClimbableObjectHit))
	{
		return FindClimbablePoint(outClimbableObjectHit);
	}

	return false;

}

bool UClimbingComponent::CheckClimbableObjectTrace(const FHitResult& outClimbableObjectHit)
{
	return 
		outClimbableObjectHit.bBlockingHit && 
		std::abs(outClimbableObjectHit.ImpactNormal.Z) < MaxImpactNormal_Z_value &&
		FMath::IsNearlyEqual(FVector::DotProduct(outClimbableObjectHit.ImpactNormal, ActorOwner->GetActorForwardVector()), -1.0f, MaxImpactNormalToForwardVector_Cos_value) ;
}

bool UClimbingComponent::FindClimbablePoint(const FHitResult& ClimbableObjectHit)
{
	float ClimbingTraceHeight = MovementComponent->IsFalling() ? ClimbingTraceHeight_Jumping : ClimbingTraceHeight_Idle;
	ClimbingTraceHeight = bIsClimbing ? 30.f : ClimbingTraceHeight; // debugging
	const FVector Start = ClimbableObjectHit.ImpactPoint + FVector(0.f, 0.f, ClimbingTraceHeight);
	const FVector End = ClimbableObjectHit.ImpactPoint;

	const FHitResult outClimbingPointHit = DoSphereTraceSingleForObjects(Start, End, Radius_SecondTrace);

	if (outClimbingPointHit.bBlockingHit )
	{
		return CheckClimbableSpaceCondition(outClimbingPointHit, ClimbableObjectHit);
	}

	return false;

}

bool UClimbingComponent::CheckClimbableSpaceCondition(const FHitResult& ClimbablePointHit,const  FHitResult& ClimbableObjectHit)
{
	const FVector Start = ClimbablePointHit.ImpactPoint + FVector(0.f, 0.f, CheckingClimbable_Z_Offset);
	const FVector End = Start + ActorOwner->GetActorForwardVector() * CheckingClimbable_Forward_Offset;

	const FHitResult outClimbableConditionHit = DoSphereTraceSingleForChannel(Start, End, Radius_ThirdTrace);

	if (!outClimbableConditionHit.bBlockingHit)
	{
		ProcessClimbableSurfaceInfo(ClimbableObjectHit);
		LedgeLocation = CalculateLedgeLocation(CurrentClimbableSurfaceLocation, ClimbablePointHit.ImpactPoint, ClimbRotation , -1);
	}

	return !outClimbableConditionHit.bBlockingHit;

}

void UClimbingComponent::ProcessClimbableSurfaceInfo(const FHitResult& ClimbableObjectHit)
{
	CurrentClimbableSurfaceLocation = ClimbableObjectHit.ImpactPoint;
	CurrentClimbableSurfaceNormal = ClimbableObjectHit.ImpactNormal;
	ClimbRotation = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::MakeRotFromX(CurrentClimbableSurfaceNormal), FRotator(0.f, 180.f, 0.f));

}

FVector UClimbingComponent::CalculateLedgeLocation(const FVector& ImpactObjectPoint, const FVector& ClimbablePoint, const FRotator& Rotation,  int ForwardDirectionAdjusted)
{
	FVector LedgeLocationXY = ImpactObjectPoint + UKismetMathLibrary::GetForwardVector(Rotation) * LedgeHeightLocationXY * ForwardDirectionAdjusted;
	float heightLedgeLocation = ClimbablePoint.Z - LedgeHeightLocationZ;
	return FVector(LedgeLocationXY.X, LedgeLocationXY.Y, heightLedgeLocation);
}

void UClimbingComponent::LedgeRightMove(float Direction)
{
		const FVector CharacterLocation = ActorOwner->GetActorLocation();
		const FVector ForwardVector = ActorOwner->GetActorForwardVector();
		const FVector UpVector = ActorOwner->GetActorUpVector() * CharacterOwner->BaseEyeHeight * MoveUPOffset;
		const FVector RightVersor = Direction > 0.f ? ActorOwner->GetActorRightVector() : -ActorOwner->GetActorRightVector();
		const FVector RightVector = RightVersor * MoveRightOffset;

		const FVector Start = CharacterLocation + UpVector + RightVector;
		const FVector End = Start + ForwardVector * ForwardOffset;

		const FHitResult outClimbableObjectHit = DoSphereTraceSingleForObjects(Start, End, Radius_FirstTrace);

		if (outClimbableObjectHit.bBlockingHit)
		{
			const FVector Start_ClimbablePoint = outClimbableObjectHit.ImpactPoint + FVector(0.f, 0.f, 20.f);
			const FVector End_ClimbablePoint = Start_ClimbablePoint + FVector(0.f, 0.f, -20.f);

			const FHitResult outClimbablePointHit = DoSphereTraceSingleForObjects(Start_ClimbablePoint, End_ClimbablePoint, Radius_FirstTrace);


			if (outClimbablePointHit.bBlockingHit)
			{
				HandleRightMove(outClimbableObjectHit,outClimbablePointHit, Direction);
			}
		}
	
}

void UClimbingComponent::LedgeUpMove(float Direction)
{
	if (Direction > 0)
	{
		bool bClimbableSurface = FindClimbableObject(1.f);
		if (bClimbableSurface)
		{
			PlayClimbMontage(HangToHang_UP);
		}
	}

	else
	{
		bool bClimbableSurface = FindClimbableObject(-1.f);
		if (bClimbableSurface)
		{
			PlayClimbMontage(HangToHang_DOWN);
		}
	}
}

void UClimbingComponent::HandleRightMove(const FHitResult& outClimbableObjectHit,const FHitResult& outClimbablePointHit, float Direction)
{
	const FVector RightVersor = Direction > 0.f ? ActorOwner->GetActorRightVector() : -ActorOwner->GetActorRightVector();
	const FVector Start = outClimbablePointHit.ImpactPoint;
	const FVector End = Start + RightVersor * 28.f;

	const FHitResult outEndLedgePointHit = DoSphereTraceSingleForObjects(End, End, Radius_FirstTrace);

	const FHitResult outClimbableConditionHit = DoSphereTraceSingleForChannel(Start - ActorOwner->GetActorForwardVector()*15.f, End - ActorOwner->GetActorForwardVector()*15.f, Radius_ThirdTrace);

	if (outEndLedgePointHit.bBlockingHit && !outClimbableConditionHit.bBlockingHit)
	{
		MoveOnLedge(outClimbableObjectHit.ImpactPoint, outClimbablePointHit.ImpactPoint, UKismetMathLibrary::MakeRotFromX(outClimbableObjectHit.ImpactNormal));
		ClimbDirection = Direction;
	}
	else
	{
		ClimbDirection = 0.f;
	}

}

void UClimbingComponent::TryClimbUpon()
{
	if (CheckCanClimbUpon() && !OwningPlayerAnimInstance->IsAnyMontagePlaying())
	{

		ClimbDirection = 0.f;
		bIsClimbing = false;
		PlayClimbMontage(ClimbToTopMontage);
	}
}

bool UClimbingComponent::CheckCanClimbUpon()
{
	const FVector ComponentLocation = ActorOwner->GetActorLocation();
	const FVector EyeHeightOffset = ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight + 100.f);

	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + ActorOwner->GetActorForwardVector() * 100.f;

	const FHitResult LedgeHitResult = DoSphereTraceSingleForChannel(Start,End,25.f);
	if (!LedgeHitResult.bBlockingHit)
	{
		const FVector Start2 = LedgeHitResult.TraceEnd;
		const FVector DownVector = -ActorOwner->GetActorUpVector();
		const FVector End2 = Start2 + DownVector * 100.f;

		if (DoSphereTraceSingleForChannel(Start2, End2, 25.f).bBlockingHit)
		{
			return true;
		}
	}

	return false;
}

#pragma endregion

#pragma region ClimbingMove

void UClimbingComponent::GrabLedge()
{
	UAnimMontage* Montage = MovementComponent->IsFalling() ? JumpToHang : IdleToHang;
	PlayClimbMontage(Montage);
}

void UClimbingComponent::MoveToLedgeLocation()
{
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	FRotator Rotator = FRotator(0.f, ClimbRotation.Yaw, 0.f);
	float OverTime = FVector::Distance(LedgeLocation, ActorOwner->GetActorLocation()) / 250.f;
	UKismetSystemLibrary::MoveComponentTo(CapsuleComponent, LedgeLocation, Rotator, true, false, OverTime, true, EMoveComponentAction::Move, LatentInfo);
}

void UClimbingComponent::MoveOnLedge(const FVector& ImpactObjectPoint, const FVector& ClimbablePoint, const FRotator& Rotation)
{
	FVector Location = CalculateLedgeLocation(ImpactObjectPoint, ClimbablePoint, Rotation, 1);
	FRotator Rotator = FRotator(Rotation.Roll, Rotation.Yaw - 180, Rotation.Roll);

	FVector NewLocation = UKismetMathLibrary::VInterpTo(CharacterOwner->GetActorLocation(), Location, GetWorld()->GetDeltaSeconds(), 2.f);
	FRotator NewRotation = UKismetMathLibrary::RInterpTo(CharacterOwner->GetActorRotation(), Rotator, GetWorld()->GetDeltaSeconds(), 5.f);

	CharacterOwner->SetActorLocationAndRotation(NewLocation, NewRotation);
}

#pragma endregion

#pragma region MontageSection

void UClimbingComponent::OnClimbMontageStartedHanging(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == FName("StartHanging"))
	{

		StartClimbing();
		CharacterOwner->Jump();
		MoveToLedgeLocation();

	}

}

void UClimbingComponent::OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ClimbToTopMontage )
	{
		MovementComponent->SetMovementMode(MOVE_Walking);

		MovementComponent->bOrientRotationToMovement = true;
		//CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(96.f);

		const FRotator DirtyRotation = MovementComponent->UpdatedComponent->GetComponentRotation();
		const FRotator CleanStandRotation = FRotator(0.f, DirtyRotation.Yaw, 0.f);
		MovementComponent->UpdatedComponent->SetRelativeRotation(CleanStandRotation);

		MovementComponent->StopMovementImmediately();

		OnExitClimbStateDelegate.ExecuteIfBound();


	}

	else
	{
		MovementComponent->StopMovementImmediately();
	}

}

void UClimbingComponent::PlayClimbMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) return;
	if (!OwningPlayerAnimInstance) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
}

#pragma endregion

#pragma region ClimbTraces

const FHitResult UClimbingComponent::DoSphereTraceSingleForObjects(const FVector& Start, const FVector& End, float Radius)
{
	FHitResult OutHit;
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, Radius, ClimbableObjectTypes, false, TArray<AActor*>(), DebugTraceType, OutHit, true);

	return OutHit;
}

const FHitResult UClimbingComponent::DoSphereTraceSingleForChannel(const FVector& Start, const FVector& End, float Radius)
{
	FHitResult OutHit;
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::SphereTraceSingle(this, Start, End, Radius, TraceType, false, TArray<AActor*>(), DebugTraceType, OutHit, true, FLinearColor::Yellow);

	return OutHit;
}

#pragma endregion













