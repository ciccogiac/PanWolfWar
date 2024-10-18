#include "Actors/MissionManager.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Enemy/BaseEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Components/UI/PandoUIComponent.h"
#include "Actors/MissionTargetReachable.h"

#include "PanWolfWar/DebugHelper.h"


#pragma region EngineFunctions

AMissionManager::AMissionManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMissionManager::BeginPlay()
{
	Super::BeginPlay();

   PanWolfCharacter = Cast<APanWolfWarCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
   if (PanWolfCharacter)
   {
	   PandoUIComponent = PanWolfCharacter->GetPandoUIComponent();
   }

}

#pragma endregion

void AMissionManager::LoadMission()
{
	if (Missions.IsEmpty()) return;
	if (!Missions.IsValidIndex(CurrentMission)) return;
	if (!PanWolfCharacter) return;

	FMissionValues& Mission = Missions[CurrentMission];
	EMissionType MissionType = Mission.MissionType;

	if (PandoUIComponent)
		PandoUIComponent->OnNewHintDelegate.Broadcast(Mission.MissionText);


	switch (MissionType)
	{
	case EMissionType::EMT_KillEnemies:
		CurrentMissionType = EMissionType::EMT_KillEnemies;
		LoadKillEnemiesMission(Mission);
		break;

	case EMissionType::EMT_ReachLocation:
		CurrentMissionType = EMissionType::EMT_ReachLocation;
		LoadReachLocationMission(Mission);
		break;

	default:
		break;
	}
}

#pragma region KillEnemiesMission

void AMissionManager::LoadKillEnemiesMission(FMissionValues& Mission)
{
	TArray<ABaseEnemy*>& CurrentEnemiesToKill = Mission.EnemiesToKill;

	for (ABaseEnemy* Enemy : CurrentEnemiesToKill)
	{
		if (IsValid(Enemy)) 
		{
			Enemy->OnEnemyDeath.AddDynamic(this, &AMissionManager::OnEnemyDeathHandler);
			if (PandoUIComponent) PandoUIComponent->OnEnemyActorTargetDelegate.Broadcast(Enemy);
		}
		else { CurrentEnemiesToKill.Remove(Enemy); }		
	}

	if (CurrentEnemiesToKill.IsEmpty())
	{
		CurrentMission++;
		LoadMission();
	}
}

void AMissionManager::OnEnemyDeathHandler(ABaseEnemy* Enemy)
{
	if (CurrentMissionType != EMissionType::EMT_KillEnemies) return;
	if (Missions.IsEmpty()) return;
	if (!Missions.IsValidIndex(CurrentMission)) return;

	TArray<ABaseEnemy*>& CurrentEnemiesToKill = Missions[CurrentMission].EnemiesToKill;

	if (CurrentEnemiesToKill.Contains(Enemy))
	{
		CurrentEnemiesToKill.Remove(Enemy);
		if (PandoUIComponent) PandoUIComponent->OnEnemyActorTargetDelegate.Broadcast(Enemy);
	}

	if (CurrentEnemiesToKill.IsEmpty())
	{
		CurrentMission++;
		LoadMission();  
	}
}

#pragma endregion

#pragma region ReachLocationMission

void AMissionManager::LoadReachLocationMission(FMissionValues& Mission)
{

	if (Mission.MissionTargetReachable && Mission.MissionTargetReachable->IsCharacterInside())
	{
		Mission.MissionTargetReachable->BP_OnTargetReached();
		PandoUIComponent->OnTargetActorChangedDelegate.Broadcast(nullptr);
		CurrentMission++;
		LoadMission();
		return;
	}

	if (PandoUIComponent)
		PandoUIComponent->OnTargetActorChangedDelegate.Broadcast(Mission.MissionTargetReachable);

	Mission.MissionTargetReachable->BP_OnTargetMissionActivated();
}

void AMissionManager::MissionTargetReached(AMissionTargetReachable* MissionTargetReachable)
{
	if (CurrentMissionType != EMissionType::EMT_ReachLocation) return;
	if (Missions.IsEmpty()) return;
	if (!Missions.IsValidIndex(CurrentMission)) return;

	AMissionTargetReachable* CurrentMissionTargetReachable = Missions[CurrentMission].MissionTargetReachable;

	if (CurrentMissionTargetReachable == MissionTargetReachable)
	{
		CurrentMissionTargetReachable->BP_OnTargetReached();
		PandoUIComponent->OnTargetActorChangedDelegate.Broadcast(nullptr);
		CurrentMission++;
		LoadMission();
	}
}

#pragma endregion


	

