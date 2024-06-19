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



	virtual bool Interact( ACharacter* _CharacterOwner = nullptr);
	void SetMovingState(const bool state , const float WalkSpeed , const float Mass);
	virtual void Move(const FInputActionValue& Value) override;

	void MoveObject(FVector2D& MovementVector);


private:

	void SetObjectlocation(bool bForwardDirection, float DirectionSpeed, bool bRotate = false);
	void SetCharacterPosition();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Object Movement", meta = (AllowPrivateAccess = "true"))
	class UPhysicsConstraintComponent* PhysicsConstraintComponent;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsMovingObject = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Movement", meta = (AllowPrivateAccess = "true"))
	float InterpSpeed = 4.F;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Movement", meta = (AllowPrivateAccess = "true"))
	float RotationMultiplier = 5.F;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Movement", meta = (AllowPrivateAccess = "true"))
	float FWD_DirectionSpeed = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Movement", meta = (AllowPrivateAccess = "true"))
	float RIGHT_DirectionSpeed = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Object Movement", meta = (AllowPrivateAccess = "true"))
	float TURN_DirectionSpeed = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
	float FWD_Offset_Character = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Movement", meta = (AllowPrivateAccess = "true"))
	float FWD_AcceptanceOffset_Character = 4.f;
	

	FORCEINLINE FVector GetBoxPosition() const { return BoxComponent->GetComponentLocation(); }
	FORCEINLINE FVector GetBoxForward() const { return BoxComponent->GetForwardVector(); }
	FORCEINLINE FVector GetBoxRight() const { return BoxComponent->GetRightVector(); }

};
