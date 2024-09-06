// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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

};
