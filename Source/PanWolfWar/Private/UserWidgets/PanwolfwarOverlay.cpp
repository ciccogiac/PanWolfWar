// Fill out your copyright notice in the Description page of Project Settings.


#include "UserWidgets/PanwolfwarOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPanwolfwarOverlay::SetHealthBarPercent(float Percent)
{
	if (HealthProgressBar)
	{
		HealthProgressBar->SetPercent(Percent);
	}
}

void UPanwolfwarOverlay::SetFlowerStaminaBarPercent(float Percent)
{
	if (FlowerStaminaProgressBar)
	{
		FlowerStaminaProgressBar->SetPercent(Percent);
	}
}

void UPanwolfwarOverlay::SetBeerBarVisibility(bool bVisibility)
{
	Beer_ProgressBar->SetVisibility(bVisibility ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UPanwolfwarOverlay::SetBeerBarPercent(float Percent)
{
	if (Beer_ProgressBar)
	{
		Beer_ProgressBar->SetPercent(Percent);
	}
}

void UPanwolfwarOverlay::SetBeers(int32 Beer)
{
	if (BeerText)
	{
		const FString String = FString::Printf(TEXT("%d"), Beer);
		const FText Text = FText::FromString(String);
		BeerText->SetText(Text);
	}
}