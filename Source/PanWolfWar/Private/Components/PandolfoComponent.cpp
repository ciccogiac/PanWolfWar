#include "Components/PandolfoComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Components/ClimbingComponent.h"


#include "GameFramework/Character.h"

UPandolfoComponent::UPandolfoComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);

	ClimbingComponent = CreateDefaultSubobject<UClimbingComponent>(TEXT("ClimbingComponent"));
}

void UPandolfoComponent::Activate(bool bReset)
{
	Super::Activate();

	Debug::Print(TEXT("Pandolfo Activate"));

	PanWolfCharacter->AddMappingContext(PandolfoMappingContext, 1);

}

void UPandolfoComponent::Deactivate()
{
	Super::Deactivate();

	Debug::Print(TEXT("Pandolfo Deactivate"));

	PanWolfCharacter->RemoveMappingContext(PandolfoMappingContext);
	ClimbingComponent->Deactivate();
}

void UPandolfoComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPandolfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

const bool UPandolfoComponent::IsClimbing()
{
	return ClimbingComponent->IsClimbing();
}

void UPandolfoComponent::Jump()
{
	if (!ClimbingComponent->TryClimbing())
	{
		ClimbingComponent->Activate();
		CharacterOwner->Jump();
	}

}
