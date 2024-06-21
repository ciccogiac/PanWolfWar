// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Interfaces/InteractInterface.h"
#include "PanWolfWarCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UNiagaraComponent;

class UMotionWarpingComponent;
class UClimbingComponent;
class UInteractComponent;
class UTransformationComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class APanWolfWarCharacter : public ACharacter , public IInteractInterface
{
	GENERATED_BODY()

#pragma region PublicFunctions

public:
	APanWolfWarCharacter();

#pragma endregion

#pragma region ProtectedFunctions

protected:

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay();

	virtual void Landed(const FHitResult& Hit) override;

#pragma endregion

#pragma region PrivateVariables

private:

	#pragma region Components

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* NiagaraTransformation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UMotionWarpingComponent* MotionWarpingComponent;

	/** Climbing Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UClimbingComponent* ClimbingComponent;

	/** Interact Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UInteractComponent* InteractComponent;

	/** Transformation Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UTransformationComponent* TransformationComponent;

	#pragma endregion

	#pragma region InputAction

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* ClimbingMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* InteractableMappingContext;

	/** InputActions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;


	#pragma endregion

#pragma endregion

#pragma region PrivateFunctions
private:

	// Delegates
	void OnPlayerEnterClimbState();
	void OnPlayerExitClimbState();

	void OnPlayerEnterInteractState();
	void OnPlayerExitInteractState();

	void AddMappingContext(UInputMappingContext* MappingContextToAdd, int32 Priority);
	void RemoveMappingContext(UInputMappingContext* MappingContextToRemove);

	//FVector2D Get8DirectionVector(const FVector2D& InputVector);

	#pragma region InputCallback

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	/** Climbing CallBacks */
	void JumpClimbTrace();


	//Interfaces

	virtual bool SetOverlappingObject(AInteractableObject* InteractableObject, bool bEnter = true) override;

	#pragma endregion

#pragma endregion

#pragma region FORCEINLINE_functions

public:

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	
	FORCEINLINE UClimbingComponent* GetClimbingComponent()  const { return ClimbingComponent; }
	FORCEINLINE UInteractComponent* GetInteractComponent() const { return InteractComponent; }

	FORCEINLINE UNiagaraComponent* GetNiagaraTransformation() { return NiagaraTransformation; }

#pragma endregion



};

