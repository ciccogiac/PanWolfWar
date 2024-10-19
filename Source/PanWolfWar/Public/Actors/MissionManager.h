#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MissionManager.generated.h"

class ABaseEnemy;
class APanWolfWarCharacter;
class UPandoUIComponent;
class AMissionTargetReachable;
class AInteractableObject;

UENUM(BlueprintType)
enum class EMissionType : uint8
{
	EMT_KillEnemies UMETA(DisplayName = "KillEnemies"),
	EMT_ReachLocation UMETA(DisplayName = "ReachLocation"),
	EMT_InteractableObject UMETA(DisplayName = "InteractableObject")
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
	AMissionTargetReachable* MissionTargetReachable;

	UPROPERTY(EditInstanceOnly, Category = "Mission", meta = (AllowPrivateAccess = "true"))
	AInteractableObject* MissionInteractableObject;

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

	void MissionTargetReached(AMissionTargetReachable* MissionTargetReachable);

protected:
	virtual void BeginPlay() override;

private:

	void LoadKillEnemiesMission(FMissionValues& Mission);
	void LoadReachLocationMission(FMissionValues& Mission);
	void LoadInteractableObjectMission(FMissionValues& Mission);

	UFUNCTION()
	void OnEnemyDeathHandler(ABaseEnemy* Enemy);

	UFUNCTION()
	void OnObjectInteracted(AInteractableObject* InteractableObject);

	

private:
	APanWolfWarCharacter* PanWolfCharacter;
	UPandoUIComponent* PandoUIComponent;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Mission", meta = (AllowPrivateAccess = "true"))
	TArray<FMissionValues> Missions;

	int32 CurrentMission = 0;
	EMissionType CurrentMissionType;
};
