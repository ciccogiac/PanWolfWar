// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractInterface.generated.h"

UENUM(BlueprintType)
enum class ETransformationObjectTypes : uint8
{
	ETOT_Pandolfo_Object UMETA(DisplayName = "Pandolfo_Object"),
	ETOT_PanWolf_Object UMETA(DisplayName = "PanWolf_Object"),
	ETOT_PandolFlower_Object UMETA(DisplayName = "PandolFlower_Object"),
	ETOT_PanBird_Object UMETA(DisplayName = "PanBird_Object")
};

UENUM(BlueprintType)
enum class EStoneTypes : uint8
{
	EST_HealingStone UMETA(DisplayName = "HealingStone"),
	EST_BeerStone UMETA(DisplayName = "BeerStone")
};


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PANWOLFWAR_API IInteractInterface
{
	GENERATED_BODY()

public:
	virtual bool SetOverlappingObject(class AInteractableObject* InteractableObject, bool bEnter=true) = 0;

	static FName GetSelectedFName(ETransformationObjectTypes TransformationObjectType);

	virtual void ConsumeStone(float StoneValue, EStoneTypes StoneType) = 0;
};
