// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PanWarTypes/PanWarEnumTypes.h"
#include "PanWarBaseGameMode.generated.h"

/**
 * 
 */
UCLASS()
class PANWOLFWAR_API APanWarBaseGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	APanWarBaseGameMode();

	UFUNCTION(BlueprintCallable)
	void CancelAllTimer();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayerDie();

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Settings")
	EPanWarGameDifficulty CurrentGameDifficulty;

public:
	FORCEINLINE EPanWarGameDifficulty GetCurrentGameDifficulty() const { return CurrentGameDifficulty; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetCurrentGameDifficulty(EPanWarGameDifficulty PanWarGameDifficulty) { CurrentGameDifficulty = PanWarGameDifficulty; }
};
