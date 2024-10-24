// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/PickUps/PanWarPickUpBase.h"
#include "Interfaces/InteractInterface.h"
#include "PanWarStoneBase.generated.h"

/**
 * 
 */
UCLASS()
class PANWOLFWAR_API APanWarStoneBase : public APanWarPickUpBase
{
	GENERATED_BODY()
	
protected:
	virtual void OnPickUpCollisionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Stone Consumed"))
	void BP_OnStoneConsumed();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Stone Check"))
	bool BP_OnStoneCheck(AActor* OtherActor);

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stone Params", meta = (AllowPrivateAccess = "true"))
	float StoneValue = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stone Params", meta = (AllowPrivateAccess = "true"))
	EStoneTypes StoneType;
};
