#include "CharacterActor/FlowerCable.h"

#include "CableComponent.h"

AFlowerCable::AFlowerCable()
{
	PrimaryActorTick.bCanEverTick = false;

	CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("FlowerCable"));
	CableComponent->SetupAttachment(GetRootComponent());
}

void AFlowerCable::BeginPlay()
{
	Super::BeginPlay();

	
	
}

void AFlowerCable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

