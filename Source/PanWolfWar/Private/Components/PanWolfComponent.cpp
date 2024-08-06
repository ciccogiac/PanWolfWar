#include "Components/PanWolfComponent.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CombatComponent.h"

#include "PanWolfWar/DebugHelper.h"
#include "Components/TargetingComponent.h"

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

void UPanWolfComponent::Dodge()
{
	if (!PanWolfCharacter->CanPerformDodge()) return;

	//if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;

	if (!PandolWolfDodgeMontage) return;
	UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return;
	//if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;
	//if (CharacterOwner->GetCharacterMovement()->GetLastInputVector().Length() < 0.5f) return;

	//CharacterOwner->DisableInput(CharacterOwner->GetLocalViewingPlayerController());
	//Debug::Print(TEXT("Dodge"));
	OwningPlayerAnimInstance->Montage_Play(PandolWolfDodgeMontage);
}

void UPanWolfComponent::Attack()
{
	if (!CombatComponent->IsAttacking())
	{
		if (!PanWolfCharacter->GetTargetingComponent()->IsTargeting())
			PanWolfCharacter->GetTargetingComponent()->TryLock();

		const bool isTargeting = PanWolfCharacter->GetTargetingComponent()->IsActive() && PanWolfCharacter->GetTargetingComponent()->IsTargeting();
		const AActor* ClosestEnemy = isTargeting ? PanWolfCharacter->GetTargetingComponent()->GetTargetActor() : CombatComponent->GetClosestEnemy();

		//if (!ClosestEnemy) return;

		const bool EnemyDirection = CombatComponent->GetEnemyDirection(ClosestEnemy);
		CombatComponent->RotateToClosestEnemy(ClosestEnemy);
		EAttackType AttackType = EnemyDirection ? EAttackType::EAT_LightAttack_Right : EAttackType::EAT_LightAttack_Left;
		CombatComponent->PerformAttack(AttackType);
	}
	else
		CombatComponent->PerformAttack(EAttackType::EAT_LightAttack_Right);

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

