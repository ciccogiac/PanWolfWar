#include "Components/PandolfoComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Components/ClimbingComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GameFramework/Character.h"

#include "Kismet/KismetSystemLibrary.h"

#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

UPandolfoComponent::UPandolfoComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);

	ClimbingComponent = CreateDefaultSubobject<UClimbingComponent>(TEXT("ClimbingComponent"));
}

void UPandolfoComponent::Activate(bool bReset)
{
	Super::Activate();

	PanWolfCharacter->AddMappingContext(PandolfoMappingContext, 1);


	Capsule->SetCapsuleRadius(35.f);
	Capsule->SetCapsuleHalfHeight(90.f);
	CameraBoom->TargetArmLength = 400.f;

	PanWolfCharacter->bUseControllerRotationPitch = false;
	PanWolfCharacter->bUseControllerRotationYaw = false;

	PanWolfCharacter->SetTransformationCharacter(SkeletalMeshAsset, Anim);

	PanWolfCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	PanWolfCharacter->GetCharacterMovement()->MaxFlySpeed = 0.f;

	ClimbingComponent->SetAnimationBindings();
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
}

void UPandolfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

const bool UPandolfoComponent::IsClimbing()
{
	return ClimbingComponent->IsClimbing();
}

void UPandolfoComponent::Jump()
{
	if (!ClimbingComponent->TryClimbing()  && !ClimbingComponent->TryMantle() && !ClimbingComponent->TryVault())
	{		
		CharacterOwner->Jump();
	}

	ClimbingComponent->Activate();

}

void UPandolfoComponent::Sliding()
{

	if (!SlidingMontage) return;
	UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	
	if (CharacterOwner->GetCharacterMovement()->GetLastInputVector().Length() < 0.5f) return;

	EDrawDebugTrace::Type DebugTraceType =  EDrawDebugTrace::None ;
	FVector LineStart; FVector LineEnd; FHitResult OutHitLine;
	FVector CapsuleStart; FHitResult OutHitCapsule;
	FVector Start_ForwardSpace; FVector End_ForwardSpace; FHitResult OutHit_ForwardSpace;

	LineStart = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 550.f;
	LineEnd = LineStart + FVector(0.f,0.f,-350.f);
	UKismetSystemLibrary::LineTraceSingle (this, LineStart, LineEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTraceType, OutHitLine, true);

	if (!OutHitLine.bBlockingHit) return;

	CapsuleStart = OutHitLine.Location + FVector(0.f,0.f,95.f);	
	UKismetSystemLibrary::CapsuleTraceSingle(this, CapsuleStart, CapsuleStart,35.f,90.f, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTraceType, OutHitCapsule, true);

	Start_ForwardSpace = CharacterOwner->GetMesh()->GetComponentLocation() +FVector(0.f, 0.f, 60.f) + CharacterOwner->GetActorForwardVector() * 35.f;
	End_ForwardSpace = OutHitLine.Location + FVector(0.f, 0.f, 60.f);	
	UKismetSystemLibrary::SphereTraceSingle(this, Start_ForwardSpace, End_ForwardSpace,25.f, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTraceType, OutHit_ForwardSpace, true);

	if (OutHitCapsule.bBlockingHit || OutHit_ForwardSpace.bBlockingHit)
	{
		for (size_t i = 0; i < 5; i++)
		{
			if (i == 2) continue; 

			LineStart = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 550.f + CharacterOwner->GetActorRightVector() * i * 35.f - CharacterOwner->GetActorRightVector() * 105.f;
			LineEnd = LineStart + FVector(0.f, 0.f, -350.f);
			UKismetSystemLibrary::LineTraceSingle(this, LineStart, LineEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTraceType, OutHitLine, true);

			if (!OutHitLine.bBlockingHit) continue;

			CapsuleStart = OutHitLine.Location + FVector(0.f, 0.f, 95.f) ;
			UKismetSystemLibrary::CapsuleTraceSingle(this, CapsuleStart, CapsuleStart, 35.f, 90.f, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTraceType, OutHitCapsule, true);

			Start_ForwardSpace = CharacterOwner->GetMesh()->GetComponentLocation() + FVector(0.f, 0.f, 60.f) + CharacterOwner->GetActorForwardVector() * 35.f;
			End_ForwardSpace = OutHitLine.Location + FVector(0.f, 0.f, 60.f);
			UKismetSystemLibrary::SphereTraceSingle(this, Start_ForwardSpace, End_ForwardSpace, 25.f, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTraceType, OutHit_ForwardSpace, true);

			if (!OutHitCapsule.bBlockingHit && !OutHit_ForwardSpace.bBlockingHit)
				break;
		}
	}


	if (!OutHitCapsule.bBlockingHit && !OutHit_ForwardSpace.bBlockingHit)
	{
		CharacterOwner->DisableInput(CharacterOwner->GetLocalViewingPlayerController());
		PanWolfCharacter->SetMotionWarpTarget(FName("SlidingPoint"), OutHitLine.Location);
		OwningPlayerAnimInstance->Montage_Play(SlidingMontage);
	}



}

void UPandolfoComponent::StartSliding()
{
	TimeElapsed = 0.f;
	GetWorld()->GetTimerManager().SetTimer(Sliding_TimerHandle, [this]() {this->SetSlidingValues(false); }, 0.01f, true);
}

void UPandolfoComponent::EndSliding()
{
	CharacterOwner->EnableInput(CharacterOwner->GetLocalViewingPlayerController());
	TimeElapsed = 0.35f;
	GetWorld()->GetTimerManager().SetTimer(Sliding_TimerHandle, [this]() {this->SetSlidingValues(true); }, 0.001f, true);
}

void UPandolfoComponent::SetSlidingValues(bool IsReverse)
{
	TimeElapsed = TimeElapsed + GetWorld()->GetTimerManager().GetTimerElapsed(Sliding_TimerHandle) * (IsReverse ? -1 : 1 );
	TimeElapsed = UKismetMathLibrary::FClamp(TimeElapsed, 0.00f, 0.35f);

	const float NewCapsuleSize = CapsuleSize_Curve->GetFloatValue(TimeElapsed);
	const float NewMeshPosition = MeshPosition_Curve->GetFloatValue(TimeElapsed);
	const float NewCameraHeight = CameraHeight_Curve->GetFloatValue(TimeElapsed);

	Capsule->SetCapsuleHalfHeight(NewCapsuleSize, true);
	CharacterOwner->GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, NewMeshPosition));
	CameraBoom->SetRelativeLocation(FVector(CameraBoom->GetRelativeLocation().X, CameraBoom->GetRelativeLocation().Y, NewCameraHeight));

	if((!IsReverse && TimeElapsed >= 0.35f) || (IsReverse && TimeElapsed <= 0.0f))
		GetWorld()->GetTimerManager().ClearTimer(Sliding_TimerHandle);
}