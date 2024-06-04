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

	void LedgeMove(const FVector2D MovementVector);

	void TryClimbUpon();


#pragma endregion

#pragma region ProtectedFunctions

protected:

	virtual void BeginPlay() override;

#pragma endregion

#pragma region PrivateFunctions

private:

	bool CanClimbDownLedge();
	const FHitResult DoLineTraceSingleByObject(const FVector& Start, const FVector& End);
	const FHitResult DoLineTraceSingleByChannel(const FVector& Start, const FVector& End);

	void StartClimbing();
	void StopClimbing();


	// Sphere Traces to Find PointLocation Climbable
	const FHitResult DoSphereTraceSingleForObjects(const FVector& Start, const FVector& End, float Radius);
	const FHitResult DoSphereTraceSingleForChannel(const FVector& Start, const FVector& End, float Radius);
	const FHitResult DoCapsuleTraceSingleForChannel(const FVector& Start, const FVector& End, float Radius, float HalfHeight);

	const FHitResult TraceFromEyeHeight(const float Radius, const float BaseEyeHeightOffset_UP, const float BaseEyeHeightOffset_Right = 0.f);
	const FHitResult TraceForObject(const float Radius, const float BaseEyeHeightOffset_UP, const float BaseEyeHeightOffset_Right = 0.f);
	const FHitResult TraceFromClimbableObject(const float Radius, const FVector& ImpactPoint);


	bool FindClimbableObject(const float BaseEyeHeightOffset_UP = 0.f, const float BaseEyeHeightOffset_Right = 0.f);
	bool CheckClimbableObjectTrace(const FHitResult& outClimbableObjectHit);
	bool FindClimbablePoint(const FHitResult& ClimbableObjectHit);
	bool CheckClimbableSpaceCondition(const FHitResult& CLimbablePointHit);
	bool CheckCapsuleSpaceCondition(const FVector& CLimbablePoint , bool FullHeight = false);

	bool CanClimbUpon();
	bool CanClimbCorner(const FHitResult& outEndLedgePointHit, float Direction, bool InternLedge = false);
	bool CanClimbJump(const FHitResult& outEndLedgePointHit, float Direction);

	void ProcessClimbableSurfaceInfo(const FHitResult& ClimbableObjectHit);
	FVector CalculateLedgeLocation(const FVector& ImpactObjectPoint, const FVector& ClimbablePoint, const FRotator& Rotation, int ForwardDirectionAdjusted);

	//Set Climbing State and Move to Climbing Point
	void MoveToLedgeLocation();

	void LedgeRightMove(float Direction);
	bool LedgeUpMove(const FVector2D& Direction);


	bool MoveOnLedge(const FVector& ImpactObjectPoint, const FVector& ClimbablePoint, const FRotator& Rotation);
	void HandleRightMove(const FHitResult& outClimbableObjectHit,const FHitResult& outClimbablePointHit, float Direction);

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
	bool bClimbDown = false;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params ", meta = (AllowPrivateAccess = "true"))
	float MoveRightOffset = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params ", meta = (AllowPrivateAccess = "true"))
	float MoveUPOffset = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params ", meta = (AllowPrivateAccess = "true"))
	float HandOffset = 28.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params ", meta = (AllowPrivateAccess = "true"))
	float HandBorder_Forward = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params ", meta = (AllowPrivateAccess = "true"))
	float HandBorder_Backward = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params ", meta = (AllowPrivateAccess = "true"))
	float MaxImpactNormal_Z_value = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params ", meta = (AllowPrivateAccess = "true"))
	float MaxImpactNormalToForwardVector_Cos_value = 0.3f;

	#pragma region TraceVariables

	#pragma region First Trace

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float BaseEyeHeightOffset_Idle = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float BaseEyeHeightOffset_Jumping = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float BaseEyeHeightOffset_Landing = 2.5f;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float ForwardOffset = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float ForwardOffset_Landing = 90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float Radius_FirstTrace = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float Radius_FirstTrace_Landing = 25.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float Radius_FirstTrace_Hand = 10.f;

	#pragma endregion

	#pragma region Second Trace

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Second Trace", meta = (AllowPrivateAccess = "true"))
	float ClimbingTraceHeight_Idle = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Second Trace", meta = (AllowPrivateAccess = "true"))
	float ClimbingTraceHeight_Jumping = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Second Trace", meta = (AllowPrivateAccess = "true"))
	float ClimbingTraceHeight_Landing = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Second Trace", meta = (AllowPrivateAccess = "true"))
	float ClimbingTraceHeight_Hanging_UP = 30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Second Trace", meta = (AllowPrivateAccess = "true"))
	float ClimbingTraceHeight_Hanging_Right = 15.f;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Second Trace", meta = (AllowPrivateAccess = "true"))
	float Radius_SecondTrace = 10.f;

	#pragma endregion

	#pragma region Third Trace

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


	#pragma endregion

	#pragma region ClimbMontages

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* IdleToHang;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* JumpToHang;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HangToHang_UP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HangToHang_DOWN;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbToTopMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* TopToClimbMontage;

	/** Corner */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbExternCornerLeftMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbExternCornerRightMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbInternCornerLeftMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbInternCornerRightMontage;

	/** Jump */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbJumpLeftMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ClimbJumpRightMontage;

	#pragma endregion



#pragma endregion

#pragma region FORCEINLINE_functions

public:

	UFUNCTION(BlueprintCallable, Category = "Climbing")
	FORCEINLINE bool IsClimbing() const { return bIsClimbing; }

	UFUNCTION(BlueprintCallable, Category = "Climbing")
	FORCEINLINE float GetClimbDirection() const { return ClimbDirection; }

	FORCEINLINE void SetClimbDirection(float Direction)  { ClimbDirection = Direction; }
	FORCEINLINE void SetClimbDown(bool Value) { bClimbDown = Value; }

#pragma endregion


};
