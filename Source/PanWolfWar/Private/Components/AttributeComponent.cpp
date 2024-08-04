#include "Components/AttributeComponent.h"
#include "UserWidgets/PanwolfwarOverlay.h"

#pragma region EngineFunctions

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	//Add Overlay
	UWorld* World = GetWorld();
	if (World)
	{
		if (PanwolfwarOverlayClass)
		{
			PanwolfwarOverlay = CreateWidget<UPanwolfwarOverlay>(World, PanwolfwarOverlayClass);
			PanwolfwarOverlay->AddToViewport();
		}
	}

	if (PanwolfwarOverlayClass)
	{
		PanwolfwarOverlay->SetBeers(Beers);
		PanwolfwarOverlay->SetHealthBarPercent(0.f);
		PanwolfwarOverlay->SetFlowerStaminaBarPercent(FlowerStamina);
		PanwolfwarOverlay->SetBirdStaminaBarPercent(BirdStamina);

	    Beers > 0 ? PanwolfwarOverlay->SetBeerBarPercent(1.f) : PanwolfwarOverlay->SetBeerBarPercent(0.f);
		
	}

}

#pragma endregion

#pragma region Health

void UAttributeComponent::ReceiveDamage(float Damage)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	if (PanwolfwarOverlay)
	{
		PanwolfwarOverlay->SetHealthBarPercent(1 - GetHealthPercent());
	}
}

float UAttributeComponent::GetHealthPercent()
{
	return Health / MaxHealth;
}


bool UAttributeComponent::IsAlive()
{
	return Health > 0.f;
}

void UAttributeComponent::AddHealth(float healthToAdd)
{
	Health = FMath::Clamp(Health + healthToAdd, 0.f, MaxHealth);
}

#pragma endregion

#pragma region Wolf

bool UAttributeComponent::IsBeerInventoryFull()
{
	return Beers >= MaxBeers;
}

void UAttributeComponent::AddBeers(int32 NumberOfBeers)
{
	Beers = FMath::Clamp(Beers + NumberOfBeers, 0.f, MaxBeers);

	if (PanwolfwarOverlay)
	{
		PanwolfwarOverlay->SetBeers(Beers);
		PanwolfwarOverlay->SetBeerBarPercent(1);
	}
		

}

bool UAttributeComponent::ConsumeBeer()
{
	if (Beers < 0) return false;

	if (BeerConsuming < 8.f && Beers >0)
	{
		Beers = FMath::Clamp(Beers - 1, 0.f, MaxHealth);
		BeerConsuming = BeerConsumingMAX;
	}
	else if (BeerConsuming < 8.f &&  Beers == 0)
		return false;
		

	if (PanwolfwarOverlay)
	{
		PanwolfwarOverlay->SetBeers(Beers);
	}

	return true;
}

bool UAttributeComponent::ConsumingBeer()
{
		BeerConsuming = FMath::Clamp(BeerConsuming - BeerConsumingRate, 0.f, BeerConsumingMAX);
		PanwolfwarOverlay->SetBeerBarPercent(BeerConsuming / BeerConsumingMAX);

		if (BeerConsuming <= 0.f)
		{
			if (Beers > 0) { PanwolfwarOverlay->SetBeerBarPercent(1); }
			return false;
		}

		return true;
}

void UAttributeComponent::AddBeerStamina(float Value)
{
	BeerConsuming = FMath::Clamp(BeerConsuming + Value, 0.f, BeerConsumingMAX);
	PanwolfwarOverlay->SetBeerBarPercent(BeerConsuming / BeerConsumingMAX);
}

#pragma endregion

#pragma region Flower

float UAttributeComponent::GetFlowerStaminaPercent()
{
	return FlowerStamina / MaxFlowerStamina;
}

bool UAttributeComponent::ConsumeFlowerStamina()
{
	//if (FlowerStamina < MaxFlowerStamina) return false;

	if (FlowerStamina <= 0) return false;

	return true;

}

void UAttributeComponent::RegenFlowerStamina()
{
	FlowerStamina = FMath::Clamp(FlowerStamina + FlowerStaminaRegenRate , 0.f, MaxFlowerStamina);
	PanwolfwarOverlay->SetFlowerStaminaBarPercent(FlowerStamina / MaxFlowerStamina);
}

bool UAttributeComponent::ConsumingFlowerStamina()
{
	FlowerStamina = FMath::Clamp(FlowerStamina - FlowerStaminaCost , 0.f, MaxFlowerStamina);
	PanwolfwarOverlay->SetFlowerStaminaBarPercent(FlowerStamina / MaxFlowerStamina);

	if (FlowerStamina <= 0.f)
	{
		return false;
	}

	return true;
}

void UAttributeComponent::AddFlowerStamina(float Value)
{
	FlowerStamina = FMath::Clamp(FlowerStamina + Value, 0.f, MaxFlowerStamina);
	PanwolfwarOverlay->SetFlowerStaminaBarPercent(FlowerStamina / MaxFlowerStamina);
}

#pragma endregion

#pragma region Bird

float UAttributeComponent::GetBirdStaminaPercent()
{
	return BirdStamina / MaxBirdStamina;
}

bool UAttributeComponent::ConsumeBirdStamina()
{
	if (BirdStamina < MaxBirdStamina) return false;

	return true;
}

void UAttributeComponent::RegenBirdStamina()
{
	BirdStamina = FMath::Clamp(BirdStamina + BirdStaminaRegenRate, 0.f, MaxBirdStamina);
	PanwolfwarOverlay->SetBirdStaminaBarPercent(BirdStamina / MaxBirdStamina);
}

bool UAttributeComponent::ConsumingBirdStamina()
{
	BirdStamina = FMath::Clamp(BirdStamina - BirdStaminaCost, 0.f, MaxBirdStamina);
	PanwolfwarOverlay->SetBirdStaminaBarPercent(BirdStamina / MaxBirdStamina);

	if (BirdStamina <= 0.f)
	{
		return false;
	}

	return true;
}

void UAttributeComponent::AddBirdStamina(float Value)
{
	BirdStamina = FMath::Clamp(BirdStamina + Value, 0.f, MaxBirdStamina);
	PanwolfwarOverlay->SetBirdStaminaBarPercent(BirdStamina / MaxBirdStamina);
}
#pragma endregion


void UAttributeComponent::SetTransformationIcon(bool bVisibility, bool bRight)
{
	PanwolfwarOverlay->SetTransformationIcon(bVisibility, bRight);
}


