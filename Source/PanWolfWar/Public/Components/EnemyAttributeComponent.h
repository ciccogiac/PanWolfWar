// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EnemyAttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UEnemyAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEnemyAttributeComponent();

	void InitializeAttributeStats(float _MaxHealth , float _StoneSpawnProbability);
	float GetHealthPercent();
	bool IsAlive();
	void AddHealth(float healthToAdd);
	void ReceiveDamage(float Damage);

protected:
	virtual void BeginPlay() override;

#pragma region Variables

	// Health
	float CurrentHealth = 100.f;
	float MaxHealth = 100.f;

	// StoneSpawn
	float StoneSpawnProbability = 0.6f;

#pragma endregion

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetStoneSpawnProbability() const { return StoneSpawnProbability; }
		
};
