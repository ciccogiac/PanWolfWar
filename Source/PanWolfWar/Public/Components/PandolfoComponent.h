#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <Components/TimelineComponent.h>
#include "PandolfoComponent.generated.h"


class UInputMappingContext;
class APanWolfWarCharacter;
class UClimbingComponent;
class UInputAction;
class UCapsuleComponent;
class USpringArmComponent;
class UAnimMontage;
class UKiteComponent;
class AKiteBoard;
class USneakCoverComponent;

UENUM(BlueprintType)
enum class EPandolfoState : uint8
{
	EPS_Pandolfo UMETA(DisplayName = "Pandolfo"),
	EPS_Climbing UMETA(DisplayName = "Climbing"),
	EPS_Covering UMETA(DisplayName = "Covering"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UPandolfoComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPandolfoComponent();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	const bool IsClimbing();

	void Jump();
	bool TryClimbOrMantle();
	void Sliding();
	void Crouch();

	void EnterKiteMode(AKiteBoard* KiteBoard);

	void TryGliding();
	void UnGlide();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	void SetSlidingValues(bool IsReverse = false);

	UFUNCTION(BlueprintCallable)
	void StartSliding();

	UFUNCTION(BlueprintCallable)
	void EndSliding();

	bool PredictJump();

	UFUNCTION()
	void StartPredictJump(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	UFUNCTION()
	void EndPredictJump(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void StopPredictJump(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION()
	void CrouchCameraUpdate(float Alpha);

	void DoPredictJump();
	const FHitResult TraceIsOnGround(const FVector RootLocation, const FVector ForwardVector);
	const FHitResult PredictProjectileTrace(const FVector ActorLocation, const FVector ForwardVector, AActor* OnGroundActor);
	const FHitResult TraceLandConditions(const FVector ImpactPoint, const FVector ForwardVector);
	const FHitResult TraceObstacles(const FVector ActorLocation, const FVector ImpactPoint);
	void LoadPredictJump(const FVector ActorLocation, UAnimInstance* OwningPlayerAnimInstance);

private:
	ACharacter* CharacterOwner;
	APanWolfWarCharacter* PanWolfCharacter;
	UCapsuleComponent* Capsule;
	USpringArmComponent* CameraBoom;

	bool bIsGliding = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	bool ShowDebugTrace = false;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> Anim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* PandolfoMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UClimbingComponent* ClimbingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	USneakCoverComponent* SneakCoverComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UKiteComponent* KiteComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sliding", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* SlidingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sliding", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* PredictJumpMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sliding", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* CameraHeight_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sliding", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* PredictJump_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Crouching", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* CrouchCameraLenght_Curve;

	FTimeline CrouchingTimeline;

	FTimerHandle Sliding_TimerHandle;
	FTimerHandle PredictJump_TimerHandle;
	float TimeElapsed = 0.f;

	FVector LandPredictLocation;
	FVector LandPredictStartLocation;

	//Umbrella
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gliding", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> UmbrellaActorClass;

	AActor* UmbrellaActor;

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pandolfo State ")
	EPandolfoState PandolfoState = EPandolfoState::EPS_Pandolfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Pandolfo_JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Pandolfo_SlidingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Pandolfo_CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* TransformationSelectRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* TransformationSelectLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* TransformationSelectUPAction;

public:
	FORCEINLINE UClimbingComponent* GetClimbingComponent()  const { return ClimbingComponent; }
	FORCEINLINE USneakCoverComponent* GetSneakCoverComponent()  const { return SneakCoverComponent; }
	FORCEINLINE UKiteComponent* GetKiteComponent()  const { return KiteComponent; }

	UFUNCTION(BlueprintCallable, Category = "Gliding")
	FORCEINLINE bool IsGliding() const { return bIsGliding; }
	
};
