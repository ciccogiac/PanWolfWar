// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlowerCable.generated.h"

class UCableComponent;

UCLASS()
class PANWOLFWAR_API AFlowerCable : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlowerCable();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params")
	UCableComponent* CableComponent;
};
