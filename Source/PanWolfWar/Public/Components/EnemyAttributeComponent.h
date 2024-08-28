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

	float GetHealthPercent();
	bool IsAlive();
	void AddHealth(float healthToAdd);
	void ReceiveDamage(float Damage);

protected:
	virtual void BeginPlay() override;

#pragma region Variables

	// Health
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth;

#pragma endregion

		
};
