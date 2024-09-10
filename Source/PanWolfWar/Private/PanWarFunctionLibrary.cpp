#include "PanWarFunctionLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Kismet/GameplayStatics.h"

bool UPanWarFunctionLibrary::IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn)
{
	check(QueryPawn && TargetPawn);

	IGenericTeamAgentInterface* QueryTeamAgent = Cast<IGenericTeamAgentInterface>(QueryPawn->GetController());
	IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(TargetPawn->GetController());

	if (QueryTeamAgent && TargetTeamAgent)
	{
		return QueryTeamAgent->GetGenericTeamId() != TargetTeamAgent->GetGenericTeamId();
	}

	return false;
}

bool UPanWarFunctionLibrary::IsValidBlock(AActor* InAttacker, AActor* InDefender)
{
	check(InAttacker && InDefender);

	const float DotResult = FVector::DotProduct(InAttacker->GetActorForwardVector(), InDefender->GetActorForwardVector());
	return DotResult < -0.1f;
}

void UPanWarFunctionLibrary::SaveCurrentGameDifficulty(EPanWarGameDifficulty InDifficultyToSave)
{
   /* USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(UWarriorSaveGame::StaticClass());

    if (UWarriorSaveGame* WarriorSaveGameObject = Cast<UWarriorSaveGame>(SaveGameObject))
    {
        WarriorSaveGameObject->SavedCurrentGameDifficulty = InDifficultyToSave;

        const bool bWasSaved = UGameplayStatics::SaveGameToSlot(WarriorSaveGameObject, WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0);

        Debug::Print(bWasSaved ? TEXT("Difficulty Saved") : TEXT("Difficulty NOT Saved"));
    }*/
}

bool UPanWarFunctionLibrary::TryLoadSavedGameDifficulty(EPanWarGameDifficulty& OutSavedDifficulty)
{
   /* if (UGameplayStatics::DoesSaveGameExist(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0))
    {
        USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0);

        if (UWarriorSaveGame* WarriorSaveGameObject = Cast<UWarriorSaveGame>(SaveGameObject))
        {
            OutSavedDifficulty = WarriorSaveGameObject->SavedCurrentGameDifficulty;

            Debug::Print(TEXT("Loading Successful"), FColor::Green);

            return true;
        }
    }*/

    return false;
}
