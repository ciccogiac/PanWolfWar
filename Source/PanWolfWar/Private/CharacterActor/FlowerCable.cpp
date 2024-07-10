#include "CharacterActor/FlowerCable.h"

#include "CableComponent.h"
#include "PanWolfWar/DebugHelper.h"

//#include "PhysicsEngine/PhysicsConstraintComponent.h"

AFlowerCable::AFlowerCable()
{
	PrimaryActorTick.bCanEverTick = true;

	//StartCable = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlowerStartCable"));
	//StartCable->SetupAttachment(GetRootComponent());

	CableComponent = CreateDefaultSubobject<UCableComponent>(TEXT("FlowerCable"));
	CableComponent->SetupAttachment(GetRootComponent());

	EndCable = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlowerEndCable"));
	EndCable->SetupAttachment(GetRootComponent());

	CableComponent->SetAttachEndToComponent(EndCable);

	//PhysicsConstraintComponent = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraintComponent"));
	//PhysicsConstraintComponent->SetupAttachment(EndCable);



}

void AFlowerCable::SetCableAttachment(USceneComponent* Component, FName SocketName)
{
	CableComponent->PrimaryComponentTick.bCanEverTick = true;

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	CableComponent->AttachToComponent(Component, AttachmentRules, SocketName);

	//CableComponent->AttachToComponent(StartCable, AttachmentRules);
	CableComponent->SetAttachEndToComponent(EndCable);
	CableComponent->bAttachEnd = true;

	//PhysicsConstraintComponent->SetConstrainedComponents(EndCable, FName(NAME_None), StartCable, FName(NAME_None));

	CableComponent->Activate();
}

//void AFlowerCable::SwingCable(FVector Force)
//{
//	//PhysicsConstraintComponent->SetConstrainedComponents(EndCable, FName(NAME_None), StartCable, FName(NAME_None));
//	StartCable->SetSimulatePhysics(true);
//	StartCable->AddForce(Force);
//}

void AFlowerCable::SetCableLength(float Length)
{
	CableComponent->CableLength = Length;
}

void AFlowerCable::SetCableVisibility(bool NewVisibility)
{
	CableComponent->SetVisibility(NewVisibility);
	EndCable->SetVisibility(NewVisibility);
}

void AFlowerCable::SetEndCableLocation(FVector NewLocation)
{
	EndCable->SetWorldLocation(NewLocation);
}

void AFlowerCable::BeginPlay()
{
	Super::BeginPlay();
	SetCableVisibility(false);
}



