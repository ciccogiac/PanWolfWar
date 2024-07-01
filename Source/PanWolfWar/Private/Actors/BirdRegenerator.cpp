#include "Actors/BirdRegenerator.h"


#include "Interfaces/CharacterInterface.h"
#include "Components/TransformationComponent.h"

#include "Components/BoxComponent.h"

ABirdRegenerator::ABirdRegenerator()
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

void ABirdRegenerator::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ABirdRegenerator::BoxCollisionEnter);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ABirdRegenerator::BoxCollisionExit);
}

void ABirdRegenerator::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (CharacterInterface)
		{
			CharacterInterface->GetTransformationComponent()->SetCanRegenBird(true);
		}
	}
}

void ABirdRegenerator::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (CharacterInterface)
		{
			CharacterInterface->GetTransformationComponent()->SetCanRegenBird(false);
		}
	}
}

