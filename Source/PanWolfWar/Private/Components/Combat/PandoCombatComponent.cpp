#include "Components/Combat/PandoCombatComponent.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "PanWarFunctionLibrary.h"

#include "PanWolfWar/DebugHelper.h"

void UPandoCombatComponent::BeginPlay()
{
	PlayerController = Cast<APlayerController>(CharacterOwner->GetController());
	AttackState = EAttackState::EAS_Nothing;
}

void UPandoCombatComponent::SetCombatEnabled(UAnimInstance* PlayerAnimInstance, ETransformationCombatType TransformationCombatType)
{
	OwningPlayerAnimInstance = PlayerAnimInstance;
	if (!OwningPlayerAnimInstance) return;

	CurrentTransformationCombatType = TransformationCombatType;

	switch (CurrentTransformationCombatType)
	{
	case ETransformationCombatType::ETCT_Pandolfo:
		InitializeCombatStats(PANDO_BaseAttackDamage, PANDO_AttackPower, PANDO_DefensePower);
		break;
	case ETransformationCombatType::ETCT_PandolFlower:
		InitializeCombatStats(FLOWER_BaseAttackDamage, FLOWER_AttackPower, FLOWER_DefensePower);
		break;
	case ETransformationCombatType::ETCT_PanWolf:
		InitializeCombatStats(WOLF_BaseAttackDamage, WOLF_AttackPower, WOLF_DefensePower);
		break;
	default:
		break;
	}
}

float UPandoCombatComponent::GetDefensePower()
{
	switch (CurrentTransformationCombatType)
	{
	case ETransformationCombatType::ETCT_Pandolfo:
		return PANDO_DefensePower;
		break;
	case ETransformationCombatType::ETCT_PandolFlower:
		return FLOWER_DefensePower;
		break;
	case ETransformationCombatType::ETCT_PanWolf:
		return WOLF_DefensePower;
		break;
	default:
		break;
	}

	return 1.0f;
}

#pragma region AttackPerforming

void UPandoCombatComponent::PerformAttack(EAttackType AttackType)
{
	if (!OwningPlayerAnimInstance) return;
	if (UPanWarFunctionLibrary::IsPlayingAnyMontage_ExcludingBlendOut(OwningPlayerAnimInstance)) return;

	// Emetti la notifica in broadcast
	OnPerformAttack.Broadcast(CharacterOwner);
	CachedStunnedAttack = false;

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


#pragma region LightAttack

void UPandoCombatComponent::LightAttack()
{
	switch (CurrentTransformationCombatType)
	{
	case ETransformationCombatType::ETCT_Pandolfo:
		PandoLightAttack();
		break;
	case ETransformationCombatType::ETCT_PandolFlower:
		FlowerLightAttack();
		break;
	case ETransformationCombatType::ETCT_PanWolf:
		WolfLightAttack();
		break;
	default:
		break;
	}
}

void UPandoCombatComponent::WolfLightAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(ComboLightCountReset_TimerHandle);
	UAnimMontage* AttackMontage = *WOLF_LightAttackMontages.Find(CurrentLightAttackComboCount);
	if (!AttackMontage) return;

	AttackState = EAttackState::EAS_Attacking;
	UsedLightComboCount = CurrentLightAttackComboCount;
	CurrentLightAttackComboCount++;

	OwningPlayerAnimInstance->Montage_Play(AttackMontage);
	BindLightAttackMontageEnded(AttackMontage);
	LastAttackType = EAttackType::EAT_LightAttack;


	if (UsedLightComboCount == WOLF_LightAttackMontages.Num()) { ResetLightAttackComboCount(); return; }

	if (UsedLightComboCount == (WOLF_LightAttackMontages.Num() - 1)) { bJumpToFinisher = true; }
	else ResetHeavyAttackComboCount();
	
}

void UPandoCombatComponent::PandoLightAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(ComboLightCountReset_TimerHandle);
	UAnimMontage* AttackMontage = *PANDO_LightAttackMontages.Find(CurrentLightAttackComboCount);
	if (!AttackMontage) return;

	AttackState = EAttackState::EAS_Attacking;

	OwningPlayerAnimInstance->Montage_Play(AttackMontage);
	BindLightAttackMontageEnded(AttackMontage);
	LastAttackType = EAttackType::EAT_LightAttack;
	UsedLightComboCount = CurrentLightAttackComboCount;

	if (CurrentLightAttackComboCount == PANDO_LightAttackMontages.Num()) { ResetLightAttackComboCount(); return; }


	CurrentLightAttackComboCount++;
}

void UPandoCombatComponent::FlowerLightAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(ComboLightCountReset_TimerHandle);
	UAnimMontage* AttackMontage = *FLOWER_LightAttackMontages.Find(CurrentLightAttackComboCount);
	if (!AttackMontage) return;

	AttackState = EAttackState::EAS_Attacking;

	OwningPlayerAnimInstance->Montage_Play(AttackMontage);
	BindLightAttackMontageEnded(AttackMontage);
	LastAttackType = EAttackType::EAT_LightAttack;
	UsedLightComboCount = CurrentLightAttackComboCount;

	if (CurrentLightAttackComboCount == FLOWER_LightAttackMontages.Num()) { ResetLightAttackComboCount(); return; }


	CurrentLightAttackComboCount++;
}

#pragma endregion

#pragma region HeavyAttack

void UPandoCombatComponent::HeavyAttack()
{
	switch (CurrentTransformationCombatType)
	{
	case ETransformationCombatType::ETCT_Pandolfo:
		break;
	case ETransformationCombatType::ETCT_PandolFlower:
		break;
	case ETransformationCombatType::ETCT_PanWolf:
		WolfHeavyAttack();
		break;
	default:
		break;
	}
}

void UPandoCombatComponent::WolfHeavyAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(ComboHeavyCountReset_TimerHandle);
	if (bJumpToFinisher) { CurrentHeavyAttackComboCount = WOLF_HeavyAttackMontages.Num(); }
	UAnimMontage* AttackMontage = *WOLF_HeavyAttackMontages.Find(CurrentHeavyAttackComboCount);
	if (!AttackMontage) return;

	AttackState = EAttackState::EAS_Attacking;
	UsedHeavyComboCount = CurrentHeavyAttackComboCount;
	CurrentHeavyAttackComboCount++;

	OwningPlayerAnimInstance->Montage_Play(AttackMontage);
	BindHeavyAttackMontageEnded(AttackMontage);
	LastAttackType = EAttackType::EAT_HeavyAttack;
	

	if (UsedHeavyComboCount < (WOLF_LightAttackMontages.Num()-1)) { CurrentLightAttackComboCount = 2; }
	else ResetLightAttackComboCount(); 

	if (UsedHeavyComboCount == WOLF_HeavyAttackMontages.Num()) { ResetHeavyAttackComboCount(); return; }



}

void UPandoCombatComponent::Counterattack()
{
	if (!OwningPlayerAnimInstance) return;
	if (UPanWarFunctionLibrary::IsPlayingAnyMontage_ExcludingBlendOut(OwningPlayerAnimInstance)) return;

	AttackState = EAttackState::EAS_Attacking;
	CachedStunnedAttack = true;
	OwningPlayerAnimInstance->Montage_Play(WOLF_CounterattackMontage);
	BindCounterAttackMontageEnded(WOLF_CounterattackMontage);

	CurrentLightAttackComboCount = 2;
}

#pragma endregion

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

float UPandoCombatComponent::CalculateFinalDamage(float BaseDamage, float TargetDefensePower)
{
	if (LastAttackType == EAttackType::EAT_LightAttack)
	{
		const float DamageIncreasePercentLight = (UsedLightComboCount - 1)  * 0.05f + 1.f ;

		BaseDamage *= DamageIncreasePercentLight;
	}

	else if (LastAttackType == EAttackType::EAT_HeavyAttack)
	{
		const float DamageIncreasePercentHeavy = UsedHeavyComboCount * 0.15f + 1.f;

		BaseDamage *= DamageIncreasePercentHeavy;
	}

	const float FinalDamageDone = BaseDamage * AttackPower / TargetDefensePower;

	return FinalDamageDone;
}

#pragma endregion

#pragma region HitActor

bool UPandoCombatComponent::ExecuteHitActor(FHitResult& Hit)
{
	if (!Super::ExecuteHitActor(Hit)) return false;

	/** HitPause */

	GetWorld()->GetTimerManager().ClearTimer(HitPause_TimerHandle);
	UGameplayStatics::SetGlobalTimeDilation(this, 0.3f);
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

	CurrentLightAttackComboCount = 1;
	CurrentHeavyAttackComboCount = 1;
	bJumpToFinisher = false;

	GetWorld()->GetTimerManager().ClearTimer(ComboLightCountReset_TimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ComboHeavyCountReset_TimerHandle);
}

#pragma region AttackMontageEnded

void UPandoCombatComponent::BindLightAttackMontageEnded(UAnimMontage* Montage)
{
	FOnMontageEnded AttackMontageEndedDelegate;
	AttackMontageEndedDelegate.BindUObject(this, &UPandoCombatComponent::OnLightAttackMontageEnded);
	OwningPlayerAnimInstance->Montage_SetEndDelegate(AttackMontageEndedDelegate, Montage);
}

void UPandoCombatComponent::BindHeavyAttackMontageEnded(UAnimMontage* Montage)
{
	FOnMontageEnded AttackMontageEndedDelegate;
	AttackMontageEndedDelegate.BindUObject(this, &UPandoCombatComponent::OnHeavyAttackMontageEnded);
	OwningPlayerAnimInstance->Montage_SetEndDelegate(AttackMontageEndedDelegate, Montage);
}

void UPandoCombatComponent::BindCounterAttackMontageEnded(UAnimMontage* Montage)
{
	FOnMontageEnded AttackMontageEndedDelegate;
	AttackMontageEndedDelegate.BindUObject(this, &UPandoCombatComponent::OnCounterAttackMontageEnded);
	OwningPlayerAnimInstance->Montage_SetEndDelegate(AttackMontageEndedDelegate, Montage);
}

void UPandoCombatComponent::OnLightAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;

	AttackState = EAttackState::EAS_Nothing;
	if (bInterrupted) { return; }
	GetWorld()->GetTimerManager().SetTimer(ComboLightCountReset_TimerHandle, [this]() {this->ResetLightAttackComboCount(); }, 0.025f, false);
}

void UPandoCombatComponent::OnHeavyAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;

	AttackState = EAttackState::EAS_Nothing;
	if (bInterrupted) return;
	GetWorld()->GetTimerManager().SetTimer(ComboHeavyCountReset_TimerHandle, [this]() {this->ResetHeavyAttackComboCount(); }, 0.025f, false);
}

void UPandoCombatComponent::OnCounterAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;

	AttackState = EAttackState::EAS_Nothing;
	GetWorld()->GetTimerManager().SetTimer(ComboLightCountReset_TimerHandle, [this]() {this->ResetLightAttackComboCount(); }, 0.025f, false);
}

#pragma endregion
