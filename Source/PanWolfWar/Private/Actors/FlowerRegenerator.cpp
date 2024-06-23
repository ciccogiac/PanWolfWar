#include "Actors/FlowerRegenerator.h"

#include "Interfaces/CharacterInterface.h"
#include "Components/BoxComponent.h"
#include "Components/AttributeComponent.h"

#include "PanWolfWar/DebugHelper.h"

AFlowerRegenerator::AFlowerRegenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(*FString::Printf(TEXT("BoxComponent")));

	if (BoxComponent)
	{
		BoxComponent->SetupAttachment(GetRootComponent());
		BoxComponent->bHiddenInGame = true;
		BoxComponent->SetLineThickness(2.f);
	}
}

void AFlowerRegenerator::BeginPlay()
{
	Super::BeginPlay();
	
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AFlowerRegenerator::BoxCollisionEnter);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AFlowerRegenerator::BoxCollisionExit);
}

void AFlowerRegenerator::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (CharacterInterface)
		{
			CharacterInterface->GetAttributeComponent()->SetCanRegenFlower(true);
		}
	}
}

void AFlowerRegenerator::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (CharacterInterface)
		{
			CharacterInterface->GetAttributeComponent()->SetCanRegenFlower(false);
		}
	}
}



