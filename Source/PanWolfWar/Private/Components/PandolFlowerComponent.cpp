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

#include "InputActionValue.h"

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

	//if (CurrentGrapplePoint && CurrentGrapplePoint->GrapplePointType == EGrapplePointType::EGPT_Swing)
	//{	
	//	//Debug::Print(TEXT("Velocity: ") + FString::SanitizeFloat(CurrentGrapplePoint->LandingZone_Mesh->ComponentVelocity.Length()));
	//	Debug::Print(TEXT("Velocity: ") + FString::SanitizeFloat(PanWolfCharacter->GetVelocity().Length()));
	//}
}

void UPandolFlowerComponent::Move(const FInputActionValue& Value)
{
	if (bInGrapplingAnimation) return;

	if (!bSwinging) { PanWolfCharacter->Move(Value); }
	else
	{
		FVector2D MovementVector = Value.Get<FVector2D>();

		//const float ForceX = UKismetMathLibrary::MapRangeClamped(MovementVector.X, -1, 1, -40000.f, 40000.f);
		const float ForceY = UKismetMathLibrary::MapRangeClamped(MovementVector.Y, -1, 1, -100000.f, 100000.f);

		CurrentGrapplePoint->LandingZone_Mesh->AddForce(CurrentGrapplePoint->LandingZone_Mesh->GetForwardVector() * ForceY);
		//CurrentGrapplePoint->LandingZone_Mesh->AddForce(CurrentGrapplePoint->LandingZone_Mesh->GetRightVector() * ForceX);
		
		//Debug::Print(TEXT("V: ") + FString::SanitizeFloat(CurrentGrapplePoint->LandingZone_Mesh->GetComponentRotation().Pitch));
		//const float X = CurrentGrapplePoint->LandingZone_Mesh->GetComponentRotation().Pitch > -25.f ? MovementVector.X : -MovementVector.X;
		const float X = MovementVector.Y >= 0.f ? MovementVector.X : -MovementVector.X;
		CurrentGrapplePoint->LandingZone_Mesh->AddWorldRotation(FRotator(0.f,X, 0.f));
	}
	//	if (CharacterOwner->Controller == nullptr) return;
	//	
	//	// find out which way is forward
	//	const FRotator Rotation = CharacterOwner->Controller->GetControlRotation();
	//	const FRotator YawRotation(0, Rotation.Yaw, 0);

	//	// get forward vector
	//	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X) ;

	//	// get right vector 
	//	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y) ;

	//	const float ForceX = UKismetMathLibrary::MapRangeClamped(MovementVector.X, -1, 1, -40000.f, 40000.f);
	//	const float ForceY = UKismetMathLibrary::MapRangeClamped(MovementVector.Y, -1, 1, -40000.f, 40000.f);
	//	//CurrentGrapplePoint->LandingZone_Mesh->AddImpulse(FVector(ForceX, ForceY,0.f));
	//	CurrentGrapplePoint->LandingZone_Mesh->AddForce(-CurrentGrapplePoint->LandingZone_Mesh->GetForwardVector() * ForceY);
	//	CurrentGrapplePoint->LandingZone_Mesh->AddForce(CurrentGrapplePoint->LandingZone_Mesh->GetRightVector() * ForceX);
	//	//CurrentGrapplePoint->LandingZone_Mesh->AddForce(CurrentGrapplePoint->LandingZone_Mesh->GetRightVector() * RightDirection * ForceX);

	//}
	
}

void UPandolFlowerComponent::Jump()
{
	if (bInGrapplingAnimation) return;

	//CharacterOwner->LaunchCharacter(CharacterOwner->GetActorForwardVector() * 2000.f, true, true);

	if (bSwinging) 
	{
		const FVector v = CurrentGrapplePoint->LandingZone_Mesh->GetForwardVector() * PanWolfCharacter->GetVelocity().Length();
		PanWolfCharacter->DetachRootComponentFromParent();
		CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);


		Debug::Print(TEXT("V: ") + v.ToString());
		//CharacterOwner->LaunchCharacter(v * 50.f, true, true);

		CurrentGrapplePoint->ResetLandingZone();

		ResetMovement(); RopeVisibility(false);

		
		CharacterOwner->LaunchCharacter(v *2 , false, false);
		//CharacterOwner->LaunchCharacter(CharacterOwner->GetActorForwardVector() * 2000.f, true, true);
	}

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

	FlowerCable = GetWorld()->SpawnActor<AFlowerCable>(BP_FlowerCable, CharacterOwner->GetActorLocation(), CharacterOwner->GetActorRotation());
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	//FlowerCable->AttachToComponent(CharacterOwner->GetMesh(), AttachmentRules, FName("hand_l"));
	FlowerCable->SetCableAttachment(CharacterOwner->GetMesh(), FName("hand_l"));

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

#pragma region Grappling

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

		if (CurrentGrapplePoint && CurrentGrapplePoint->GrapplePointType == EGrapplePointType::EGPT_Swing)
		{
			PanWolfCharacter->DetachRootComponentFromParent();
			CurrentGrapplePoint->ResetLandingZone();
		}

		bInGrapplingAnimation = true;
		bMovingWithGrapple = false;
		CurrentGrapplePoint = GrapplePointRef;
		GrapplingDestination = CurrentGrapplePoint->GetLandingZone() + FVector(0.f, 0.f, 110.f);

		const FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(CharacterOwner->GetActorLocation(), GrapplingDestination);
		CharacterOwner->SetActorRotation(FRotator(0.f, NewRotation.Yaw,0.f));

		RopeBaseLenght = (CharacterOwner->GetActorLocation() - GrapplingDestination).Length();

		CurrentGrapplePoint->Use();

		UAnimMontage* GrappleMontage = nullptr;

		if (CurrentGrapplePoint->GrapplePointType == EGrapplePointType::EGPT_Grapple)
		{
			GrappleMontage = CharacterOwner->GetCharacterMovement()->IsFalling() ? GrappleAir_Montage : GrappleGround_Montage;
		}
		else if (CurrentGrapplePoint->GrapplePointType == EGrapplePointType::EGPT_Swing)
		{
			GrapplingDestination = CurrentGrapplePoint->GetLandingZone() + FVector(0.f, 0.f, -110.f);
			//GrappleMontage = CharacterOwner->GetCharacterMovement()->IsFalling() ? GrappleAir_Swing_Montage : GrappleGround_Swing_Montage;
			GrappleMontage = GrappleAir_Swing_Montage;
		}

		
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

	if ((CurrentMontage == GrappleGround_Swing_Montage || CurrentMontage == GrappleAir_Swing_Montage) && MontagePosition >= 0.5f) return;

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
	FlowerCable->SetEndCableLocation(NewLocation);
}

void UPandolFlowerComponent::GrapplingMovement()
{
	UAnimMontage* CurrentMontage = OwningPlayerAnimInstance->GetCurrentActiveMontage();
	float MontagePosition = OwningPlayerAnimInstance->Montage_GetPosition(CurrentMontage);

	if ((CurrentMontage == GrappleGround_Swing_Montage || CurrentMontage == GrappleAir_Swing_Montage) && MontagePosition >= 2.f) return;

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
	UKismetSystemLibrary::LineTraceSingle(this, Start, End, VisibleTraceType, false, TArray<AActor*>(), DebugTraceType, hit,true);

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

	#pragma region AnimNotifyFunctions

void UPandolFlowerComponent::RopeVisibility(bool NewVisibility)
{
	//SetCableVisibility
	FlowerCable->SetCableVisibility(NewVisibility);
}

void UPandolFlowerComponent::ResetMovement()
{
	bMovingWithGrapple = false;
	CurrentGrapplePoint = nullptr;
	bInGrapplingAnimation = false;
	bSwinging = false;
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

void UPandolFlowerComponent::StartSwinging()
{
	bMovingWithGrapple = false;
	bInGrapplingAnimation = false;
	bSwinging = true;
	//CharacterOwner->GetCharacterMovement()->GravityScale = 0.2f;

	CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	//CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//CharacterOwner->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterOwner->GetCharacterMovement()->StopMovementImmediately();

	CurrentGrapplePoint->LandingZone_Mesh->SetWorldRotation(FRotator(0.f, PanWolfCharacter->GetCapsuleComponent()->GetComponentRotation().Yaw, 0.f),false,nullptr,ETeleportType::TeleportPhysics);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	PanWolfCharacter->AttachToComponent(CurrentGrapplePoint->LandingZone_Mesh, AttachmentRules);
	
	CurrentGrapplePoint->LandingZone_Mesh->SetSimulatePhysics(true);
	CurrentGrapplePoint->LandingZone_Mesh->AddImpulse(CurrentGrapplePoint->LandingZone_Mesh->GetForwardVector() * 150000.f);
}
	#pragma endregion

#pragma endregion


void UPandolFlowerComponent::PlayMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) return;
	if (!OwningPlayerAnimInstance) return;
	//if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) { return;}

	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
}








