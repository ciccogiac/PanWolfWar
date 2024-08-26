#include "Actors/FlowerHideObject.h"

#include "Components/BoxComponent.h"

AFlowerHideObject::AFlowerHideObject()
{
	PrimaryActorTick.bCanEverTick = false;

	Object_Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Object_Mesh"));
	Object_Mesh->SetupAttachment(GetRootComponent());

	Collision_Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision_Box"));
	Collision_Box->SetupAttachment(Object_Mesh);
	Collision_Box->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);



	Border1_Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Border1_Box"));
	Border1_Box->SetupAttachment(Collision_Box);

	Border2_Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Border2_Box"));
	Border2_Box->SetupAttachment(Collision_Box);

}

void AFlowerHideObject::BeginPlay()
{
	Super::BeginPlay();
	
}

void AFlowerHideObject::ChangeCollisionType(bool Enabled)
{
	ECollisionResponse CollisionType = Enabled ? ECollisionResponse::ECR_Block : ECollisionResponse::ECR_Ignore;
	Collision_Box->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, CollisionType);

}

