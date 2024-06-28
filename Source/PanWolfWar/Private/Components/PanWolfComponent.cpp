#include "Components/PanWolfComponent.h"

#include <PanWolfWar/PanWolfWarCharacter.h>

#include "PanWolfWar/DebugHelper.h"

UPanWolfComponent::UPanWolfComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	PanWolfCharacter = Cast<APanWolfWarCharacter>(GetOwner());
}

void UPanWolfComponent::Activate(bool bReset)
{
	Debug::Print(TEXT("PanWolf Activate"));

	PanWolfCharacter->AddMappingContext(PanWolfMappingContext, 1);
}

void UPanWolfComponent::Deactivate()
{
	Debug::Print(TEXT("PanWolf Deactivate"));

	PanWolfCharacter->RemoveMappingContext(PanWolfMappingContext);
}

void UPanWolfComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UPanWolfComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

