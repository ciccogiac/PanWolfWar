// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/PanWarBaseGameMode.h"

APanWarBaseGameMode::APanWarBaseGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true; 
}

void APanWarBaseGameMode::CancelAllTimer()
{

    // Ottieni il PlayerController dell'indice 0 (il primo giocatore)
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

    if (PlayerController)
    {
        // Ottieni il Pawn controllato (il player)
        APawn* PlayerPawn = PlayerController->GetPawn();

        if (PlayerPawn)
        {
            // Cancella tutti i timer attivi per il PlayerPawn
            GetWorld()->GetTimerManager().ClearAllTimersForObject(PlayerPawn);
        }
    }
}


