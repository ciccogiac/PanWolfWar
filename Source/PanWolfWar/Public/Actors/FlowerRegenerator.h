// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlowerRegenerator.generated.h"

UCLASS()
class PANWOLFWAR_API AFlowerRegenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	AFlowerRegenerator();

protected:
	virtual void BeginPlay() override;


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact Params", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* BoxComponent;

	UFUNCTION()
	void BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

};
