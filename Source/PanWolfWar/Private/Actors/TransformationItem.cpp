#include "Actors/TransformationItem.h"

#include "Components/BoxComponent.h"
#include "Interfaces/CharacterInterface.h"
#include "Components/TransformationComponent.h"

#include "NiagaraComponent.h"
#include "TimerManager.h"

#include "PanWolfWar/DebugHelper.h"

ATransformationItem::ATransformationItem()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	StaticMesh->SetupAttachment(GetRootComponent());
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	BoxComponent->SetupAttachment(StaticMesh);
	BoxComponent->bHiddenInGame = true;
	BoxComponent->SetLineThickness(2.f);
	BoxComponent->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);

	NiagaraCollectObjectEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraCollectObjectEffect"));
	NiagaraCollectObjectEffect->SetupAttachment(StaticMesh);
}

void ATransformationItem::BeginPlay()
{
	Super::BeginPlay();
	
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ATransformationItem::BoxCollisionEnter);
	BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ATransformationItem::BoxCollisionExit);

}


void ATransformationItem::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (CharacterInterface)
		{
			if (CharacterInterface->GetTransformationComponent()->AddItemStamina(TransformationItemType, StaminaToAdd))
				CollectObject();
		}
	}
}

void ATransformationItem::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	OverlappedComponent->Deactivate();
}

void ATransformationItem::CollectObject()
{
	BoxComponent->Activate();
	NiagaraCollectObjectEffect->Activate(true);
	StaticMesh->SetVisibility(false);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetWorld()->GetTimerManager().SetTimer(Item_TimerHandle, this, &ATransformationItem::ResetObject, SecondToRespawn, false);

}

void ATransformationItem::ResetObject()
{
	StaticMesh->SetVisibility(true);
	BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

