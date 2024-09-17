#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "EnemyCombatComponent.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FEnemyAttackMontage
{
    GENERATED_BODY()

    // Puntatore a UAnimMontage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Montages")
    UAnimMontage* AnimMontage;

    // Variabile booleana
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Montages")
    bool bIsUnblockable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Montages", meta = (EditCondition = "bIsUnblockable"))
    float UnblockableWarning_Delay;

    // Costruttore di default
    FEnemyAttackMontage()
        : AnimMontage(nullptr), bIsUnblockable(false), UnblockableWarning_Delay(0.2f)
    {
    }
};

UCLASS()
class PANWOLFWAR_API UEnemyCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:
	void PerformAttack();
	virtual void ResetAttack() override;

protected:
	virtual void BeginPlay() override;

private:
    void UnblockableAttackWarning();
    void DoUnblockableAttack(UAnimMontage* AttackMontage);

private:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Montages", meta = (AllowPrivateAccess = "true"))
    TArray<FEnemyAttackMontage> EnemyAttackMontages;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UnblockableAttack", meta = (AllowPrivateAccess = "true"))
    UNiagaraSystem* UnblockableNiagaraSystem;

	FTimerHandle UnblockableWarning_TimerHandle;

};
