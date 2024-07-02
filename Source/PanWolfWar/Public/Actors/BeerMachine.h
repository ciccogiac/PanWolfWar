// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/InteractableObject.h"
#include "BeerMachine.generated.h"

class UWidgetComponent;
class USoundCue;

UCLASS()
class PANWOLFWAR_API ABeerMachine : public AInteractableObject
{
	GENERATED_BODY()
	
public:
	ABeerMachine();


	virtual bool Interact(ACharacter* _CharacterOwner = nullptr) override;


	virtual void BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact Params", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* BeerWidget;

	UPROPERTY(EditAnywhere, Category = "Audio")
	USoundCue* SoundCue;

};
