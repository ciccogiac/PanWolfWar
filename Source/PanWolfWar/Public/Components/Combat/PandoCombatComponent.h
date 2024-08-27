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

UCLASS()
class PANWOLFWAR_API UPandoCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()
	
public:

	void PerformAttack(EAttackType AttackType);
	virtual void ResetAttack() override;

protected:
	virtual void BeginPlay();

private:
	virtual bool ExecuteHitActor(FHitResult& Hit) override;
	void HitPause();


	void LightAttack();
	void HeavyAttack();
	void ResetLightAttackComboCount();
	void ResetHeavyAttackComboCount();

	virtual void HandleNewAnimInstance();

	UFUNCTION()
	void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	APlayerController* PlayerController = nullptr;
	FTimerHandle HitPause_TimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TSubclassOf<UCameraShakeBase> CameraShake_CombatWolf;

	int32 CurrentLightAttackComboCount = 1;
	int32 CurrentHeavyAttackComboCount = 1;
	bool bJumpToFinisher = false;
	FTimerHandle ComboLightCountReset_TimerHandle;
	FTimerHandle ComboHeavyCountReset_TimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages", meta = (AllowPrivateAccess = "true"))
	TMap<int32, UAnimMontage*> LightAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages", meta = (AllowPrivateAccess = "true"))
	TMap<int32, UAnimMontage*> HeavyAttackMontages;
};
