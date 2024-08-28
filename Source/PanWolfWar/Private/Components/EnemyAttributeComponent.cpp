#include "Components/EnemyAttributeComponent.h"

UEnemyAttributeComponent::UEnemyAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UEnemyAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

#pragma region Health

void UEnemyAttributeComponent::ReceiveDamage(float Damage)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
}

float UEnemyAttributeComponent::GetHealthPercent()
{
	return Health / MaxHealth;
}


bool UEnemyAttributeComponent::IsAlive()
{
	return Health > 0.f;
}

void UEnemyAttributeComponent::AddHealth(float healthToAdd)
{
	Health = FMath::Clamp(Health + healthToAdd, 0.f, MaxHealth);
}

#pragma endregion