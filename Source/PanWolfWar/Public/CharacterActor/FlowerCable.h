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

	void HookCable(const FVector Hook_TargetLocation, const FRotator Hook_TargetRotation, const FVector CharacterLocation);


	void SetAttachEndCable(USceneComponent* Component, FName SocketName = NAME_None);
	void SetAttachStartCable(bool Value);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params")
	UCableComponent* CableComponent;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	float HookCable_Divisor = 2500.f;


};
