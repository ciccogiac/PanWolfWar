// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AnimInstances/PanWarCharacterAnimInstance.h"
#include "PanWarBaseEnemyAnimInstance.generated.h"

class ABaseEnemy;
class UCharacterMovementComponent;
enum class EEnemyState : uint8;
/**
 * 
 */
UCLASS()
class PANWOLFWAR_API UPanWarBaseEnemyAnimInstance : public UPanWarCharacterAnimInstance
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

protected:
	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	bool IsEnemyInState(EEnemyState StateToCheck) const;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Components")
	ABaseEnemy* OwningEnemyCharacter;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	float RotationIdle = 0.f;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|LocomotionData")
	bool bShouldMove;

private:
	float LastRotationIdle = 0.f;

};
