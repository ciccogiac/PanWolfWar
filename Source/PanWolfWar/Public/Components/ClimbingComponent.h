// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbingComponent.generated.h"

class UCharacterMovementComponent;
class UCapsuleComponent;
class UAnimMontage;
struct FInputActionValue;
class UInputAction;
class APanWolfWarCharacter;
class UInputMappingContext;

UENUM(BlueprintType)
enum class EClimbingState : uint8
{
	ECS_NOTClimbing UMETA(DisplayName = "NOTClimbing"),
	ECS_SearchingClimbingDown UMETA(DisplayName = "SearchingClimbingDown"),
	ECS_Climbing UMETA(DisplayName = "Climbing"),
	ECS_Falling UMETA(DisplayName = "Falling")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UClimbingComponent : public UActorComponent
{
	GENERATED_BODY()


#pragma region PublicFunctions

public:

	UClimbingComponent();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	bool TryClimbing();

	#pragma region InputCallback

	bool ActivateJumpTrace();
	void ToggleClimbing();
	void ClimbMove(const FInputActionValue& Value);
	void ClimbMoveEnd(const FInputActionValue& Value);
	void ClimbJump();
	void ClimbDownActivate();
	void ClimbDownDeActivate();
	void Landed();
	bool TryMantle();
	bool TryVault();

	void SetAnimationBindings();

	#pragma endregion

#pragma endregion

#pragma region ProtectedFunctions

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

#pragma endregion

#pragma region PrivateFunctions

private:

	void VaultMotionWarp(const FVector VaultStartPos,const FVector VaultMiddlePos,const FVector VaultLandPos);

	FVector2D Get8DirectionVector(const FVector2D& InputVector);

	#pragma region CheckClimbingCondition

	bool CheckClimbDownLedgeConditions();
	bool CheckTryClimbingConditions();
	bool CheckClimbableDownLedgeTrace(const FHitResult& ClimbableSurfaceHit);
	bool CheckCapsuleEndPositionCollision();
	bool CheckClimbableObjectTrace(const FHitResult& outClimbableObjectHit);
	bool CheckClimbableSpaceCondition(const FHitResult& CLimbablePointHit);
	bool CheckCapsuleSpaceCondition(const FVector& CLimbablePoint, bool FullHeight = false);

	#pragma endregion

	#pragma region SetClimbState

	void StartClimbing();
	void StopClimbing();

	#pragma endregion

	#pragma region CalculateClimbingCondition


	bool FindClimbableObject( const float BaseEyeHeightOffset_UP = 0.f, const float BaseEyeHeightOffset_Right = 0.f ,  float StartingClimbOffset_UP = 0.f);
	bool FindClimbablePoint(const FHitResult& ClimbableObjectHit);
	void ProcessClimbableSurfaceInfo(const FHitResult& ClimbableObjectHit);
	FVector CalculateLedgeLocation(const FVector& ImpactObjectPoint, const FVector& ClimbablePoint, const FRotator& Rotation, int ForwardDirectionAdjusted);
	void LedgeRightMove(float Direction, float DirectionY = 0.f);
	void TryCornerOrDirectionalJump(float Direction, const FHitResult& outClimbableObjectHit , bool InternLedge = false, bool BlindPoint = false,float DirectionY = 0.f);
	void HandleRightMove(const FHitResult& outClimbableObjectHit, const FHitResult& outClimbablePointHit, float Direction, float DirectionY = 0.f);
	bool CanClimbUpon();
	bool CanClimbDownLedge();
	bool CanClimbCorner(const FHitResult& outEndLedgePointHit, float Direction, bool InternLedge = false, bool BlindPoint = false);
	bool CanClimbJump();
	bool CanClimbBackJump();
	bool CanClimbDirectionalJump(float Direction, float UP_Offset = 0.f);

	#pragma endregion

	#pragma region ClimbingMove

	void LedgeMove(const FVector2D MovementVector);
	bool LedgeUpMove(const FVector2D& Direction);
	bool TryClimbUpon();
	bool TryJumping();
	bool TryDirectionalJumping();
	void MoveToLedgeLocation();
	bool MoveOnLedge(const FVector& ImpactObjectPoint, const FVector& ClimbablePoint, const FRotator& Rotation);

	#pragma endregion

	#pragma region ClimbTraces

	const FHitResult DoLineTraceSingleByChannel(const FVector& Start, const FVector& End);
	const FHitResult DoLineTraceSingleByWorldStatic(const FVector& Start, const FVector& End);
	const FHitResult DoSphereTraceSingleForObjects(const FVector& Start, const FVector& End, float Radius, bool TraceWorldStatic = false);
	const FHitResult DoSphereTraceSingleForChannel(const FVector& Start, const FVector& End, float Radius);
	const FHitResult DoCapsuleTraceSingleForObjects(const FVector& Start, const FVector& End, float Radius, float HalfHeight);
	const FHitResult DoCapsuleTraceSingleForChannel(const FVector& Start, const FVector& End, float Radius, float HalfHeight);

	const FHitResult TraceFromEyeHeight(const float Radius, const float BaseEyeHeightOffset_UP, const float BaseEyeHeightOffset_Right = 0.f, bool DoSphereTrace = false, const float StartingClimbOffset_UP = 0.f);
	float GetBaseEyeHeightOffset(const float BaseEyeHeightOffset_UP);
	const FHitResult TraceFromClimbableObject(const float Radius, const FVector& ImpactPoint);

	const FHitResult DoClimbableDownLedgeTrace();
	const FHitResult DoClimbCornerTrace(const FHitResult& outEndLedgePointHit, float Direction, bool InternLedge, bool BlindPoint);
	const FHitResult DoClimbJumpTrace(size_t i);
	const FHitResult DoClimbDirectionalJumpTrace(size_t i, float Direction, float UP_Offset);
	const FHitResult DoClimbBackJumpTrace(size_t i);
	const FHitResult DoClimbUponTrace();
	const bool DoMantleTrace(const FVector TraceStart , FVector& FirstPoint ,FVector& SecondPoint);

	#pragma endregion

	#pragma region MontageSection

	void PlayClimbMontage(UAnimMontage* MontageToPlay);

	UFUNCTION()
	void OnClimbMontageStartedHanging(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION()
	void OnClimbMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	#pragma endregion


#pragma endregion

#pragma region PrivateVariables

private:

	#pragma region Components

	AActor* ActorOwner;
	ACharacter* CharacterOwner;
	APanWolfWarCharacter* PanWolfCharacter;
	UCharacterMovementComponent* MovementComponent;
	UCapsuleComponent* CapsuleComponent;

	UPROPERTY()
	UAnimInstance* OwningPlayerAnimInstance;

	#pragma endregion

	#pragma region ClimbCoreVariables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb State ", meta = (AllowPrivateAccess = "true"))
	EClimbingState ClimbingState = EClimbingState::ECS_NOTClimbing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* ClimbingMappingContext;

	FVector CurrentClimbableSurfaceLocation;
	FVector CurrentClimbableSurfaceNormal;
	FRotator ClimbRotation;
	FVector LedgeLocation;
	FVector MotionWarpingLocation;
	FRotator MotionWarpingRotator;

	bool bJumpSaved = false;
	UAnimMontage* SavedJumpMontage;

	AActor* ClimbedObject;
	AActor* SavedClimbedObject;

	float ClimbDirection = 0;
	FVector2D LastClimb_MovementVector = FVector2D(0.0f, 0.0f);

	#pragma endregion

	#pragma region ClimbBPVariables

	#pragma region Climb Params

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	bool ShowDebugTrace = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > ClimbableObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > WorldStaticObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ETraceTypeQuery> TraceType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ETraceTypeQuery> ClimbableTraceType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	float LedgeHeightLocationXY = 40.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	float LedgeHeightLocationZ = 110.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	int N_JumpCapsuleIteration = 14;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	int N_BackJumpCapsuleIteration = 9;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	int N_DirectionalJumpCapsuleIteration = 24;

	float MoveRightOffset = 28.f;
	float MoveUPOffset = 0.15f;
	float HandBorder_Forward = 80.f;
	float HandBorder_Backward = 70.f;
	float MaxImpactNormal_Z_value = 0.2f;
	float MaxImpactNormal_Cos_value = 0.3f;
	float Max_Cos_value_PointToObject = 0.15f;

	#pragma endregion

	#pragma region TraceVariables

	#pragma region First Trace

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float Radius_FirstTrace = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float BaseEyeHeightOffset_Idle = 1.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float BaseEyeHeightOffset_Jumping = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float BaseEyeHeightOffset_Landing = 2.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float ForwardOffset = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float ForwardOffset_Landing = 150.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float ForwardOffset_NoClimbLanding = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float Radius_Corner = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | First Trace", meta = (AllowPrivateAccess = "true"))
	float Radius_FirstTrace_Hand = 10.f;

	#pragma endregion

	#pragma region Second Trace

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Second Trace", meta = (AllowPrivateAccess = "true"))
	float ClimbingTraceHeight_Idle = 120.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params | Second Trace", meta = (AllowPrivateAccess = "true"))
	float ClimbingTraceHeight_Jumping = 50.f;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* MantleNoClimbMontage;


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

	/** Vault */
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* VaultMontage;

	#pragma endregion



#pragma endregion

#pragma region FORCEINLINE_functions

public:

	#pragma region BlueprintCallable

	UFUNCTION(BlueprintCallable, Category = "Climbing")
	FORCEINLINE bool IsClimbing() const { return ClimbingState == EClimbingState::ECS_Climbing; }

	UFUNCTION(BlueprintCallable, Category = "Climbing")
	FORCEINLINE float GetClimbDirection() const { return ClimbDirection; }

	UFUNCTION(BlueprintCallable, Category = "Climbing")
	FORCEINLINE bool GetJumpSaved() const { return bJumpSaved; }

	UFUNCTION(BlueprintCallable, Category = "Climbing")
	FORCEINLINE FVector2D GetLastMovementVector() const { return LastClimb_MovementVector; }

	#pragma endregion

#pragma endregion

#pragma region InputActions

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Climb")
	UInputAction* ClimbDownAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Climb")
	UInputAction* ToggleClimbAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Climb")
	UInputAction* ClimbMoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Climb")
	UInputAction* ClimbJumpAction;

#pragma endregion

};
