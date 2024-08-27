#include "Components/Combat/PandoCombatComponent.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

#include "PanWolfWar/DebugHelper.h"

void UPandoCombatComponent::BeginPlay()
{
	PlayerController = Cast<APlayerController>(CharacterOwner->GetController());
	
}

void UPandoCombatComponent::HandleNewAnimInstance()
{
	if (!OwningPlayerAnimInstance) return;
	OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UPandoCombatComponent::OnAttackMontageEnded);
}

#pragma region AttackPerforming

void UPandoCombatComponent::PerformAttack(EAttackType AttackType)
{
	if (!OwningPlayerAnimInstance) return;
	if (IsPlayingMontage_ExcludingBlendOut()) return;

	switch (AttackType)
	{
	case EAttackType::EAT_LightAttack:
		LightAttack();
		break;
	case EAttackType::EAT_HeavyAttack:
		HeavyAttack();
		break;
	default:
		break;
	}

}

void UPandoCombatComponent::LightAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(ComboLightCountReset_TimerHandle);
	UAnimMontage* AttackMontage = *LightAttackMontages.Find(CurrentLightAttackComboCount);
	if (!AttackMontage) return;

	OwningPlayerAnimInstance->Montage_Play(AttackMontage);

	if (CurrentLightAttackComboCount == LightAttackMontages.Num()) { ResetLightAttackComboCount(); return; }

	if (CurrentLightAttackComboCount == (LightAttackMontages.Num() - 1)) { bJumpToFinisher = true; }
	else ResetHeavyAttackComboCount();

	CurrentLightAttackComboCount++;
}

void UPandoCombatComponent::HeavyAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(ComboHeavyCountReset_TimerHandle);
	if (bJumpToFinisher) { CurrentHeavyAttackComboCount = HeavyAttackMontages.Num(); }
	UAnimMontage* AttackMontage = *HeavyAttackMontages.Find(CurrentHeavyAttackComboCount);
	if (!AttackMontage) return;

	OwningPlayerAnimInstance->Montage_Play(AttackMontage);

	ResetLightAttackComboCount();

	if (CurrentHeavyAttackComboCount == HeavyAttackMontages.Num()) { ResetHeavyAttackComboCount(); return; }


	CurrentHeavyAttackComboCount++;
}

void UPandoCombatComponent::ResetLightAttackComboCount()
{
	CurrentLightAttackComboCount = 1;
	bJumpToFinisher = false;
}

void UPandoCombatComponent::ResetHeavyAttackComboCount()
{
	CurrentHeavyAttackComboCount = 1;
	bJumpToFinisher = false;
}

void UPandoCombatComponent::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{

	if (LightAttackMontages.FindKey(Montage))
	{
		if (bInterrupted) return;
		GetWorld()->GetTimerManager().SetTimer(ComboLightCountReset_TimerHandle, [this]() {this->ResetLightAttackComboCount(); }, 0.025f, false);
	}

	else if (HeavyAttackMontages.FindKey(Montage))
	{
		if (bInterrupted) return;
		GetWorld()->GetTimerManager().SetTimer(ComboHeavyCountReset_TimerHandle, [this]() {this->ResetHeavyAttackComboCount(); }, 0.025f, false);
	}
}

#pragma endregion

#pragma region HitActor

bool UPandoCombatComponent::ExecuteHitActor(FHitResult& Hit)
{
	if (!Super::ExecuteHitActor(Hit)) return false;

	/** HitPause */

	GetWorld()->GetTimerManager().ClearTimer(HitPause_TimerHandle);
	UGameplayStatics::SetGlobalTimeDilation(this, 0.1f);
	GetWorld()->GetTimerManager().SetTimer(HitPause_TimerHandle, [this]() {this->HitPause(); }, 0.025f, false);

	return true;
}

void UPandoCombatComponent::HitPause()
{
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);

	if (!PlayerController) return;
	PlayerController->ClientStartCameraShake(CameraShake_CombatWolf);
}

#pragma endregion

void UPandoCombatComponent::ResetAttack()
{
	Super::ResetAttack();

	Debug::Print(TEXT("RsetAttack"));
}