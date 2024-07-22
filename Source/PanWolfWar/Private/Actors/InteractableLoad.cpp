#include "Actors/InteractableLoad.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Components/PandolfoComponent.h"
#include "Components/BoxComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "PanWolfWar/DebugHelper.h"

#include "TimerManager.h"

AInteractableLoad::AInteractableLoad()
{
	PrimaryActorTick.bCanEverTick = false;

	N_InteractBox = 1;
	InitializeBoxComponents();
}

bool AInteractableLoad::Interact(ACharacter* _CharacterOwner)
{
	Super::Interact(_CharacterOwner);

	if (!_CharacterOwner) return false;

	APanWolfWarCharacter* PanWarCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
	if (!PanWarCharacter) return false;

	UPandolfoComponent* PandolfoComponent = PanWarCharacter->GetPandolfoComponent();
	if (!PandolfoComponent) return false;

	//BoxComponent->GetChildComponent(1)->SetVisibility(false);

	if (bFirstInteraction) 
	{
		CharacterOwner->SetActorLocation(BoxComponent->GetComponentLocation());
		CharacterOwner->SetActorRotation(UKismetMathLibrary::MakeRotFromX(BoxComponent->GetForwardVector()));
		bFirstInteraction = false;

		GetWorld()->GetTimerManager().SetTimer(Percentage_TimerHandle, [this]() {this->DecreasePercentage(); }, DecreaseTime, true);
	}

	Percentage = UKismetMathLibrary::FClamp(Percentage + InteractPercent, 0.f, 100.f);
	Debug::Print(TEXT("Percent : ") + FString::SanitizeFloat(Percentage), FColor::Cyan, 1);

	if (Percentage >= 100.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(Percentage_TimerHandle);
		BoxComponent->GetChildComponent(1)->SetVisibility(false);
	}
	return true;
}

void AInteractableLoad::BeginPlay()
{
	Super::BeginPlay();
}


void AInteractableLoad::DecreasePercentage()
{	
	Percentage = UKismetMathLibrary::FClamp(Percentage - DecreasePercent, 0.f, 100.f);
	Debug::Print(TEXT("Percent : ") + FString::SanitizeFloat(Percentage),FColor::Cyan,1);
}
