// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PandolFlowerComponent.h"

#include "PanWolfWar/DebugHelper.h"
#include "GameFramework/Character.h"
#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "NiagaraComponent.h"
#include "CharacterActor/FlowerCable.h"

#include "Actors/GrapplePoint.h"

#include "Kismet/GameplayStatics.h"

#pragma region EngineFunctions

UPandolFlowerComponent::UPandolFlowerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
}

void UPandolFlowerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (PanWolfCharacter)
	{
		FollowCamera = PanWolfCharacter->GetFollowCamera();
	}

}

void UPandolFlowerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckForGrapplePoint();

	if (bInGrapplingAnimation)
	{
		MoveRope();
		if (bMovingWithGrapple) GrapplingMovement();
	}

}

void UPandolFlowerComponent::Move(const FInputActionValue& Value)
{
	if (bInGrapplingAnimation) return;

	PanWolfCharacter->Move(Value);
}

void UPandolFlowerComponent::Jump()
{
	if (bInGrapplingAnimation) return;

	CharacterOwner->Jump();
}

#pragma endregion

#pragma region ActivationComponent

void UPandolFlowerComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	//Debug::Print(TEXT("OVA"));
	PanWolfCharacter->AddMappingContext(PandolFlowerMappingContext, 1);

	PanWolfCharacter->bUseControllerRotationPitch = false;
	PanWolfCharacter->bUseControllerRotationYaw = false;

	PanWolfCharacter->SetTransformationCharacter(SkeletalMeshAsset, Anim);
	PanWolfCharacter->GetNiagaraTransformation()->SetAsset(Pandolflower_Niagara);

	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (OwningPlayerAnimInstance)
	{
		OwningPlayerAnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UPandolFlowerComponent::OnFlowerNotifyStarted);
		OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UPandolFlowerComponent::OnFlowerMontageEnded);
	}

	//EndCable = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EndCable"));

	EndCable = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("EndCable"));
	EndCable->AttachToComponent(CharacterOwner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	EndCable->RegisterComponent();
	EndCable->SetStaticMesh(FlowerCable_EndMesh);
	EndCable->AddWorldRotation(FQuat(0.f,-90.f,0.f,0.f));
	EndCable->SetWorldScale3D(FVector(1.5f, 1.5f, 1.5f));

	FlowerCable = GetWorld()->SpawnActor<AFlowerCable>(BP_FlowerCable, CharacterOwner->GetActorLocation(), CharacterOwner->GetActorRotation());
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	FlowerCable->AttachToComponent(CharacterOwner->GetMesh(), AttachmentRules, FName("hand_l"));
	FlowerCable->SetAttachEndCable(EndCable);

}

void UPandolFlowerComponent::Deactivate()
{
	Super::Deactivate();

	PanWolfCharacter->RemoveMappingContext(PandolFlowerMappingContext);

	PanWolfCharacter->GetNiagaraTransformation()->SetAsset(nullptr);

	if (FlowerCable)
		FlowerCable->Destroy();
}

#pragma endregion

#pragma region Hooking

void UPandolFlowerComponent::Hook()
{
	if (!GrapplePointRef) return;

	const float DistanceFromGrapplePoint = (CharacterOwner->GetActorLocation() - GrapplePointRef->GetActorLocation()).Length();

	if (DistanceFromGrapplePoint <= GrappleThrowDistance && CurrentGrapplePoint != GrapplePointRef)
	{
		if (bMovingWithGrapple)
		{
			const FVector LaunchVelocity = UKismetMathLibrary::GetDirectionUnitVector(CharacterOwner->GetActorLocation(), GrapplingDestination) * 200.f;
			CharacterOwner->LaunchCharacter(LaunchVelocity, false, false);
		}

		bInGrapplingAnimation = true;
		bMovingWithGrapple = false;
		CurrentGrapplePoint = GrapplePointRef;
		GrapplingDestination = CurrentGrapplePoint->GetLandingZone() + FVector(0.f, 0.f, 110.f);

		const FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(CharacterOwner->GetActorLocation(), GrapplingDestination);
		CharacterOwner->SetActorRotation(FRotator(0.f, NewRotation.Yaw,0.f));

		RopeBaseLenght = (CharacterOwner->GetActorLocation() - GrapplingDestination).Length();

		CurrentGrapplePoint->Use();

		UAnimMontage* GrappleMontage = CharacterOwner->GetCharacterMovement()->IsFalling() ? GrappleAir_Montage : GrappleGround_Montage;
		PlayMontage(GrappleMontage);
	}
}

void UPandolFlowerComponent::CheckForGrapplePoint()
{
	const FVector Start = CharacterOwner->GetActorLocation();
	const FVector End = Start;

	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	TArray<FHitResult> outHits;
	bool bBlockingHits = UKismetSystemLibrary::SphereTraceMultiForObjects(this,Start,End, DetectionRadius, GrapplingObjectTypes,false,TArray<AActor*>(), DebugTraceType, outHits,true);

	if (bBlockingHits)
	{
		float HighestDotProduct = 0.7f;
		AActor* DetectedActor = nullptr;

		for (const FHitResult& hit : outHits)
		{
			const FVector CameraForward = FollowCamera->GetForwardVector();
			const FVector CameraLocation = FollowCamera->GetComponentLocation();
			const FVector HitActorLocation = hit.GetActor()->GetActorLocation();

			const FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(CameraLocation, HitActorLocation);
			const double Dot = FVector::DotProduct(CameraForward, Direction);

			if (Dot > HighestDotProduct)
			{
				DetectedActor = hit.GetActor();
				HighestDotProduct = Dot;
			}

			else
			{
				//Deactivate Grapple Point Ref
				DeactivateGrapplePoint();
			}

		}

		//Activate Grapple Point Ref
		ActivateGrapplePoint(DetectedActor);
	}
	else
	{
		//Deactivate Grapple Point Ref
		DeactivateGrapplePoint();
	}
}

void UPandolFlowerComponent::MoveRope()
{
	UAnimMontage* CurrentMontage = OwningPlayerAnimInstance->GetCurrentActiveMontage();
	float MontagePosition = OwningPlayerAnimInstance->Montage_GetPosition(CurrentMontage);

	UCurveFloat* RopeLength_Curve = CurrentMontage == GrappleGround_Montage ? GroundRopeLength_Curve : AirRopeLength_Curve;
	const float CableLength = RopeLength_Curve->GetFloatValue(MontagePosition) * RopeBaseLenght;
	FlowerCable->SetCableLength(CableLength);
	//Set Cable Length

	UCurveFloat* RopePosition_Curve = CurrentMontage == GrappleGround_Montage ? GroundRopePosition_Curve : AirRopePosition_Curve;
	const float AlphaLerp = RopePosition_Curve->GetFloatValue(MontagePosition);
	const FVector HandLocation = CharacterOwner->GetMesh()->GetSocketLocation(FName("hand_r"));
	const FVector GrappleLocation = CurrentGrapplePoint->GetActorLocation();
	const FVector NewLocation =  UKismetMathLibrary::VLerp(HandLocation, GrappleLocation, AlphaLerp);
	//Set Kunai Location
	EndCable->SetWorldLocation(NewLocation);
}

void UPandolFlowerComponent::GrapplingMovement()
{
	UAnimMontage* CurrentMontage = OwningPlayerAnimInstance->GetCurrentActiveMontage();
	float MontagePosition = OwningPlayerAnimInstance->Montage_GetPosition(CurrentMontage);

	UCurveFloat* Speed_Curve = CurrentMontage == GrappleGround_Montage ? GroundSpeed_Curve : AirSpeed_Curve;
	const float AlphaLerp = Speed_Curve->GetFloatValue(MontagePosition);
	const FVector LerpedVector = UKismetMathLibrary::VLerp(StartingPosition, GrapplingDestination, AlphaLerp);

	UCurveFloat* HeightOffset_Curve = CurrentMontage == GrappleGround_Montage ? GroundHeightOffset_Curve : AirHeightOffset_Curve;
	const float HeightOffset = HeightOffset_Curve->GetFloatValue(MontagePosition);
	const FVector NewLocation = LerpedVector + FVector(0.f,0.f, HeightOffset);
	CharacterOwner->SetActorLocation(NewLocation);
}

void UPandolFlowerComponent::ActivateGrapplePoint(AActor* DetectedActor)
{
	if (!DetectedActor) return;

	const FVector Start = FollowCamera->GetComponentLocation();
	const FVector End = DetectedActor->GetActorLocation();

	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	FHitResult hit;
	UKismetSystemLibrary::LineTraceSingle(this, Start, End, HookingTraceType, false, TArray<AActor*>(), DebugTraceType, hit,true);

	if (hit.GetActor() != DetectedActor) { DeactivateGrapplePoint(); return; }
	if (DetectedActor == GrapplePointRef) return;

	AGrapplePoint* GrapplePoint = Cast<AGrapplePoint>(DetectedActor);
	if (GrapplePoint)
	{
		DeactivateGrapplePoint();
		GrapplePointRef = GrapplePoint;
		GrapplePointRef->Activate(this);
	}

}

void UPandolFlowerComponent::DeactivateGrapplePoint()
{
	if (GrapplePointRef)
	{
		GrapplePointRef->Deactivate();
		GrapplePointRef = nullptr;
	}
}

void UPandolFlowerComponent::RopeVisibility(bool NewVisibility)
{
	//SetCableVisibility
	FlowerCable->SetCableVisibility(NewVisibility);
	//SetKunaiVisibility
	EndCable->SetVisibility(NewVisibility);
}

void UPandolFlowerComponent::ResetMovement()
{
	bMovingWithGrapple = false;
	CurrentGrapplePoint = nullptr;
	bInGrapplingAnimation = false;
	CharacterOwner->GetCharacterMovement()->GravityScale = 2.2f;
}

void UPandolFlowerComponent::ThrowRope()
{
	UGameplayStatics::PlaySound2D(this, ThrowRope_Sound);
}

void UPandolFlowerComponent::StartGrapplingMovement()
{
	UGameplayStatics::PlaySound2D(this, GrappleJump_Sound);
	CharacterOwner->GetCharacterMovement()->GravityScale = 0.0f;
	CharacterOwner->GetCharacterMovement()->StopMovementImmediately();
	StartingPosition = CharacterOwner->GetActorLocation();
	CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	bMovingWithGrapple = true;
}

#pragma endregion

#pragma region MontageSection

void UPandolFlowerComponent::PlayMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) return;
	if (!OwningPlayerAnimInstance) return;
	//if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) { return;}

	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
}

void UPandolFlowerComponent::OnFlowerNotifyStarted(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == FName("StartHooking"))
	{
		//FlowerCable->HookCable(Hook_TargetLocation, Hook_TargetRotation, CharacterOwner->GetActorLocation());

	}

}

void UPandolFlowerComponent::OnFlowerMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	/*if (Montage == ThrowFlowerCable_Montage)
	{
		StartHooking();
	}

	else if (Montage == HookJump_Montage)
	{
		EndHooking();
	}*/
}

#pragma endregion






