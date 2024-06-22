#include "Components/AttributeComponent.h"
#include "UserWidgets/PanwolfwarOverlay.h"

#include "Components/TransformationComponent.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

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
		PanwolfwarOverlay->SetBeers(MaxBeers);
		PanwolfwarOverlay->SetHealthBarPercent(MaxHealth);
		PanwolfwarOverlay->SetFlowerStaminaBarPercent(MaxFlowerStamina);
		PanwolfwarOverlay->SetBeerBarPercent(0.f);
		PanwolfwarOverlay->SetBeerBarVisibility(false);
	}

}




void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (AttributeState == EAttributeState::EAS_ConsumingBeer)
	{
		BeerConsuming = FMath::Clamp(BeerConsuming - BeerConsumingRate * DeltaTime, 0.f, BeerConsumingMAX);
		PanwolfwarOverlay->SetBeerBarPercent(BeerConsuming/BeerConsumingMAX);

		if (BeerConsuming <= 0.f)
		{
			PanwolfwarOverlay->SetBeerBarVisibility(false);
			AttributeState = EAttributeState::EAS_Normal;
			TransformationComponent->SelectDesiredTransformation(0);
		}

		return;
	}

	else if (AttributeState == EAttributeState::EAS_ConsumingFlower && !bCanRegenFlower)
	{
		FlowerStamina = FMath::Clamp(FlowerStamina - FlowerStaminaCost * DeltaTime , 0.f, MaxFlowerStamina);
		PanwolfwarOverlay->SetFlowerStaminaBarPercent(FlowerStamina / MaxFlowerStamina);

		if (FlowerStamina <= 0.f)
		{
			//PanwolfwarOverlay->SetBeerBarVisibility(false);
			AttributeState = EAttributeState::EAS_Normal;
			TransformationComponent->SelectDesiredTransformation(0);
		}

	}

	 if (bCanRegenFlower)
	{
		RegenFlowerStamina(DeltaTime);
	}
}

#pragma region Health

void UAttributeComponent::ReceiveDamage(float Damage)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
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

void UAttributeComponent::AddBeers(int32 NumberOfBeers)
{
	Beers = FMath::Clamp(Beers + NumberOfBeers, 0.f, MaxBeers);

	if (PanwolfwarOverlay)
		PanwolfwarOverlay->SetBeers(Beers);

}

bool UAttributeComponent::ConsumeBeer()
{
	if (Beers <= 0) return false;

	Beers = FMath::Clamp(Beers - 1, 0.f, MaxHealth);
	if (PanwolfwarOverlay)
	{
		PanwolfwarOverlay->SetBeers(Beers);
		BeerConsuming = BeerConsumingMAX;
		PanwolfwarOverlay->SetBeerBarVisibility(true);
		AttributeState = EAttributeState::EAS_ConsumingBeer;
	}


	return true;
}


#pragma endregion





#pragma region Flower

float UAttributeComponent::GetFlowerStaminaPercent()
{
	return FlowerStamina / MaxFlowerStamina;
}

bool UAttributeComponent::ConsumeFlowerStamina()
{
	if (FlowerStamina < MaxFlowerStamina) return false;

	if (PanwolfwarOverlay)
	{
		//PanwolfwarOverlay->SetBeerBarVisibility(true);
		AttributeState = EAttributeState::EAS_ConsumingFlower;
	}


	return true;

}

void UAttributeComponent::RegenFlowerStamina(float DeltaTime)
{
	FlowerStamina = FMath::Clamp(FlowerStamina + FlowerStaminaRegenRate * DeltaTime, 0.f, MaxFlowerStamina);
	PanwolfwarOverlay->SetFlowerStaminaBarPercent(FlowerStamina / MaxFlowerStamina);
}


#pragma endregion




