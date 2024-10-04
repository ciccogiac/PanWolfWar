// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include <PanWolfWar/CharacterStates.h>
#include "CharacterInterface.generated.h"

class UTransformationComponent;
class UAttributeComponent;
class UPandolfoComponent;
class UPandolFlowerComponent;
class UPanWolfComponent;
class UTargetingComponent;

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
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
	 UFUNCTION(BlueprintCallable)
	 virtual UPanWolfComponent* GetPanWolfComponent() const = 0;
	 UFUNCTION(BlueprintCallable)
	 virtual UTargetingComponent* GetTargetingComponent() const = 0;

	 virtual ETransformationState GetCurrentTransformationState() const = 0;

	 UFUNCTION(BlueprintCallable)
	 virtual void SetIsInsideHideBox(bool Value, bool ForceCrouch = false)  = 0;
	 UFUNCTION(BlueprintCallable)
	 virtual bool IsHiding() const = 0;
	 UFUNCTION(BlueprintCallable)
	 virtual void AddEnemyAware(AActor* Enemy) = 0;
	 UFUNCTION(BlueprintCallable)
	 virtual void RemoveEnemyAware(AActor* Enemy) = 0;
};
