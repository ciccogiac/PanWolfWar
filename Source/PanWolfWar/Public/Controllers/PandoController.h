// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GenericTeamAgentInterface.h"
#include "PandoController.generated.h"

/**
 * 
 */
UCLASS()
class PANWOLFWAR_API APandoController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	APandoController();

	//~ Begin IGenericTeamAgentInterface Interface.
	virtual FGenericTeamId GetGenericTeamId() const override;
	//~ End IGenericTeamAgentInterface Interface

private:
	FGenericTeamId PandoTeamID;

};
