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
	virtual void ActivateCollision(FString CollisionPart, bool bIsUnblockableAttack = false) = 0;
	UFUNCTION(BlueprintCallable)
	virtual void DeactivateCollision(FString CollisionPart) = 0;
	UFUNCTION(BlueprintCallable)
	virtual void SetInvulnerability(bool NewInvulnerability) = 0;
	UFUNCTION(BlueprintCallable)
	virtual FRotator GetDesiredDodgeRotation() = 0;
	UFUNCTION(BlueprintCallable)
	virtual bool IsCombatActorAlive() = 0;
	UFUNCTION(BlueprintCallable)
	virtual float PerformAttack() = 0;
	UFUNCTION(BlueprintCallable)
	virtual bool IsUnderAttack() = 0;
	UFUNCTION(BlueprintCallable)
	virtual void SetUnderAttack() = 0;
	UFUNCTION(BlueprintCallable)
	virtual float GetDefensePower() = 0;
	UFUNCTION(BlueprintCallable)
	virtual void OnDeathEnter() = 0;
	UFUNCTION(BlueprintCallable)
	virtual bool IsBlocking() = 0;
	UFUNCTION(BlueprintCallable)
	virtual void SuccesfulBlock(AActor* Attacker) = 0;
	UFUNCTION(BlueprintCallable)
	virtual void FireProjectile() ;
	UFUNCTION(BlueprintCallable)
	virtual float GetHealthPercent() = 0;
};
