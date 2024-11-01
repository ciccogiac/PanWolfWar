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
#include "GameFramework/SpringArmComponent.h"

#include "Actors/FlowerHideObject.h"

#include "Components/PandolfoComponent.h"
#include "Components/TransformationComponent.h"

#include "Components/Combat/PandoCombatComponent.h"

#pragma region EngineFunctions

UPandolFlowerComponent::UPandolFlowerComponent()
{
}

void UPandolFlowerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (PanWolfCharacter)
	{
		FollowCamera = PanWolfCharacter->GetFollowCamera();
		PandolfoComponent = PanWolfCharacter->GetPandolfoComponent();
		TransformationComponent = PanWolfCharacter->GetTransformationComponent();
	}

}

void UPandolFlowerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CheckForGrapplePoint();

	/*if (bInGrapplingAnimation)*/
	if(PandolFlowerState == EPandolFlowerState::EPFS_Hooking)
	{
		MoveRope();
		if (bMovingWithGrapple) GrapplingMovement();
	}

}

void UPandolFlowerComponent::OnHardLandMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	Super::OnHardLandMontageEnded(Montage, bInterrupted);

	if (PandolFlowerState == EPandolFlowerState::EPFS_Hooking)
	{
		PandolFlowerState = EPandolFlowerState::EPFS_PandolFlower;
	}
}

#pragma endregion

#pragma region ActivationSection

void UPandolFlowerComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	PandolFlowerState = EPandolFlowerState::EPFS_PandolFlower;

	PanWolfCharacter->GetNiagaraTransformation()->SetAsset(Pandolflower_Niagara);

	if (OwningPlayerAnimInstance)
	{
		CombatComponent->SetCombatEnabled(OwningPlayerAnimInstance, ETransformationCombatType::ETCT_PandolFlower);
	}

	if (PanWolfCharacter->IsHiding())
		CharacterOwner->GetMesh()->SetScalarParameterValueOnMaterials(FName("HideFxSwitch"), 0.2f);

	if (PanWolfCharacter->IsInsideHideBox() && PanWolfCharacter->IsForcedCrouch() && !PanWolfCharacter->IsHiding())
	{
		MovementComponent->bWantsToCrouch = true;
		PanWolfCharacter->SetIsHiding(true);
	}

	FlowerCable = GetWorld()->SpawnActor<AFlowerCable>(BP_FlowerCable, CharacterOwner->GetActorLocation(), CharacterOwner->GetActorRotation());
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	FlowerCable->SetCableAttachment(CharacterOwner->GetMesh(), FName("hand_l"));


	GetWorld()->GetTimerManager().SetTimer(Flower_AirAssassination_TimerHandle, [this]() {this->CheckCanAirAssassin(); }, 0.2f, true);
}

void UPandolFlowerComponent::Deactivate()
{
	Super::Deactivate();


	if (PandolFlowerState == EPandolFlowerState::EPFS_Dodging)
	{
		PanWolfCharacter->EndDodge();
	}


	PanWolfCharacter->GetNiagaraTransformation()->SetAsset(nullptr);

	if (FlowerCable)
		FlowerCable->Destroy();

	if (PandolFlowerState == EPandolFlowerState::EPFS_FlowerCover)
		FlowerUnHide();

	if (PandolFlowerState == EPandolFlowerState::EPFS_Swinging) {
		PanWolfCharacter->DetachRootComponentFromParent();
		CurrentGrapplePoint->ResetLandingZone();
		ResetMovement();
		PanWolfCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	}	

	DeactivateGrapplePoint();

	ResetMovement();



	GetWorld()->GetTimerManager().ClearTimer(Flower_AirAssassination_TimerHandle);

	
		

	PandolFlowerState = EPandolFlowerState::EPFS_PandolFlower;
}

#pragma endregion

#pragma region Actions

void UPandolFlowerComponent::Move(const FInputActionValue& Value)
{
	//if (PandolFlowerState == EPandolFlowerState::EPFS_Hooking || PandolFlowerState == EPandolFlowerState::EPFS_Dodging) return;
	if (PandolFlowerState == EPandolFlowerState::EPFS_Hooking) return;


	if (PandolFlowerState == EPandolFlowerState::EPFS_FlowerCover)
	{
		const FVector CameraForward = FollowCamera->GetForwardVector();
		const double Dot = FVector::DotProduct(CameraForward, CharacterOwner->GetActorForwardVector());
		float Multiplier = 1;
		if (Dot > 0.4f)
			Multiplier = -1.f;

		FVector2D MovementVector = Value.Get<FVector2D>().GetSafeNormal();

		CoverDirection = MovementVector.X > 0.5f ?
			1.f : (MovementVector.X < -0.5f ? -1.f : 0.f);

		if (CoverDirection == 0.f && MovementVector.Y > 0.5f)
			CoverDirection = LastCoverDirection;
		else if (CoverDirection == 0.f && MovementVector.Y < -0.5f)
			CoverDirection = -LastCoverDirection;
		else
			LastCoverDirection = CoverDirection;

		if (CoverDirection == 0.f) return;

		const FVector MoveDirection = -CharacterOwner->GetActorRightVector();

		CharacterOwner->AddMovementInput(MoveDirection * Multiplier, CoverDirection);
	}

	else if (PandolFlowerState == EPandolFlowerState::EPFS_Swinging)
	{
		FVector2D MovementVector = Value.Get<FVector2D>().GetSafeNormal();

		const float ForceY = UKismetMathLibrary::MapRangeClamped(MovementVector.Y, -1, 1, -SwingForce, SwingForce);

		CurrentGrapplePoint->LandingZone_Mesh->AddForce(CurrentGrapplePoint->LandingZone_Mesh->GetForwardVector() * ForceY);

		const float X = MovementVector.Y >= 0.f ? MovementVector.X : -MovementVector.X;
		CurrentGrapplePoint->LandingZone_Mesh->AddWorldRotation(FRotator(0.f, X, 0.f));
	}

	//else if (PandolFlowerState == EPandolFlowerState::EPFS_PandolFlower)
	else
	{
		PanWolfCharacter->Move(Value);
	}

}

void UPandolFlowerComponent::Jump()
{
	if (PandolFlowerState != EPandolFlowerState::EPFS_PandolFlower && PandolFlowerState != EPandolFlowerState::EPFS_Swinging) return;


	if (PandolFlowerState == EPandolFlowerState::EPFS_Swinging)
	{

		const FVector v = CurrentGrapplePoint->LandingZone_Mesh->GetComponentVelocity().GetSafeNormal() * PanWolfCharacter->GetVelocity().Length();
		PanWolfCharacter->DetachRootComponentFromParent();
		MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);


		CurrentGrapplePoint->ResetLandingZone();

		ResetMovement();
		RopeVisibility(false);


		CharacterOwner->LaunchCharacter(v * 2.3, false, false);
	}

	CharacterOwner->Jump();
}

void UPandolFlowerComponent::Dodge()
{
	if (!PanWolfCharacter->CanPerformDodge() || (PandolFlowerState != EPandolFlowerState::EPFS_PandolFlower)) return;
	if (!PandolFlowerDodgeMontage || !OwningPlayerAnimInstance) return;

	PandolFlowerState = EPandolFlowerState::EPFS_Dodging;

	PanWolfCharacter->StartDodge();
	OwningPlayerAnimInstance->Montage_Play(PandolFlowerDodgeMontage);

	FOnMontageEnded DodgeMontageEndedDelegate;
	DodgeMontageEndedDelegate.BindUObject(this, &UPandolFlowerComponent::OnDodgeMontageEnded);
	OwningPlayerAnimInstance->Montage_SetEndDelegate(DodgeMontageEndedDelegate, PandolFlowerDodgeMontage);
}

void UPandolFlowerComponent::Crouch()
{
	if (PandolFlowerState != EPandolFlowerState::EPFS_PandolFlower) return;
	if (PanWolfCharacter && PanWolfCharacter->IsForcedCrouch()) return;


	const bool IsCrouched = CharacterOwner->bIsCrouched;
	MovementComponent->bWantsToCrouch = !IsCrouched;



	if (!IsCrouched)
	{
		//CrouchingTimeline.PlayFromStart();


		if (PanWolfCharacter->IsInsideHideBox())
		{
			PanWolfCharacter->SetIsHiding(true);
		}

	}

	else
	{
		//CrouchingTimeline.Reverse();

		if (PanWolfCharacter->IsInsideHideBox())
		{
			PanWolfCharacter->SetIsHiding(false);
		}
	}
}

#pragma endregion

#pragma region Grappling

void UPandolFlowerComponent::Hook()
{
	if (bIsHardLanding) return;
	if (!GrapplePointRef) return;
	if (PandolFlowerState == EPandolFlowerState::EPFS_Dodging || PandolFlowerState == EPandolFlowerState::EPFS_Hooking) return;
	if (TransformationComponent && PandolfoComponent && PandolfoComponent->IsAssassinableEnemy()) return;
	//if (PandolFlowerState == EPandolFlowerState::EPFS_Dodging ) return;

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
			/*MovementComponent->GravityScale = 2.2f;*/
			MovementComponent->GravityScale = 1.75f;
			MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);
			CharacterOwner->Jump();
		}

		MovementComponent->bWantsToCrouch = false;
		if (PandolFlowerState == EPandolFlowerState::EPFS_FlowerCover)
			FlowerUnHide();

		/*bInGrapplingAnimation = true;*/
		PandolFlowerState = EPandolFlowerState::EPFS_Hooking;
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
			GrappleMontage = MovementComponent->IsFalling() ? GrappleAir_Montage : GrappleGround_Montage;

			PlayMontage(GrappleMontage);
			FOnMontageEnded HookMontageEndedDelegate;
			HookMontageEndedDelegate.BindUObject(this, &UPandolFlowerComponent::OnHookMontageEnded);
			OwningPlayerAnimInstance->Montage_SetEndDelegate(HookMontageEndedDelegate, GrappleMontage);
		}
		else if (CurrentGrapplePoint->GrapplePointType == EGrapplePointType::EGPT_Swing)
		{
			GrapplingDestination = CurrentGrapplePoint->GetLandingZone() + FVector(0.f, 0.f, -110.f);
			//GrappleMontage = CharacterOwner->GetCharacterMovement()->IsFalling() ? GrappleAir_Swing_Montage : GrappleGround_Swing_Montage;
			GrappleMontage = GrappleAir_Swing_Montage;

			PlayMontage(GrappleMontage);

			FOnMontageEnded HookMontageEndedDelegate;
			HookMontageEndedDelegate.BindUObject(this, &UPandolFlowerComponent::OnHookMontageEnded);
			OwningPlayerAnimInstance->Montage_SetEndDelegate(HookMontageEndedDelegate, GrappleMontage);
		}


	}
}

void UPandolFlowerComponent::CheckForGrapplePoint()
{

	const FVector CameraLocation = FollowCamera->GetComponentLocation();
	const FVector CameraForward = FollowCamera->GetForwardVector();

	const FVector Start = CameraLocation + FollowCamera->GetUpVector() * GrappleDetection_CameraUPOffset;
	const FVector End = Start + CameraForward * GrappleDetectionDistance;


	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	TArray<FHitResult> outHits;
	bool bBlockingHits = UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, End, GrappleDetectionRadius, GrapplingObjectTypes, false, TArray<AActor*>(), DebugTraceType, outHits, true);

	if (bBlockingHits)
	{
		float HighestAngle = 75.f;        // L'angolo massimo accettabile per considerare un punto di aggancio
		float NearestAngle = 75.f;        // L'angolo più basso trovato per gli attori entro GrappleThrowDistance
		AActor* DetectedActor = nullptr;  // L'attore rilevato come punto di aggancio più valido
		AActor* PreferredActor = nullptr; // Attore preferito se è più vicino di GrappleThrowDistance

		for (const FHitResult& hit : outHits)
		{
			const FVector HitActorLocation = hit.GetActor()->GetActorLocation();

			// Calcola la distanza dal personaggio al punto di aggancio rilevato
			const float DistanceToPlayer = FVector::Dist(CharacterOwner->GetActorLocation(), HitActorLocation);

			// Calcola la direzione e l'angolo tra la camera e l'attore rilevato
			const FVector Direction = (HitActorLocation - Start).GetSafeNormal();
			const double Dot = FVector::DotProduct(CameraForward, Direction);
			const float AngleInRadians = FMath::Acos(Dot);
			const float Angle = FMath::RadiansToDegrees(AngleInRadians);

			// Se l'attore è entro GrappleThrowDistance, seleziona quello con l'angolo minore
			if (DistanceToPlayer <= GrappleThrowDistance)
			{
				if (Angle < NearestAngle)
				{
					AGrapplePoint* GrapplePoint = Cast<AGrapplePoint>(hit.GetActor());

					// Controlla se il punto di grapple è diverso dall'attuale punto di aggancio o se non c'è nessun punto attivo
					if ((GrapplePoint && CurrentGrapplePoint && GrapplePoint != CurrentGrapplePoint) || CurrentGrapplePoint == nullptr)
					{
						PreferredActor = hit.GetActor();
						NearestAngle = Angle; // Aggiorna l'angolo più basso trovato per gli attori vicini
					}
				}
			}
			// Altrimenti, se l'attore è fuori da GrappleThrowDistance ma entro GrappleDetectionDistance

			else if (Angle < HighestAngle)
			{
				AGrapplePoint* GrapplePoint = Cast<AGrapplePoint>(hit.GetActor());

				// Controlla se il punto di grapple è diverso dall'attuale punto di aggancio o se non c'è nessun punto attivo
				if ((GrapplePoint && CurrentGrapplePoint && GrapplePoint != CurrentGrapplePoint) || CurrentGrapplePoint == nullptr)
				{
					DetectedActor = hit.GetActor();  // Aggiorna l'attore rilevato
					HighestAngle = Angle;            // Aggiorna l'angolo più piccolo trovato
				}
			}
			
		}

		// Se è stato trovato un punto preferito entro la distanza ridotta, usa quello
		if (PreferredActor)
		{
			ActivateGrapplePoint(PreferredActor);
		}
		// Altrimenti, usa l'attore con l'angolo migliore
		else if (DetectedActor)
		{
			ActivateGrapplePoint(DetectedActor);
		}
		else
		{
			// Disattiva il punto di grapple se non è stato trovato niente di valido
			DeactivateGrapplePoint();
		}
		
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

	PandolFlowerState = EPandolFlowerState::EPFS_PandolFlower;
	/*MovementComponent->GravityScale = 2.2f;*/
	MovementComponent->GravityScale = 1.75f;

	CharacterOwner->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

}

void UPandolFlowerComponent::ThrowRope()
{
	UGameplayStatics::PlaySound2D(this, ThrowRope_Sound);
}

void UPandolFlowerComponent::StartGrapplingMovement()
{
	UGameplayStatics::PlaySound2D(this, GrappleJump_Sound);
	MovementComponent->GravityScale = 0.0f;
	MovementComponent->StopMovementImmediately();
	StartingPosition = CharacterOwner->GetActorLocation();
	MovementComponent->SetMovementMode(EMovementMode::MOVE_Falling);
	bMovingWithGrapple = true;
}

void UPandolFlowerComponent::StartSwinging()
{
	bMovingWithGrapple = false;
	/*bInGrapplingAnimation = false;
	bSwinging = true;*/

	PandolFlowerState = EPandolFlowerState::EPFS_Swinging;

	//CharacterOwner->GetCharacterMovement()->GravityScale = 0.2f;

	MovementComponent->SetMovementMode(EMovementMode::MOVE_Flying);
	//CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(35.f);
	Capsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	//CharacterOwner->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MovementComponent->StopMovementImmediately();

	CurrentGrapplePoint->LandingZone_Mesh->SetWorldRotation(FRotator(0.f, Capsule->GetComponentRotation().Yaw, 0.f),false,nullptr,ETeleportType::TeleportPhysics);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	PanWolfCharacter->AttachToComponent(CurrentGrapplePoint->LandingZone_Mesh, AttachmentRules);
	
	CurrentGrapplePoint->LandingZone_Mesh->SetSimulatePhysics(true);
	CurrentGrapplePoint->LandingZone_Mesh->AddImpulse(CurrentGrapplePoint->LandingZone_Mesh->GetForwardVector() * SwingForce *1.75);
}

	#pragma endregion

#pragma endregion

#pragma region FlowerHide

void UPandolFlowerComponent::FlowerHide()
{

	//Is a normal hiding
	if (PanWolfCharacter->IsHiding() && PandolFlowerState != EPandolFlowerState::EPFS_FlowerCover) return;

	if (PandolFlowerState == EPandolFlowerState::EPFS_FlowerCover)
	{
		FlowerUnHide();
	}
	else
	{
		if (!PanWolfCharacter->GetEnemyAware().IsEmpty()) return;

		FHitResult hit;
		FlowerHideTrace(hit);
		if (!CheckCanFlowerHide(hit)) return;

		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (PlayerController)
		{
			CharacterOwner->DisableInput(PlayerController);
			MovementComponent->StopMovementImmediately();
		}

		FlowerHideObject->ChangeCollisionType(false);
		
		if (MovementComponent->IsCrouching())
			MovementComponent->bWantsToCrouch = false;

		PandolFlowerState = EPandolFlowerState::EPFS_FlowerCover;

		SetCharacterToFlowerHide(hit);
		PanWolfCharacter->SetIsHiding(true);

		if (PlayerController){ CharacterOwner->EnableInput(PlayerController);}

		SetCameraToFlowerHide();

	}
}

void UPandolFlowerComponent::FlowerHideTrace(FHitResult& hit)
{
	const FVector Start = CharacterOwner->GetActorLocation();
	const FVector End = Start + CharacterOwner->GetActorForwardVector() * 80.f;
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, 100.f, PandolFlowerHideObjectTypes, false, TArray<AActor*>(), DebugTraceType, hit, true);
}

bool UPandolFlowerComponent::CheckCanFlowerHide(const FHitResult& hit)
{
	if (!hit.bBlockingHit) return false;

	FlowerHideObject = Cast<AFlowerHideObject>(hit.GetActor());
	if (!FlowerHideObject) return false;

	const double Dot = FVector::DotProduct(hit.ImpactNormal, FlowerHideObject->GetFlowerHideBoxForward());
	const double Tolerance = 0.01;
	if (!FMath::IsNearlyEqual(FMath::Abs(Dot), 1.0, Tolerance)) { return false; }

	return true;
}

void UPandolFlowerComponent::SetCharacterToFlowerHide(const FHitResult& hit)
{
	FRotator NewRotation = FRotator(0.f, UKismetMathLibrary::MakeRotFromX(hit.ImpactNormal).Yaw, 0.f);
	CharacterOwner->SetActorRotation(NewRotation);
	//NewRotation = Istantaneus ? NewRotation : UKismetMathLibrary::RInterpTo(CharacterOwner->GetActorRotation(), NewRotation, GetWorld()->GetDeltaSeconds(), 1.5f);

	CharacterOwner->SetActorLocation(hit.ImpactPoint);
	CharacterOwner->AddActorLocalOffset(FVector(-FlowerHideObject->GetFlowerHideBoxWidthX() / 2, 0.f, 0.f));

	MovementComponent->bOrientRotationToMovement = false;
	MovementComponent->MaxWalkSpeed = 100.f;
}

void UPandolFlowerComponent::SetCameraToFlowerHide()
{
	if (CameraBoom && FollowCamera) 
	{


		// Blocca la rotazione della telecamera
		CameraBoom->bUsePawnControlRotation = false; // La telecamera non seguirà la rotazione del controller
		CameraBoom->bInheritPitch = false;
		CameraBoom->bInheritYaw = false;
		CameraBoom->bInheritRoll = false;

		FVector NewLocation = CharacterOwner->GetActorLocation(); /*+CharacterOwner->GetActorRightVector() * 350.f + hit.ImpactNormal * 250.f;*/
		FRotator NewRotation(0.f, CharacterOwner->GetActorRotation().Yaw + 180.f + 45.f, 0.f);


		CameraBoom->SetWorldLocation(NewLocation);
		CameraBoom->SetRelativeRotation(NewRotation);

		CameraBoom->TargetArmLength = 700.f;
		FollowCamera->SetFieldOfView(100.f);
	}
}

void UPandolFlowerComponent::FlowerUnHide()
{
	if (FlowerHideObject)
	{
		FlowerHideObject->ChangeCollisionType(true);
		FlowerHideObject = nullptr;
	}

	CharacterOwner->SetActorLocation(CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 225.f);
	MovementComponent->MaxWalkSpeed = TransformationCharacterData.MaxWalkSpeed;

	PanWolfCharacter->SetIsHiding(false);

	ResetCameraFromFlowerHide();
	/*NewLocation = Istantaneus ? NewLocation : UKismetMathLibrary::VInterpTo(CharacterOwner->GetActorLocation(), NewLocation, GetWorld()->GetDeltaSeconds(), 1.f);*/

	MovementComponent->bOrientRotationToMovement = true;

	PandolFlowerState = EPandolFlowerState::EPFS_PandolFlower;

}

void UPandolFlowerComponent::ResetCameraFromFlowerHide()
{
	if (CameraBoom && FollowCamera)
	{

		CameraBoom->bUsePawnControlRotation = true;
		CameraBoom->bInheritPitch = true;
		CameraBoom->bInheritYaw = true;
		CameraBoom->bInheritRoll = true;

		CameraBoom->TargetArmLength = TransformationCharacterData.TargetArmLength;
		FollowCamera->SetFieldOfView(90.f);

		CameraBoom->SetWorldLocation(CharacterOwner->GetActorLocation());
		CameraBoom->SetWorldRotation(FRotator::ZeroRotator);

		FRotator DesiredCameraRotation = FRotator(-30.f, CharacterOwner->GetActorRotation().Yaw, 0.f);

		APlayerController* PlayerController = Cast<APlayerController>(CharacterOwner->GetController());
		if (PlayerController)
		{
			PlayerController->SetControlRotation(DesiredCameraRotation);
		}
	}
}

#pragma endregion

#pragma region Assassination & Attack

void UPandolFlowerComponent::Assassination()
{

	if (TransformationComponent && PandolfoComponent && PandolfoComponent->IsAssassinableEnemy())
	{
		bool bIsHidingAssassination = false;
		ABaseEnemy* Enemy = nullptr;
		if (PanWolfCharacter->IsHiding())
		{
			bIsHidingAssassination = true;
			Enemy = PandolfoComponent->GetAssassinableEnemy();
		}

		TransformationComponent->SelectDesiredTransformation(ETransformationState::ETS_Pandolfo);

		if(bIsHidingAssassination)
			PandolfoComponent->AssassinationFromFlowerHiding(Enemy);
		else
			PandolfoComponent->Assassination();
	}

}

void UPandolFlowerComponent::CheckCanAirAssassin()
{
	//Debug::Print(TEXT("CheckCanAirAssassin"));
	if (PandolfoComponent)
		PandolfoComponent->CheckCanAirAssassin();

}

void UPandolFlowerComponent::LightAttack()
{
	if (!CombatComponent || PandolFlowerState != EPandolFlowerState::EPFS_PandolFlower) return;

	MovementComponent->bWantsToCrouch = false;
	if (PanWolfCharacter->IsInsideHideBox())
	{
		PanWolfCharacter->SetIsHiding(false);
	}

	CombatComponent->PerformAttack(EAttackType::EAT_LightAttack);
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

void UPandolFlowerComponent::OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;

	PandolFlowerState = EPandolFlowerState::EPFS_PandolFlower;
	PanWolfCharacter->EndDodge();


}

void UPandolFlowerComponent::OnHookMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;

	if (bInterrupted && (PanWolfCharacter->IsHitted() || bIsHardLanding))
	{
		RopeVisibility(false);
		ResetMovement();
	}
		
}

#pragma endregion

void UPandolFlowerComponent::ClearAllTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(Flower_AirAssassination_TimerHandle);
}