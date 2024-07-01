#include "Actors/BeerMachine.h"

#include "PanWolfWar/DebugHelper.h"

ABeerMachine::ABeerMachine()
{
	PrimaryActorTick.bCanEverTick = false;

	N_InteractBox = 1;
	InitializeBoxComponents();
}

bool ABeerMachine::Interact(ACharacter* _CharacterOwner)
{
	Super::Interact(_CharacterOwner);
	Debug::Print(TEXT("OVa"));
	return false;
}
