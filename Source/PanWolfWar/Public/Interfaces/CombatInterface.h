#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

class UPawnCombatComponent;

//UINTERFACE(MinimalAPI, Blueprintable)
UINTERFACE(MinimalAPI,BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UCombatInterface : public UInterface
{
	GENERATED_BODY()

};


class PANWOLFWAR_API ICombatInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual UPawnCombatComponent* GetCombatComponent() const = 0;
	UFUNCTION(BlueprintCallable)
	virtual void SetInvulnerability(bool NewInvulnerability) = 0;
	UFUNCTION(BlueprintCallable)
	virtual FRotator GetDesiredDodgeRotation() = 0;
	UFUNCTION(BlueprintCallable)
	virtual bool IsCombatActorAlive() = 0;
	UFUNCTION(BlueprintCallable)
	virtual float PerformAttack() = 0;
	UFUNCTION(BlueprintCallable)
	virtual void CancelAttack();
	UFUNCTION(BlueprintCallable)
	virtual void AttackWarning() ;
	UFUNCTION(BlueprintCallable)
	virtual bool IsUnderAttack() = 0;
	UFUNCTION(BlueprintCallable)
	virtual void SetUnderAttack() = 0;
	UFUNCTION(BlueprintCallable)
	virtual float GetDefensePower() = 0;
	UFUNCTION(BlueprintCallable)
	virtual void OnDeathEnter() = 0;
	UFUNCTION(BlueprintCallable)
	virtual void ShortStunned();
	UFUNCTION(BlueprintCallable)
	virtual void LongStunned();
	UFUNCTION(BlueprintCallable)
	virtual bool IsStunned();
	UFUNCTION(BlueprintCallable)
	virtual void Block() ;
	UFUNCTION(BlueprintCallable)
	virtual void UnBlock();
	UFUNCTION(BlueprintCallable)
	virtual bool IsBlocking() = 0;
	UFUNCTION(BlueprintCallable)
	virtual bool IsBlockingAttackRecently();
	UFUNCTION(BlueprintCallable)
	virtual bool IsBlockingCharged() ;
	UFUNCTION(BlueprintCallable)
	virtual void SuccesfulBlock(AActor* Attacker) = 0;
	UFUNCTION(BlueprintCallable)
	virtual void FireProjectile() ;
	UFUNCTION(BlueprintCallable)
	virtual float GetHealthPercent() = 0;

	UFUNCTION(BlueprintCallable)
	virtual void AssassinationKilled();

	virtual bool IsValidBlock(AActor* InAttacker, AActor* InDefender) = 0;

	UFUNCTION(BlueprintCallable)
	virtual bool IsDodging();
};
