#include "Components/PanWolfComponent.h"

#include <PanWolfWar/PanWolfWarCharacter.h>

#include "PanWolfWar/DebugHelper.h"

UPanWolfComponent::UPanWolfComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
}

void UPanWolfComponent::Activate(bool bReset)
{
	Super::Activate();

	PanWolfCharacter->AddMappingContext(PanWolfMappingContext, 1);

	PanWolfCharacter->SetTransformationCharacter(SkeletalMeshAsset, Anim);
}

void UPanWolfComponent::Deactivate()
{
	Super::Deactivate();

	PanWolfCharacter->RemoveMappingContext(PanWolfMappingContext);
}

void UPanWolfComponent::Jump()
{
	CharacterOwner->Jump();
}

void UPanWolfComponent::BeginPlay()
{
	Super::BeginPlay();	
}

void UPanWolfComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

