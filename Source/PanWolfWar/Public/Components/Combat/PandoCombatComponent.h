#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "PandoCombatComponent.generated.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	EAT_LightAttack UMETA(DisplayName = "LightAttack"),
	EAT_HeavyAttack UMETA(DisplayName = "HeavyAttack")
};

UENUM(BlueprintType)
enum class ETransformationCombatType : uint8
{
	ETCT_Pandolfo UMETA(DisplayName = "PandolfoCombat"),
	ETCT_PandolFlower UMETA(DisplayName = "PandolFlowerCombat"),
	ETCT_PanWolf UMETA(DisplayName = "PanWolfCombat")
};

UENUM(BlueprintType)
enum class EAttackState : uint8
{
	EAS_Nothing UMETA(DisplayName = "Nothing"),
	EAS_Attacking UMETA(DisplayName = "Attacking")
};

UCLASS()
class PANWOLFWAR_API UPandoCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
	
public:

	void PerformAttack(EAttackType AttackType);
	void SetCombatEnabled(UAnimInstance* PlayerAnimInstance, ETransformationCombatType TransformationCombatType);
	float GetDefensePower();
	virtual void ResetAttack() override;

	void Counterattack();

protected:
	virtual void BeginPlay();

private:
	virtual bool ExecuteHitActor(FHitResult& Hit) override;
	void HitPause();

	void LightAttack();
	void WolfLightAttack();
	void PandoLightAttack();
	void FlowerLightAttack();

	void HeavyAttack();
	void WolfHeavyAttack();

	void ResetLightAttackComboCount();
	void ResetHeavyAttackComboCount();

	virtual float CalculateFinalDamage(float BaseDamage, float TargetDefensePower) override;

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	APlayerController* PlayerController = nullptr;
	ETransformationCombatType CurrentTransformationCombatType = ETransformationCombatType::ETCT_Pandolfo;

	FTimerHandle HitPause_TimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TSubclassOf<UCameraShakeBase> CameraShake_CombatWolf;

	int32 CurrentLightAttackComboCount = 1;
	int32 CurrentHeavyAttackComboCount = 1;
	bool bJumpToFinisher = false;
	FTimerHandle ComboLightCountReset_TimerHandle;
	FTimerHandle ComboHeavyCountReset_TimerHandle;

	EAttackState AttackState = EAttackState::EAS_Nothing;

#pragma region CombatStats

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats | WOLF", meta = (AllowPrivateAccess = "true"))
	float WOLF_BaseAttackDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats | WOLF", meta = (AllowPrivateAccess = "true"))
	float WOLF_AttackPower = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats | WOLF", meta = (AllowPrivateAccess = "true"))
	float WOLF_DefensePower = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats | PANDO", meta = (AllowPrivateAccess = "true"))
	float PANDO_BaseAttackDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats | PANDO", meta = (AllowPrivateAccess = "true"))
	float PANDO_AttackPower = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats | PANDO", meta = (AllowPrivateAccess = "true"))
	float PANDO_DefensePower = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats | FLOWER", meta = (AllowPrivateAccess = "true"))
	float FLOWER_BaseAttackDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats | FLOWER", meta = (AllowPrivateAccess = "true"))
	float FLOWER_AttackPower = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats | FLOWER", meta = (AllowPrivateAccess = "true"))
	float FLOWER_DefensePower = 4.f;

#pragma endregion

#pragma region Montages

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages | WOLF", meta = (AllowPrivateAccess = "true"))
	TMap<int32, UAnimMontage*> WOLF_LightAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages | WOLF", meta = (AllowPrivateAccess = "true"))
	TMap<int32, UAnimMontage*>  WOLF_HeavyAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages | WOLF", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* WOLF_CounterattackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages | PANDO", meta = (AllowPrivateAccess = "true"))
	TMap<int32, UAnimMontage*> PANDO_LightAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages | FLOWER", meta = (AllowPrivateAccess = "true"))
	TMap<int32, UAnimMontage*> FLOWER_LightAttackMontages;

#pragma endregion


	EAttackType LastAttackType;
	int32 UsedLightComboCount;
	int32 UsedHeavyComboCount;

public:
	FORCEINLINE EAttackState GetAttackState() const { return AttackState; }
	FORCEINLINE bool IsAttacking() const { return AttackState == EAttackState::EAS_Attacking; }
};
