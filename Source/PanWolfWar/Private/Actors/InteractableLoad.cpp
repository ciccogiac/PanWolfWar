#include "Actors/InteractableLoad.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Components/PandolfoComponent.h"
#include "Components/BoxComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "PanWolfWar/DebugHelper.h"

#include "TimerManager.h"

#include "Components/WidgetComponent.h"
#include "UserWidgets/InteractionLoadWidget.h"
#include "GameFramework/CharacterMovementComponent.h"

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
		CharacterOwner->SetActorLocation(FVector(BoxComponent->GetComponentLocation().X, BoxComponent->GetComponentLocation().Y, CharacterOwner->GetActorLocation().Z));
		CharacterOwner->SetActorRotation(UKismetMathLibrary::MakeRotFromX(BoxComponent->GetForwardVector()));
		bFirstInteraction = false;

		GetWorld()->GetTimerManager().SetTimer(Percentage_TimerHandle, [this]() {this->DecreasePercentage(); }, DecreaseTime, true);

		CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = false;
		CharacterOwner->GetCharacterMovement()->Deactivate();

	}

	if (!bLoadFull)
	{
		Percentage = UKismetMathLibrary::FClamp(Percentage + InteractPercent, 0.f, 100.f);
		InteractionLoadWidget->SetInteractionBarPercent(Percentage/100.f);
		Interaction(Percentage);
	}


	if (Percentage >= 100.f)
	{
		bLoadFull = true;
		GetWorld()->GetTimerManager().ClearTimer(Percentage_TimerHandle);
		BoxComponent->GetChildComponent(1)->SetVisibility(false);

		CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = true;
		CharacterOwner->GetCharacterMovement()->Activate();
	}

	return true;
}

void AInteractableLoad::BeginPlay()
{
	Super::BeginPlay();

	InteractionLoadWidget = Cast<UInteractionLoadWidget>(InteractionWidgetArray[0]->GetWidget());
	BoxComponentArray[0]->DetachFromParent(true);
}

void AInteractableLoad::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (bLoadFull) return;
	Super::BoxCollisionEnter(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
}

void AInteractableLoad::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bLoadFull) return;
	Super::BoxCollisionExit(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	Percentage = 0.f;
	InteractionLoadWidget->SetInteractionBarPercent(0.f);
}


void AInteractableLoad::DecreasePercentage()
{	
	Percentage = UKismetMathLibrary::FClamp(Percentage - DecreasePercent, 0.f, 100.f);
	InteractionLoadWidget->SetInteractionBarPercent(Percentage / 100.f);
	Interaction(Percentage);

	if (Percentage <= 0.f)
	{
		GetWorld()->GetTimerManager().ClearTimer(Percentage_TimerHandle);
		CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = true;
		CharacterOwner->GetCharacterMovement()->Activate();
		bFirstInteraction = true;
	}
}
