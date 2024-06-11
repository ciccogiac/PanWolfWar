// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ClimbingComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

#include "MotionWarpingComponent.h"
#include <PanWolfWar/PanWolfWarCharacter.h>

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

	APanWolfWarCharacter* PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
	if(PanWolfCharacter) MotionWarpingComponent = PanWolfCharacter->GetMotionWarpingComponent();

	MotionWarpingComponent->Activate();
	
}

void UClimbingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CheckTryClimbingConditions()) { TryClimbing(); }
	
	if (CheckClimbDownLedgeConditions())
	{
		PlayClimbMontage(TopToClimbMontage);
	}


}

#pragma endregion

#pragma region SetClimbState

void UClimbingComponent::ToggleClimbing()
{
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	if (ClimbingState == EClimbingState::ECS_Climbing) {
		ClimbingState = EClimbingState::ECS_Falling;
		SavedClimbedObject = nullptr; 
		MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(90);
		CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	
}

void UClimbingComponent::StartClimbing()
{
	ClimbingState = EClimbingState::ECS_Climbing;
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
	ClimbedObject = nullptr;
	SavedClimbedObject = nullptr;
	ClimbingState = EClimbingState::ECS_NOTClimbing;

	MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);

	const FRotator DirtyRotation = MovementComponent->UpdatedComponent->GetComponentRotation();
	const FRotator CleanStandRotation = FRotator(0.f, DirtyRotation.Yaw, 0.f);
	MovementComponent->UpdatedComponent->SetRelativeRotation(CleanStandRotation);

	OnExitClimbStateDelegate.ExecuteIfBound();

	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(90);
	CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

void UClimbingComponent::Landed()
{
	ClimbingState = EClimbingState::ECS_NOTClimbing;
	ClimbedObject = nullptr;
}

#pragma endregion

#pragma region CheckClimbingCondition

bool UClimbingComponent::CheckTryClimbingConditions()
{
	return (ClimbingState == EClimbingState::ECS_NOTClimbing && MovementComponent->IsFalling() && MovementComponent->Velocity.Z > 0.f) ||
		ClimbingState == EClimbingState::ECS_Falling;
}

bool UClimbingComponent::CheckClimbDownLedgeConditions()
{
	return ClimbingState == EClimbingState::ECS_SearchingClimbingDown &&
		!MovementComponent->IsFalling() && !OwningPlayerAnimInstance->IsAnyMontagePlaying() && CanClimbDownLedge();
}

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
	float CapsuleHalfHeight = FullHeight ? 90.f : 45.f;
	float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector Start = CLimbablePoint + FVector(0.f, 0.f, 1.f + CapsuleHalfHeight);
	return !DoCapsuleTraceSingleForChannel(Start, Start, CapsuleRadius, CapsuleHalfHeight).bBlockingHit;
}

bool UClimbingComponent::CheckClimbableDownLedgeTrace(const FHitResult& ClimbableSurfaceHit)
{
	return ClimbableSurfaceHit.bBlockingHit && std::abs(ClimbableSurfaceHit.ImpactNormal.Z) < MaxImpactNormal_Z_value &&
		FMath::IsNearlyEqual(FVector::DotProduct(ClimbableSurfaceHit.ImpactNormal, ActorOwner->GetActorForwardVector()), 1.0f, MaxImpactNormalToForwardVector_Cos_value);
}

bool UClimbingComponent::CheckCapsuleEndPositionCollision()
{
	const float CapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
	return (!DoCapsuleTraceSingleForChannel(ActorOwner->GetActorLocation(), LedgeLocation, CapsuleRadius, CapsuleHalfHeight).bBlockingHit);
}

#pragma endregion

#pragma region CalculateClimbingCondition

bool UClimbingComponent::TryClimbing()
{
	if (FindClimbableObject())
	{
		UAnimMontage* Montage = MovementComponent->IsFalling() ? JumpToHang : IdleToHang;
		PlayClimbMontage(Montage);
		return true;
	}	
	return false;
}

bool UClimbingComponent::FindClimbableObject(const float BaseEyeHeightOffset_UP, const float BaseEyeHeightOffset_Right )
{
	const FHitResult outClimbableObjectHit = TraceFromEyeHeight(Radius_FirstTrace,BaseEyeHeightOffset_UP, BaseEyeHeightOffset_Right);

	if (CheckClimbableObjectTrace(outClimbableObjectHit) && FindClimbablePoint(outClimbableObjectHit))
	{
		ClimbedObject = outClimbableObjectHit.GetActor();
		return true; 
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

		return CheckCapsuleSpaceCondition(LedgeLocation - FVector(0.f, 0.f, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));
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

		const FHitResult outClimbableObjectHit  = TraceFromEyeHeight(Radius_FirstHand, MoveUPOffset, MoveRightOffset * FMath::Sign(Direction), true);
			
		if (outClimbableObjectHit.bBlockingHit)
		{
			// Same as TraceFromClimbableObject except for the negative ending position to garanteed to handle slping surface movement
			//const FVector Start_ClimbablePoint = outClimbableObjectHit.ImpactPoint + FVector(0.f, 0.f, ClimbingTraceHeight_Hanging_Right);
			//const FVector End_ClimbablePoint = Start_ClimbablePoint + FVector(0.f, 0.f, -ClimbingTraceHeight_Hanging_Right);
			const FVector Start_ClimbablePoint = outClimbableObjectHit.ImpactPoint + FVector(0.f, 0.f, ClimbingTraceHeight_Hanging_Right *2);
			const FVector End_ClimbablePoint = Start_ClimbablePoint + FVector(0.f, 0.f, -ClimbingTraceHeight_Hanging_Right*2);
			const FHitResult outClimbablePointHit = DoSphereTraceSingleForObjects(Start_ClimbablePoint, End_ClimbablePoint, Radius_FirstHand);

			if (outClimbablePointHit.bBlockingHit)
			{
				HandleRightMove(outClimbableObjectHit, outClimbablePointHit, Direction);
			}

			else
			{
				const float SignDirection = FMath::Sign(Direction);
				if (CanClimbCorner(outClimbableObjectHit, SignDirection))
				{
					bJumpSaved = false;
					UAnimMontage* Montage = SignDirection > 0 ? ClimbExternCornerRightMontage : ClimbExternCornerLeftMontage;
					PlayClimbMontage(Montage);

				}

				else
				{
					//TryJumping
					if (CanClimbDirectionalJump( SignDirection))
					{
						UAnimMontage* Montage = SignDirection > 0 ? ClimbJumpRightMontage : ClimbJumpLeftMontage;
						SavedJumpMontage = Montage;
						bJumpSaved = true;
					}
					else
					{
						bJumpSaved = false;
					}
				}
			}
		}

		else
		{
			ClimbDirection = 0.f;
			const float SignDirection = FMath::Sign(Direction);

			//Caso in cui sono nu pizzo (Bisogna spostare in avanti il trace end del hitresult) - ActorOwner->GetActorForwardVector() * HandBorder_Backward
			if (CanClimbCorner(outClimbableObjectHit, SignDirection,false,true))
			{
				bJumpSaved = false;
				UAnimMontage* Montage = SignDirection > 0 ? ClimbExternCornerRightMontage : ClimbExternCornerLeftMontage;
				PlayClimbMontage(Montage);

			}

			else
			{
				//TryJumping
				if (CanClimbDirectionalJump(SignDirection))
				{
					UAnimMontage* Montage = SignDirection > 0 ? ClimbJumpRightMontage : ClimbJumpLeftMontage;
					SavedJumpMontage = Montage;
					bJumpSaved = true;
				}
				else
				{
					bJumpSaved = false;
				}
			}
		}

		
	
}

void UClimbingComponent::HandleRightMove(const FHitResult& outClimbableObjectHit,const FHitResult& outClimbablePointHit, float Direction)
{
	const float SignDirection = FMath::Sign(Direction);
	const FVector RightVersor = ActorOwner->GetActorRightVector() * SignDirection;
	const FVector Start = outClimbablePointHit.ImpactPoint;
	const FVector End = Start + RightVersor * (Radius_FirstHand + 1.f);

	const FHitResult outEndLedgePointHit = DoSphereTraceSingleForObjects(End - ActorOwner->GetActorForwardVector() * HandBorder_Backward, End + ActorOwner->GetActorForwardVector() * HandBorder_Forward, Radius_FirstTrace_Hand);

	if (outEndLedgePointHit.bBlockingHit )
	{
		if(MoveOnLedge(outClimbableObjectHit.ImpactPoint, outClimbablePointHit.ImpactPoint, UKismetMathLibrary::MakeRotFromX(outClimbableObjectHit.ImpactNormal)))
		{
			if (outClimbableObjectHit.GetActor() != ClimbedObject) ClimbedObject  = outClimbableObjectHit.GetActor() ;

			ClimbDirection = Direction;
			bJumpSaved = false;
		}
		else
		{
			ClimbDirection = 0.f;
			if (CanClimbCorner(outEndLedgePointHit, SignDirection,true))
			{
				bJumpSaved = false;
				UAnimMontage* Montage = SignDirection > 0 ? ClimbInternCornerRightMontage : ClimbInternCornerLeftMontage;
				PlayClimbMontage(Montage);
			}
			//TryJumping
			else if (CanClimbDirectionalJump(SignDirection))
			{
				UAnimMontage* Montage = SignDirection > 0 ? ClimbJumpRightMontage : ClimbJumpLeftMontage;
				SavedJumpMontage = Montage;
				bJumpSaved = true;
			}
			
			
		}

	}
	else
	{
		if (CanClimbCorner(outEndLedgePointHit, SignDirection))
		{
			bJumpSaved = false;
			UAnimMontage* Montage = SignDirection > 0 ? ClimbExternCornerRightMontage : ClimbExternCornerLeftMontage;
			PlayClimbMontage(Montage);

		}

		else
		{
			//TryJumping
			if (CanClimbDirectionalJump(SignDirection))
			{
				UAnimMontage* Montage = SignDirection > 0 ? ClimbJumpRightMontage : ClimbJumpLeftMontage;
				SavedJumpMontage = Montage;
				bJumpSaved = true;
			}
			else
			{
				bJumpSaved = false;
			}
		}
		ClimbDirection = 0.f;
	}

}

bool UClimbingComponent::CanClimbUpon()
{
	if (ClimbingState == EClimbingState::ECS_Falling) return false;

	const FHitResult LandingObstacleHit = DoClimbUponTrace();

	if (!LandingObstacleHit.bBlockingHit)
	{
		FHitResult hit = DoClimbUponLineDownTrace(LandingObstacleHit.TraceEnd);
		
		if (hit.bBlockingHit && CheckCapsuleSpaceCondition(hit.ImpactPoint, true))
		{
			ClimbedObject = nullptr;
			SavedClimbedObject = nullptr;
			return true;
		}
	}

	return false;
}

bool UClimbingComponent::CanClimbDownLedge()
{
	if (MovementComponent->IsFalling() || ClimbingState == EClimbingState::ECS_Climbing) return false;

	const FHitResult ClimbableSurfaceHit = DoClimbableDownLedgeTrace();
	
	if (CheckClimbableDownLedgeTrace(ClimbableSurfaceHit))
	{
		ClimbedObject = ClimbableSurfaceHit.GetActor();
		return FindClimbablePoint(ClimbableSurfaceHit);
	}

	return false;
}

bool UClimbingComponent::CanClimbCorner(const FHitResult& outEndLedgePointHit, float Direction, bool InternLedge, bool BlindPoint )
{
	if (ClimbingState == EClimbingState::ECS_Falling) return false;

	const FHitResult outClimbableObjectHit = DoClimbCornerTrace(outEndLedgePointHit, Direction, InternLedge, BlindPoint);

	if (outClimbableObjectHit.bBlockingHit && FindClimbablePoint(outClimbableObjectHit))
	{
		ClimbedObject = outClimbableObjectHit.GetActor();
		return true;
	}

	return false;
}

bool UClimbingComponent::CanClimbJump()
{
	if (FindClimbableObject(ClimbingTraceHeight_Hanging_UP)) {  return true; }

	for (size_t i = 0; i < 7; i++)
	{
		const FHitResult hit = DoClimbJumpTrace(i);

		if (hit.bBlockingHit && FindClimbablePoint(hit) && CheckCapsuleEndPositionCollision())
		{
				ClimbedObject = hit.GetActor();
				return true;
		}
	}

	return false;

}

bool UClimbingComponent::CanClimbBackJump()
{
	if (ClimbingState == EClimbingState::ECS_Falling) return false;

	for (size_t i = 0; i < 12; i++)
	{
		const FHitResult hit = DoClimbBackJumpTrace(i);

		if (hit.bBlockingHit && FindClimbablePoint(hit) && CheckCapsuleEndPositionCollision())
		{
			SavedClimbedObject = hit.GetActor();
			return true;
		}
	}

	return false;
}

bool UClimbingComponent::CanClimbDirectionalJump( float Direction , float UP_Offset)
{
	if (ClimbingState == EClimbingState::ECS_Falling) return false;

	for (size_t i = 0; i < 12; i++)
	{
		const FHitResult hit = DoClimbDirectionalJumpTrace(i, Direction, UP_Offset);

		if (hit.bBlockingHit && FindClimbablePoint(hit) && CheckCapsuleEndPositionCollision())
		{
				SavedClimbedObject = hit.GetActor();
				return true;			
		}
	}

	return false;
}

#pragma endregion

#pragma region ClimbingMove

void UClimbingComponent::LedgeMove(const FVector2D MovementVector)
{
	if (ClimbingState != EClimbingState::ECS_Climbing) return;
	if(OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	LastClimb_MovementVector = MovementVector;

	if (MovementVector.Y == 0 && MovementVector.X != 0)
	{
		LedgeRightMove(MovementVector.X);
	}

	else if (MovementVector.Y != 0)
	{
		if (!LedgeUpMove(MovementVector) && MovementVector.X != 0 && ClimbDirection != 0.f)
		{
			LedgeRightMove(MovementVector.X);
			
		}
		else
			ClimbDirection = 0.f;
			

		
	}
}

bool UClimbingComponent::LedgeUpMove(const FVector2D& Direction)
{
	const float RightOffset = Direction.X == 0 ? 0.f : Direction.X > 0 ? 50.f : -50.f;
	const float Y_Offset = Direction.Y == 0 ? 0.f : Direction.Y > 0 ? (ClimbingTraceHeight_Hanging_UP) : (- 3 * ClimbingTraceHeight_Hanging_UP);
	const float X_SignDirection = FMath::Sign(Direction.X);

	if (Direction.Y > 0 && FindClimbableObject(Y_Offset, RightOffset))
	{
			PlayClimbMontage(HangToHang_UP);
			return true;
		
	}

	else if (Direction.Y < 0 && FindClimbableObject(Y_Offset, RightOffset))
	{
			PlayClimbMontage(HangToHang_DOWN);
			return true;
		
	}

	//TryJumping Traverse
	else if (Direction.X != 0 && ClimbDirection == 0.f)
	{
		if (CanClimbDirectionalJump( X_SignDirection, Direction.Y > 0 ? Y_Offset + ClimbingTraceHeight_Hanging_UP : Y_Offset))
		{
			UAnimMontage* Montage = X_SignDirection > 0 ? ClimbJumpRightMontage : ClimbJumpLeftMontage;
			SavedJumpMontage = Montage;
			bJumpSaved = true;

			return true;
		}
		else
		{
			bJumpSaved = false;
		}

	}

	else if (Direction.X == 0 && ClimbDirection == 0.f && Direction.Y < 0 && CanClimbBackJump())
	{
		UAnimMontage* Montage = ClimbJumpRightMontage;
		SavedJumpMontage = Montage;
		bJumpSaved = true;

		return true;
	}

	

	else if(Direction.X == 0) bJumpSaved = false;


	return false;
}

bool UClimbingComponent::TryClimbUpon()
{
	//if (!(LastClimb_MovementVector.X == 0.0f && LastClimb_MovementVector.Y >= 0.0f)) return false;
	if (!( LastClimb_MovementVector.Y >= 0.0f)) return false;

	if (!OwningPlayerAnimInstance->IsAnyMontagePlaying() && CanClimbUpon())
	{
		FVector VaultStartPosition;
		FVector VaultLandPosition;

		if (CanStartUponVaulting(VaultStartPosition, VaultLandPosition))
		{
			//Start Vaulting
			SetMotionWarpTarget(FName("VaultStartPoint"), VaultStartPosition);
			SetMotionWarpTarget(FName("VaultLandPoint"), VaultLandPosition);
		}
		else return false ;
		ClimbDirection = 0.f;
		ClimbingState = EClimbingState::ECS_NOTClimbing;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(90);
		PlayClimbMontage(ClimbToTopMontage);
		return true;
	}
	return false;
}

bool UClimbingComponent::TryJumping()
{
	if (!(LastClimb_MovementVector.X == 0.0f && LastClimb_MovementVector.Y >= 0.0f)) return false;

	if (!bJumpSaved && !OwningPlayerAnimInstance->IsAnyMontagePlaying() && CanClimbJump())
	{
		ClimbDirection = 0.f;
		PlayClimbMontage(HangToHang_UP);
		return true;
	}
	return false;

}

bool UClimbingComponent::TryDirectionalJumping()
{
	if (bJumpSaved && SavedClimbedObject && !OwningPlayerAnimInstance->IsAnyMontagePlaying())
	{
		bJumpSaved = false;
		ClimbedObject = SavedClimbedObject;
		SavedClimbedObject = nullptr;

		PlayClimbMontage(SavedJumpMontage);
		return true;
	}
	return false;
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

	else if (Montage == VaultMontage)
	{
		MovementComponent->SetMovementMode(MOVE_Walking);
	}

	else if (Montage == TopToClimbMontage)
	{
		MovementComponent->StopMovementImmediately();
	}

	else
	{
		MoveToLedgeLocation();
		MovementComponent->StopMovementImmediately();
	}


}

void UClimbingComponent::PlayClimbMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) return;
	if (!OwningPlayerAnimInstance) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	if ( MontageToPlay == TopToClimbMontage || MontageToPlay == VaultMontage)
	{
		OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
	}

	else if (MontageToPlay == ClimbToTopMontage)
	{
		

			//StartClimbing();
			OwningPlayerAnimInstance->Montage_Play(ClimbToTopMontage);
	}

	else
	{
		//FVector MotionWarpingLocation = (LedgeLocation - FVector( 0.f, 0.f , CapsuleComponent->GetScaledCapsuleHalfHeight() * 2)) + ActorOwner->GetActorRightVector() * CapsuleComponent->GetScaledCapsuleRadius() /2;
		MotionWarpingLocation = (LedgeLocation - FVector(0.f, 0.f, CapsuleComponent->GetScaledCapsuleHalfHeight() * 2));
		//DrawDebugSphere(GetWorld(), LedgeLocation, 15.f, 12, FColor::Magenta, false, 8.f);
		//DrawDebugSphere(GetWorld(), MotionWarpingLocation, 15.f, 12, FColor::Blue, false, 8.f);
		MotionWarpingRotator = FRotator(0.f, ClimbRotation.Yaw, 0.f);

		MotionWarpingLocation -= CurrentClimbableSurfaceNormal.GetSafeNormal() * CapsuleComponent->GetScaledCapsuleRadius();

		SetMotionWarpTarget(FName("ClimbStartPoint"), CapsuleComponent->GetComponentLocation(), CapsuleComponent->GetComponentRotation());
		SetMotionWarpTarget(FName("ClimbLandPoint"), MotionWarpingLocation, MotionWarpingRotator);

		//StartClimbing();
		MovementComponent->MaxFlySpeed = 0.f;
		MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying, 0);
		OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
		//MoveToLedgeLocation();
		return;
	}


}

#pragma endregion

#pragma region ClimbTraces

const FHitResult UClimbingComponent::DoLineTraceSingleByChannel(const FVector& Start, const FVector& End)
{
	FHitResult OutHit;
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	UKismetSystemLibrary::LineTraceSingle(this, Start, End, TraceType, false, TArray<AActor*>(), DebugTraceType, OutHit, false);

	return OutHit;
}

const FHitResult UClimbingComponent::DoLineTraceSingleByObject(const FVector& Start, const FVector& End)
{
	FHitResult OutHit;

	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::LineTraceSingleForObjects(this, Start, End, ClimbableObjectTypes, false, TArray<AActor*>(), DebugTraceType, OutHit, false);

	return OutHit;
}

const FHitResult UClimbingComponent::DoSphereTraceSingleForObjects(const FVector& Start, const FVector& End, float Radius)
{
	FHitResult OutHit;
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	TArray<AActor*> ActorsToIgnoreArray;
	if(ClimbingState == EClimbingState::ECS_Falling)  ActorsToIgnoreArray.Add(ClimbedObject);

	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, Radius, ClimbableObjectTypes, false, ActorsToIgnoreArray, DebugTraceType, OutHit, true);

	return OutHit;
}

const FHitResult UClimbingComponent::DoSphereTraceSingleForChannel(const FVector& Start, const FVector& End, float Radius)
{
	FHitResult OutHit;
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	UKismetSystemLibrary::SphereTraceSingle(this, Start, End, Radius, TraceType, false, TArray<AActor*>(), DebugTraceType, OutHit, true, FLinearColor::Yellow);

	return OutHit;
}

const FHitResult UClimbingComponent::DoCapsuleTraceSingleForObjects(const FVector& Start, const FVector& End, float Radius, float HalfHeight)
{
	FHitResult OutHit;
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	TArray<AActor*> ActorsToIgnoreArray;
	ActorsToIgnoreArray.Add(ClimbedObject);

	UKismetSystemLibrary::CapsuleTraceSingleForObjects(this, Start, End, Radius, HalfHeight, ClimbableObjectTypes, false, ActorsToIgnoreArray, DebugTraceType, OutHit, true, FLinearColor::White);

	return OutHit;
}

const FHitResult UClimbingComponent::DoCapsuleTraceSingleForChannel(const FVector& Start, const FVector& End, float Radius, float HalfHeight)
{
	FHitResult OutHit;
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	UKismetSystemLibrary::CapsuleTraceSingle(this, Start, End, Radius, HalfHeight, TraceType, false, TArray<AActor*>(), DebugTraceType, OutHit, true, FLinearColor::Yellow);

	return OutHit;
}

const FHitResult UClimbingComponent::TraceFromEyeHeight(const float Radius, const float BaseEyeHeightOffset_UP, const float BaseEyeHeightOffset_Right, bool DoSphereTrace)
{
	const FVector CharacterLocation = ActorOwner->GetActorLocation();
	const FVector ForwardVector = ActorOwner->GetActorForwardVector();
	const float BaseEyeHeightOffset = GetBaseEyeHeightOffset(BaseEyeHeightOffset_UP);
	const FVector UpVector = ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight + BaseEyeHeightOffset);
	const FVector RightVector = ActorOwner->GetActorRightVector() * BaseEyeHeightOffset_Right;

	const FVector Start = CharacterLocation + UpVector + RightVector;
	const FVector End = Start + ForwardVector * ForwardOffset;

	if(DoSphereTrace) return DoSphereTraceSingleForObjects(Start, End, Radius);

	return DoCapsuleTraceSingleForObjects(Start, End, Radius,25.f);

}

float UClimbingComponent::GetBaseEyeHeightOffset(const float BaseEyeHeightOffset_UP)
{
	float BaseEyeHeightOffset = (MovementComponent->IsFalling() && ClimbingState != EClimbingState::ECS_Falling) ? BaseEyeHeightOffset_Jumping : BaseEyeHeightOffset_Idle;
	BaseEyeHeightOffset = (ClimbingState == EClimbingState::ECS_Climbing) ? BaseEyeHeightOffset_UP : BaseEyeHeightOffset;
	BaseEyeHeightOffset = ClimbingState == EClimbingState::ECS_Falling ? -BaseEyeHeightOffset * 2.f : BaseEyeHeightOffset;

	return BaseEyeHeightOffset;
}

const FHitResult UClimbingComponent::TraceFromClimbableObject(const float Radius, const FVector& ImpactPoint)
{
	float ClimbingTraceHeight = MovementComponent->IsFalling() ? ClimbingTraceHeight_Jumping : ClimbingTraceHeight_Idle;
	ClimbingTraceHeight = ClimbingState == EClimbingState::ECS_Climbing ? ClimbingTraceHeight_Hanging_UP : ClimbingTraceHeight;
	const FVector Start = ImpactPoint + FVector(0.f, 0.f, ClimbingTraceHeight);
	const FVector End = ImpactPoint;

	return DoSphereTraceSingleForObjects(Start, End, Radius_SecondTrace);
}

const FHitResult UClimbingComponent::DoClimbableDownLedgeTrace()
{
	const FVector ComponentLocation = ActorOwner->GetActorLocation();
	const FVector ComponentForward = ActorOwner->GetActorForwardVector();
	const FVector DownVector = -ActorOwner->GetActorUpVector();

	const float height = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector ClimbableSurfaceTraceStart = ComponentLocation + FVector(0.f, 0.f, -height - 20.f);
	const FVector ClimbableSurfaceTraceEnd = ClimbableSurfaceTraceStart + ComponentForward * 80.f;

	return DoSphereTraceSingleForObjects(ClimbableSurfaceTraceEnd, ClimbableSurfaceTraceStart, 30.f);
}

const FHitResult UClimbingComponent::DoClimbCornerTrace(const FHitResult& outEndLedgePointHit, float Direction, bool InternLedge, bool BlindPoint)
{
	FVector Start;
	FVector End;

	if (!InternLedge)
	{
		FVector a = ActorOwner->GetActorLocation() + ActorOwner->GetActorForwardVector() * (LedgeHeightLocationXY + HandBorder_Forward) + ActorOwner->GetActorRightVector() * (MoveRightOffset)*FMath::Sign(Direction) + ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight + MoveUPOffset - Radius_FirstTrace_Hand + Radius_FirstTrace_Hand / 4);
		//Start = outEndLedgePointHit.TraceEnd + (BlindPoint? ActorOwner->GetActorForwardVector() * ( HandBorder_Backward + HandBorder_Forward - ForwardOffset) : FVector::ZeroVector);
		Start = BlindPoint ? a : outEndLedgePointHit.TraceEnd + FVector::ZeroVector;
		End = Start + ActorOwner->GetActorRightVector() * 40.f * -Direction;
	}

	else
	{
		FVector a = ActorOwner->GetActorLocation() + ActorOwner->GetActorForwardVector() * (-LedgeHeightLocationXY) + ActorOwner->GetActorRightVector() * (MoveRightOffset)*FMath::Sign(Direction) + ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight + MoveUPOffset - Radius_FirstTrace_Hand + Radius_FirstTrace_Hand / 4);

		Start = a + ActorOwner->GetActorRightVector() * 40.f * -Direction;
		End = a - ActorOwner->GetActorRightVector() * 50.f * -Direction;
	}

	return DoSphereTraceSingleForObjects(Start, End, Radius_Corner);
}

const FHitResult UClimbingComponent::DoClimbJumpTrace(size_t i)
{
	const FVector Start = ActorOwner->GetActorLocation() + ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight + ClimbingTraceHeight_Hanging_UP) + ActorOwner->GetActorUpVector() * Radius_FirstTrace * (i + 1);
	const FVector End = Start + ActorOwner->GetActorForwardVector() * ForwardOffset;
	return DoCapsuleTraceSingleForObjects(Start, End, Radius_FirstTrace, Radius_FirstTrace);
}

const FHitResult UClimbingComponent::DoClimbDirectionalJumpTrace(size_t i , float Direction, float UP_Offset)
{
	const float Radius = 25.f;
	const float HalfHeight = 50.f;

	const float HandOffset = Radius_FirstHand + 1.f;

	const FVector Start = ActorOwner->GetActorLocation() + ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight + UP_Offset) +
		ActorOwner->GetActorForwardVector() * (-HandBorder_Backward * 1.25f) +
		ActorOwner->GetActorRightVector() * (HandOffset + Radius) * Direction +
		ActorOwner->GetActorRightVector() * Direction * Radius * (i + 1);

	const FVector End = Start + ActorOwner->GetActorForwardVector() * HandBorder_Backward * 4.f;

	return DoCapsuleTraceSingleForObjects(Start, End, Radius, HalfHeight);
}

const FHitResult UClimbingComponent::DoClimbBackJumpTrace(size_t i)
{
	const float Radius = 25.f;
	const float HalfHeight = 100.f;
	const float UP_Offset = -3.f * ClimbingTraceHeight_Hanging_UP;
	const float HandOffset = Radius_FirstHand + 1.f;

	const FVector Start = ActorOwner->GetActorLocation() + ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight +  UP_Offset) +
		ActorOwner->GetActorForwardVector() * (-HandBorder_Backward * 2.75f) -
		ActorOwner->GetActorRightVector() * (HandOffset + 6*Radius)  +
		ActorOwner->GetActorRightVector() * Radius * (i );

	const FVector End = Start + ActorOwner->GetActorForwardVector() * HandBorder_Backward * 2.25f;

	return DoCapsuleTraceSingleForObjects(Start, End, Radius, HalfHeight);
}

const FHitResult UClimbingComponent::DoClimbUponTrace()
{
	const FVector ComponentLocation = ActorOwner->GetActorLocation();
	const FVector EyeHeightOffset = ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight * BaseEyeHeightOffset_Landing);

	const FVector Start = ComponentLocation + EyeHeightOffset;
	const FVector End = Start + ActorOwner->GetActorForwardVector() * ForwardOffset_Landing;

	return DoSphereTraceSingleForChannel(Start, End, Radius_FirstTrace_Landing);
}

const FHitResult UClimbingComponent::DoClimbUponLineDownTrace(const FVector Start_Height)
{
	const FVector DownVector = -ActorOwner->GetActorUpVector();
	const FVector End_Height = Start_Height + DownVector * ClimbingTraceHeight_Landing;

	return DoLineTraceSingleByChannel(Start_Height, End_Height);
}

#pragma endregion



void UClimbingComponent::SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition, const FRotator& InTargetRotation)
{
	if (!MotionWarpingComponent) return;

	if (InTargetRotation != FRotator::ZeroRotator) 
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(InWarpTargetName, InTargetPosition, InTargetRotation);
	else 
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(InWarpTargetName, InTargetPosition);

}

void UClimbingComponent::TryStartUponVaulting()
{
	FVector VaultStartPosition;
	FVector VaultLandPosition;

	if (CanStartUponVaulting(VaultStartPosition, VaultLandPosition))
	{
		//Start Vaulting
		SetMotionWarpTarget(FName("VaultStartPoint"), VaultStartPosition);
		SetMotionWarpTarget(FName("VaultLandPoint"), VaultLandPosition);

		//StartClimbing();
		MovementComponent->MaxFlySpeed = 0.f;
		MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying, 0);
		PlayClimbMontage(VaultMontage);
	}

}

bool UClimbingComponent::CanStartUponVaulting(FVector& OutVaultStartPosition, FVector& OutVaultLandPosition)
{
	if (MovementComponent->IsFalling()) return false;

	OutVaultStartPosition = FVector::ZeroVector;
	OutVaultLandPosition = FVector::ZeroVector;

	const FVector ComponentLocation = ActorOwner->GetActorLocation();
	const FVector ComponentForward = ActorOwner->GetActorForwardVector();
	const FVector UpVector = ActorOwner->GetActorUpVector();
	const FVector DownVector = -ActorOwner->GetActorUpVector();

	for (int32 i = 0; i < 5; i++)
	{
		const FVector Start = ComponentLocation + UpVector * 125.f  + ComponentForward * 40.f + ComponentForward * 20.f * (i + 1);
		const FVector End = Start + DownVector * 100.f * (i + 1);

		//FHitResult VaultTraceHit = DoLineTraceSingleByObject(Start, End);
		FHitResult VaultTraceHit = DoLineTraceSingleByChannel(Start, End);

		if (i == 0 && VaultTraceHit.bBlockingHit)
		{
			OutVaultStartPosition = VaultTraceHit.ImpactPoint + UpVector * 45.f;
		}

		if (i == 4 && VaultTraceHit.bBlockingHit)
		{
			OutVaultLandPosition = VaultTraceHit.ImpactPoint ;
		}
	}

	if (OutVaultStartPosition != FVector::ZeroVector && OutVaultLandPosition != FVector::ZeroVector)
	{
		return true;
	}
	else
	{
		return false;
	}
}









