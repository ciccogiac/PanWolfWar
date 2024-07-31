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

#include "Enemy/AssassinableEnemy.h"
#include "Camera/CameraComponent.h"

UPandolfoComponent::UPandolfoComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);

	ClimbingComponent = CreateDefaultSubobject<UClimbingComponent>(TEXT("ClimbingComponent"));
	SneakCoverComponent = CreateDefaultSubobject<USneakCoverComponent>(TEXT("SneakCoverComponent"));
	KiteComponent = CreateDefaultSubobject<UKiteComponent>(TEXT("KiteComponent"));
	
	Knife = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Knife"));
	Knife->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void UPandolfoComponent::Activate(bool bReset)
{
	Super::Activate();

	PanWolfCharacter->AddMappingContext(PandolfoMappingContext, 1);

	PandolfoState = EPandolfoState::EPS_Pandolfo;

	Capsule->SetCapsuleRadius(35.f);
	Capsule->SetCapsuleHalfHeight(90.f);
	CameraBoom->TargetArmLength = 400.f;

	PanWolfCharacter->bUseControllerRotationPitch = false;
	PanWolfCharacter->bUseControllerRotationYaw = false;

	PanWolfCharacter->SetTransformationCharacter(SkeletalMeshAsset, Anim);

	if (PanWolfCharacter->IsHiding())
		CharacterOwner->GetMesh()->SetScalarParameterValueOnMaterials(FName("Emissive Multiplier"), 10.f);


	if(CharacterOwner->GetCharacterMovement()->IsFalling())
		ClimbingComponent->Activate();

	//PanWolfCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	PanWolfCharacter->GetCharacterMovement()->MaxFlySpeed = 0.f;

	ClimbingComponent->SetAnimationBindings();

	Knife->AttachToComponent(CharacterOwner->GetMesh(),FAttachmentTransformRules::SnapToTargetNotIncludingScale,FName("foot_Knife_Socket"));
	Knife->SetVisibility(true);

	//KiteComponent->Activate();

	GetWorld()->GetTimerManager().SetTimer(AirAssassination_TimerHandle, [this]() {this->CheckCanAirAssassin(); }, 0.25f, true);

	if (!CharacterOwner->GetMovementComponent()->IsMovingOnGround())
	{
		bIsGlideTimerActive = true;
		GetWorld()->GetTimerManager().SetTimer(Glide_TimerHandle, [this]() {this->TryGliding(); }, 0.25f, true);
	}
		
}

void UPandolfoComponent::Deactivate()
{
	Super::Deactivate();

	PanWolfCharacter->RemoveMappingContext(PandolfoMappingContext);
	ClimbingComponent->Deactivate();


	Knife->SetVisibility(false);

	GetWorld()->GetTimerManager().ClearTimer(AirAssassination_TimerHandle);

	PandolfoState = EPandolfoState::EPS_Pandolfo;
}

void UPandolfoComponent::BeginPlay()
{
	Super::BeginPlay();

	Capsule = PanWolfCharacter->GetCapsuleComponent();
	CameraBoom = PanWolfCharacter->GetCameraBoom();

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

const bool UPandolfoComponent::IsClimbing()
{
	return ClimbingComponent->IsClimbing();
}

void UPandolfoComponent::Jump()
{
	if (!TryClimbOrMantle() && !ClimbingComponent->TryVault())
	{		
		if (!PredictJump())
		{
			CharacterOwner->Jump();
		}
	}

}

void UPandolfoComponent::HandleFalling()
{
	//Debug::Print(TEXT("FAlling"));
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

void UPandolfoComponent::Crouch()
{
	if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;

	const bool IsCrouched = CharacterOwner->bIsCrouched;
	CharacterOwner->GetCharacterMovement()->bWantsToCrouch = !IsCrouched;

	if (!IsCrouched)
	{
		CrouchingTimeline.PlayFromStart();

		CheckCanHide();
		
	}
		
	else
	{		
		CrouchingTimeline.Reverse();

		//if (PanWolfCharacter->IsHiding())
		//	PanWolfCharacter->SetIsHiding(false);

		if (PanWolfCharacter->IsHiding())
			CheckCanHideStandUP();
	}

}

void UPandolfoComponent::CheckCanHide()
{
	if (PanWolfCharacter->IsHiding()) return;

	const FVector Start = CharacterOwner->GetActorLocation();
	const FVector End = Start + CharacterOwner->GetActorForwardVector();
	FHitResult Hit;
	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, 60.f, HidingObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true);
	if(Hit.bBlockingHit)
		PanWolfCharacter->SetIsHiding(true,false);
}

void UPandolfoComponent::CheckCanHideStandUP()
{
	const FVector Start = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorUpVector() * CharacterOwner->BaseEyeHeight * 3.f;
	const FVector End = Start + CharacterOwner->GetActorForwardVector();
	FHitResult Hit;
	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, 20.f, HidingObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, Hit, true);
	if (!Hit.bBlockingHit)
		PanWolfCharacter->SetIsHiding(false);
}

void UPandolfoComponent::CrouchCameraUpdate(float Alpha)
{
	CameraBoom->TargetArmLength = UKismetMathLibrary::Lerp(400.f, 550.f, Alpha);
}

bool UPandolfoComponent::TryClimbOrMantle()
{
	if (!ClimbingComponent->TryClimbing() && !ClimbingComponent->TryMantle())
		return false;
	return true;
}



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

void UPandolfoComponent::LoadPredictJump(const FVector ActorLocation, UAnimInstance* OwningPlayerAnimInstance)
{
	const float NewYaw = UKismetMathLibrary::FindLookAtRotation(ActorLocation, LandPredictLocation).Yaw;
	const FRotator NewRotation = FRotator(CharacterOwner->GetActorRotation().Pitch, NewYaw, CharacterOwner->GetActorRotation().Roll);
	CharacterOwner->SetActorRotation(NewRotation);
	CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying, 0);

	OwningPlayerAnimInstance->Montage_Play(PredictJumpMontage);

	OwningPlayerAnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UPandolfoComponent::StartPredictJump);
	OwningPlayerAnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UPandolfoComponent::StopPredictJump);
	OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UPandolfoComponent::EndPredictJump);
}

bool UPandolfoComponent::PredictJump()
{
	UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
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
	LoadPredictJump(ActorLocation, OwningPlayerAnimInstance);

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
	CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling, 0);
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

	if (!SlidingMontage) return;
	UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;
	if (CharacterOwner->GetCharacterMovement()->GetLastInputVector().Length() < 0.5f) return;

	CharacterOwner->DisableInput(CharacterOwner->GetLocalViewingPlayerController());
	OwningPlayerAnimInstance->Montage_Play(SlidingMontage);
}

void UPandolfoComponent::StartSliding()
{
	if (CharacterOwner->GetCharacterMovement()->IsCrouching())
	{
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(35.f,40.f);
		CrouchingTimeline.Reverse();
	}
		

	CharacterOwner->GetCharacterMovement()->CrouchedHalfHeight = 40.f;
	CharacterOwner->GetCharacterMovement()->bWantsToCrouch = true;
	

	TimeElapsed = 0.f;
	GetWorld()->GetTimerManager().SetTimer(Sliding_TimerHandle, [this]() {this->SetSlidingValues(false); }, 0.01f, true);
}

void UPandolfoComponent::EndSliding()
{
	CharacterOwner->GetCharacterMovement()->CrouchedHalfHeight = 55.f;
	CharacterOwner->GetCharacterMovement()->bWantsToCrouch = false;

	CharacterOwner->EnableInput(CharacterOwner->GetLocalViewingPlayerController());
	TimeElapsed = 0.35f;
	GetWorld()->GetTimerManager().SetTimer(Sliding_TimerHandle, [this]() {this->SetSlidingValues(true); }, 0.001f, true);
}

void UPandolfoComponent::SetSlidingValues(bool IsReverse)
{
	TimeElapsed = TimeElapsed + GetWorld()->GetTimerManager().GetTimerElapsed(Sliding_TimerHandle) * (IsReverse ? -1 : 1);
	TimeElapsed = UKismetMathLibrary::FClamp(TimeElapsed, 0.00f, 0.35f);

	const float NewCameraHeight = CameraHeight_Curve->GetFloatValue(TimeElapsed);

	CameraBoom->SetRelativeLocation(FVector(CameraBoom->GetRelativeLocation().X, CameraBoom->GetRelativeLocation().Y, NewCameraHeight));

	if ((!IsReverse && TimeElapsed >= 0.35f) || (IsReverse && TimeElapsed <= 0.0f))
		GetWorld()->GetTimerManager().ClearTimer(Sliding_TimerHandle);
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
	UKismetSystemLibrary::LineTraceSingle(this, Start, End, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, Hit, true);

	
	if (!Hit.bBlockingHit && CharacterOwner->GetCharacterMovement()->GetLastUpdateVelocity().Z < -GlidingVelocity)
	{
		//Debug::Print(TEXT("Glide"));
		PandolfoState = EPandolfoState::EPS_Gliding;
		GetWorld()->GetTimerManager().ClearTimer(Glide_TimerHandle);

		CharacterOwner->GetCharacterMovement()->StopMovementImmediately();
		CharacterOwner->GetCharacterMovement()->GravityScale = GlidingGravityScale;
		CharacterOwner->GetCharacterMovement()->AirControl = GlidingAirControl;

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
		CharacterOwner->GetCharacterMovement()->GravityScale = 1.75f;
		CharacterOwner->GetCharacterMovement()->AirControl = 0.35f;
	}

}

#pragma endregion

#pragma region Assassination

void UPandolfoComponent::Assassination()
{
	if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;
	if (!AssassinableOverlapped && !AIR_AssassinableOverlapped) return;
	if (AIR_AssassinableOverlapped) AssassinableOverlapped = AIR_AssassinableOverlapped;

	Debug::Print(TEXT("Assassination"));

	UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	if (!AIR_AssassinableOverlapped && AssassinableOverlapped)
	{
		
		PlayStealthAssassination(OwningPlayerAnimInstance);
	}
	else
	{
		PlayAirAssassination(OwningPlayerAnimInstance);
		
	}

	
	
}

void UPandolfoComponent::PlayAirAssassination(UAnimInstance* OwningPlayerAnimInstance)
{
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

	AssassinableOverlapped->DisableBoxAssassination();
	OwningPlayerAnimInstance->Montage_Play(AirAssassinMontage);
}

void UPandolfoComponent::PlayStealthAssassination(UAnimInstance* OwningPlayerAnimInstance)
{
	int32 MapIndex = FMath::RandRange(0, AssassinationMontage_Map.Num() - 1);
	TPair<UAnimMontage*, UAnimMontage*> MontageCouple = AssassinationMontage_Map.Get(FSetElementId::FromInteger(MapIndex));
	if (!MontageCouple.Key) return;
	if (!MontageCouple.Value) return;
	const FTransform AssassinTransform = AssassinableOverlapped->GetAssassinationTransform();
	const FVector WarpLocation = AssassinTransform.GetLocation();
	const FRotator WarpRotator = AssassinTransform.Rotator() + FRotator(0.f, 90.f, 0.f);
	PanWolfCharacter->SetMotionWarpTarget(FName("AssasinationWarp"), WarpLocation, WarpRotator);
	OwningPlayerAnimInstance->Montage_Play(MontageCouple.Key);
	AssassinableOverlapped->Assassinated(MontageCouple.Value, this);
}

void UPandolfoComponent::AirKill()
{
	if(AssassinableOverlapped)
		AssassinableOverlapped->Assassinated(AirAssassinDeathMontage,this, true);
}

void UPandolfoComponent::RiattachCamera()
{
	GetWorld()->GetTimerManager().ClearTimer(AirAssassinationCamera_TimerHandle);

	PanWolfCharacter->GetFollowCamera()->AttachToComponent(CameraBoom, FAttachmentTransformRules::KeepWorldTransform);
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(PanWolfCharacter->GetFollowCamera(),FVector::ZeroVector, FRotator::ZeroRotator, true, true, 1.5f, true, EMoveComponentAction::Move, LatentInfo);

}

void UPandolfoComponent::TakeKnife(bool Take)
{
	FName KnifeSocket = Take ?  FName("hand_Knife_Reverse_Socket") : FName("foot_Knife_Socket");
	Knife->AttachToComponent(CharacterOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, KnifeSocket);
}

void UPandolfoComponent::CheckCanAirAssassin()
{
	if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;

	UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
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
			AIR_AssassinableOverlapped->MarkAsTarget(false);
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
			AIR_AssassinableOverlapped->MarkAsTarget(false);
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

		AAssassinableEnemy* TemporaryOverlapped = Cast<AAssassinableEnemy>(PredictResult.HitResult.GetActor());
		if (!TemporaryOverlapped || TemporaryOverlapped->IsDead() || TemporaryOverlapped->IsAware()) return;

		if (AIR_AssassinableOverlapped)
		{
			AIR_AssassinableOverlapped->MarkAsTarget(false);
		}
		
		AIR_AssassinableOverlapped = TemporaryOverlapped;
		AIR_AssassinableOverlapped->MarkAsTarget(true);
	}
	
	else if (AIR_AssassinableOverlapped)
	{
		AIR_AssassinableOverlapped->MarkAsTarget(false);
		AIR_AssassinableOverlapped = nullptr;
	}
}

#pragma endregion

void UPandolfoComponent::EnterKiteMode(AKiteBoard* KiteBoard)
{
	ClimbingComponent->Deactivate();
	KiteComponent->SetKiteBoard(KiteBoard);
	KiteComponent->Activate();

}