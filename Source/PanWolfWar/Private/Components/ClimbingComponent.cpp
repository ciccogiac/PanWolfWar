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

	if (!bIsClimbing && bCanClimb && MovementComponent->IsFalling() && MovementComponent->Velocity.Z >0.f) { TryClimbing(); }
	
	
	if (bClimbDown && !MovementComponent->IsFalling() && !OwningPlayerAnimInstance->IsAnyMontagePlaying()  && CanClimbDownLedge())
	{
		PlayClimbMontage(TopToClimbMontage);
	}
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
	CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(45);
	MovementComponent->MaxFlySpeed = 0.f;
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying, 0);

	OnEnterClimbStateDelegate.ExecuteIfBound();

	MovementComponent->StopMovementImmediately();
}

void UClimbingComponent::StopClimbing()
{
	bIsClimbing = false;
	ClimbDirection = 0.f;


	MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);

	const FRotator DirtyRotation = MovementComponent->UpdatedComponent->GetComponentRotation();
	const FRotator CleanStandRotation = FRotator(0.f, DirtyRotation.Yaw, 0.f);
	MovementComponent->UpdatedComponent->SetRelativeRotation(CleanStandRotation);

	OnExitClimbStateDelegate.ExecuteIfBound();

	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(90);
	CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

#pragma endregion

#pragma region CheckClimbingCondition

bool UClimbingComponent::CheckClimbableObjectTrace(const FHitResult& outClimbableObjectHit)
{
	return
		outClimbableObjectHit.bBlockingHit &&
		std::abs(outClimbableObjectHit.ImpactNormal.Z) < MaxImpactNormal_Z_value &&
		FMath::IsNearlyEqual(FVector::DotProduct(outClimbableObjectHit.ImpactNormal, ActorOwner->GetActorForwardVector()), -1.0f, MaxImpactNormalToForwardVector_Cos_value);
}

bool UClimbingComponent::CheckClimbableSpaceCondition(const FHitResult& ClimbablePointHit)
{
	const FVector Start = ClimbablePointHit.ImpactPoint + FVector(0.f, 0.f, CheckingClimbable_Z_Offset);
	const FVector End = Start + ActorOwner->GetActorForwardVector() * CheckingClimbable_Forward_Offset;

	const FHitResult outClimbableConditionHit = DoSphereTraceSingleForChannel(Start, End, Radius_ThirdTrace);

	return !outClimbableConditionHit.bBlockingHit;

}

bool UClimbingComponent::CheckCapsuleSpaceCondition(const FVector& CLimbablePoint, bool FullHeight)
{
	//float CapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float CapsuleHalfHeight = FullHeight ? 90.f : 45.f;
	float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector Start = CLimbablePoint + FVector(0.f, 0.f, 1.f + CapsuleHalfHeight);
	return !DoCapsuleTraceSingleForChannel(Start, Start, CapsuleRadius, CapsuleHalfHeight).bBlockingHit;
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

		UAnimMontage* Montage = MovementComponent->IsFalling() ? JumpToHang : IdleToHang;
		PlayClimbMontage(Montage);
	}	
	return bClimbableSurface;
}

bool UClimbingComponent::FindClimbableObject(const float BaseEyeHeightOffset_UP, const float BaseEyeHeightOffset_Right )
{
	const FHitResult outClimbableObjectHit = TraceFromEyeHeight(Radius_FirstTrace,BaseEyeHeightOffset_UP, BaseEyeHeightOffset_Right);

	if (CheckClimbableObjectTrace(outClimbableObjectHit))
	{
		return FindClimbablePoint(outClimbableObjectHit);
	}

	return false;

}

bool UClimbingComponent::FindClimbablePoint(const FHitResult& ClimbableObjectHit)
{
	const FHitResult outClimbingPointHit = TraceFromClimbableObject(Radius_SecondTrace, ClimbableObjectHit.ImpactPoint);

	if (outClimbingPointHit.bBlockingHit && CheckClimbableSpaceCondition(outClimbingPointHit))
	{
		
		ProcessClimbableSurfaceInfo(ClimbableObjectHit);
		LedgeLocation = CalculateLedgeLocation(CurrentClimbableSurfaceLocation, outClimbingPointHit.ImpactPoint, ClimbRotation, -1);
		return CheckCapsuleSpaceCondition(LedgeLocation - FVector(0.f,0.f, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
		//return true;
	}

	return false;

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

		const FHitResult outClimbableObjectHit = TraceFromEyeHeight(Radius_FirstTrace, MoveUPOffset, MoveRightOffset * FMath::Sign(Direction));

		if (outClimbableObjectHit.bBlockingHit)
		{
			// Same as TraceFromClimbableObject except for the negative ending position to garanteed to handle slping surface movement
			const FVector Start_ClimbablePoint = outClimbableObjectHit.ImpactPoint + FVector(0.f, 0.f, ClimbingTraceHeight_Hanging_Right);
			const FVector End_ClimbablePoint = Start_ClimbablePoint + FVector(0.f, 0.f, -ClimbingTraceHeight_Hanging_Right);

			const FHitResult outClimbablePointHit = DoSphereTraceSingleForObjects(Start_ClimbablePoint, End_ClimbablePoint, Radius_FirstTrace);

			if (outClimbablePointHit.bBlockingHit)
			{
				HandleRightMove(outClimbableObjectHit,outClimbablePointHit, Direction);
			}
		}
	
}

void UClimbingComponent::HandleRightMove(const FHitResult& outClimbableObjectHit,const FHitResult& outClimbablePointHit, float Direction)
{
	const float SignDirection = FMath::Sign(Direction);
	const FVector RightVersor = ActorOwner->GetActorRightVector() * SignDirection;
	const FVector Start = outClimbablePointHit.ImpactPoint;
	const FVector End = Start + RightVersor * HandOffset;

	const FHitResult outEndLedgePointHit = DoSphereTraceSingleForObjects(End - ActorOwner->GetActorForwardVector() * HandBorder_Backward, End + ActorOwner->GetActorForwardVector() * HandBorder_Forward, Radius_FirstTrace_Hand);

	if (outEndLedgePointHit.bBlockingHit )
	{
		if(MoveOnLedge(outClimbableObjectHit.ImpactPoint, outClimbablePointHit.ImpactPoint, UKismetMathLibrary::MakeRotFromX(outClimbableObjectHit.ImpactNormal)))
		{
			ClimbDirection = Direction;
		}
		else
		{
			if (CanClimbCorner(outEndLedgePointHit, SignDirection,true))
			{
				UAnimMontage* Montage = SignDirection > 0 ? ClimbInternCornerRightMontage : ClimbInternCornerLeftMontage;
				PlayClimbMontage(Montage);
			}
			else
			ClimbDirection = 0.f;
		}

	}
	else
	{
		if (CanClimbCorner(outEndLedgePointHit, SignDirection))
		{
			UAnimMontage* Montage = SignDirection > 0 ? ClimbExternCornerRightMontage : ClimbExternCornerLeftMontage;
			PlayClimbMontage(Montage);

		}

		else
		{
			//TryJumping
			if (CanClimbJump(outEndLedgePointHit, SignDirection))
			{
				//Debug::Print(TEXT("I Can Jump!!"));
				UAnimMontage* Montage = SignDirection > 0 ? ClimbJumpRightMontage : ClimbJumpLeftMontage;
				PlayClimbMontage(Montage);
			}
		}
		ClimbDirection = 0.f;
	}

}

bool UClimbingComponent::CanClimbUpon()
{
	const FVector ComponentLocation = ActorOwner->GetActorLocation();
	const FVector EyeHeightOffset = ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight * BaseEyeHeightOffset_Landing);

	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + ActorOwner->GetActorForwardVector() * ForwardOffset_Landing;

	const FHitResult LandingObstacleHit = DoSphereTraceSingleForChannel(Start, End, Radius_FirstTrace_Landing);

	if (!LandingObstacleHit.bBlockingHit)
	{
		const FVector Start_Height = LandingObstacleHit.TraceEnd;
		const FVector DownVector = -ActorOwner->GetActorUpVector();
		const FVector End_Height = Start_Height + DownVector * ClimbingTraceHeight_Landing;

		

		//return DoSphereTraceSingleForChannel(Start_Height, End_Height, Radius_FirstTrace_Landing).bBlockingHit;

		FHitResult hit = DoLineTraceSingleByChannel(Start_Height, End_Height);
		if (hit.bBlockingHit)
		{
			/*float CapsuleHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
			return !DoSphereTraceSingleForChannel(hit.ImpactPoint + FVector (0.f,0.f,50.f), hit.ImpactPoint + FVector(0.f, 0.f, 50.f + CapsuleHeight), CapsuleRadius).bBlockingHit;*/
			return CheckCapsuleSpaceCondition(hit.ImpactPoint,true);
		}

	}

	return false;
}

bool UClimbingComponent::CanClimbDownLedge()
{
	if (MovementComponent->IsFalling() || bIsClimbing) return false;

	const FVector ComponentLocation = ActorOwner->GetActorLocation();
	const FVector ComponentForward = ActorOwner->GetActorForwardVector();
	const FVector DownVector = -ActorOwner->GetActorUpVector();

	const float height = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector WalkableSurfaceTraceStart = ComponentLocation + FVector(0.f,0.f,-height -20.f);
	const FVector WalkableSurfaceTraceEnd = WalkableSurfaceTraceStart + ComponentForward * 50.f;

	FHitResult WalkableSurfaceHit = DoSphereTraceSingleForObjects( WalkableSurfaceTraceEnd, WalkableSurfaceTraceStart, 25.f);
	
	if (WalkableSurfaceHit.bBlockingHit && std::abs(WalkableSurfaceHit.ImpactNormal.Z) < MaxImpactNormal_Z_value &&
		FMath::IsNearlyEqual(FVector::DotProduct(WalkableSurfaceHit.ImpactNormal, ActorOwner->GetActorForwardVector()), 1.0f, MaxImpactNormalToForwardVector_Cos_value))
	{
		return FindClimbablePoint(WalkableSurfaceHit);
	}


	return false;
}

bool UClimbingComponent::CanClimbCorner(const FHitResult& outEndLedgePointHit, float Direction, bool InternLedge)
{
	FVector Start ;
	FVector End ;

	if (!InternLedge)
	{
		 Start = outEndLedgePointHit.TraceEnd;
		 End = Start + ActorOwner->GetActorRightVector() * 40.f * -Direction;
	}

	else
	{
		Start = outEndLedgePointHit.ImpactPoint + ActorOwner->GetActorRightVector() * 40.f * -Direction;
		End = outEndLedgePointHit.ImpactPoint - ActorOwner->GetActorRightVector() * 40.f * -Direction;
	}
	

	const FHitResult outClimbableObjectHit = DoSphereTraceSingleForObjects(Start , End, Radius_FirstTrace);

	if (outClimbableObjectHit.bBlockingHit)
	{
		return FindClimbablePoint(outClimbableObjectHit);
	}
	//PlayClimbMontage(ClimbCornerLeftMontage);

	return false;
}

bool UClimbingComponent::CanClimbJump(const FHitResult& outEndLedgePointHit, float Direction)
{
	const FVector Start = outEndLedgePointHit.TraceStart + ActorOwner->GetActorForwardVector() * HandBorder_Backward + ActorOwner->GetActorRightVector() * Direction * 10.f;
	const FVector End = Start + ActorOwner->GetActorRightVector() * Direction * 150.f;

	const FHitResult hit = DoSphereTraceSingleForObjects(Start, End, 20.f);

	if (hit.bBlockingHit)
	{
		const FVector Start2 = hit.ImpactPoint + ActorOwner->GetActorRightVector() * -Direction;
		const FVector End2 = hit.ImpactPoint - ActorOwner->GetActorRightVector() * 5.f * -Direction;

		const FHitResult outClimbableObjectHit = DoSphereTraceSingleForObjects( End2, Start2, Radius_FirstTrace);

		if (outClimbableObjectHit.bBlockingHit)
		{
			return FindClimbablePoint(outClimbableObjectHit);
		}
	}


	return false;
}
#pragma endregion

#pragma region ClimbingMove

void UClimbingComponent::LedgeMove(const FVector2D MovementVector)
{
	if (!bIsClimbing) return;
	if(OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	if (MovementVector.Y == 0 && MovementVector.X != 0)
	{
		LedgeRightMove(MovementVector.X);
	}

	else if (MovementVector.Y != 0)
	{
		ClimbDirection = 0.f;
		if (!LedgeUpMove(MovementVector) && MovementVector.X != 0)
			LedgeRightMove(MovementVector.X);
	}

}

bool UClimbingComponent::LedgeUpMove(const FVector2D& Direction)
{
	const float RightOffset = Direction.X == 0 ? 0.f : Direction.X > 0 ? 50.f : -50.f;

	if (Direction.Y > 0 && FindClimbableObject(1.f, RightOffset))
	{
			PlayClimbMontage(HangToHang_UP);
			return true;
		
	}

	else if (Direction.Y < 0 && FindClimbableObject(-1.f, RightOffset))
	{
			PlayClimbMontage(HangToHang_DOWN);
			return true;
		
	}

	return false;
}

void UClimbingComponent::TryClimbUpon()
{
	if (CanClimbUpon() && !OwningPlayerAnimInstance->IsAnyMontagePlaying())
	{
		ClimbDirection = 0.f;
		bIsClimbing = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(90);
		PlayClimbMontage(ClimbToTopMontage);
	}
}

void UClimbingComponent::MoveToLedgeLocation()
{
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	FRotator Rotator = FRotator(0.f, ClimbRotation.Yaw, 0.f);
	float OverTime = FVector::Distance(LedgeLocation, ActorOwner->GetActorLocation()) / 250.f;
	UKismetSystemLibrary::MoveComponentTo(CapsuleComponent, LedgeLocation, Rotator, true, false, OverTime, true, EMoveComponentAction::Move, LatentInfo);
}

bool UClimbingComponent::MoveOnLedge(const FVector& ImpactObjectPoint, const FVector& ClimbablePoint, const FRotator& Rotation)
{
	FVector Location = CalculateLedgeLocation(ImpactObjectPoint, ClimbablePoint, Rotation, 1);
	if (!CheckCapsuleSpaceCondition(Location - FVector(0.f, 0.f, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()))) return false;
	FRotator Rotator = FRotator(Rotation.Roll, Rotation.Yaw - 180, Rotation.Roll);

	FVector NewLocation = UKismetMathLibrary::VInterpTo(CharacterOwner->GetActorLocation(), Location, GetWorld()->GetDeltaSeconds(), 2.f);
	FRotator NewRotation = UKismetMathLibrary::RInterpTo(CharacterOwner->GetActorRotation(), Rotator, GetWorld()->GetDeltaSeconds(), 5.f);

	CharacterOwner->SetActorLocationAndRotation(NewLocation, NewRotation);

	return true;
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
		StopClimbing();

		MovementComponent->SetMovementMode(MOVE_Walking);
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
const FHitResult UClimbingComponent::DoLineTraceSingleByObject(const FVector& Start, const FVector& End)
{
	FHitResult OutHit;

	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::LineTraceSingleForObjects(this, Start, End, ClimbableObjectTypes, false, TArray<AActor*>(), DebugTraceType, OutHit, false);

	return OutHit;
}

const FHitResult UClimbingComponent::DoLineTraceSingleByChannel(const FVector& Start, const FVector& End)
{
	FHitResult OutHit;

	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::LineTraceSingle(this, Start, End, TraceType, false, TArray<AActor*>(), DebugTraceType, OutHit, false);

	return OutHit;
}

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

const FHitResult UClimbingComponent::DoCapsuleTraceSingleForChannel(const FVector& Start, const FVector& End, float Radius, float HalfHeight)
{
	FHitResult OutHit;
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::CapsuleTraceSingle(this, Start, End, Radius, HalfHeight, TraceType, false, TArray<AActor*>(), DebugTraceType, OutHit, true, FLinearColor::Yellow);

	return OutHit;
}

const FHitResult UClimbingComponent::TraceFromEyeHeight(const float Radius, const float BaseEyeHeightOffset_UP, const float BaseEyeHeightOffset_Right)
{
	const FVector CharacterLocation = ActorOwner->GetActorLocation();
	const FVector ForwardVector = ActorOwner->GetActorForwardVector();
	float BaseEyeHeightOffset = MovementComponent->IsFalling() ? BaseEyeHeightOffset_Jumping : BaseEyeHeightOffset_Idle;
	BaseEyeHeightOffset = bIsClimbing ? BaseEyeHeightOffset + BaseEyeHeightOffset_UP : BaseEyeHeightOffset; 
	const FVector UpVector = ActorOwner->GetActorUpVector() * CharacterOwner->BaseEyeHeight * BaseEyeHeightOffset;

	const FVector RightVector = ActorOwner->GetActorRightVector() * BaseEyeHeightOffset_Right;

	const FVector Start = CharacterLocation + UpVector + RightVector;
	const FVector End = Start + ForwardVector * ForwardOffset;

	return DoSphereTraceSingleForObjects(Start, End, Radius);

}

const FHitResult UClimbingComponent::TraceForObject(const float Radius, const float BaseEyeHeightOffset_UP, const float BaseEyeHeightOffset_Right)
{
	const FVector CharacterLocation = ActorOwner->GetActorLocation();
	const FVector ForwardVector = ActorOwner->GetActorForwardVector();
	const FVector UpVector = ActorOwner->GetActorUpVector() * BaseEyeHeightOffset_UP;

	const FVector RightVector = ActorOwner->GetActorRightVector() * BaseEyeHeightOffset_Right;

	const FVector Start = CharacterLocation + UpVector + RightVector;
	const FVector End = Start + ForwardVector * ForwardOffset;

	return DoSphereTraceSingleForObjects(End, Start,Radius);
}

const FHitResult UClimbingComponent::TraceFromClimbableObject(const float Radius, const FVector& ImpactPoint)
{
	float ClimbingTraceHeight = MovementComponent->IsFalling() ? ClimbingTraceHeight_Jumping : ClimbingTraceHeight_Idle;
	ClimbingTraceHeight = bIsClimbing ? ClimbingTraceHeight_Hanging_UP : ClimbingTraceHeight;
	const FVector Start = ImpactPoint + FVector(0.f, 0.f, ClimbingTraceHeight);
	const FVector End = ImpactPoint;

	return DoSphereTraceSingleForObjects(Start, End, Radius_SecondTrace);
}

#pragma endregion













