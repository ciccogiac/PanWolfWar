// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PanwolfwarOverlay.generated.h"

/**
 * 
 */
UCLASS()
class PANWOLFWAR_API UPanwolfwarOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetHealthBarPercent(float Percent);
	void SetFlowerStaminaBarPercent(float Percent);
	void SetBirdStaminaBarPercent(float Percent);
	void SetBeerBarVisibility(bool bVisibility);
	void SetBeerBarPercent(float Percent);
	void SetBeers(int32 Beers);

private:

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* FlowerStaminaProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* BirdStaminaProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* BeerText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* Beer_ProgressBar;
	
};
