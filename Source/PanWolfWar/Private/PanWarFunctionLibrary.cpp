#include "PanWarFunctionLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Kismet/GameplayStatics.h"
#include "PanWarTypes/PanWarCountDownAction.h"
#include "PanWarGameInstance.h"
#include "SaveGame/PanWarSaveGame.h"

#include "PanWolfWar/DebugHelper.h"

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
    USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(UPanWarSaveGame::StaticClass());

    if (UPanWarSaveGame* PanWarSaveGameObject = Cast<UPanWarSaveGame>(SaveGameObject))
    {
        PanWarSaveGameObject->SavedCurrentGameDifficulty = InDifficultyToSave;

       /* const bool bWasSaved = UGameplayStatics::SaveGameToSlot(PanWarSaveGameObject, WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0);*/
        const bool bWasSaved = UGameplayStatics::SaveGameToSlot(PanWarSaveGameObject, FString("SaveGame.Slot.1"), 0);

        Debug::Print(bWasSaved ? TEXT("Difficulty Saved") : TEXT("Difficulty NOT Saved"));
    }
}

bool UPanWarFunctionLibrary::TryLoadSavedGameDifficulty(EPanWarGameDifficulty& OutSavedDifficulty)
{
   /* if (UGameplayStatics::DoesSaveGameExist(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0))*/
    if (UGameplayStatics::DoesSaveGameExist(FString("SaveGame.Slot.1"), 0))
    {
       /* USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0);*/
        USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(FString("SaveGame.Slot.1"), 0);

        if (UPanWarSaveGame* PanWarSaveGameObject = Cast<UPanWarSaveGame>(SaveGameObject))
        {
            OutSavedDifficulty = PanWarSaveGameObject->SavedCurrentGameDifficulty;

            Debug::Print(TEXT("Loading Successful"), FColor::Green);

            return true;
        }
    }

    return false;
}

void UPanWarFunctionLibrary::CountDown(const UObject* WorldContextObject, float TotalTime, float UpdateInterval, float& OutRemainingTime, EPanWarCountDownActionInput CountDownInput, UPARAM(DisplayName = "Output") EPanWarCountDownActionOutput& CountDownOutput, FLatentActionInfo LatentInfo)
{
    UWorld* World = nullptr;

    if (GEngine)
    {
        World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    }

    if (!World)
    {
        return;
    }

    FLatentActionManager& LatentActionManager = World->GetLatentActionManager();

    FPanWarCountDownAction* FoundAction = LatentActionManager.FindExistingAction<FPanWarCountDownAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

    if (CountDownInput == EPanWarCountDownActionInput::Start)
    {
        if (!FoundAction)
        {
            LatentActionManager.AddNewAction(
                LatentInfo.CallbackTarget,
                LatentInfo.UUID,
                new FPanWarCountDownAction(TotalTime, UpdateInterval, OutRemainingTime, CountDownOutput, LatentInfo)
            );
        }
    }

    if (CountDownInput == EPanWarCountDownActionInput::Cancel)
    {
        if (FoundAction)
        {
            FoundAction->CancelAction();
        }
    }
}

void UPanWarFunctionLibrary::ToggleInputMode(const UObject* WorldContextObject, EPanWarInputMode InInputMode)
{
    APlayerController* PlayerController = nullptr;

    if (GEngine)
    {
        if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
        {
            PlayerController = World->GetFirstPlayerController();
        }
    }

    if (!PlayerController)
    {
        return;
    }

    FInputModeGameOnly GameOnlyMode;
    FInputModeUIOnly UIOnlyMode;

    switch (InInputMode)
    {
    case EPanWarInputMode::GameOnly:

        PlayerController->SetInputMode(GameOnlyMode);
        PlayerController->bShowMouseCursor = false;

        break;

    case EPanWarInputMode::UIOnly:

        PlayerController->SetInputMode(UIOnlyMode);
        PlayerController->bShowMouseCursor = true;

        break;

    default:
        break;
    }
}

UPanWarGameInstance* UPanWarFunctionLibrary::GetPanWarGameInstance(const UObject* WorldContextObject)
{
    if (GEngine)
    {
        if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
        {
            return World->GetGameInstance<UPanWarGameInstance>();
        }
    }

    return nullptr;
}
