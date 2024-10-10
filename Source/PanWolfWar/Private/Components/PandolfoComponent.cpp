#include "Components/PandolfoComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Components/ClimbingComponent.h"
#include "Components/KiteComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GameFramework/Character.h"

#include "Kismet/KismetSystemLibrary.h"

#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

#include "Actors/KiteBoard.h"

#include "Kismet/GameplayStatics.h"

#include "Components/SneakCoverComponent.h"

#include <Components/TimelineComponent.h>

#include "Camera/CameraComponent.h"
#include "Enemy/AssassinableComponent.h"
#include "Enemy/BaseEnemy.h"

#include "Components/Combat/PandoCombatComponent.h"

#pragma region EngineFunctions

UPandolfoComponent::UPandolfoComponent()
{
	ClimbingComponent = CreateDefaultSubobject<UClimbingComponent>(TEXT("ClimbingComponent"));
	SneakCoverComponent = CreateDefaultSubobject<USneakCoverComponent>(TEXT("SneakCoverComponent"));
	KiteComponent = CreateDefaultSubobject<UKiteComponent>(TEXT("KiteComponent"));

	Knife = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Knife"));
	Knife->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UPandolfoComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!CrouchCameraLenght_Curve) return;

	FOnTimelineFloat ProgressUpdate;
	ProgressUpdate.BindUFunction(this, FName("CrouchCameraUpdate"));
	CrouchingTimeline.AddInterpFloat(CrouchCameraLenght_Curve, ProgressUpdate);

}

void UPandolfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CrouchingTimeline.TickTimeline(DeltaTime);

}

#pragma endregion

#pragma region ActivationSection

void UPandolfoComponent::Activate(bool bReset)
{
	Super::Activate();

	PandolfoState = EPandolfoState::EPS_Pandolfo;

	PanWolfCharacter->SetMetaHumanVisibility(true);

	if (OwningPlayerAnimInstance)
	{
		CombatComponent->SetCombatEnabled(OwningPlayerAnimInstance, ETransformationCombatType::ETCT_Pandolfo);
	}

	if (PanWolfCharacter->IsHiding())
	{
		CharacterOwner->GetMesh()->SetScalarParameterValueOnMaterials(FName("HideFxSwitch"), 10.f);
		PanWolfCharacter->SetMetaHumanHideFX(10.f);
	}

	if (PanWolfCharacter->IsInsideHideBox() && PanWolfCharacter->IsForcedCrouch() && !PanWolfCharacter->IsHiding())
	{
		MovementComponent->bWantsToCrouch = true;
		PanWolfCharacter->SetIsHiding(true);
	}



	if (MovementComponent->IsFalling())
		ClimbingComponent->Activate();


	PanWolfCharacter->GetCharacterMovement()->MaxFlySpeed = 0.f;

	ClimbingComponent->SetAnimationBindings();



	Knife->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("foot_Knife_Socket"));
	Knife->SetVisibility(true);

	//KiteComponent->Activate();


	if (!CharacterOwner->GetMovementComponent()->IsMovingOnGround())
	{
		bIsGlideTimerActive = true;
		GetWorld()->GetTimerManager().SetTimer(Glide_TimerHandle, [this]() {this->TryGliding(); }, 0.25f, true);
	}

	GetWorld()->GetTimerManager().SetTimer(AirAssassination_TimerHandle, [this]() {this->CheckCanAirAssassin(); }, 0.25f, true);

	if (MovementComponent->IsCrouching())
		CrouchingTimeline.PlayFromStart();
	else
		CrouchingTimeline.Reverse();
}

void UPandolfoComponent::Deactivate()
{
	Super::Deactivate();

	ClimbingComponent->Deactivate();

	Knife->SetVisibility(false);

	GetWorld()->GetTimerManager().ClearTimer(AirAssassination_TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(Glide_TimerHandle);

	PanWolfCharacter->SetMetaHumanVisibility(false);

	if (PandolfoState == EPandolfoState::EPS_Dodging)
		PanWolfCharacter->EndDodge();

	PandolfoState = EPandolfoState::EPS_Pandolfo;
}

#pragma endregion

#pragma region Actions

void UPandolfoComponent::Jump()
{

	if (PandolfoState != EPandolfoState::EPS_Gliding)
	{
		if (!TryClimbOrMantle() && !ClimbingComponent->TryVault())
		{
			if (!PredictJump())
			{
				if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;
				CharacterOwner->Jump();
			}
		}
	}
}

void UPandolfoComponent::Dodge()
{
	if (!PanWolfCharacter->CanPerformDodge() || PandolfoState != EPandolfoState::EPS_Pandolfo) return;
	if (!PandolfoDodgeMontage || !OwningPlayerAnimInstance) return;

	PandolfoState = EPandolfoState::EPS_Dodging;

	PanWolfCharacter->StartDodge();
	OwningPlayerAnimInstance->Montage_Play(PandolfoDodgeMontage);

	FOnMontageEnded DodgeMontageEndedDelegate;
	DodgeMontageEndedDelegate.BindUObject(this, &UPandolfoComponent::OnDodgeMontageEnded);
	OwningPlayerAnimInstance->Montage_SetEndDelegate(DodgeMontageEndedDelegate, PandolfoDodgeMontage);
}

void UPandolfoComponent::Crouch()
{
	if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;
	if (PanWolfCharacter && PanWolfCharacter->IsForcedCrouch()) return;

	const bool IsCrouched = CharacterOwner->bIsCrouched;
	MovementComponent->bWantsToCrouch = !IsCrouched;

	if (!IsCrouched)
	{
		CrouchingTimeline.PlayFromStart();

		if (PanWolfCharacter->IsInsideHideBox())
		{
			PanWolfCharacter->SetIsHiding(true);
		}

	}

	else
	{
		CrouchingTimeline.Reverse();


		if (PanWolfCharacter->IsInsideHideBox())
		{
			PanWolfCharacter->SetIsHiding(false);
		}

	}

}

void UPandolfoComponent::LightAttack()
{
	if (!CombatComponent) return;
	if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;

	GetWorld()->GetTimerManager().ClearTimer(KnifeEquipped_TimerHandle);
	TakeKnife(true);
	GetWorld()->GetTimerManager().SetTimer(KnifeEquipped_TimerHandle, [this]() {this->TakeKnife(false); }, KnifeTime, false);

	CombatComponent->PerformAttack(EAttackType::EAT_LightAttack);
}

void UPandolfoComponent::HandleFalling()
{
	ClimbingComponent->Activate();
}

void UPandolfoComponent::HandleLand()
{
	switch (PandolfoState)
	{
	case EPandolfoState::EPS_Pandolfo:
		break;
	case EPandolfoState::EPS_Climbing:
		ClimbingComponent->Landed();
		break;
	case EPandolfoState::EPS_Covering:
		break;
	case EPandolfoState::EPS_Gliding:
		UnGlide();
		break;
	default:
		break;
	}

	if (ClimbingComponent->IsActive())
		ClimbingComponent->Deactivate();

	if (bIsGlideTimerActive)
	{
		bIsGlideTimerActive = false;
		GetWorld()->GetTimerManager().ClearTimer(Glide_TimerHandle);
	}

}

void UPandolfoComponent::CrouchCameraUpdate(float Alpha)
{
	CameraBoom->TargetArmLength = UKismetMathLibrary::Lerp(TransformationCharacterData.TargetArmLength, 550.f, Alpha);
}

bool UPandolfoComponent::TryClimbOrMantle()
{
	if (!ClimbingComponent->TryClimbing() && !ClimbingComponent->TryMantle())
		return false;
	return true;
}

const bool UPandolfoComponent::IsClimbing()
{
	return ClimbingComponent->IsClimbing();
}

#pragma endregion

#pragma region PredictableJump

const FHitResult UPandolfoComponent::TraceIsOnGround(const FVector RootLocation, const FVector ForwardVector)
{
	const FVector OnGroundStart = RootLocation + ForwardVector * 5.f;
	const FVector OnGroundEnd = OnGroundStart + FVector(0.f, 0.f, 0.1f);
	EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	FHitResult OnGroundHit;
	UKismetSystemLibrary::SphereTraceSingle(this, OnGroundStart, OnGroundEnd, 10.f, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, OnGroundHit, true);

	return OnGroundHit;
}

const FHitResult UPandolfoComponent::PredictProjectileTrace(const FVector ActorLocation, const FVector ForwardVector, AActor* OnGroundActor)
{
	const FVector ProjectileStart = ActorLocation + ForwardVector * 250.f;
	const FVector ProjectileVelocity = ForwardVector * 10.f;

	FPredictProjectilePathParams PredictParams(75.f, ProjectileStart, ProjectileVelocity, 7, PredictJumpObjectTypes, OnGroundActor);
	PredictParams.OverrideGravityZ = -10.f;
	PredictParams.DrawDebugTime = 3.f;
	PredictParams.SimFrequency = 0.4;
	PredictParams.DrawDebugType =  ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	FPredictProjectilePathResult PredictResult;

	UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);

	return PredictResult.HitResult;
}

const FHitResult UPandolfoComponent::TraceLandConditions(const FVector ImpactPoint, const FVector ForwardVector)
{
	const FVector SpaceCapsuleStart = ImpactPoint + ForwardVector * 50.f + FVector(0.f, 0.f, 30.f);
	const FVector SpaceCapsuleEnd = SpaceCapsuleStart + FVector(0.f, 0.f, -60.f);
	FHitResult OnSpaceCapsuleHit;
	EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	UKismetSystemLibrary::LineTraceSingle(this, SpaceCapsuleStart, SpaceCapsuleEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, OnSpaceCapsuleHit, true);

	return OnSpaceCapsuleHit;
}

const FHitResult UPandolfoComponent::TraceObstacles(const FVector ActorLocation, const FVector ImpactPoint)
{
	const FVector ObastacleStart = ActorLocation;
	const FVector  ObastacleEnd = ImpactPoint + FVector(0.f, 0.f, 120.f);
	FHitResult OnObastacleHit;
	EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	UKismetSystemLibrary::LineTraceSingle(this, ObastacleStart, ObastacleEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, OnObastacleHit, true);

	return OnObastacleHit;
}

void UPandolfoComponent::LoadPredictJump(const FVector ActorLocation)
{
	if (!OwningPlayerAnimInstance) return;

	const float NewYaw = UKismetMathLibrary::FindLookAtRotation(ActorLocation, LandPredictLocation).Yaw;
	const FRotator NewRotation = FRotator(CharacterOwner->GetActorRotation().Pitch, NewYaw, CharacterOwner->GetActorRotation().Roll);
	CharacterOwner->SetActorRotation(NewRotation);
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying, 0);

	OwningPlayerAnimInstance->Montage_Play(PredictJumpMontage);

	OwningPlayerAnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UPandolfoComponent::StartPredictJump);
	OwningPlayerAnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UPandolfoComponent::StopPredictJump);
	OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UPandolfoComponent::EndPredictJump);
}

bool UPandolfoComponent::PredictJump()
{
	//UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return false;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return false;

	const FVector RootLocation = CharacterOwner->GetMesh()->GetSocketLocation(FName("root"));
	const FVector ForwardVector = CharacterOwner->GetActorForwardVector();
	const FVector ActorLocation = CharacterOwner->GetActorLocation();

	FHitResult OnGroundHit = TraceIsOnGround(RootLocation, ForwardVector);
	if (!OnGroundHit.bBlockingHit) return false;

	const FHitResult PredictHit = PredictProjectileTrace(ActorLocation, ForwardVector, OnGroundHit.GetActor());
	if (!PredictHit.bBlockingHit) return false;


	//DrawDebugPoint(GetWorld(), PredictHit.ImpactPoint, 22, FColor::Magenta, false, 2.f);

	const FHitResult OnSpaceCapsuleHit = TraceLandConditions(PredictHit.ImpactPoint,ForwardVector);
	if (!OnSpaceCapsuleHit.bBlockingHit || OnSpaceCapsuleHit.bStartPenetrating) return false;


	FHitResult OnObastacleHit = TraceObstacles(ActorLocation, OnSpaceCapsuleHit.ImpactPoint);
	if (OnObastacleHit.bBlockingHit) return false;

	LandPredictLocation = OnSpaceCapsuleHit.ImpactPoint + FVector(0.f, 0.f, 120.f);
	LoadPredictJump(ActorLocation);

	return true;
}

void UPandolfoComponent::StartPredictJump(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName != FName("PredictJump")) return;

	TimeElapsed = 0.f;
	LandPredictStartLocation = CharacterOwner->GetActorLocation();
	GetWorld()->GetTimerManager().SetTimer(PredictJump_TimerHandle, [this]() {this->DoPredictJump(); }, 0.01f, true);
}

void UPandolfoComponent::StopPredictJump(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName != FName("PredictJump")) return;
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling, 0);
	GetWorld()->GetTimerManager().ClearTimer(PredictJump_TimerHandle);
}

void UPandolfoComponent::EndPredictJump(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != PredictJumpMontage) return;
	
	//CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking, 0);
	GetWorld()->GetTimerManager().ClearTimer(PredictJump_TimerHandle);

}

void UPandolfoComponent::DoPredictJump()
{
	float min; float max;
	PredictJump_Curve->GetTimeRange(min, max);
	TimeElapsed = TimeElapsed + GetWorld()->GetTimerManager().GetTimerElapsed(PredictJump_TimerHandle);
	TimeElapsed = UKismetMathLibrary::FClamp(TimeElapsed, min, max);
	const float Alpha = PredictJump_Curve->GetFloatValue(TimeElapsed);

	
	FVector NewLocation = UKismetMathLibrary::VLerp(LandPredictStartLocation, LandPredictLocation, Alpha);
	CharacterOwner->SetActorLocation(NewLocation);
}

#pragma endregion

#pragma region Sliding

void UPandolfoComponent::Sliding()
{
	if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;

	if (!SlidingMontage) return;
	if (!OwningPlayerAnimInstance) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;
	if (MovementComponent->GetLastInputVector().Length() < 0.5f) return;

	PandolfoState = EPandolfoState::EPS_Sliding;
	OwningPlayerAnimInstance->Montage_Play(SlidingMontage);

	FOnMontageEnded SlidingMontageEndedDelegate;
	SlidingMontageEndedDelegate.BindUObject(this, &UPandolfoComponent::OnSlidingMontageEnded);
	OwningPlayerAnimInstance->Montage_SetEndDelegate(SlidingMontageEndedDelegate, SlidingMontage);
}

void UPandolfoComponent::StartSliding()
{
	if (MovementComponent->IsCrouching())
	{
		Capsule->SetCapsuleSize(35.f,40.f);
		CrouchingTimeline.Reverse();
	}
		
	MovementComponent->CrouchedHalfHeight = 40.f;
	Capsule->SetCapsuleRadius(40.f);
	MovementComponent->bWantsToCrouch = true;
	CharacterOwner->GetMesh()->AddLocalOffset(FVector(-15.f, 0.f, 0.f));

	if (PanWolfCharacter->IsInsideHideBox())
	{
		//Debug::Print(TEXT("Try to hide"));
		PanWolfCharacter->SetIsHiding(true);
	}
}

void UPandolfoComponent::EndSliding()
{
	MovementComponent->CrouchedHalfHeight = 55.f;
	Capsule->SetCapsuleRadius(TransformationCharacterData.CapsuleRadius);
	MovementComponent->bWantsToCrouch = false;
	CharacterOwner->GetMesh()->AddLocalOffset(FVector(15.f, 0.f, 0.f));

	PandolfoState = EPandolfoState::EPS_Pandolfo;

	if (PanWolfCharacter->IsInsideHideBox())
	{
		PanWolfCharacter->SetIsHiding(false);
	}
}

#pragma endregion

#pragma region Gliding

void UPandolfoComponent::TryGliding()
{
	if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;
	if (CharacterOwner->GetMovementComponent()->IsMovingOnGround()) return;

	const FVector Start = CharacterOwner->GetActorLocation() ;
	const FVector End = Start - CharacterOwner->GetActorUpVector() * GlidingHeight;
	EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	FHitResult Hit;
	UKismetSystemLibrary::LineTraceSingle(this, Start, End, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, Hit, true);

	
	if (!Hit.bBlockingHit && MovementComponent->GetLastUpdateVelocity().Z < -GlidingVelocity)
	{
		//Debug::Print(TEXT("Glide"));
		PandolfoState = EPandolfoState::EPS_Gliding;
		GetWorld()->GetTimerManager().ClearTimer(Glide_TimerHandle);

		MovementComponent->StopMovementImmediately();
		MovementComponent->GravityScale = GlidingGravityScale;
		MovementComponent->AirControl = GlidingAirControl;

		UmbrellaActor = GetWorld()->SpawnActor<AActor>(UmbrellaActorClass, CharacterOwner->GetActorLocation(), CharacterOwner->GetActorRotation());
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		UmbrellaActor->AttachToComponent(CharacterOwner->GetMesh(), AttachmentRules, FName("hand_l_Umbrella"));

		
	}
}

void UPandolfoComponent::UnGlide()
{
	if(PandolfoState == EPandolfoState::EPS_Gliding)
	{
		//Debug::Print(TEXT("UnGlide"));
		PandolfoState = EPandolfoState::EPS_Pandolfo;

		UmbrellaActor->Destroy();
		MovementComponent->GravityScale = 1.75f;
		MovementComponent->AirControl = 0.35f;
	}

}

#pragma endregion

#pragma region Assassination

void UPandolfoComponent::AssassinationFromHiding(ABaseEnemy* HidingAssassinatedEnemy)
{
	AssassinableOverlapped = HidingAssassinatedEnemy;
	Assassination();
}

void UPandolfoComponent::Assassination()
{
	if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;
	if (!AssassinableOverlapped && !AIR_AssassinableOverlapped) return;
	if (AIR_AssassinableOverlapped) AssassinableOverlapped = AIR_AssassinableOverlapped;

	//UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	if (!AIR_AssassinableOverlapped && AssassinableOverlapped)
	{
		
		PlayStealthAssassination();
	}
	else
	{
		PlayAirAssassination();
		
	}

}

void UPandolfoComponent::PlayAirAssassination()
{
	if (!AssassinableOverlapped) return;
	AIR_AssassinableOverlapped = nullptr;

	const FRotator NewRotation = FRotator(0.f, UKismetMathLibrary::FindLookAtRotation(CharacterOwner->GetActorLocation(), AssassinableOverlapped->GetActorLocation()).Yaw, 0.f);
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(CharacterOwner->GetCapsuleComponent(), CharacterOwner->GetActorLocation(), NewRotation, true, true, 0.2f, true, EMoveComponentAction::Move, LatentInfo);

	PanWolfCharacter->GetFollowCamera()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	const FRotator NewCameraRotation = FRotator(0.f, UKismetMathLibrary::FindLookAtRotation(AssassinableOverlapped->GetActorLocation(), CharacterOwner->GetActorLocation()).Yaw, 0.f);
	const FVector V = UKismetMathLibrary::GetForwardVector(NewCameraRotation) * (-400.f);
	const FVector NewCameraLocation = AssassinableOverlapped->GetActorLocation() + FVector(V.X, V.Y, -50.f);

	PanWolfCharacter->GetFollowCamera()->SetWorldLocationAndRotation(NewCameraLocation, NewCameraRotation);
	GetWorld()->GetTimerManager().SetTimer(AirAssassinationCamera_TimerHandle, [this]() {this->PanWolfCharacter->GetFollowCamera()->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(PanWolfCharacter->GetFollowCamera()->GetComponentLocation(), CharacterOwner->GetActorLocation())); }, 0.01f, true);

	const FVector WarpLocation = AssassinableOverlapped->GetActorLocation();
	const FRotator WarpRotator = FRotator(0.f, UKismetMathLibrary::FindLookAtRotation(CharacterOwner->GetActorLocation(), WarpLocation).Yaw, 0.f);
	PanWolfCharacter->SetMotionWarpTarget(FName("AssasinationWarp"), WarpLocation, WarpRotator);

	AssassinableOverlapped->SetCollisionBoxAssassination(ECollisionEnabled::NoCollision);
	OwningPlayerAnimInstance->Montage_Play(AirAssassinMontage);
}

void UPandolfoComponent::PlayStealthAssassination()
{
	if (!AssassinableOverlapped) return;
	if (AssassinationMontage_Map.Num() == 0) return;

	int32 AssassinationIndex = FMath::RandRange(1, AssassinationMontage_Map.Num());
	UAnimMontage* AssassinationMontage = *AssassinationMontage_Map.Find(AssassinationIndex); 
	if (AssassinationIndex == 0 || AssassinationIndex > AssassinationMontage_Map.Num()) return;
	if (!AssassinationMontage ) return;

	const FTransform AssassinTransform = AssassinableOverlapped->GetAssassinationTransform();
	const FVector WarpLocation = AssassinTransform.GetLocation();
	const FRotator WarpRotator = AssassinTransform.Rotator() + FRotator(0.f, 90.f, 0.f);
	PanWolfCharacter->SetMotionWarpTarget(FName("AssasinationWarp"), WarpLocation, WarpRotator);
	OwningPlayerAnimInstance->Montage_Play(AssassinationMontage);
	AssassinableOverlapped->GetAssassinableComponent()->Assassinated(AssassinationIndex, this);
}

void UPandolfoComponent::AirKill()
{
	if(AssassinableOverlapped)
		AssassinableOverlapped->GetAssassinableComponent()->Assassinated(0,this, true);
}

void UPandolfoComponent::RiattachCamera()
{
	GetWorld()->GetTimerManager().ClearTimer(AirAssassinationCamera_TimerHandle);

	PanWolfCharacter->GetFollowCamera()->AttachToComponent(CameraBoom, FAttachmentTransformRules::KeepWorldTransform);
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(PanWolfCharacter->GetFollowCamera(),FVector::ZeroVector, FRotator::ZeroRotator, true, true, 1.5f, true, EMoveComponentAction::Move, LatentInfo);

}

void UPandolfoComponent::TakeKnife(bool Take, bool IsReverseSocket)
{
	FName SocketName = IsReverseSocket ? FName("hand_Knife_Reverse_Socket") : FName("hand_Knife_Socket");
	FName KnifeSocket = Take ? SocketName : FName("foot_Knife_Socket");
	Knife->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, KnifeSocket); 
}

void UPandolfoComponent::CheckCanAirAssassin()
{
	if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;
	if (!CharacterOwner || !CharacterOwner->GetMesh()) return;
	//UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	const FVector V = UKismetMathLibrary::GetForwardVector(CharacterOwner->GetControlRotation()) ;
	const FVector LineForwardStart = CharacterOwner->GetActorLocation() ;
	const FVector LineForwardEnd = LineForwardStart + FVector(V.X, V.Y, 0.f) * 250.f;
	FHitResult LineForwardHit;
	UKismetSystemLibrary::LineTraceSingle(this, LineForwardStart, LineForwardEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, LineForwardHit, true);
	if (LineForwardHit.bBlockingHit)
	{
		if (AIR_AssassinableOverlapped)
		{
			AIR_AssassinableOverlapped->GetAssassinableComponent()->MarkAsTarget(false);
			AIR_AssassinableOverlapped = nullptr;
		}
		return;
	}

	const FVector LineDownEnd = LineForwardEnd + FVector(0.f,0.f,-300.f) ;
	FHitResult LineDownHit;
	UKismetSystemLibrary::LineTraceSingle(this, LineForwardEnd, LineDownEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, LineDownHit, true);
	if (LineDownHit.bBlockingHit)
	{
		if (AIR_AssassinableOverlapped)
		{
			AIR_AssassinableOverlapped->GetAssassinableComponent()->MarkAsTarget(false);
			AIR_AssassinableOverlapped = nullptr;
		}
		return;
	}

	DetectAirAssassinableEnemy();

}

void UPandolfoComponent::DetectAirAssassinableEnemy()
{
	

	const FVector V = UKismetMathLibrary::GetForwardVector(CharacterOwner->GetControlRotation());
	const FVector ProjectileStart = CharacterOwner->GetActorLocation() + FVector(0.f,0.f,-CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()*3) + FVector(V.X, V.Y, 0.f) * 250.f;
	const FVector ProjectileVelocity = FVector(V.X, V.Y, 0.f) * 100.f; 

	FPredictProjectilePathParams PredictParams(100.f, ProjectileStart, ProjectileVelocity, 10, EObjectTypeQuery::ObjectTypeQuery3, CharacterOwner);
	PredictParams.OverrideGravityZ = -20.f;
	PredictParams.DrawDebugTime = 3.f;
	PredictParams.SimFrequency = 0.4;
	PredictParams.DrawDebugType = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	//PredictParams.DrawDebugType = EDrawDebugTrace::ForDuration ;

	FPredictProjectilePathResult PredictResult;

	UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);


	if (PredictResult.HitResult.bBlockingHit)
	{
		const FVector LineStart = CharacterOwner->GetActorLocation() ;
		//const FVector LineEnd = Hit.ImpactPoint;
		const FVector LineEnd = PredictResult.HitResult.ImpactPoint;
		FHitResult LineHit;
		EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
		UKismetSystemLibrary::LineTraceSingle(this, LineStart, LineEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, LineHit, true);
		if (LineHit.bBlockingHit) return;

		ABaseEnemy* TemporaryOverlapped = Cast<ABaseEnemy>(PredictResult.HitResult.GetActor());
		if (!TemporaryOverlapped || !TemporaryOverlapped->ActorHasTag("Assassinable") || !TemporaryOverlapped->IsCombatActorAlive() || TemporaryOverlapped->IsEnemyAware()) return;

		if (AIR_AssassinableOverlapped)
		{
			AIR_AssassinableOverlapped->GetAssassinableComponent()->MarkAsTarget(false);
		}
		
		AIR_AssassinableOverlapped = TemporaryOverlapped;
		AIR_AssassinableOverlapped->GetAssassinableComponent()->MarkAsTarget(true);
	}
	
	else if (AIR_AssassinableOverlapped)
	{
		AIR_AssassinableOverlapped->GetAssassinableComponent()->MarkAsTarget(false);
		AIR_AssassinableOverlapped = nullptr;
	}
}

#pragma endregion

#pragma region MontageSection

void UPandolfoComponent::OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;

	PanWolfCharacter->EndDodge();
	PandolfoState = EPandolfoState::EPS_Pandolfo;

}

void UPandolfoComponent::OnSlidingMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;

	if (PandolfoState == EPandolfoState::EPS_Sliding)
	{
		EndSliding();
	}

}

#pragma endregion


void UPandolfoComponent::EnterKiteMode(AKiteBoard* KiteBoard)
{
	ClimbingComponent->Deactivate();
	KiteComponent->SetKiteBoard(KiteBoard);
	KiteComponent->Activate();
	PandolfoState = EPandolfoState::EPS_Kiting;
}

void UPandolfoComponent::ExitKiteMode()
{
	KiteComponent->Deactivate();
	PandolfoState = EPandolfoState::EPS_Pandolfo;
}

void UPandolfoComponent::ClearAllTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(AirAssassination_TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(Glide_TimerHandle);
}

