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

	if (!bIsClimbing && ((MovementComponent->IsFalling() && MovementComponent->Velocity.Z >0.f) || ClimbingState == EClimbingState::ECS_Falling)) {TryClimbing(); }
	
	
	if (bClimbDown && !MovementComponent->IsFalling() && !OwningPlayerAnimInstance->IsAnyMontagePlaying()  && CanClimbDownLedge())
	{
		PlayClimbMontage(TopToClimbMontage);
	}

	//Debug::Print(TEXT("Climbing State : ") + EnumToString(ClimbingState), FColor::Cyan, 7);

}

#pragma endregion

#pragma region SetClimbState

void UClimbingComponent::ToggleClimbing()
{
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	//if (bIsClimbing) {  StopClimbing(); bCanClimb = false; }
	if (bIsClimbing) {
		ClimbingState = EClimbingState::ECS_Falling;
		bIsClimbing = false;
		SavedClimbedObject = nullptr; 
		MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling); CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(90);
	CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	
}

void UClimbingComponent::StartClimbing()
{
	bIsClimbing = true;
	bClimbDown = false;
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
	bIsClimbing = false;
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
	if (FindClimbableObject())
	{
		//DrawDebugSphere(GetWorld(), LedgeLocation, 8.f, 12, FColor::Emerald, false, 20.f);
		//DrawDebugLine(GetWorld(),   FVector(CurrentClimbableSurfaceLocation.X, CurrentClimbableSurfaceLocation.Y,LedgeLocation.Z), LedgeLocation, FColor::Blue, false, 3.f);
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
		//return CheckCapsuleSpaceCondition(LedgeLocation - FVector(0.f,0.f, CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));


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

		const FHitResult outClimbableObjectHit = TraceFromEyeHeight(Radius_FirstHand, MoveUPOffset, MoveRightOffset * FMath::Sign(Direction),true);

		if (outClimbableObjectHit.bBlockingHit)
		{
			// Same as TraceFromClimbableObject except for the negative ending position to garanteed to handle slping surface movement
			const FVector Start_ClimbablePoint = outClimbableObjectHit.ImpactPoint + FVector(0.f, 0.f, ClimbingTraceHeight_Hanging_Right);
			const FVector End_ClimbablePoint = Start_ClimbablePoint + FVector(0.f, 0.f, -ClimbingTraceHeight_Hanging_Right);

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
						//Debug::Print(TEXT("I Can Jump!!"));
						UAnimMontage* Montage = SignDirection > 0 ? ClimbJumpRightMontage : ClimbJumpLeftMontage;
						//PlayClimbMontage(Montage);
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
					//Debug::Print(TEXT("I Can Jump!!"));
					UAnimMontage* Montage = SignDirection > 0 ? ClimbJumpRightMontage : ClimbJumpLeftMontage;
					//PlayClimbMontage(Montage);
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
				//Debug::Print(TEXT("I Can Jump!!"));
				UAnimMontage* Montage = SignDirection > 0 ? ClimbJumpRightMontage : ClimbJumpLeftMontage;
				//PlayClimbMontage(Montage);
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
				//Debug::Print(TEXT("I Can Jump!!"));
				UAnimMontage* Montage = SignDirection > 0 ? ClimbJumpRightMontage : ClimbJumpLeftMontage;
				//PlayClimbMontage(Montage);
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
	if (MovementComponent->IsFalling() || bIsClimbing) return false;

	const FVector ComponentLocation = ActorOwner->GetActorLocation();
	const FVector ComponentForward = ActorOwner->GetActorForwardVector();
	const FVector DownVector = -ActorOwner->GetActorUpVector();

	const float height = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	const FVector WalkableSurfaceTraceStart = ComponentLocation + FVector(0.f,0.f,-height -20.f);
	const FVector WalkableSurfaceTraceEnd = WalkableSurfaceTraceStart + ComponentForward * 80.f;

	FHitResult WalkableSurfaceHit = DoSphereTraceSingleForObjects( WalkableSurfaceTraceEnd, WalkableSurfaceTraceStart, 30.f);
	
	if (WalkableSurfaceHit.bBlockingHit && std::abs(WalkableSurfaceHit.ImpactNormal.Z) < MaxImpactNormal_Z_value &&
		FMath::IsNearlyEqual(FVector::DotProduct(WalkableSurfaceHit.ImpactNormal, ActorOwner->GetActorForwardVector()), 1.0f, MaxImpactNormalToForwardVector_Cos_value))
	{
		ClimbedObject = WalkableSurfaceHit.GetActor();
		return FindClimbablePoint(WalkableSurfaceHit);
	}


	return false;
}

bool UClimbingComponent::CanClimbCorner(const FHitResult& outEndLedgePointHit, float Direction, bool InternLedge, bool BlindPoint )
{
	if (ClimbingState == EClimbingState::ECS_Falling) return false;

	FVector Start ;
	FVector End ;

	if (!InternLedge)
	{
		 FVector a = ActorOwner->GetActorLocation() + ActorOwner->GetActorForwardVector() * (LedgeHeightLocationXY + HandBorder_Forward) + ActorOwner->GetActorRightVector() * (MoveRightOffset) * FMath::Sign(Direction) + ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight + MoveUPOffset - Radius_FirstTrace_Hand + Radius_FirstTrace_Hand/4);
		 //Start = outEndLedgePointHit.TraceEnd + (BlindPoint? ActorOwner->GetActorForwardVector() * ( HandBorder_Backward + HandBorder_Forward - ForwardOffset) : FVector::ZeroVector);
		 Start = BlindPoint ?  a : outEndLedgePointHit.TraceEnd + FVector::ZeroVector ;
		 End = Start + ActorOwner->GetActorRightVector() * 40.f * -Direction;
	}

	else
	{
		FVector a = ActorOwner->GetActorLocation() + ActorOwner->GetActorForwardVector() * (-LedgeHeightLocationXY) + ActorOwner->GetActorRightVector() * (MoveRightOffset)*FMath::Sign(Direction) + ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight + MoveUPOffset - Radius_FirstTrace_Hand + Radius_FirstTrace_Hand / 4);
		/*Start = outEndLedgePointHit.ImpactPoint + ActorOwner->GetActorRightVector() * 40.f * -Direction;
		End = outEndLedgePointHit.ImpactPoint - ActorOwner->GetActorRightVector() * 40.f * -Direction;*/

		Start = a + ActorOwner->GetActorRightVector() * 40.f * -Direction;
		End = a - ActorOwner->GetActorRightVector() * 50.f * -Direction;
	}
	

	const FHitResult outClimbableObjectHit = DoSphereTraceSingleForObjects(Start , End, Radius_Corner);

	if (outClimbableObjectHit.bBlockingHit && FindClimbablePoint(outClimbableObjectHit))
	{
		ClimbedObject = outClimbableObjectHit.GetActor();
		return true;
	}

	return false;
}

bool UClimbingComponent::CanClimbJump()
{
	if (FindClimbableObject(ClimbingTraceHeight_Hanging_UP)) return true;

	const float Radius = Radius_FirstTrace;

	for (size_t i = 0; i < 5; i++)
	{
		const FVector Start = ActorOwner->GetActorLocation() + ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight + ClimbingTraceHeight_Hanging_UP) + ActorOwner->GetActorUpVector() * Radius * (i+1);
		const FVector End = Start + ActorOwner->GetActorForwardVector() * ForwardOffset;


		const FHitResult hit = DoCapsuleTraceSingleForObjects(Start, End, Radius, Radius_FirstTrace);

		if (hit.bBlockingHit && FindClimbablePoint(hit))
		{
			const float CapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			const float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
			if (!DoCapsuleTraceSingleForChannel(ActorOwner->GetActorLocation(), LedgeLocation, CapsuleRadius, CapsuleHalfHeight).bBlockingHit)
			{

				ClimbedObject = hit.GetActor();
				return true;
			}
		}
	}

	return false;

}

bool UClimbingComponent::CanClimbDirectionalJump( float Direction , float UP_Offset)
{
	if (ClimbingState == EClimbingState::ECS_Falling) return false;

	const float Radius = 25.f;
	const float HalfHeiht = 50.f;

	for (size_t i = 0; i < 15; i++)
	{
		const FVector Start = ActorOwner->GetActorLocation() + ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight + UP_Offset) +
			ActorOwner->GetActorForwardVector() * (-HandBorder_Backward * 2.25f) +
			ActorOwner->GetActorRightVector() * Direction * Radius * (i + 1);

		const FVector End = Start + ActorOwner->GetActorForwardVector() * HandBorder_Backward * 4.f;


		const FHitResult hit = DoCapsuleTraceSingleForObjects(Start, End, Radius, HalfHeiht);

		if (hit.bBlockingHit && FindClimbablePoint(hit))
		{
			const float CapsuleHalfHeight = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			const float CapsuleRadius = CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
			if (!DoCapsuleTraceSingleForChannel(ActorOwner->GetActorLocation(), LedgeLocation, CapsuleRadius, CapsuleHalfHeight).bBlockingHit)
			{

				SavedClimbedObject = hit.GetActor();
				return true;
			}

		}

	}


	return false;
}

#pragma endregion

#pragma region ClimbingMove

void UClimbingComponent::LedgeMove(const FVector2D MovementVector)
{
	if (!bIsClimbing ) return;

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
	else if (Direction.X != 0 )
	{
		//const FHitResult outClimbableObjectHit = TraceFromEyeHeight(Radius_FirstTrace, Y_Offset, RightOffset);
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

	if(Direction.X == 0) bJumpSaved = false;


	return false;
}

bool UClimbingComponent::TryClimbUpon()
{
	if (!OwningPlayerAnimInstance->IsAnyMontagePlaying() && CanClimbUpon())
	{
		ClimbDirection = 0.f;
		bIsClimbing = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(90);
		PlayClimbMontage(ClimbToTopMontage);
		return true;
	}
	return false;
}

bool UClimbingComponent::TryJumping()
{
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
	float BaseEyeHeightOffset = (MovementComponent->IsFalling() && ClimbingState != EClimbingState::ECS_Falling) ? BaseEyeHeightOffset_Jumping : BaseEyeHeightOffset_Idle;
	BaseEyeHeightOffset = (bIsClimbing ) ?  BaseEyeHeightOffset_UP : BaseEyeHeightOffset;

	BaseEyeHeightOffset =    ClimbingState == EClimbingState::ECS_Falling ? -BaseEyeHeightOffset*2.f : BaseEyeHeightOffset;
	const FVector UpVector = ActorOwner->GetActorUpVector() * (CharacterOwner->BaseEyeHeight + BaseEyeHeightOffset);

	const FVector RightVector = ActorOwner->GetActorRightVector() * BaseEyeHeightOffset_Right;

	const FVector Start = CharacterLocation + UpVector + RightVector;
	const FVector End = Start + ForwardVector * ForwardOffset;

	if(DoSphereTrace) return DoSphereTraceSingleForObjects(Start, End, Radius);

	return DoCapsuleTraceSingleForObjects(Start, End, Radius,25.f);

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













