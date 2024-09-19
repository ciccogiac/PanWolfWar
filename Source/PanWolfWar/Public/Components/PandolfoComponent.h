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
class ABaseEnemy;

UENUM(BlueprintType)
enum class EPandolfoState : uint8
{
	EPS_Pandolfo UMETA(DisplayName = "Pandolfo"),
	EPS_Climbing UMETA(DisplayName = "Climbing"),
	EPS_Covering UMETA(DisplayName = "Covering"),
	EPS_Gliding UMETA(DisplayName = "Gliding"),
	EPS_Kiting UMETA(DisplayName = "Kiting")
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
	void Assassination();
	void CheckCanAirAssassin();
	void Dodge();


	void ClearAllTimer();

	void PlayAirAssassination(UAnimInstance* OwningPlayerAnimInstance);

	void PlayStealthAssassination(UAnimInstance* OwningPlayerAnimInstance);

	void EnterKiteMode(AKiteBoard* KiteBoard);
	UFUNCTION(BlueprintCallable)
	void ExitKiteMode();

	void TryGliding();
	void UnGlide();

	void HandleLand();
	void HandleFalling();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

private:

	void CheckCanHide();
	void CheckCanHideStandUP();
	void DetectAirAssassinableEnemy();
	


	void SetSlidingValues(bool IsReverse = false);

	UFUNCTION(BlueprintCallable)
	void StartSliding();

	UFUNCTION(BlueprintCallable)
	void EndSliding();

	bool PredictJump();

	UFUNCTION(BlueprintCallable)
	void TakeKnife(bool Take);

	UFUNCTION()
	void StartPredictJump(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	UFUNCTION()
	void EndPredictJump(UAnimMontage* Montage, bool bInterrupted);
	UFUNCTION()
	void StopPredictJump(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION()
	void CrouchCameraUpdate(float Alpha);

	UFUNCTION(BlueprintCallable)
	void AirKill();

	UFUNCTION(BlueprintCallable)
	void RiattachCamera();

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

	ABaseEnemy* AssassinableOverlapped = nullptr;
	ABaseEnemy* AIR_AssassinableOverlapped = nullptr;
	FTimerHandle AirAssassination_TimerHandle;
	FTimerHandle AirAssassinationCamera_TimerHandle;

	FTimerHandle Glide_TimerHandle;
	bool bIsGlideTimerActive = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > HidingObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Knife", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Knife;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AirAssassinMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* PandolfoDodgeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination", meta = (AllowPrivateAccess = "true"))
	TMap<int32, UAnimMontage*> AssassinationMontage_Map;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gliding", meta = (AllowPrivateAccess = "true"))
	float GlidingHeight = 700.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gliding", meta = (AllowPrivateAccess = "true"))
	float GlidingVelocity = 500.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gliding", meta = (AllowPrivateAccess = "true"))
	float GlidingGravityScale = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gliding", meta = (AllowPrivateAccess = "true"))
	float GlidingAirControl = 0.7f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PredictJump Params", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EObjectTypeQuery> PredictJumpObjectTypes;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FVector CombatHandBoxExtent;

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pandolfo State ")
	EPandolfoState PandolfoState = EPandolfoState::EPS_Pandolfo;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Pandolfo_JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Pandolfo_SlidingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Pandolfo_CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Pandolfo_GlideAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Pandolfo_AssassinAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* TransformationSelectRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* TransformationSelectLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* TransformationSelectUPAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

public:
	FORCEINLINE UClimbingComponent* GetClimbingComponent()  const { return ClimbingComponent; }
	FORCEINLINE USneakCoverComponent* GetSneakCoverComponent()  const { return SneakCoverComponent; }
	FORCEINLINE UKiteComponent* GetKiteComponent()  const { return KiteComponent; }
	FORCEINLINE void SetAssassinableEnemy(ABaseEnemy* Enemy) { AssassinableOverlapped = Enemy; }
	FORCEINLINE void SetAssassinableAirEnemy(ABaseEnemy* Enemy) { AIR_AssassinableOverlapped = Enemy; }
	FORCEINLINE bool IsAssassinableEnemy() { return (AssassinableOverlapped!= nullptr || AIR_AssassinableOverlapped != nullptr); }

	UFUNCTION(BlueprintCallable, Category = "Gliding")
	FORCEINLINE bool IsGliding() const { return PandolfoState == EPandolfoState::EPS_Gliding; }
	
};
