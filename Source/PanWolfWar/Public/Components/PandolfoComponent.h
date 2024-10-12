#pragma once

#include "CoreMinimal.h"
#include "Components/TransformationCharacterComponent.h"
#include "Components/ActorComponent.h"
#include <Components/TimelineComponent.h>
#include "PandolfoComponent.generated.h"


class UClimbingComponent;
class UInputAction;
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
	EPS_Mantle UMETA(DisplayName = "Mantle"),
	EPS_Covering UMETA(DisplayName = "Covering"),
	EPS_Gliding UMETA(DisplayName = "Gliding"),
	EPS_Kiting UMETA(DisplayName = "Kiting"),
	EPS_Dodging UMETA(DisplayName = "Dodging"),
	EPS_Sliding UMETA(DisplayName = "Sliding"),
	EPS_Vaulting UMETA(DisplayName = "Vaulting"),
	EPS_Interacting UMETA(DisplayName = "Interacting")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UPandolfoComponent : public UTransformationCharacterComponent
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
	void AssassinationFromHiding(ABaseEnemy* HidingAssassinatedEnemy = nullptr);
	void CheckCanAirAssassin();
	void Dodge();
	void LightAttack();

	void ClearAllTimer();

	void PlayAirAssassination();

	void PlayStealthAssassination();

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

private:

	UFUNCTION()
	void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnSlidingMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void DetectAirAssassinableEnemy();


	UFUNCTION(BlueprintCallable)
	void StartSliding();

	UFUNCTION(BlueprintCallable)
	void EndSliding();

	bool PredictJump();

	UFUNCTION(BlueprintCallable)
	void TakeKnife(bool Take, bool IsReverseSocket = false);

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
	void LoadPredictJump(const FVector ActorLocation);

private:
	ABaseEnemy* AssassinableOverlapped = nullptr;
	ABaseEnemy* AIR_AssassinableOverlapped = nullptr;
	FTimerHandle AirAssassination_TimerHandle;
	FTimerHandle AirAssassinationCamera_TimerHandle;

	FTimerHandle Glide_TimerHandle;
	bool bIsGlideTimerActive = false;

	bool IsKnifeEquipped = false;
	FTimerHandle KnifeEquipped_TimerHandle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Knife", meta = (AllowPrivateAccess = "true"))
	float KnifeTime = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > HidingObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Knife", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Knife;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	bool ShowDebugTrace = false;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

public:
	FORCEINLINE UClimbingComponent* GetClimbingComponent()  const { return ClimbingComponent; }
	FORCEINLINE USneakCoverComponent* GetSneakCoverComponent()  const { return SneakCoverComponent; }
	FORCEINLINE UKiteComponent* GetKiteComponent()  const { return KiteComponent; }
	FORCEINLINE void SetAssassinableEnemy(ABaseEnemy* Enemy) { AssassinableOverlapped = Enemy; }
	FORCEINLINE void SetAssassinableAirEnemy(ABaseEnemy* Enemy) { AIR_AssassinableOverlapped = Enemy; }
	FORCEINLINE bool IsAssassinableEnemy() { return (AssassinableOverlapped!= nullptr || AIR_AssassinableOverlapped != nullptr); }
	FORCEINLINE ABaseEnemy* GetAssassinableEnemy() const { return AssassinableOverlapped; }

	UFUNCTION(BlueprintCallable, Category = "Gliding")
	FORCEINLINE bool IsGliding() const { return PandolfoState == EPandolfoState::EPS_Gliding; }


	
};
