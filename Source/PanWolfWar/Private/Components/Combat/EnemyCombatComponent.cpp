#include "Components/Combat/EnemyCombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"

#include "PanWolfWar/DebugHelper.h"

void UEnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
}


void UEnemyCombatComponent::PerformAttack()
{

	if (!OwningPlayerAnimInstance || OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;


	if (AttackMontages.Num() == 0) return;
	const int32 RandIndex = UKismetMathLibrary::RandomIntegerInRange(0, AttackMontages.Num() - 1);
	UAnimMontage* AttackMontage = AttackMontages[RandIndex];
	if (!AttackMontage) return;

	OwningPlayerAnimInstance->Montage_Play(AttackMontage);
}

void UEnemyCombatComponent::ResetAttack()
{
	Super::ResetAttack();

}
