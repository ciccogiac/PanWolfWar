// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbingComponent.generated.h"

DECLARE_DELEGATE(FOnEnterClimbState)
DECLARE_DELEGATE(FOnExitClimbState)

class UCharacterMovementComponent;
class UCapsuleComponent;
class UAnimMontage;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UClimbingComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	//Delegates
	FOnEnterClimbState OnEnterClimbStateDelegate;
	FOnExitClimbState OnExitClimbStateDelegate;

#pragma region PublicFunctions

public:

	UClimbingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ToggleClimbing();
	bool TryClimbing();

	void LedgeMoveRight(float Direction);


#pragma endregion

#pragma region ProtectedFunctions

protected:

	virtual void BeginPlay() override;

#pragma endregion

#pragma region PrivateFunctions

private:


	void StartClimbing();
	void StopClimbing();


	// Sphere Traces to Find PointLocation Climbable
	bool FindCLimbableObjectLocation();
	bool FindCLimbablePointLocation(FHitResult ClimbableObjectHit);
	bool CheckClimbableCondition(FHitResult CLimbablePointHit);

	//Set Climbing State and Move to Climbing Point
	void GrabLedge();
	void MoveToLedgeLocation();

	void MoveOnLedge(FVector Trace1ImpactPoint, FVector Trac2ImpactPoint, FRotator Rotation);

	void PlayClimbMontage(UAnimMontage* MontageToPlay);
	UFUNCTION()
	void OnClimbMontageStartedHanging(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION()
	void OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted);



#pragma endregion

#pragma region PrivateVariables

private:

	#pragma region Components

	AActor* ActorOwner;
	ACharacter* CharacterOwner;
	UCharacterMovementComponent* MovementComponent;
	UCapsuleComponent* CapsuleComponent;


	UPROPERTY()
	UAnimInstance* OwningPlayerAnimInstance;

	#pragma endregion

	#pragma region ClimbCoreVariables

	bool bIsClimbing;
	FVector CurrentClimbableSurfaceLocation;
	FVector CurrentClimbableSurfaceNormal;
	FRotator ClimbRotation;
	FVector LedgeLocation;
	bool bCanClimb = true;


	float ClimbDirection = 0;

	#pragma endregion

	#pragma region ClimbBPVariables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	bool ShowDebugTrace = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > ClimbableObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	float LedgeHeightLocationXY = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	float LedgeHeightLocationZ = 110.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float MoveRightOffset = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float MoveUPOffset = 1.5f;

	#pragma region TraceVariables


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float BaseEyeHeightOffset = 1.5f;



	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float ForwardOffset = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float Radius_FirstTrace = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Second Trace", meta = (AllowPrivateAccess = "true"))
	float ClimbingTraceHeight = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Second Trace", meta = (AllowPrivateAccess = "true"))
	float Radius_SecondTrace = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Third Trace", meta = (AllowPrivateAccess = "true"))
	float CheckingClimbable_Z_Offset = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Third Trace", meta = (AllowPrivateAccess = "true"))
	float CheckingClimbable_Forward_Offset = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Third Trace", meta = (AllowPrivateAccess = "true"))
	float Radius_ThirdTrace = 5.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Third Trace", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ETraceTypeQuery> TraceType;

	#pragma endregion


	#pragma endregion

	#pragma region ClimbMontages

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* IdleToHang;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* JumpToHang;


	#pragma endregion



#pragma endregion

#pragma region FORCEINLINE_functions

public:

	UFUNCTION(BlueprintCallable, Category = "Climbing")
	FORCEINLINE bool IsClimbing() const { return bIsClimbing; }

	UFUNCTION(BlueprintCallable, Category = "Climbing")
	FORCEINLINE float GetClimbDirection() const { return ClimbDirection; }

	FORCEINLINE void SetClimbDirection(float Value)  { ClimbDirection = Value; }

	FORCEINLINE FVector GetClimbableSurfaceNormal() const { return CurrentClimbableSurfaceNormal; }

#pragma endregion


};
