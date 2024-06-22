// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

class UPanwolfwarOverlay;
class UTransformationComponent;

UENUM(BlueprintType)
enum class EAttributeState : uint8
{
	EAS_Normal UMETA(DisplayName = "Normal"),
	EAS_ConsumingBeer UMETA(DisplayName = "ConsumingBeer"),
	EAS_ConsumingFlower UMETA(DisplayName = "ConsumingFlower"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	void RegenFlowerStamina(float DeltaTime);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	EAttributeState AttributeState = EAttributeState::EAS_Normal;

	//Current Health
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth;

	//Current Stamina
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float FlowerStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxFlowerStamina;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float FlowerStaminaRegenRate = 8.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 Beers;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	int32 MaxBeers;

	float BeerConsuming = 1.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float BeerConsumingRate = 4.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float BeerConsumingMAX = 100.f;

	
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float FlowerStaminaCost = 4.f;

	UPanwolfwarOverlay* PanwolfwarOverlay;

	UPROPERTY(EditDefaultsOnly, Category = HUD)
	TSubclassOf<UPanwolfwarOverlay> PanwolfwarOverlayClass;

	UTransformationComponent* TransformationComponent;

	bool bCanRegenFlower = false;

public:
	void ReceiveDamage(float Damage);
	bool ConsumeFlowerStamina();
	float GetHealthPercent();
	float GetFlowerStaminaPercent();
	bool IsAlive();
	void AddBeers(int32 NumberOfBeers);
	bool ConsumeBeer();
	void AddHealth(float healthToAdd);

	FORCEINLINE int32 GetBeers() const { return Beers; }
	FORCEINLINE float GetFlowerStamina() const { return FlowerStamina; }

	FORCEINLINE void SetTransformationComponent(UTransformationComponent* _TransformationComponent) { TransformationComponent = _TransformationComponent; }
	FORCEINLINE bool IsInConsumingState() const { return AttributeState != EAttributeState::EAS_Normal; }

	UFUNCTION(BlueprintCallable, Category = "attribute")
	FORCEINLINE void SetCanRegenFlower(bool Value) { bCanRegenFlower = Value; }
		
};
