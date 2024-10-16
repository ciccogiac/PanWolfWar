#include "Components/TransformationCharacterComponent.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/Combat/PandoCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/TargetingComponent.h"

#include "PanWolfWar/DebugHelper.h"

UTransformationCharacterComponent::UTransformationCharacterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
}

void UTransformationCharacterComponent::Activate(bool bReset)
{
	Super::Activate();

	const bool IsCrouched = CharacterOwner->bIsCrouched;

	if (!IsCrouched && bCanCrouch)
	{
		Capsule->SetCapsuleHalfHeight(TransformationCharacterData.CapsuleHalfHeight);
		Capsule->SetCapsuleRadius(TransformationCharacterData.CapsuleRadius);
	}
	else if (!bCanCrouch)
	{

		MovementComponent->bWantsToCrouch = false;

		// Forza l'aggiornamento della capsula con un piccolo ritardo
		GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
			{
				Capsule->SetCapsuleHalfHeight(TransformationCharacterData.CapsuleHalfHeight, true);
				Capsule->SetCapsuleRadius(TransformationCharacterData.CapsuleRadius);
				Capsule->UpdateOverlaps();  // Forza il ricalcolo delle sovrapposizioni
				CharacterOwner->GetCharacterMovement()->ForceReplicationUpdate();  // Forza la replicazione
			});


	}

	CameraBoom->TargetArmLength = TransformationCharacterData.TargetArmLength;
	CharacterOwner->GetCharacterMovement()->JumpZVelocity = TransformationCharacterData.JumpZVelocity;
	MovementComponent->MaxWalkSpeedCrouched = TransformationCharacterData.MaxWalkSpeedCrouched;
	if (!TargetingComponent->IsTargeting())
		MovementComponent->MaxWalkSpeed = TransformationCharacterData.MaxWalkSpeed;

	PanWolfCharacter->bUseControllerRotationPitch = false;
	PanWolfCharacter->bUseControllerRotationYaw = false;

	PanWolfCharacter->AddMappingContext(TransformationCharacterData.TransformationCharacterMappingContext, 1);

	PanWolfCharacter->SetTransformationCharacter(TransformationCharacterData.SkeletalMeshAsset, TransformationCharacterData.Anim);
	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

	PanWolfCharacter->SetCollisionHandBoxExtent(TransformationCharacterData.CombatHandBoxExtent);

}

void UTransformationCharacterComponent::Deactivate()
{
	Super::Deactivate();

	PanWolfCharacter->RemoveMappingContext(TransformationCharacterData.TransformationCharacterMappingContext);

	CombatComponent->ResetAttack();
}

bool UTransformationCharacterComponent::CheckCapsuleSpace()
{

	FVector CharacterLocation = CharacterOwner->GetActorLocation();
	FQuat CapsuleRotation = CharacterOwner->GetActorQuat();

	// Parametri della capsula attuale
	float CurrentCapsuleHalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();

	// Parametri della capsula da controllare
	float TargetCapsuleHalfHeight = TransformationCharacterData.CapsuleHalfHeight;
	/*float TargetCapsuleRadius = TransformationCharacterData.CapsuleRadius;*/
	float TargetCapsuleRadius = Capsule->GetUnscaledCapsuleRadius();

	// Calcola la differenza di altezza tra la capsula attuale e la nuova capsula
	float HeightDifference = TargetCapsuleHalfHeight - CurrentCapsuleHalfHeight;

	// Se la nuova capsula è più alta, simula lo spostamento del centro verso l'alto per evitare collisioni col pavimento
	FVector SimulatedCapsuleLocation = CharacterLocation;
	if (HeightDifference > 0.0f)
	{
		// Sposta virtualmente il centro della capsula verso l'alto della metà della differenza di altezza
		SimulatedCapsuleLocation += FVector(0.0f, 0.0f, HeightDifference);
	}

	// Costruire i parametri per il test di collisione
	FCollisionShape CapsuleShape = FCollisionShape::MakeCapsule(TargetCapsuleRadius, TargetCapsuleHalfHeight);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CharacterOwner); // Ignora il personaggio stesso

	// Effettua il test di collisione sulla nuova posizione virtuale
	bool bHasSpace = !GetWorld()->OverlapBlockingTestByChannel(
		SimulatedCapsuleLocation,   // Posizione simulata (senza spostare il personaggio)
		CapsuleRotation,            // Rotazione della capsula
		ECC_Pawn,                   // Canale di collisione
		CapsuleShape,               // Forma della capsula
		QueryParams                 // Parametri di query per la collisione
	);

	//// Disegna la capsula virtuale per vedere dove sarebbe posizionata
	//DrawDebugCapsule(
	//	GetWorld(),
	//	SimulatedCapsuleLocation,   // Posizione simulata della capsula
	//	TargetCapsuleHalfHeight,    // Altezza della nuova capsula
	//	TargetCapsuleRadius,        // Raggio della nuova capsula
	//	CapsuleRotation,            // Rotazione
	//	bHasSpace ? FColor::Green : FColor::Red, // Verde se c'è spazio, rosso se no
	//	false,                      // Persistente (falso per disegnarla temporaneamente)
	//	5.0f                        // Durata del debug (5 secondi)
	//);

	// Debug output
	if (bHasSpace)
	{
		//Debug::Print(TEXT("C'è spazio per la nuova capsula."), FColor::Green);
		return true;
	}
	else
	{
		//Debug::Print(TEXT("Non c'è spazio per la nuova capsula."), FColor::Red);
		return false;
	}

	

}

void UTransformationCharacterComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PanWolfCharacter)
	{
		Capsule = PanWolfCharacter->GetCapsuleComponent();
		CameraBoom = PanWolfCharacter->GetCameraBoom();
		CombatComponent = Cast<UPandoCombatComponent>(PanWolfCharacter->GetCombatComponent());
		MovementComponent = CharacterOwner->GetCharacterMovement();
		TargetingComponent = PanWolfCharacter->GetTargetingComponent();
	}

}

void UTransformationCharacterComponent::PlayHardLandMontage()
{
	bIsHardLanding = true;

	OwningPlayerAnimInstance->Montage_Play(HardLandMontage);

	FOnMontageEnded HardLandMontageEndedDelegate;
	HardLandMontageEndedDelegate.BindUObject(this, &UTransformationCharacterComponent::OnHardLandMontageEnded);
	OwningPlayerAnimInstance->Montage_SetEndDelegate(HardLandMontageEndedDelegate, HardLandMontage);

	CharacterOwner->GetMesh()->SetScalarParameterValueOnMaterials(FName("HitFxSwitch"), 1.f);
}

void UTransformationCharacterComponent::OnHardLandMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;

	bIsHardLanding = false;
	CharacterOwner->GetMesh()->SetScalarParameterValueOnMaterials(FName("HitFxSwitch"), 0.f);
}

float UTransformationCharacterComponent::GetCombatDistanceRange(ECombatAttackRange CombatAttackRange)
{
	switch (CombatAttackRange)
	{
	case ECombatAttackRange::ECAR_UnderAttackDistanceRange:
		return TransformationCharacterData.CombatDistanceRange.UnderAttackDistanceRange;
		break;
	case ECombatAttackRange::ECAR_MeleeAttackDistanceRange:
		return TransformationCharacterData.CombatDistanceRange.MeleeAttackDistanceRange;
		break;
	case ECombatAttackRange::ECAR_MeleeAttackDistance:
		return TransformationCharacterData.CombatDistanceRange.MeleeAttackDistance;
		break;
	default:
		break;
	}
	return 300.f;
}

void UTransformationCharacterComponent::ClearAllTimer()
{
}
