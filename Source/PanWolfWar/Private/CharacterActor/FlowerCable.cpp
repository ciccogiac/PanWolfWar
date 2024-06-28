#include "CharacterActor/FlowerCable.h"

#include "CableComponent.h"
#include "Kismet/KismetSystemLibrary.h"

#include "PanWolfWar/DebugHelper.h"
AFlowerCable::AFlowerCable()
{
	PrimaryActorTick.bCanEverTick = true;

	CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("FlowerCable"));
	CableComponent->SetupAttachment(GetRootComponent());
}

void AFlowerCable::HookCable(const FVector Hook_TargetLocation, const FRotator Hook_TargetRotation, const FVector CharacterLocation)
{
	CableComponent->bAttachStart = true;

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	float OverTime = FVector::Distance(Hook_TargetLocation, CharacterLocation) / HookCable_Divisor;
	UKismetSystemLibrary::MoveComponentTo(CableComponent, Hook_TargetLocation, Hook_TargetRotation, true, false, OverTime, true, EMoveComponentAction::Move, LatentInfo);

}

void AFlowerCable::SetAttachEndCable(USceneComponent* Component, FName SocketName)
{
	CableComponent->PrimaryComponentTick.bCanEverTick = true;

	CableComponent->SetAttachEndToComponent(Component, SocketName);
	


	CableComponent->bAttachEnd = true;

	CableComponent->Activate();

}

void AFlowerCable::SetAttachStartCable(bool Value)
{
	CableComponent->bAttachStart = Value;
}

void AFlowerCable::BeginPlay()
{
	Super::BeginPlay();

}

void AFlowerCable::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}


