// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PanWarTypes/PanWarEnumTypes.h"
#include "PanWarFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class PANWOLFWAR_API UPanWarFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure, Category = "Warrior|FunctionLibrary")
	static bool IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn);

	UFUNCTION(BlueprintPure, Category = "Warrior|FunctionLibrary")
	static bool IsValidBlock(AActor* InAttacker, AActor* InDefender);

	UFUNCTION(BlueprintCallable, Category = "Warrior|FunctionLibrary")
	static void SaveCurrentGameDifficulty(EPanWarGameDifficulty InDifficultyToSave);

	UFUNCTION(BlueprintCallable, Category = "Warrior|FunctionLibrary")
	static bool TryLoadSavedGameDifficulty(EPanWarGameDifficulty& OutSavedDifficulty);

};
