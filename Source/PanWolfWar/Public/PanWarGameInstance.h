// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "PanWarGameInstance.generated.h"

UENUM(BlueprintType)
enum class EPanWarLevel: uint8
{
	MainMenuMap,
	SurvivalGameModeMap
};

USTRUCT(BlueprintType)
struct FPanWarGameLevelSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "GameData.Level"))
	EPanWarLevel EnumLevel;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> Level;

	bool IsValid() const
	{
		return !Level.IsNull();
	}


};

/**
 * 
 */
UCLASS()
class PANWOLFWAR_API UPanWarGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

protected:
	virtual void OnPreLoadMap(const FString& MapName);
	virtual void OnDestinationWorldLoaded(UWorld* LoadedWorld);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FPanWarGameLevelSet> GameLevelSets;

public:
	UFUNCTION(BlueprintPure)
	TSoftObjectPtr<UWorld> GetGameLevelByEnum(EPanWarLevel InEnumLevel) const;
};
