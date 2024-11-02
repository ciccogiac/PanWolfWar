// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/PanWarSurvivalGameMode.h"
#include "Engine/AssetManager.h"
#include "Enemy/BaseEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "NavigationSystem.h"
#include "PanWarFunctionLibrary.h"

#include "PanWolfWar/DebugHelper.h"

void APanWarSurvivalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	EPanWarGameDifficulty SavedGameDifficulty;

	if (UPanWarFunctionLibrary::TryLoadSavedGameDifficulty(SavedGameDifficulty))
	{
		CurrentGameDifficulty = SavedGameDifficulty;
	}
}

void APanWarSurvivalGameMode::BeginPlay()
{
	Super::BeginPlay();

	checkf(EnemyWaveSpawnerDataTable, TEXT("Forgot to assign a valid datat table in survial game mode blueprint"));

	/*SetCurrentSurvivalGameModeState(EPanWarSurvivalGameModeState::WaitSpawnNewWave);*/
	SetCurrentSurvivalGameModeState(EPanWarSurvivalGameModeState::InProgress);

	TotalWavesToSpawn = EnemyWaveSpawnerDataTable->GetRowNames().Num();

	PreLoadNextWaveEnemies();

}

void APanWarSurvivalGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (CurrentSurvivalGameModeState)
	{

	case EPanWarSurvivalGameModeState::WaitSpawnNewWave:
		HandleWaitSpawnNewWave(DeltaTime);
		break;

	case EPanWarSurvivalGameModeState::SpawningNewWave:
		HandleSpawningNewWave(DeltaTime);
		break;

	case EPanWarSurvivalGameModeState::WaveCompleted:
		HandleWaveCompleted(DeltaTime);
		break;

	default:
		break;
	}

}

void APanWarSurvivalGameMode::HandleWaveCompleted(float DeltaTime)
{
	TimePassedSinceStart += DeltaTime;
	if (TimePassedSinceStart >= WaveCompletedWaitTime)
	{
		TimePassedSinceStart = 0.f;

		CurrentWaveCount++;

		if (HasFinishedAllWaves())
		{
			SetCurrentSurvivalGameModeState(EPanWarSurvivalGameModeState::AllWavesDone);
		}
		else
		{
			SetCurrentSurvivalGameModeState(EPanWarSurvivalGameModeState::WaitSpawnNewWave);
			PreLoadNextWaveEnemies();
		}
	}
}

void APanWarSurvivalGameMode::HandleSpawningNewWave(float DeltaTime)
{
	TimePassedSinceStart += DeltaTime;
	if (TimePassedSinceStart >= SpawnEnemiesDelayTime && PreLoadedEnemyClassMap.Num() >= GetCurrentWaveSpawnerTableRow()->EnemyWaveSpawnerDefinitions.Num())
	{
		CurrentSpawnedEnemiesCounter += TrySpawnWaveEnemies();

		TimePassedSinceStart = 0.f;

		SetCurrentSurvivalGameModeState(EPanWarSurvivalGameModeState::InProgress);
	}
}

void APanWarSurvivalGameMode::HandleWaitSpawnNewWave(float DeltaTime)
{
	TimePassedSinceStart += DeltaTime;
	if (TimePassedSinceStart >= SpawnNewWaveWaitTime)
	{
		TimePassedSinceStart = 0.f;

		SetCurrentSurvivalGameModeState(EPanWarSurvivalGameModeState::SpawningNewWave);
	}
}

void APanWarSurvivalGameMode::SetCurrentSurvivalGameModeState(EPanWarSurvivalGameModeState InState)
{
	CurrentSurvivalGameModeState = InState;
	OnSurvivalGameModeStateChanged.Broadcast(CurrentSurvivalGameModeState);
}

bool APanWarSurvivalGameMode::HasFinishedAllWaves() const
{
	return CurrentWaveCount > TotalWavesToSpawn;
}

void APanWarSurvivalGameMode::PreLoadNextWaveEnemies()
{
	if (HasFinishedAllWaves())
	{
		return;
	}

	PreLoadedEnemyClassMap.Empty();

	for (const FPanWarEnemyWaveSpawnerInfo& SpawnerInfo : GetCurrentWaveSpawnerTableRow()->EnemyWaveSpawnerDefinitions)
	{
		if (SpawnerInfo.SoftEnemyClassToSpawn.IsNull()) continue;

		UAssetManager::GetStreamableManager().RequestAsyncLoad(
			SpawnerInfo.SoftEnemyClassToSpawn.ToSoftObjectPath(),
			FStreamableDelegate::CreateLambda(
				[SpawnerInfo, this]()
				{
					if (UClass* LoadedEnemyClass = SpawnerInfo.SoftEnemyClassToSpawn.Get())
					{
						PreLoadedEnemyClassMap.Emplace(SpawnerInfo.SoftEnemyClassToSpawn, LoadedEnemyClass);
					}
				}
			)
		);
	}
}

FPanWarEnemyWaveSpawnerTableRow* APanWarSurvivalGameMode::GetCurrentWaveSpawnerTableRow() const
{
	const FName RowName = FName(TEXT("Wave") + FString::FromInt(CurrentWaveCount));

	FPanWarEnemyWaveSpawnerTableRow* FoundRow = EnemyWaveSpawnerDataTable->FindRow<FPanWarEnemyWaveSpawnerTableRow>(RowName, FString());

	checkf(FoundRow, TEXT("Could not find a valid row under the name %s in the data table"), *RowName.ToString());

	return FoundRow;
}

int32 APanWarSurvivalGameMode::TrySpawnWaveEnemies()
{
	if (TargetPointsArray.IsEmpty())
	{
		UGameplayStatics::GetAllActorsOfClass(this, ATargetPoint::StaticClass(), TargetPointsArray);
	}

	checkf(!TargetPointsArray.IsEmpty(), TEXT("No valid target point found in level: %s for spawning enemies"), *GetWorld()->GetName());

	uint32 EnemiesSpawnedThisTime = 0;

	FActorSpawnParameters SpawnParam;
	SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (const FPanWarEnemyWaveSpawnerInfo& SpawnerInfo : GetCurrentWaveSpawnerTableRow()->EnemyWaveSpawnerDefinitions)
	{
		if (SpawnerInfo.SoftEnemyClassToSpawn.IsNull()) continue;

		const int32 NumToSpawn = FMath::RandRange(SpawnerInfo.MinPerSpawnCount, SpawnerInfo.MaxPerSpawnCount);

		UClass* LoadedEnemyClass = PreLoadedEnemyClassMap.FindChecked(SpawnerInfo.SoftEnemyClassToSpawn);

		for (int32 i = 0; i < NumToSpawn; i++)
		{
			const int32 RandomTargetPointIndex = FMath::RandRange(0, TargetPointsArray.Num() - 1);
			const FVector SpawnOrigin = TargetPointsArray[RandomTargetPointIndex]->GetActorLocation();
			const FRotator SpawnRotation = TargetPointsArray[RandomTargetPointIndex]->GetActorForwardVector().ToOrientationRotator();

			FVector RandomLocation;
			UNavigationSystemV1::K2_GetRandomLocationInNavigableRadius(this, SpawnOrigin, RandomLocation, 400.f);

			RandomLocation += FVector(0.f, 0.f, 150.f);
			//RandomLocation += FVector(0.f, 0.f, 15.f);

			ABaseEnemy* SpawnedEnemy = GetWorld()->SpawnActor<ABaseEnemy>(LoadedEnemyClass, RandomLocation, SpawnRotation, SpawnParam);

			if (SpawnedEnemy)
			{
				SpawnedEnemy->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnEnemyDestroyed);

				EnemiesSpawnedThisTime++;
				TotalSpawnedEnemiesThisWaveCounter++;
			}

			if (!ShouldKeepSpawnEnemies())
			{
				return EnemiesSpawnedThisTime;
			}
		}
	}

	return EnemiesSpawnedThisTime;

}

bool APanWarSurvivalGameMode::ShouldKeepSpawnEnemies() const
{
	return TotalSpawnedEnemiesThisWaveCounter < GetCurrentWaveSpawnerTableRow()->TotalEnemyToSpawnThisWave;
}

void APanWarSurvivalGameMode::OnEnemyDestroyed(AActor* DestroyedActor)
{
	CurrentSpawnedEnemiesCounter--;

	if (ShouldKeepSpawnEnemies())
	{
		CurrentSpawnedEnemiesCounter += TrySpawnWaveEnemies();
	}
	else if (CurrentSpawnedEnemiesCounter == 0)
	{
		TotalSpawnedEnemiesThisWaveCounter = 0;
		CurrentSpawnedEnemiesCounter = 0;

		SetCurrentSurvivalGameModeState(EPanWarSurvivalGameModeState::WaveCompleted);
	}
}

void APanWarSurvivalGameMode::RegisterSpawnedEnemies(const TArray<ABaseEnemy*>& InEnemiesToRegister)
{
	for (ABaseEnemy* SpawnedEnemy : InEnemiesToRegister)
	{
		if (SpawnedEnemy)
		{
			CurrentSpawnedEnemiesCounter++;

			SpawnedEnemy->OnDestroyed.AddUniqueDynamic(this, &ThisClass::OnEnemyDestroyed);
		}
	}
}