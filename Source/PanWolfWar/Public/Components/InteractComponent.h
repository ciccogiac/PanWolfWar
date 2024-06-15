// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/InteractInterface.h"
#include "InteractComponent.generated.h"

DECLARE_DELEGATE(FOnEnterInteractState)
DECLARE_DELEGATE(FOnExitInteractState)

class UCharacterMovementComponent;
class UCapsuleComponent;
struct FInputActionValue;
class UInputAction;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UInteractComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Delegates

public:

	//Delegates
	FOnEnterInteractState OnEnterInteractStateDelegate;
	FOnExitInteractState OnExitInteractStateDelegate;

#pragma endregion

public:	
	// Sets default values for this component's properties
	UInteractComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void Interact();
	void MoveObject(const FInputActionValue& Value);
	void SetOverlappingObject(AInteractableObject* InteractableObject);
		
private:

	AActor* ActorOwner;
	ACharacter* CharacterOwner;
	UCharacterMovementComponent* MovementComponent;
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY(VisibleInstanceOnly)
	class AInteractableObject* OverlappingObject;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	bool bIsMovingObject = false;

#pragma region InputActions

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Interact")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Interact")
	UInputAction* InteractMoveAction;

#pragma endregion
};
