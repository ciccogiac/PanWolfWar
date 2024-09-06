#include "Components/Combat/EnemyCombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"

#include "PanWolfWar/DebugHelper.h"

void UEnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
}


void UEnemyCombatComponent::PerformAttack(bool bIsUnblockableAttack)
{

	if (!OwningPlayerAnimInstance || OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	

	if (AttackMontages.Num() == 0) return;
	const int32 RandIndex = UKismetMathLibrary::RandomIntegerInRange(0, AttackMontages.Num() - 1);
	UAnimMontage* AttackMontage = AttackMontages[RandIndex];
	if (!AttackMontage) return;

	if(bIsUnblockableAttack)
		GetWorld()->GetTimerManager().SetTimer(UnblockableWarning_TimerHandle, [this, AttackMontage]() {this->OwningPlayerAnimInstance->Montage_Play(AttackMontage); }, UnblockableWarning_Delay, false);
	else
		OwningPlayerAnimInstance->Montage_Play(AttackMontage);
}


void UEnemyCombatComponent::ResetAttack()
{
	Super::ResetAttack();

}
