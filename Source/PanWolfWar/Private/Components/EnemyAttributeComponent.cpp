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

void UEnemyAttributeComponent::InitializeAttributeStats(float _MaxHealth, float _StoneSpawnProbability)
{
	MaxHealth = _MaxHealth;
	CurrentHealth = _MaxHealth;

	StoneSpawnProbability = _StoneSpawnProbability;
}

void UEnemyAttributeComponent::ReceiveDamage(float Damage)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
}

float UEnemyAttributeComponent::GetHealthPercent()
{
	return CurrentHealth / MaxHealth;
}


bool UEnemyAttributeComponent::IsAlive()
{
	return CurrentHealth > 0.f;
}

void UEnemyAttributeComponent::AddHealth(float healthToAdd)
{
	CurrentHealth = FMath::Clamp(CurrentHealth + healthToAdd, 0.f, MaxHealth);
}

#pragma endregion