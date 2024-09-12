// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/BaseBoss.h"
#include "Engine/AssetManager.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameModes/PanWarSurvivalGameMode.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "PanWolfWar/DebugHelper.h"

void ABaseBoss::BeginPlay()
{
	Super::BeginPlay();

}

void ABaseBoss::SummonsEnemies()
{
	if (AnimInstance && SummonEnemiesMontage)
	{
		AnimInstance->Montage_Play(SummonEnemiesMontage);
	}
}

void ABaseBoss::SummonsEnemies_Notify()
{

    if (ensure(!SoftEnemyClassToSpawn.IsNull()))
    {
        Debug::Print(TEXT("TryToLoad"));

        UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(
            SoftEnemyClassToSpawn.ToSoftObjectPath(),
            FStreamableDelegate::CreateUObject(this, &ThisClass::OnSummonEnemyClassLoaded)
        );
    }
   
}

void ABaseBoss::OnSummonEnemyClassLoaded()
{
    Debug::Print(TEXT("Loaded"));

    UClass* LoadedClass = SoftEnemyClassToSpawn.Get();
    UWorld* World = GetWorld();

    if (!LoadedClass || !World) return;


    TArray<ABaseEnemy*> SpawnedEnemies;

    FActorSpawnParameters SpawnParam;
    SpawnParam.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    for (int32 i = 0; i < N_EnemyToSpawn; i++)
    {
        FVector RandomLocation;
        UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this, GetActorLocation(), RandomLocation, RandomSpawmRadius);

        RandomLocation += FVector(0.f, 0.f, 150.f);

        const FRotator SpawnFacingRotation = GetActorForwardVector().ToOrientationRotator();

        ABaseEnemy* SpawnedEnemy = World->SpawnActor<ABaseEnemy>(LoadedClass, RandomLocation, SpawnFacingRotation, SpawnParam);

        if (SpawnedEnemy)
        {
            SpawnedEnemies.Add(SpawnedEnemy);
        }
    }

    UBlackboardComponent* BlackboardComponent =  UAIBlueprintHelperLibrary::GetBlackboard(this);
    if (!BlackboardComponent) return;
    BlackboardComponent->SetValueAsBool(FName("HasSpawnedEnemy"), true);

    if (SpawnedEnemies.IsEmpty()) return;  
    AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
    if (!GameMode) return; 
    APanWarSurvivalGameMode* SurvivalGameMode = Cast<APanWarSurvivalGameMode>(GameMode);
    if (!SurvivalGameMode) return;

    SurvivalGameMode->RegisterSpawnedEnemies(SpawnedEnemies);
         
    
}
