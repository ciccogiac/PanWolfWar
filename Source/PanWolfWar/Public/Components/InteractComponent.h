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
class AMovableObject;
class UTransformationComponent;

UENUM(BlueprintType)
enum class EInteractState : uint8
{
	EIS_NOTinteracting UMETA(DisplayName = "NOTinteracting"),
	EIS_Interacting UMETA(DisplayName = "Interacting"),
	EIS_MovingObject UMETA(DisplayName = "MovingObject")
};

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
	UInteractComponent();
	
	void Interact();
	void InteractMove(const FInputActionValue& Value);
	bool SetOverlappingObject(AInteractableObject* _InteractableObject, bool bEnter );
	void ResetOverlappingObject();

		
protected:
	virtual void BeginPlay() override;

private:

	void SetInteractState();


	ACharacter* CharacterOwner;
	UTransformationComponent* TransformationComponent;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact State ", meta = (AllowPrivateAccess = "true"))
	EInteractState InteractState = EInteractState::EIS_NOTinteracting;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interact State ", meta = (AllowPrivateAccess = "true"))
	class AInteractableObject* OverlappingObject;


#pragma region InputActions

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Interact")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Interact")
	UInputAction* InteractMoveAction;

#pragma endregion

UFUNCTION(BlueprintCallable, Category = "Interacting")
FORCEINLINE EInteractState GetInteractState() const { return InteractState; }
};
