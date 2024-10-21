// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PanWarTypes/PanWarEnumTypes.h"
#include "PanWarFunctionLibrary.generated.h"

class UPanWarGameInstance;

/**
 * 
 */
UCLASS()
class PANWOLFWAR_API UPanWarFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure, Category = "PanWar|FunctionLibrary")
	static bool IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn);

	UFUNCTION(BlueprintPure, Category = "PanWar|FunctionLibrary")
	static bool IsValidBlock(AActor* InAttacker, AActor* InDefender);

	UFUNCTION(BlueprintPure, Category = "PanWar|FunctionLibrary")
	static bool IsPlayingAnyMontage_ExcludingBlendOut(UAnimInstance* OwningPlayerAnimInstance);

	UFUNCTION(BlueprintPure, Category = "PanWar|FunctionLibrary")
	static bool IsPlayingMontage_ExcludingBlendOut(UAnimInstance* OwningPlayerAnimInstance, UAnimMontage* AnimMontage);


	UFUNCTION(BlueprintCallable, Category = "PanWar|FunctionLibrary")
	static int32 GetCurrentGameDifficulty(AActor* CallerReference);

	UFUNCTION(BlueprintCallable, Category = "PanWar|FunctionLibrary")
	static void SaveCurrentGameDifficulty(EPanWarGameDifficulty InDifficultyToSave);

	UFUNCTION(BlueprintCallable, Category = "PanWar|FunctionLibrary")
	static bool TryLoadSavedGameDifficulty(EPanWarGameDifficulty& OutSavedDifficulty);

	UFUNCTION(BlueprintCallable, Category = "PanWar|FunctionLibrary")
	static void SaveCurrentGameLevel(EPanWarLevel InCurrentGameLevelToSave);

	UFUNCTION(BlueprintCallable, Category = "PanWar|FunctionLibrary")
	static bool TryLoadSavedCurrentGameLevel(EPanWarLevel& OutSavedCurrentGameLevel);

	UFUNCTION(BlueprintCallable, Category = "PanWar|FunctionLibrary", meta = (Latent, WorldContext = "WorldContextObject", LatentInfo = "LatentInfo", ExpandEnumAsExecs = "CountDownInput|CountDownOutput", TotalTime = "1.0", UpdateInterval = "0.1"))
	static void CountDown(const UObject* WorldContextObject, float TotalTime, float UpdateInterval,
		float& OutRemainingTime, EPanWarCountDownActionInput CountDownInput,
		UPARAM(DisplayName = "Output") EPanWarCountDownActionOutput& CountDownOutput, FLatentActionInfo LatentInfo);

	UFUNCTION(BlueprintCallable, Category = "PanWar|FunctionLibrary", meta = (WorldContext = "WorldContextObject"))
	static void ToggleInputMode(const UObject* WorldContextObject, EPanWarInputMode InInputMode);

	UFUNCTION(BlueprintPure, Category = "PanWar|FunctionLibrary", meta = (WorldContext = "WorldContextObject"))
	static UPanWarGameInstance* GetPanWarGameInstance(const UObject* WorldContextObject);


};
