// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/InteractableObject.h"

#include "Interfaces/InteractInterface.h"
#include "Components/BoxComponent.h"

// Sets default values
AInteractableObject::AInteractableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	StaticMesh->SetupAttachment(GetRootComponent());

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(StaticMesh);
	BoxComponent->bHiddenInGame = false;
	BoxComponent->SetLineThickness(2.f);

}

void AInteractableObject::Interact(bool bStartInteraction)
{
}

// Called when the game starts or when spawned
void AInteractableObject::BeginPlay()
{
	Super::BeginPlay();

	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AInteractableObject::BoxCollisionEnter);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AInteractableObject::BoxCollisionExit);
	
}

// Called every frame
void AInteractableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractableObject::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	IInteractInterface* InteractInterface = Cast<IInteractInterface>(OtherActor);
	if (InteractInterface)
	{
		InteractInterface->SetOverlappingObject(this);
	}
}

void AInteractableObject::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	IInteractInterface* InteractInterface = Cast<IInteractInterface>(OtherActor);
	if (InteractInterface)
	{
		InteractInterface->SetOverlappingObject(nullptr);
	}
}

