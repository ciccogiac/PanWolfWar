// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

class UPanwolfwarOverlay;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();

	float GetHealthPercent();
	bool IsAlive();
	void AddHealth(float healthToAdd);
	void ReceiveDamage(float Damage);

	void AddBeers(int32 NumberOfBeers);
	bool ConsumeBeer();
	bool ConsumingBeer();

	float GetFlowerStaminaPercent();
	bool ConsumeFlowerStamina();
	bool ConsumingFlowerStamina();
	void RegenFlowerStamina();

	float GetBirdStaminaPercent();
	bool ConsumeBirdStamina();
	bool ConsumingBirdStamina();
	void RegenBirdStamina();

protected:
	virtual void BeginPlay() override;

private:

	#pragma region Variables

	// Health
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth;

	//Flower Stamina
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float FlowerStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxFlowerStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float FlowerStaminaRegenRate = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float FlowerStaminaCost = 0.25f;

	//Bird Stamina
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float BirdStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxBirdStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float BirdStaminaRegenRate = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float BirdStaminaCost = 0.25f;

	//Beers
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Beers;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 MaxBeers;

	float BeerConsuming = 1.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float BeerConsumingRate = 0.25f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float BeerConsumingMAX = 100.f;

	


	#pragma endregion

	#pragma region HUD

	UPanwolfwarOverlay* PanwolfwarOverlay;

	UPROPERTY(EditDefaultsOnly, Category = HUD)
	TSubclassOf<UPanwolfwarOverlay> PanwolfwarOverlayClass;

	#pragma endregion

		
};
