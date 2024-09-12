// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/BaseEnemy.h"
#include "Interfaces/BossInterface.h"
#include "BaseBoss.generated.h"

/**
 * 
 */
UCLASS()
class PANWOLFWAR_API ABaseBoss : public ABaseEnemy , public IBossInterface
{
	GENERATED_BODY()
	
public:

	//~ Begin IBossInterface Interface.
	virtual void SummonsEnemies() override;
	virtual void SummonsEnemies_Notify() override;
	//~ End IBossInterface Interface

protected:
	virtual void BeginPlay() override;


private:

	#pragma region SummonEnemies

	void OnSummonEnemyClassLoaded();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SummonEnemies", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* SummonEnemiesMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SummonEnemies", meta = (AllowPrivateAccess = "true"))
	TSoftClassPtr<ABaseEnemy> SoftEnemyClassToSpawn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SummonEnemies", meta = (AllowPrivateAccess = "true"))
	int32 N_EnemyToSpawn = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SummonEnemies", meta = (AllowPrivateAccess = "true"))
	float RandomSpawmRadius = 200.f;

	#pragma endregion
};
