// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/InteractableObject.h"
#include "BeerMachine.generated.h"

/**
 * 
 */
UCLASS()
class PANWOLFWAR_API ABeerMachine : public AInteractableObject
{
	GENERATED_BODY()
	
public:
	ABeerMachine();
	virtual bool Interact(ACharacter* _CharacterOwner = nullptr) override;
};
