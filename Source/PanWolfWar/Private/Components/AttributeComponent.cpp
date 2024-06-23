#include "Components/AttributeComponent.h"
#include "UserWidgets/PanwolfwarOverlay.h"

#include "Components/TransformationComponent.h"

#include "TimerManager.h"

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
		PanwolfwarOverlay->SetBeers(MaxBeers);
		PanwolfwarOverlay->SetHealthBarPercent(0.f);
		PanwolfwarOverlay->SetFlowerStaminaBarPercent(MaxFlowerStamina);
		PanwolfwarOverlay->SetBeerBarPercent(1.f);
		//PanwolfwarOverlay->SetBeerBarVisibility(false);
	}

}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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
		PanwolfwarOverlay->SetBeerBarPercent(1);
		//PanwolfwarOverlay->SetBeerBarVisibility(true);
		
		AttributeState = EAttributeState::EAS_ConsumingBeer;

		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UAttributeComponent::ConsumingBeer, 0.05f , true);
	}

	return true;
}

void UAttributeComponent::ConsumingBeer()
{
	if (AttributeState == EAttributeState::EAS_ConsumingBeer)
	{
		BeerConsuming = FMath::Clamp(BeerConsuming - BeerConsumingRate, 0.f, BeerConsumingMAX);
		PanwolfwarOverlay->SetBeerBarPercent(BeerConsuming / BeerConsumingMAX);

		if (BeerConsuming <= 0.f)
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
			//PanwolfwarOverlay->SetBeerBarVisibility(false);
			AttributeState = EAttributeState::EAS_Normal;
			TransformationComponent->SelectDesiredTransformation(0);
			
		}

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
	if (FlowerStamina < MaxFlowerStamina) return false;

	if (PanwolfwarOverlay)
	{
		AttributeState = EAttributeState::EAS_ConsumingFlower;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UAttributeComponent::ConsumingFlowerStamina, 0.05f, true);
	}

	return true;

}

void UAttributeComponent::RegenFlowerStamina()
{
	if (AttributeState == EAttributeState::EAS_ConsumingBeer) return;

	FlowerStamina = FMath::Clamp(FlowerStamina + FlowerStaminaRegenRate , 0.f, MaxFlowerStamina);
	PanwolfwarOverlay->SetFlowerStaminaBarPercent(FlowerStamina / MaxFlowerStamina);
}

void UAttributeComponent::ConsumingFlowerStamina()
{
	if (AttributeState == EAttributeState::EAS_ConsumingFlower && !bCanRegenFlower)
	{
		FlowerStamina = FMath::Clamp(FlowerStamina - FlowerStaminaCost , 0.f, MaxFlowerStamina);
		PanwolfwarOverlay->SetFlowerStaminaBarPercent(FlowerStamina / MaxFlowerStamina);

		if (FlowerStamina <= 0.f)
		{
			GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
			AttributeState = EAttributeState::EAS_Normal;
			TransformationComponent->SelectDesiredTransformation(0);
		}

	}
}

void UAttributeComponent::SetCanRegenFlower(bool Value)
{
	bCanRegenFlower = Value;

	if(bCanRegenFlower)
		GetWorld()->GetTimerManager().SetTimer(RegenFlower_TimerHandle, this, &UAttributeComponent::RegenFlowerStamina, 0.05f, true);
	else
		GetWorld()->GetTimerManager().ClearTimer(RegenFlower_TimerHandle);
}

#pragma endregion




