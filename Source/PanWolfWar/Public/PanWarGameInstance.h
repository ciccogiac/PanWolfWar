#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include <PanWarTypes/PanWarEnumTypes.h>
#include "PanWarGameInstance.generated.h"




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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UDataTable* TutorialDataTable;

	TSet<FName> AlreadySeenTutorial;

	int32 CurrentLevelBattle_WaveCount = 1;

	ELanguage GameLanguage;
	EPanWarLevel CurrentLevel;

public:
	UFUNCTION(BlueprintPure)
	TSoftObjectPtr<UWorld> GetGameLevelByEnum(EPanWarLevel InEnumLevel) const;

	UDataTable* GetTutorialDataTable();

	UFUNCTION(BlueprintPure)
	ELanguage GetGameLanguage();

	UFUNCTION(BlueprintCallable)
	void SetGameLanguage(ELanguage Language);

public:

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsTutorialAlreadySeen(FName TutorialName) const { return AlreadySeenTutorial.Contains(TutorialName); }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void MarkTutorialSeen(FName TutorialName)  {  AlreadySeenTutorial.Add(TutorialName); }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ResetTutorialSeen() { AlreadySeenTutorial.Empty(); }

	UFUNCTION(BlueprintCallable)
	void SetCurrentLevelBattle_WaveCount(int32 WaveCount) { CurrentLevelBattle_WaveCount = WaveCount; };

	UFUNCTION(BlueprintCallable)
	int32 GetCurrentLevelBattle_WaveCount() const { return CurrentLevelBattle_WaveCount; };

	UFUNCTION(BlueprintCallable)
	void SetCurrentLevel(EPanWarLevel InLevel) { CurrentLevel = InLevel; };

	UFUNCTION(BlueprintCallable)
	EPanWarLevel GetCurrentLevel() const { return CurrentLevel; };

};
