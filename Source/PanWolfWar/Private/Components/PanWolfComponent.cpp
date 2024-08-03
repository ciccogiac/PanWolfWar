#include "Components/PanWolfComponent.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CombatComponent.h"

#include "PanWolfWar/DebugHelper.h"

UPanWolfComponent::UPanWolfComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
}

void UPanWolfComponent::Activate(bool bReset)
{
	Super::Activate();



	PanWolfCharacter->AddMappingContext(PanWolfMappingContext, 1);

	PanWolfCharacter->SetTransformationCharacter(SkeletalMeshAsset, Anim);

	CharacterOwner->GetCharacterMovement()->JumpZVelocity = 800.f;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleRadius(60.f);
	CharacterOwner->GetMesh()->AddLocalOffset(FVector(0.f, -25.f, 0.f));

	CombatComponent->SetCombatEnabled(true);
}

void UPanWolfComponent::Deactivate()
{
	Super::Deactivate();

	CharacterOwner->GetCharacterMovement()->JumpZVelocity = 500.f;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleRadius(35.f);

	CombatComponent->SetCombatEnabled(false);

	PanWolfCharacter->RemoveMappingContext(PanWolfMappingContext);
}

void UPanWolfComponent::Jump()
{
	CharacterOwner->Jump();
}

void UPanWolfComponent::Attack()
{
	CombatComponent->PerformAttack(EAttackType::EAT_LightAttack);
}

void UPanWolfComponent::BeginPlay()
{
	Super::BeginPlay();	

	CombatComponent = PanWolfCharacter->GetCombatComponent();
}

void UPanWolfComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

