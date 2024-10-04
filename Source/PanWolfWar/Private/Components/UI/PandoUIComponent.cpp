#include "Components/UI/PandoUIComponent.h"

#include "Components/TransformationComponent.h"

void UPandoUIComponent::BeginPlay()
{
	APawn* Player = GetWorld()->GetFirstPlayerController()->GetPawn();

	if (Player)
	{
		UTransformationComponent* TransformationComponent = Player->FindComponentByClass<UTransformationComponent>();
		if (!TransformationComponent) return;
		TransformationComponent->OnTransformationStateChanged.AddDynamic(this, &UPandoUIComponent::ChangeTransformationStateWidgets);
	}
}
