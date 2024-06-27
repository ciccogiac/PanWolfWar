// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Interfaces/InteractInterface.h"
#include "Interfaces/CharacterInterface.h"
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
class UAttributeComponent;
class UPandolFlowerComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class APanWolfWarCharacter : public ACharacter , public IInteractInterface , public ICharacterInterface
{
	GENERATED_BODY()

#pragma region PublicFunctions

public:
	APanWolfWarCharacter();

	void AddMappingContext(UInputMappingContext* MappingContextToAdd, int32 Priority);
	void RemoveMappingContext(UInputMappingContext* MappingContextToRemove);

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

	/** Niagara Transformation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* NiagaraTransformation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* NiagaraApplyTransformationEffect;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UPandolFlowerComponent* PandolFlowerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UAttributeComponent* Attributes;

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
	
	//UFUNCTION(BlueprintCallable, Category = "attribute")
	FORCEINLINE virtual UAttributeComponent* GetAttributeComponent()  const override { return Attributes; }
	FORCEINLINE UClimbingComponent* GetClimbingComponent()  const { return ClimbingComponent; }
	FORCEINLINE UInteractComponent* GetInteractComponent() const { return InteractComponent; }
	FORCEINLINE virtual UTransformationComponent* GetTransformationComponent()  const override { return TransformationComponent; } ;

	FORCEINLINE UNiagaraComponent* GetNiagaraTransformation() { return NiagaraTransformation; }
	FORCEINLINE UNiagaraComponent* GetNiagaraTransformationEffect() { return NiagaraApplyTransformationEffect; }

#pragma endregion



};

