#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "EnemyCombatComponent.generated.h"


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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages", meta = (AllowPrivateAccess = "true"))
	TArray<UAnimMontage*> AttackMontages;
};
