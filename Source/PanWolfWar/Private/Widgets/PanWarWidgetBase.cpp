#include "Widgets/PanWarWidgetBase.h"
#include "Interfaces/PawnUIInterface.h"

void UPanWarWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(GetOwningPlayerPawn()))
	{
		if (UPandoUIComponent* HeroUIComponent = PawnUIInterface->GetPandoUIComponent())
		{
			BP_OnOwningPandoUIComponentInitialized(HeroUIComponent);
		}
	}
}

void UPanWarWidgetBase::InitEnemyCreatedWidget(AActor* OwningEnemyActor)
{
	if (IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(OwningEnemyActor))
	{
		UEnemyUIComponent* EnemyUIComponent = PawnUIInterface->GetEnemyUIComponent();

		checkf(EnemyUIComponent, TEXT("Failed to extrac an EnemyUIComponent from %s"), *OwningEnemyActor->GetActorNameOrLabel());

		BP_OnOwningEnemyUIComponentInitialized(EnemyUIComponent);
	}
}