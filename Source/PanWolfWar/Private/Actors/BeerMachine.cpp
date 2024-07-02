#include "Actors/BeerMachine.h"

#include "Components/AttributeComponent.h"
#include "Interfaces/CharacterInterface.h"
#include "GameFramework/Character.h"

#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

#include "PanWolfWar/DebugHelper.h"

ABeerMachine::ABeerMachine()
{
	PrimaryActorTick.bCanEverTick = false;

	N_InteractBox = 1;
	InitializeBoxComponents();

	BeerWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("BeerWidget"));
	if (BeerWidget)
	{
		BeerWidget->SetupAttachment(StaticMesh);
		BeerWidget->SetVisibility(false);
	}
}

bool ABeerMachine::Interact(ACharacter* _CharacterOwner)
{
	Super::Interact(_CharacterOwner);

	ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(_CharacterOwner);
	if (CharacterInterface)
	{
		if (CharacterInterface->GetAttributeComponent()->IsBeerInventoryFull()) { BeerWidget->SetVisibility(true); return false; }

		CharacterInterface->GetAttributeComponent()->AddBeers(1);
		UGameplayStatics::PlaySoundAtLocation(this, SoundCue, GetActorLocation());
		return true;
	}
	
	return false;
}

void ABeerMachine::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::BoxCollisionEnter(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

}

void ABeerMachine::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::BoxCollisionExit(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	BeerWidget->SetVisibility(false);

}
