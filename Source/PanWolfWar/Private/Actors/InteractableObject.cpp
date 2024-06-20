// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/InteractableObject.h"

#include "Interfaces/InteractInterface.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include "UserWidgets/InteractionWidget.h"

#include "Components/WidgetComponent.h"

AInteractableObject::AInteractableObject()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	StaticMesh->SetupAttachment(GetRootComponent());

	InitializeBoxComponents();
}

void AInteractableObject::InitializeBoxComponents()
{

	BoxComponentArray.SetNum(N_InteractBox);
	ArrowComponentArray.SetNum(N_InteractBox);
	InteractionWidgetArray.SetNum(N_InteractBox);

	for (int32 i = 0; i < BoxComponentArray.Num(); ++i)
	{
		// Crea un nuovo UBoxComponent e lo aggiunge all'array
		BoxComponentArray[i] = CreateDefaultSubobject<UBoxComponent>(*FString::Printf(TEXT("BoxComponent%d"), i));

		// Imposta il componente come figlio della radice dell'attore
		if (BoxComponentArray[i])
		{
			BoxComponentArray[i]->SetupAttachment(StaticMesh);
			BoxComponentArray[i]->bHiddenInGame = true;
			//BoxComponentArray[i]->SetLineThickness(2.f);
			BoxComponentArray[i]->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);

			ArrowComponentArray[i] = CreateDefaultSubobject<UArrowComponent>(*FString::Printf(TEXT("ArrowComponent%d"), i));
			if (ArrowComponentArray[i])
			{
				ArrowComponentArray[i]->SetupAttachment(BoxComponentArray[i]);
				ArrowComponentArray[i]->ArrowLength = BoxComponentArray[i]->GetScaledBoxExtent().X;
			}

			InteractionWidgetArray[i] = CreateDefaultSubobject<UWidgetComponent>(*FString::Printf(TEXT("InteractionWidget%d"), i));
			if (InteractionWidgetArray[i])
			{
				 InteractionWidgetArray[i]->SetupAttachment(BoxComponentArray[i]);
				 InteractionWidgetArray[i]->SetRelativeRotation(FQuat(0.f,0.f,180.f,0.f));
				 const FVector WidgetLocation = InteractionWidgetArray[i]->GetRelativeLocation() + FVector(BoxComponentArray[i]->GetScaledBoxExtent().X, 0.f, 0.f);
				 InteractionWidgetArray[i]->SetRelativeLocationAndRotation(WidgetLocation,FQuat(0.f, 0.f, 180.f, 0.f));
				 InteractionWidgetArray[i]->SetWidgetSpace(EWidgetSpace::Screen);
				 InteractionWidgetArray[i]->SetVisibility(false);
			}
			
		}
	}
}

void AInteractableObject::BeginPlay()
{
	Super::BeginPlay();
	
	for (int32 i = 0; i < BoxComponentArray.Num(); ++i)
	{
		if (BoxComponentArray[i])
		{
			BoxComponentArray[i]->OnComponentBeginOverlap.AddDynamic(this, &AInteractableObject::BoxCollisionEnter);
			BoxComponentArray[i]->OnComponentEndOverlap.AddDynamic(this, &AInteractableObject::BoxCollisionExit);
		}
	}

}


bool AInteractableObject::Interact(ACharacter* _CharacterOwner)
{
	CharacterOwner = _CharacterOwner;
	return true;
}

void AInteractableObject::Move(const FInputActionValue& Value)
{
}

void AInteractableObject::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
		
		IInteractInterface* InteractInterface = Cast<IInteractInterface>(OtherActor);
		if (InteractInterface && InteractInterface->SetOverlappingObject(this))
		{
			//if (BoxComponent) { BoxComponent->GetChildComponent(1)->SetVisibility(false); }
				BoxComponent = Cast<UBoxComponent>(OverlappedComponent);
				SetInteractWidget(BoxComponent->GetChildComponent(1));
				SetInteractWidgetVisibility(true);
				//BoxComponent->GetChildComponent(1)->SetVisibility(true);
		}
}

void AInteractableObject::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
		//if(BoxComponent)BoxComponent->GetChildComponent(1)->SetVisibility(false);
		SetInteractWidgetVisibility(false);

		IInteractInterface* InteractInterface = Cast<IInteractInterface>(OtherActor);
		if (InteractInterface && InteractInterface->SetOverlappingObject(this, false))
		{		
			    SetInteractWidget(nullptr);
				BoxComponent = nullptr;
				
		}
}

FVector2D AInteractableObject::Get8DirectionVector(const FVector2D& InputVector)
{
	FVector2D Normalized = InputVector.GetSafeNormal();
	float Angle = FMath::Atan2(Normalized.Y, Normalized.X);
	// Aggiungere 2PI se l'angolo è negativo
	if (Angle < 0)
	{
		Angle += 2.0f * PI;
	}

	// Convertire l'angolo in gradi
	float AngleDegrees = FMath::RadiansToDegrees(Angle);

	//// Determinare la direzione in base all'angolo
	if (AngleDegrees <= 30.f || AngleDegrees > 330.f)
	{
		return FVector2D(1.0f, 0.0f);  // Right
	}
	else if (AngleDegrees <= 70.f)
	{
		return FVector2D(1.0f, 1.0f);  // Up-Right
	}
	else if (AngleDegrees <= 110.f)
	{
		return FVector2D(0.0f, 1.0f);  // Up
	}
	else if (AngleDegrees <= 150.f)
	{
		return FVector2D(-1.0f, 1.0f); // Up-Left
	}
	else if (AngleDegrees <= 210.f)
	{
		return FVector2D(-1.0f, 0.0f); // Left
	}
	else if (AngleDegrees <= 250.f)
	{
		return FVector2D(-1.0f, -1.0f); // Down-Left
	}
	else if (AngleDegrees <= 290.f)
	{
		return FVector2D(0.0f, -1.0f); // Down
	}
	else if (AngleDegrees <= 330.f)
	{
		return FVector2D(1.0f, -1.0f); // Down-Right
	}

	return FVector2D(0.0f, 0.0f);  // Default, should not be reached
}
