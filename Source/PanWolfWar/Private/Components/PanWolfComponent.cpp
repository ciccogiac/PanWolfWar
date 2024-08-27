#include "Components/PanWolfComponent.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/Combat/PandoCombatComponent.h"

#include "PanWolfWar/DebugHelper.h"
#include "Components/TargetingComponent.h"

#include "GameFramework/SpringArmComponent.h"

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
	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(110.f);
	CharacterOwner->GetCapsuleComponent()->SetCapsuleRadius(85.f);
	CharacterOwner->GetMesh()->AddLocalOffset(FVector(15.f, -20.f, -10.f));

	
	PanWolfCharacter->GetCameraBoom()->TargetArmLength = 400.f;
	CharacterOwner->GetCharacterMovement()->bWantsToCrouch = false;

	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

	if (OwningPlayerAnimInstance)
	{
		CombatComponent->SetCombatEnabled(true, OwningPlayerAnimInstance);
	}
}

void UPanWolfComponent::Deactivate()
{
	Super::Deactivate();

	CharacterOwner->GetCharacterMovement()->JumpZVelocity = 500.f;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleRadius(35.f);

	CombatComponent->SetCombatEnabled(false,nullptr);

	PanWolfCharacter->RemoveMappingContext(PanWolfMappingContext);
}

void UPanWolfComponent::Jump()
{
	//if (IsPlayingMontage_ExcludingBlendOut()) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	CharacterOwner->Jump();
}

void UPanWolfComponent::Dodge()
{
	if (!PanWolfCharacter->CanPerformDodge()) return;

	//if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;

	if (!PandolWolfDodgeMontage) return;
	if (!OwningPlayerAnimInstance) return;
	//if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;
	//if (CharacterOwner->GetCharacterMovement()->GetLastInputVector().Length() < 0.5f) return;

	//CharacterOwner->DisableInput(CharacterOwner->GetLocalViewingPlayerController());
	//Debug::Print(TEXT("Dodge"));
	OwningPlayerAnimInstance->Montage_Play(PandolWolfDodgeMontage);
}

void UPanWolfComponent::LightAttack()
{

	/*if (CharacterOwner->GetMovementComponent()->IsFalling())
	{
		
		CombatComponent->PerformAttack(EAttackType::EAT_HeavyAttack);
		return;
	}*/

	//if (!CombatComponent->IsAttacking())
	//{
	//	//if (!PanWolfCharacter->GetTargetingComponent()->IsTargeting())
	//	//	PanWolfCharacter->GetTargetingComponent()->TryLock();

	//	const bool isTargeting = PanWolfCharacter->GetTargetingComponent()->IsActive() && PanWolfCharacter->GetTargetingComponent()->IsTargeting();
	//	const AActor* ClosestEnemy = isTargeting ? PanWolfCharacter->GetTargetingComponent()->GetTargetActor() : CombatComponent->GetClosestEnemy();
	//	//if (!ClosestEnemy) return;

	//	const bool EnemyDirection = CombatComponent->GetEnemyDirection(ClosestEnemy);
	//	CombatComponent->RotateToClosestEnemy(ClosestEnemy);
	//	EAttackType AttackType = EnemyDirection ? EAttackType::EAT_LightAttack_Right : EAttackType::EAT_LightAttack_Left;
	//	CombatComponent->PerformAttack(AttackType);
	//}
	//else
	//	CombatComponent->PerformAttack(EAttackType::EAT_LightAttack_Right);

	if (!CombatComponent) return;
	CombatComponent->PerformAttack(EAttackType::EAT_LightAttack);

}

void UPanWolfComponent::HeavyAttack()
{
	if (!CombatComponent) return;
	CombatComponent->PerformAttack(EAttackType::EAT_HeavyAttack);
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

