#include "Components/AttributeComponent.h"

#include "Components/UI/PandoUIComponent.h"

#pragma region EngineFunctions

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAttributeComponent::InitializeAttributeUI(UPandoUIComponent* _PandoUIComponent)
{
	if (!_PandoUIComponent) return;

	PandoUIComponent = _PandoUIComponent;

	if (PandoUIComponent)
	{
		PandoUIComponent->OnCurrentHealthChanged.Broadcast(0.f);
		PandoUIComponent->OnCurrentBeerCounterChanged.Broadcast(Beers);
		Beers > 0 ? PandoUIComponent->OnCurrentBeerPercentChanged.Broadcast(1.f) : PandoUIComponent->OnCurrentBeerPercentChanged.Broadcast(0.f);
		PandoUIComponent->OnCurrentFlowerPercentChanged.Broadcast(FlowerStamina);
	}
}

#pragma endregion

#pragma region Health

void UAttributeComponent::ReceiveDamage(float Damage)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	if (PandoUIComponent)
	{
		PandoUIComponent->OnCurrentHealthChanged.Broadcast(1 - GetHealthPercent());
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

	if (PandoUIComponent)
	{
		PandoUIComponent->OnCurrentHealthChanged.Broadcast(1 - GetHealthPercent());
	}
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


	if (PandoUIComponent)
	{
		PandoUIComponent->OnCurrentBeerCounterChanged.Broadcast(Beers);
		PandoUIComponent->OnCurrentBeerPercentChanged.Broadcast(1.f);
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
		

	if (PandoUIComponent)
	{
		PandoUIComponent->OnCurrentBeerCounterChanged.Broadcast(Beers);
	}

	return true;
}

bool UAttributeComponent::ConsumingBeer()
{
		BeerConsuming = FMath::Clamp(BeerConsuming - BeerConsumingRate, 0.f, BeerConsumingMAX);

		if (PandoUIComponent)
		{
			PandoUIComponent->OnCurrentBeerPercentChanged.Broadcast(BeerConsuming / BeerConsumingMAX);
		}

		if (BeerConsuming <= 0.f)
		{
			if (Beers > 0) 
			{
				if (PandoUIComponent)
				{
					PandoUIComponent->OnCurrentBeerPercentChanged.Broadcast(1.f);
				}
			}
			return false;
		}

		return true;
}

void UAttributeComponent::AddBeerStamina(float Value)
{
	BeerConsuming = FMath::Clamp(BeerConsuming + Value, 0.f, BeerConsumingMAX);

	if (PandoUIComponent)
	{
		PandoUIComponent->OnCurrentBeerPercentChanged.Broadcast(BeerConsuming / BeerConsumingMAX);
	}
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

	if (PandoUIComponent)
	{
		PandoUIComponent->OnCurrentFlowerPercentChanged.Broadcast(FlowerStamina / MaxFlowerStamina);
	}
}

bool UAttributeComponent::ConsumingFlowerStamina()
{
	FlowerStamina = FMath::Clamp(FlowerStamina - FlowerStaminaCost , 0.f, MaxFlowerStamina);

	if (PandoUIComponent)
	{
		PandoUIComponent->OnCurrentFlowerPercentChanged.Broadcast(FlowerStamina / MaxFlowerStamina);
	}

	if (FlowerStamina <= 0.f)
	{
		return false;
	}

	return true;
}

void UAttributeComponent::AddFlowerStamina(float Value)
{
	FlowerStamina = FMath::Clamp(FlowerStamina + Value, 0.f, MaxFlowerStamina);

	if (PandoUIComponent)
	{
		PandoUIComponent->OnCurrentFlowerPercentChanged.Broadcast(FlowerStamina / MaxFlowerStamina);
	}
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
	/*PanwolfwarOverlay->SetBirdStaminaBarPercent(BirdStamina / MaxBirdStamina);*/
}

bool UAttributeComponent::ConsumingBirdStamina()
{
	BirdStamina = FMath::Clamp(BirdStamina - BirdStaminaCost, 0.f, MaxBirdStamina);
	/*PanwolfwarOverlay->SetBirdStaminaBarPercent(BirdStamina / MaxBirdStamina);*/

	if (BirdStamina <= 0.f)
	{
		return false;
	}

	return true;
}

void UAttributeComponent::AddBirdStamina(float Value)
{
	BirdStamina = FMath::Clamp(BirdStamina + Value, 0.f, MaxBirdStamina);
	/*PanwolfwarOverlay->SetBirdStaminaBarPercent(BirdStamina / MaxBirdStamina);*/
}
#pragma endregion
