#include "Components/TransformationCharacterComponent.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/Combat/PandoCombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/TargetingComponent.h"

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

	Capsule->SetCapsuleRadius(TransformationCharacterData.CapsuleRadius);
	Capsule->SetCapsuleHalfHeight(TransformationCharacterData.CapsuleHalfHeight);
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
