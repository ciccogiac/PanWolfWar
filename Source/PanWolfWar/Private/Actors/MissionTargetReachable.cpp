#include "Actors/MissionTargetReachable.h"
#include "Components/BoxComponent.h"
#include "Interfaces/CharacterInterface.h"
#include "Actors/MissionManager.h"
#include "Kismet/GameplayStatics.h"

AMissionTargetReachable::AMissionTargetReachable()
{
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->bHiddenInGame = true;
	BoxComponent->SetLineThickness(3.f);

}

void AMissionTargetReachable::BeginPlay()
{
	Super::BeginPlay();
	
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AMissionTargetReachable::BoxCollisionEnter);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AMissionTargetReachable::BoxCollisionExit);

	MissionManager = Cast<AMissionManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AMissionManager::StaticClass()));

}

void AMissionTargetReachable::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (CharacterInterface)
		{
			bIsCharacterInside = true;
			MissionManager->MissionTargetReached(this);
		}
	}
}

void AMissionTargetReachable::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (CharacterInterface)
		{
			bIsCharacterInside = false;

		}
	}
}


