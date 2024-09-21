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
		CameraBoom = PanWolfCharacter->GetCameraBoom();
		PandolfoComponent = PanWolfCharacter->GetPandolfoComponent();
		TransformationComponent = PanWolfCharacter->GetTransformationComponent();
		CombatComponent = Cast<UPandoCombatComponent>(PanWolfCharacter->GetCombatComponent());
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

	if (!bSwinging) 
	{ 
		if (IsCovering)
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
		else
		{
			PanWolfCharacter->Move(Value);
		}
		
	}

	else
	{
		FVector2D MovementVector = Value.Get<FVector2D>().GetSafeNormal();

		const float ForceY = UKismetMathLibrary::MapRangeClamped(MovementVector.Y, -1, 1, -SwingForce, SwingForce);

		CurrentGrapplePoint->LandingZone_Mesh->AddForce(CurrentGrapplePoint->LandingZone_Mesh->GetForwardVector() * ForceY );

		const float X = MovementVector.Y >= 0.f ? MovementVector.X : -MovementVector.X;
		CurrentGrapplePoint->LandingZone_Mesh->AddWorldRotation(FRotator(0.f,X, 0.f));
	}
	
}

void UPandolFlowerComponent::Jump()
{
	if (bInGrapplingAnimation) return;
	if (IsCovering) return;
	//CharacterOwner->LaunchCharacter(CharacterOwner->GetActorForwardVector() * 2000.f, true, true);

	if (bSwinging) 
	{
		
		//const FVector v = CurrentGrapplePoint->LandingZone_Mesh->GetForwardVector() * PanWolfCharacter->GetVelocity().Length();
		const FVector v = CurrentGrapplePoint->LandingZone_Mesh->GetComponentVelocity().GetSafeNormal() * PanWolfCharacter->GetVelocity().Length();
		PanWolfCharacter->DetachRootComponentFromParent();
		CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);


		CurrentGrapplePoint->ResetLandingZone();

		ResetMovement(); RopeVisibility(false);

		
		CharacterOwner->LaunchCharacter(v *2.3 , false, false);
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
}

void UPandolFlowerComponent::Crouch()
{
	if ( bInGrapplingAnimation || bMovingWithGrapple || bSwinging ) return;

	if (FlowerHideObject)
	{
		SetCharRotation(-CharacterOwner->GetActorForwardVector(), true);
		SetCharLocation(CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 70.f,FVector::ZeroVector, true);
		return;
	}

	const bool IsCrouched = CharacterOwner->bIsCrouched;
	CharacterOwner->GetCharacterMovement()->bWantsToCrouch = !IsCrouched;

	

	if (!IsCrouched)
	{
		//CrouchingTimeline.PlayFromStart();

		CheckCanCrouchHide();

	}

	else
	{
		//CrouchingTimeline.Reverse();

		//if (PanWolfCharacter->IsHiding())
		//	PanWolfCharacter->SetIsHiding(false);

		if (PanWolfCharacter->IsHiding())
			CheckCanHideStandUP();
	}
}

void UPandolFlowerComponent::CheckCanCrouchHide()
{
	if (PanWolfCharacter->IsHiding()) return;

	const FVector Start = CharacterOwner->GetActorLocation();
	const FVector End = Start + CharacterOwner->GetActorForwardVector();
	FHitResult Hit;
	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, 60.f, HidingObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::None, Hit, true);
	if (Hit.bBlockingHit)
		PanWolfCharacter->SetIsHiding(true, false);
}

void UPandolFlowerComponent::CheckCanHideStandUP()
{
	const FVector Start = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorUpVector() * CharacterOwner->BaseEyeHeight * 3.f;
	const FVector End = Start + CharacterOwner->GetActorForwardVector();
	FHitResult Hit;
	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, 20.f, HidingObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, Hit, true);
	if (!Hit.bBlockingHit)
		PanWolfCharacter->SetIsHiding(false);
}

#pragma endregion

#pragma region ActivationComponent

void UPandolFlowerComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	//Debug::Print(TEXT("OVA"));

	PanWolfCharacter->AddMappingContext(PandolFlowerMappingContext, 1);

	PandolFlowerState = EPandolFlowerState::EPFS_PandolFlower;

	PanWolfCharacter->bUseControllerRotationPitch = false;
	PanWolfCharacter->bUseControllerRotationYaw = false;

	PanWolfCharacter->SetTransformationCharacter(SkeletalMeshAsset, Anim);
	PanWolfCharacter->GetNiagaraTransformation()->SetAsset(Pandolflower_Niagara);

	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

	if (OwningPlayerAnimInstance)
	{
		CombatComponent->SetCombatEnabled(OwningPlayerAnimInstance, ETransformationCombatType::ETCT_PandolFlower);
		OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UPandolFlowerComponent::OnMontageEnded);
	}

	if (PanWolfCharacter->IsHiding())
		CharacterOwner->GetMesh()->SetScalarParameterValueOnMaterials(FName("Emissive Multiplier"), 10.f);

	FlowerCable = GetWorld()->SpawnActor<AFlowerCable>(BP_FlowerCable, CharacterOwner->GetActorLocation(), CharacterOwner->GetActorRotation());
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	//FlowerCable->AttachToComponent(CharacterOwner->GetMesh(), AttachmentRules, FName("hand_l"));
	FlowerCable->SetCableAttachment(CharacterOwner->GetMesh(), FName("hand_l"));

	CameraBoom->TargetArmLength = 400.f;

	PanWolfCharacter->SetCollisionHandBoxExtent(CombatHandBoxExtent);


	GetWorld()->GetTimerManager().SetTimer(AirAssassination_TimerHandle, [this]() {this->CheckCanAirAssassin(); }, 0.25f, true);
}

void UPandolFlowerComponent::Deactivate()
{
	Super::Deactivate();

	PanWolfCharacter->RemoveMappingContext(PandolFlowerMappingContext);



	PanWolfCharacter->GetNiagaraTransformation()->SetAsset(nullptr);

	CombatComponent->ResetAttack();

	if (FlowerCable)
		FlowerCable->Destroy();

	if (bSwinging) {
		PanWolfCharacter->DetachRootComponentFromParent();
		CurrentGrapplePoint->ResetLandingZone();
		ResetMovement();
		PanWolfCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	}	

	DeactivateGrapplePoint();

	ResetMovement();

	if (IsCovering)
		UnHide();

	GetWorld()->GetTimerManager().ClearTimer(AirAssassination_TimerHandle);

	PandolFlowerState = EPandolFlowerState::EPFS_PandolFlower;
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
			CharacterOwner->GetCharacterMovement()->GravityScale = 2.2f;
			CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
			CharacterOwner->Jump();
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

		CharacterOwner->GetCharacterMovement()->bWantsToCrouch = false;
		if (IsCovering)
			UnHide();
		PlayMontage(GrappleMontage);
	}
}

void UPandolFlowerComponent::CheckForGrapplePoint()
{
	 const FVector CameraLocation2 = PanWolfCharacter->GetFollowCamera()->GetComponentLocation();
	 const FVector CameraForward2 = PanWolfCharacter->GetFollowCamera()->GetForwardVector();

	const FVector Start = CameraLocation2;
	const FVector End = Start + CameraForward2 * DetectionRadius;

	//const FVector Start = CharacterOwner->GetActorLocation();
	//const FVector End = Start;


	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	TArray<FHitResult> outHits;
	bool bBlockingHits = UKismetSystemLibrary::SphereTraceMultiForObjects(this,Start,End, 1500.f, GrapplingObjectTypes,false,TArray<AActor*>(), DebugTraceType, outHits,true);

	if (bBlockingHits)
	{
		float HighestDotProduct = 0.7f;
		AActor* DetectedActor = nullptr;

		for (const FHitResult& hit : outHits)
		{
			const FVector CameraForward = FollowCamera->GetForwardVector();
			const FVector CameraLocation = FollowCamera->GetComponentLocation() + FollowCamera->GetUpVector() * 100.f;
			const FVector HitActorLocation = hit.GetActor()->GetActorLocation();

			const FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(CameraLocation, HitActorLocation);
			const double Dot = FVector::DotProduct(CameraForward, Direction);

			if (Dot > HighestDotProduct)
			{
				AGrapplePoint* GrapplePoint = Cast<AGrapplePoint>(hit.GetActor());
				if ((GrapplePoint && CurrentGrapplePoint && GrapplePoint!=CurrentGrapplePoint) || CurrentGrapplePoint==nullptr)
				{

					DetectedActor = hit.GetActor();
					HighestDotProduct = Dot;
				}
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

	CharacterOwner->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	//CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(90.f);
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
	//CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(35.f);
	CharacterOwner->GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Ignore);
	//CharacterOwner->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterOwner->GetCharacterMovement()->StopMovementImmediately();

	CurrentGrapplePoint->LandingZone_Mesh->SetWorldRotation(FRotator(0.f, PanWolfCharacter->GetCapsuleComponent()->GetComponentRotation().Yaw, 0.f),false,nullptr,ETeleportType::TeleportPhysics);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	PanWolfCharacter->AttachToComponent(CurrentGrapplePoint->LandingZone_Mesh, AttachmentRules);
	
	CurrentGrapplePoint->LandingZone_Mesh->SetSimulatePhysics(true);
	CurrentGrapplePoint->LandingZone_Mesh->AddImpulse(CurrentGrapplePoint->LandingZone_Mesh->GetForwardVector() * SwingForce *1.75);
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

void UPandolFlowerComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == PandolFlowerDodgeMontage)
	{
		PanWolfCharacter->EndDodge();
		PandolFlowerState = EPandolFlowerState::EPFS_PandolFlower;
	}
}


void UPandolFlowerComponent::Hide()
{
	//Debug::Print(TEXT("Hide"));

	if (PanWolfCharacter->IsHiding() && !IsCovering) return;

	if (IsCovering)
	{
		UnHide();
	}
	else
	{
		const FVector Start = CharacterOwner->GetActorLocation();
		const FVector End = Start + CharacterOwner->GetActorForwardVector();
		FHitResult hit;
		UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, 60.f, PandolFlowerHideObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::None, hit, true);

		if (!hit.bBlockingHit) return;

		//Debug::Print(TEXT("CanHide"));

		FlowerHideObject = Cast<AFlowerHideObject>(hit.GetActor());
		if (!FlowerHideObject) return;
		FlowerHideObject->ChangeCollisionType(false);

		if(CharacterOwner->GetCharacterMovement()->IsCrouching())
			CharacterOwner->GetCharacterMovement()->bWantsToCrouch = false;

		IsCovering = true;

		SetCharRotation(hit.ImpactNormal, true);
		SetCharLocation(hit.ImpactPoint, hit.ImpactNormal, true);

		CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = false;
		CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 100.f;

		CameraBoom->TargetArmLength = 1000.f;
		PanWolfCharacter->SetIsHiding(true, false);
	}


}

void UPandolFlowerComponent::UnHide()
{
	//Debug::Print(TEXT("UnHide"));
	//hit.GetActor()->SetActorEnableCollision(false);

	if (FlowerHideObject)
	{
		FlowerHideObject->ChangeCollisionType(true);
		FlowerHideObject = nullptr;
	}
		

	IsCovering = false;

	//SetCharRotation(hit.ImpactNormal, true);
	SetCharLocation(CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 100.f, CharacterOwner->GetActorForwardVector(), true);

	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = true;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 500.f;

	CameraBoom->TargetArmLength = 400.f;
	PanWolfCharacter->SetIsHiding(false, false);
}

void UPandolFlowerComponent::SetCharRotation(const FVector ImpactNormal, bool Istantaneus)
{
	FRotator NewRotation = FRotator(0.f, UKismetMathLibrary::MakeRotFromX(ImpactNormal).Yaw , 0.f);

	NewRotation = Istantaneus ? NewRotation : UKismetMathLibrary::RInterpTo(CharacterOwner->GetActorRotation(), NewRotation, GetWorld()->GetDeltaSeconds(), 1.5f);
	CharacterOwner->SetActorRotation(NewRotation);
}

void UPandolFlowerComponent::SetCharLocation(const FVector HitLocation, const FVector HitNormal, bool Istantaneus)
{
	FVector NewLocation = HitLocation - UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(HitNormal)) * 5.f;
	NewLocation = Istantaneus ? NewLocation : UKismetMathLibrary::VInterpTo(CharacterOwner->GetActorLocation(), NewLocation, GetWorld()->GetDeltaSeconds(), 1.f);

	CharacterOwner->SetActorLocation(NewLocation);
}


void UPandolFlowerComponent::Assassination()
{
	
	if (TransformationComponent && PandolfoComponent && PandolfoComponent->IsAssassinableEnemy())
	{
		//Debug::Print(TEXT("Assassin PandolFlower"));

		TransformationComponent->SelectDesiredTransformation(0);
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

	if (!CombatComponent) return;
		CombatComponent->PerformAttack(EAttackType::EAT_LightAttack);
}