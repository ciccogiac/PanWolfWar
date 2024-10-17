#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissionManager.generated.h"

class ABaseEnemy;
class APanWolfWarCharacter;
class UPandoUIComponent;

UENUM(BlueprintType)
enum class EMissionType : uint8
{
	EMT_KillEnemies UMETA(DisplayName = "KillEnemies"),
	EMT_ReachLocation UMETA(DisplayName = "ReachLocation")
};

USTRUCT(BlueprintType)
struct FMissionValues
{
	GENERATED_BODY()

public:

	UPROPERTY(EditInstanceOnly, Category = "Mission")
	EMissionType MissionType;

	UPROPERTY(EditInstanceOnly, Category = "Mission",  meta = (AllowPrivateAccess = "true"))
	TArray<ABaseEnemy*> EnemiesToKill;

	UPROPERTY(EditInstanceOnly, Category = "Mission" , meta = (AllowPrivateAccess = "true"))
	AActor* MissionTargetActor;

	UPROPERTY(EditInstanceOnly, Category = "Mission", meta = (AllowPrivateAccess = "true"))
	FText MissionText;
};


UCLASS()
class PANWOLFWAR_API AMissionManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AMissionManager();

	void LoadMission();

protected:
	virtual void BeginPlay() override;

private:

	void LoadKillEnemiesMission(FMissionValues& Mission);
	void LoadReachLocationMission(FMissionValues& Mission);

	UFUNCTION()
	void OnEnemyDeathHandler(ABaseEnemy* Enemy);

private:
	APanWolfWarCharacter* PanWolfCharacter;
	UPandoUIComponent* PandoUIComponent;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission", meta = (AllowPrivateAccess = "true"))
	TArray<FMissionValues> Missions;

	int32 CurrentMission = 0;
	EMissionType CurrentMissionType;
};
