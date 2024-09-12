// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PanWarTypes/PanWarEnumTypes.h"
#include "PanWarSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class PANWOLFWAR_API UPanWarSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	EPanWarGameDifficulty SavedCurrentGameDifficulty;
	
};
