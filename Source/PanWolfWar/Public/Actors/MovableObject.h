// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/BoxComponent.h"

#include "Actors/InteractableObject.h"
#include "MovableObject.generated.h"


class BoxComponent;

UCLASS()
class PANWOLFWAR_API AMovableObject : public AInteractableObject
{
	GENERATED_BODY()
	
public:	
	AMovableObject();

	virtual void Interact(bool bStartInteraction = true) override;

	void SetObjectlocation(bool bForwardDirection, float DirectionSpeed);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;


private:



	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UPhysicsConstraintComponent* PhysicsConstraintComponent;
	

public:
	FORCEINLINE FVector GetBoxPosition() const { return BoxComponent->GetComponentLocation(); }
	FORCEINLINE FVector GetBoxForward() const { return BoxComponent->GetForwardVector(); }

};
