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

	PanWolfCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	PanWolfCharacter->GetCharacterMovement()->MaxFlySpeed = 0.f;

	ClimbingComponent->SetAnimationBindings();

	//KiteComponent->Activate();
}

void UPandolfoComponent::Deactivate()
{
	Super::Deactivate();

	PanWolfCharacter->RemoveMappingContext(PandolfoMappingContext);
	ClimbingComponent->Deactivate();
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

	ClimbingComponent->Activate();

}

void UPandolfoComponent::Crouch()
{
	if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;

	const bool IsCrouched = CharacterOwner->bIsCrouched;
	CharacterOwner->GetCharacterMovement()->bWantsToCrouch = !IsCrouched;

	if (!IsCrouched)
	{
		CrouchingTimeline.PlayFromStart();
	}
		
	else 
		CrouchingTimeline.Reverse();
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

	FPredictProjectilePathParams PredictParams(75.f, ProjectileStart, ProjectileVelocity, 7, EObjectTypeQuery::ObjectTypeQuery1, OnGroundActor);
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
	if (bIsGliding) return;

	const FVector Start = CharacterOwner->GetActorLocation() ;
	const FVector End = Start - CharacterOwner->GetActorUpVector() * 800.f;
	EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	FHitResult Hit;
	UKismetSystemLibrary::LineTraceSingle(this, Start, End, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, Hit, true);

	
	if (!Hit.bBlockingHit && CharacterOwner->GetCharacterMovement()->GetLastUpdateVelocity().Z < -500.f)
	{
		Debug::Print(TEXT("Glide"));
		bIsGliding = true;

		CharacterOwner->GetCharacterMovement()->StopMovementImmediately();
		CharacterOwner->GetCharacterMovement()->GravityScale = 0.4f;
		CharacterOwner->GetCharacterMovement()->AirControl = 0.7f;

		UmbrellaActor = GetWorld()->SpawnActor<AActor>(UmbrellaActorClass, CharacterOwner->GetActorLocation(), CharacterOwner->GetActorRotation());
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
		UmbrellaActor->AttachToComponent(CharacterOwner->GetMesh(), AttachmentRules, FName("hand_l_Umbrella"));
	}
}

void UPandolfoComponent::UnGlide()
{
	if(bIsGliding)
	{
		Debug::Print(TEXT("UnGlide"));
		bIsGliding = false;

		UmbrellaActor->Destroy();
		CharacterOwner->GetCharacterMovement()->GravityScale = 1.75f;
		CharacterOwner->GetCharacterMovement()->AirControl = 0.35f;
	}

}

#pragma endregion


void UPandolfoComponent::EnterKiteMode(AKiteBoard* KiteBoard)
{
	ClimbingComponent->Deactivate();
	KiteComponent->SetKiteBoard(KiteBoard);
	KiteComponent->Activate();

}