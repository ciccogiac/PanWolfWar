#include "Actors/KiteBoard.h"
#include "GameFramework/Character.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Components/PandolfoComponent.h"

#include "Components/BoxComponent.h"

AKiteBoard::AKiteBoard()
{
	PrimaryActorTick.bCanEverTick = false;

	N_InteractBox = 1;
	InitializeBoxComponents();
}

bool AKiteBoard::Interact(ACharacter* _CharacterOwner)
{
	Super::Interact(_CharacterOwner);

	if (!_CharacterOwner) return false; 

	APanWolfWarCharacter* PanWarCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);

	if (!PanWarCharacter) return false;

	UPandolfoComponent* PandolfoComponent = PanWarCharacter->GetPandolfoComponent();

	if (!PandolfoComponent) return false;

	BoxComponent->GetChildComponent(1)->SetVisibility(false);

	PandolfoComponent->EnterKiteMode(this);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, true);
	_CharacterOwner->AttachToComponent(StaticMesh, AttachmentRules, FName("CharacterPosition"));

	AttachKite();

	return true;
}

void AKiteBoard::BeginPlay()
{
	Super::BeginPlay();
	
}

void AKiteBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

