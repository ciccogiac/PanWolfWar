// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/InteractableObject.h"
#include "CoverPassage.generated.h"

UENUM(BlueprintType)
enum class ECoverPassageType : uint8
{
	ECPT_Wall UMETA(DisplayName = "Wall"),
	ECPT_Narrow UMETA(DisplayName = "Narrow")
};

UCLASS()
class PANWOLFWAR_API ACoverPassage : public AActor
{
	GENERATED_BODY()
	
public:
	ACoverPassage();

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact Params", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* BoxComponent;

	class UArrowComponent* ArrowComponent;

	//bool bNarrowEntering = false;

	UFUNCTION()
	virtual void BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Params, meta = (AllowPrivateAccess = "true"))
	ECoverPassageType CoverPassageType ;
};
