// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/PandoController.h"

APandoController::APandoController()
{
	PandoTeamID = FGenericTeamId(0);
}

FGenericTeamId APandoController::GetGenericTeamId() const
{
	return PandoTeamID;
}
