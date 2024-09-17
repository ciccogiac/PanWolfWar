#include "Components/Combat/EnemyCombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "PanWolfWar/DebugHelper.h"

void UEnemyCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	
	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
}

void UEnemyCombatComponent::PerformAttack()
{

	if (!OwningPlayerAnimInstance || OwningPlayerAnimInstance->IsAnyMontagePlaying() || CharacterOwner->ActorHasTag("Dead") || EnemyAttackMontages.Num() == 0) return;

	const int32 RandIndex = UKismetMathLibrary::RandomIntegerInRange(0, EnemyAttackMontages.Num() - 1);
	FEnemyAttackMontage EnemyAttackMontage = EnemyAttackMontages[RandIndex];
	UAnimMontage* AttackMontage = EnemyAttackMontage.AnimMontage;
	if (!AttackMontage) return;

	if (EnemyAttackMontage.bIsUnblockable)
	{
		UnblockableAttackWarning();
		GetWorld()->GetTimerManager().SetTimer(UnblockableWarning_TimerHandle, [this, AttackMontage]() { DoUnblockableAttack(AttackMontage); }, EnemyAttackMontage.UnblockableWarning_Delay, false);
	}
	else
	{
		CachedUnblockableAttack = false;
		OwningPlayerAnimInstance->Montage_Play(AttackMontage);
	}

}

void UEnemyCombatComponent::ResetAttack()
{
	Super::ResetAttack();

}

void UEnemyCombatComponent::DoUnblockableAttack(UAnimMontage* AttackMontage)
{
	if (!OwningPlayerAnimInstance || OwningPlayerAnimInstance->IsAnyMontagePlaying() || CharacterOwner->ActorHasTag("Dead")) return;
	CachedUnblockableAttack = true;
	OwningPlayerAnimInstance->Montage_Play(AttackMontage);
}

void UEnemyCombatComponent::UnblockableAttackWarning()
{
	FVector EyeLocation; FRotator EyeRotation;
	CharacterOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	const FVector Location = EyeLocation + CharacterOwner->GetActorForwardVector() * 100.f;
	const FRotator Rotation = UKismetMathLibrary::MakeRotFromX(CharacterOwner->GetActorForwardVector());

	UNiagaraComponent* UnblockableNiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, UnblockableNiagaraSystem, Location, Rotation);
	
	FAttachmentTransformRules AttachmentTransformRules(EAttachmentRule::KeepRelative, true);
	UnblockableNiagaraComponent->AttachToComponent(CharacterOwner->GetRootComponent(), AttachmentTransformRules);
	
}

