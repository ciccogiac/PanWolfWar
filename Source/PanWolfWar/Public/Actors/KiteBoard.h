// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actors/InteractableObject.h"
#include "KiteBoard.generated.h"

UCLASS()
class PANWOLFWAR_API AKiteBoard : public AInteractableObject
{	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKiteBoard();

	virtual bool Interact(ACharacter* _CharacterOwner = nullptr) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	//UStaticMeshComponent* Board_Mesh;


public:
	FORCEINLINE UStaticMeshComponent* GetBoardMesh() const { return StaticMesh; };

};
