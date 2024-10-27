#include "PanWarFunctionLibrary.h"
#include "GenericTeamAgentInterface.h"
#include "Kismet/GameplayStatics.h"
#include "PanWarTypes/PanWarCountDownAction.h"
#include "PanWarGameInstance.h"
#include "SaveGame/PanWarSaveGame.h"
#include "GameModes/PanWarBaseGameMode.h"

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

bool UPanWarFunctionLibrary::IsPlayingAnyMontage_ExcludingBlendOut(UAnimInstance* OwningPlayerAnimInstance)
{
    if (!OwningPlayerAnimInstance) return false;

    // Ottieni il montaggio corrente
    UAnimMontage* CurrentMontage = OwningPlayerAnimInstance->GetCurrentActiveMontage();

    if (CurrentMontage && OwningPlayerAnimInstance->Montage_IsPlaying(CurrentMontage))
    {
        float CurrentMontagePosition = OwningPlayerAnimInstance->Montage_GetPosition(CurrentMontage);
        float MontageBlendOutTime = CurrentMontage->BlendOut.GetBlendTime();
        float MontageDuration = CurrentMontage->GetPlayLength();

        if ((CurrentMontagePosition >= MontageDuration - MontageBlendOutTime))
        {
            return false;
        }
        else
            return true;
    }
    else
        return false;
}

bool UPanWarFunctionLibrary::IsPlayingMontage_ExcludingBlendOut(UAnimInstance* OwningPlayerAnimInstance, UAnimMontage* AnimMontage)
{
    if (!OwningPlayerAnimInstance) return false;

    // Ottieni il montaggio corrente
    UAnimMontage* CurrentMontage = OwningPlayerAnimInstance->GetCurrentActiveMontage();
    if (CurrentMontage != AnimMontage) return false;

    if (CurrentMontage && OwningPlayerAnimInstance->Montage_IsPlaying(CurrentMontage))
    {
        float CurrentMontagePosition = OwningPlayerAnimInstance->Montage_GetPosition(CurrentMontage);
        float MontageBlendOutTime = CurrentMontage->BlendOut.GetBlendTime();
        float MontageDuration = CurrentMontage->GetPlayLength();

        if ((CurrentMontagePosition >= MontageDuration - MontageBlendOutTime))
        {
            return false;
        }
        else
            return true;
    }
    else
        return false;
}

int32 UPanWarFunctionLibrary::GetCurrentGameDifficulty(AActor* CallerReference)
{
    int32 CurrentDifficultyLevel = 1;

    if (APanWarBaseGameMode* BaseGameMode = CallerReference->GetWorld()->GetAuthGameMode<APanWarBaseGameMode>())
    {
        switch (BaseGameMode->GetCurrentGameDifficulty())
        {
        case EPanWarGameDifficulty::Easy:
            CurrentDifficultyLevel = 1;
            break;

        case EPanWarGameDifficulty::Normal:
            CurrentDifficultyLevel = 2;
            break;

        case EPanWarGameDifficulty::Hard:
            CurrentDifficultyLevel = 3;
            break;

        case EPanWarGameDifficulty::VeryHard:
            CurrentDifficultyLevel = 4;
            break;

        default:
            break;
        }
    }

    return CurrentDifficultyLevel;
}

void UPanWarFunctionLibrary::SaveCurrentGameDifficulty(EPanWarGameDifficulty InDifficultyToSave)
{
    // Prima, prova a caricare il gioco salvato esistente
    UPanWarSaveGame* PanWarSaveGameObject = Cast<UPanWarSaveGame>(UGameplayStatics::LoadGameFromSlot(FString("SaveGame.Slot.1"), 0));

    // Se il salvataggio non esiste, creane uno nuovo
    if (!PanWarSaveGameObject)
    {
        PanWarSaveGameObject = Cast<UPanWarSaveGame>(UGameplayStatics::CreateSaveGameObject(UPanWarSaveGame::StaticClass()));
    }

    // Aggiorna il livello, mantenendo gli altri dati intatti (ad esempio, la difficoltà)
    PanWarSaveGameObject->SavedCurrentGameDifficulty = InDifficultyToSave;

    // Ora salva il gioco
    UGameplayStatics::SaveGameToSlot(PanWarSaveGameObject, FString("SaveGame.Slot.1"), 0);
}

void UPanWarFunctionLibrary::SaveCurrentGameLevel(EPanWarLevel InCurrentGameLevelToSave)
{
    // Prima, prova a caricare il gioco salvato esistente
    UPanWarSaveGame* PanWarSaveGameObject = Cast<UPanWarSaveGame>(UGameplayStatics::LoadGameFromSlot(FString("SaveGame.Slot.1"), 0));

    // Se il salvataggio non esiste, creane uno nuovo
    if (!PanWarSaveGameObject)
    {
        PanWarSaveGameObject = Cast<UPanWarSaveGame>(UGameplayStatics::CreateSaveGameObject(UPanWarSaveGame::StaticClass()));
    }

    // Aggiorna il livello, mantenendo gli altri dati intatti (ad esempio, la difficoltà)
    PanWarSaveGameObject->CurrentGameLevel = InCurrentGameLevelToSave;

    // Ora salva il gioco
    UGameplayStatics::SaveGameToSlot(PanWarSaveGameObject, FString("SaveGame.Slot.1"), 0);
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

          /*  Debug::Print(TEXT("Loading Successful"), FColor::Green);*/

            return true;
        }
    } 

    return false;
}

void UPanWarFunctionLibrary::SaveCurrentGameLanguage(ELanguage InLanguageToSave)
{
    // Prima, prova a caricare il gioco salvato esistente
    UPanWarSaveGame* PanWarSaveGameObject = Cast<UPanWarSaveGame>(UGameplayStatics::LoadGameFromSlot(FString("SaveGame.Slot.1"), 0));

    // Se il salvataggio non esiste, creane uno nuovo
    if (!PanWarSaveGameObject)
    {
        PanWarSaveGameObject = Cast<UPanWarSaveGame>(UGameplayStatics::CreateSaveGameObject(UPanWarSaveGame::StaticClass()));
    }

    // Aggiorna il livello, mantenendo gli altri dati intatti (ad esempio, la difficoltà)
    PanWarSaveGameObject->CurrentLanguage = InLanguageToSave;

    // Ora salva il gioco
    UGameplayStatics::SaveGameToSlot(PanWarSaveGameObject, FString("SaveGame.Slot.1"), 0);
}

bool UPanWarFunctionLibrary::TryLoadSavedGameLanguage(ELanguage& OutSavedLanguage)
{
    /* if (UGameplayStatics::DoesSaveGameExist(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0))*/
    if (UGameplayStatics::DoesSaveGameExist(FString("SaveGame.Slot.1"), 0))
    {
        /* USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0);*/
        USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(FString("SaveGame.Slot.1"), 0);

        if (UPanWarSaveGame* PanWarSaveGameObject = Cast<UPanWarSaveGame>(SaveGameObject))
        {
            OutSavedLanguage = PanWarSaveGameObject->CurrentLanguage;

            /*  Debug::Print(TEXT("Loading Successful"), FColor::Green);*/

            return true;
        }
    }

    return false;
}

bool UPanWarFunctionLibrary::TryLoadSavedCurrentGameLevel(EPanWarLevel& OutSavedCurrentGameLevel)
{
    if (UGameplayStatics::DoesSaveGameExist(FString("SaveGame.Slot.1"), 0))
    {
        /* USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(WarriorGameplayTags::GameData_SaveGame_Slot_1.GetTag().ToString(), 0);*/
        USaveGame* SaveGameObject = UGameplayStatics::LoadGameFromSlot(FString("SaveGame.Slot.1"), 0);

        if (UPanWarSaveGame* PanWarSaveGameObject = Cast<UPanWarSaveGame>(SaveGameObject))
        {
            OutSavedCurrentGameLevel = PanWarSaveGameObject->CurrentGameLevel;
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
