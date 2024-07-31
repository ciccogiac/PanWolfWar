// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CharacterInterface.generated.h"

class UTransformationComponent;
class UAttributeComponent;
class UPandolfoComponent;
class UPandolFlowerComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PANWOLFWAR_API ICharacterInterface
{
	GENERATED_BODY()

public:
	 virtual UAttributeComponent* GetAttributeComponent() const = 0;
	 virtual UTransformationComponent* GetTransformationComponent()  const = 0;
	 virtual UPandolfoComponent* GetPandolfoComponent() const = 0;
	 virtual UPandolFlowerComponent* GetPandolFlowerComponent() const = 0;
};
