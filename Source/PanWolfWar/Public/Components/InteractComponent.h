// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/InteractInterface.h"
#include "InteractComponent.generated.h"

class UCharacterMovementComponent;
class UCapsuleComponent;
struct FInputActionValue;
class UInputAction;
class AMovableObject;
class APanWolfWarCharacter;
class UInputMappingContext;
class UPandolfoComponent;
class AMovableObject;

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


public:	
	UInteractComponent();
	
	void Interact();
	void InteractMove(const FInputActionValue& Value);
	void SetOverlappingObject(AInteractableObject* _InteractableObject, bool bEnter );
	void ResetOverlappingObject();

		
protected:
	virtual void BeginPlay() override;

private:

	void SetInteractState();
	bool IsTransformedInObjectTypes();

	ACharacter* CharacterOwner;
	APanWolfWarCharacter* PanWolfCharacter;
	UPandolfoComponent* PandolfoComponent;

	AMovableObject* CurrentMovableObject;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact State ", meta = (AllowPrivateAccess = "true"))
	EInteractState InteractState = EInteractState::EIS_NOTinteracting;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Interact State ", meta = (AllowPrivateAccess = "true"))
	class AInteractableObject* OverlappingObject;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* MovableObjectMappingContext;


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
