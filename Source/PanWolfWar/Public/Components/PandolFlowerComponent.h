#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PandolFlowerComponent.generated.h"

class UInputAction;
class APanWolfWarCharacter;
class UCameraComponent;
class AFlowerCable;
class UAnimMontage;
class UInputMappingContext;
class UNiagaraSystem;
class AGrapplePoint;
struct FInputActionValue;
class USpringArmComponent;
class AFlowerHideObject;
class UPandolfoComponent;
class UTransformationComponent;
class UPandoCombatComponent;

UENUM(BlueprintType)
enum class EPandolFlowerState : uint8
{
	EPFS_PandolFlower UMETA(DisplayName = "PandolFlower"),
	EPFS_Dodging UMETA(DisplayName = "Dodging"),
	EPFS_FlowerCover UMETA(DisplayName = "FlowerCover"),
	EPFS_Hooking UMETA(DisplayName = "Hooking"),
	EPFS_Swinging UMETA(DisplayName = "Swinging")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UPandolFlowerComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region Functions

public:	
	UPandolFlowerComponent();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	void Move(const FInputActionValue& Value);
	void Hook();
	void Jump();
	void Crouch();
	void Hide();
	void UnHide();
	void Assassination();
	void Dodge();
	void LightAttack();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void CheckForGrapplePoint();
	void ActivateGrapplePoint(AActor* DetectedActor);
	void DeactivateGrapplePoint();
	void MoveRope();
	void GrapplingMovement();

	void PlayMontage(UAnimMontage* MontageToPlay);

	void SetCharRotation(const FVector ImpactNormal, bool Istantaneus = false);
	void SetCharLocation(const FVector HitLocation, const FVector HitNormal, bool Istantaneus = false);

	void CheckCanAirAssassin();

	#pragma region AnimNotifyFunctions

	UFUNCTION(BlueprintCallable)
	void RopeVisibility(bool NewVisibility);

	UFUNCTION(BlueprintCallable)
	void ResetMovement();

	UFUNCTION(BlueprintCallable)
	void ThrowRope();

	UFUNCTION(BlueprintCallable)
	void StartGrapplingMovement();

	UFUNCTION(BlueprintCallable)
	void StartSwinging();

	#pragma endregion

#pragma endregion

#pragma region Variables

	#pragma region InputActions
	public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* PandolFlowerMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PandolFlower_CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PandolFlower_HideAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PandolFlower_AssassinAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

	#pragma endregion


private:

	#pragma region Components

	UPROPERTY()
	UAnimInstance* OwningPlayerAnimInstance;

	ACharacter* CharacterOwner;
	APanWolfWarCharacter* PanWolfCharacter;
	UCameraComponent* FollowCamera;
	USpringArmComponent* CameraBoom;
	UPandolfoComponent* PandolfoComponent;
	UTransformationComponent* TransformationComponent;
	UPandoCombatComponent* CombatComponent;

	EPandolFlowerState PandolFlowerState = EPandolFlowerState::EPFS_PandolFlower;

	FTimerHandle AirAssassination_TimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cover State ", meta = (AllowPrivateAccess = "true"))
	float CoverDirection = 0.f;
	float LastCoverDirection = 0.f;
	AFlowerHideObject* FlowerHideObject = nullptr;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> Anim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation Materials ", meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* Pandolflower_Niagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AFlowerCable> BP_FlowerCable;

	AFlowerCable* FlowerCable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > HidingObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > PandolFlowerHideObjectTypes;

	#pragma endregion

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grappling Params| Debug", meta = (AllowPrivateAccess = "true"))
	bool ShowDebugTrace = false;

	#pragma region GrapplingCore

	/*bool bInGrapplingAnimation = false;*/
	bool bMovingWithGrapple = false;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grappling Params| Debug", meta = (AllowPrivateAccess = "true"))
	//bool bSwinging = false;

	FVector GrapplingDestination;
	float RopeBaseLenght;
	FVector StartingPosition;
	AGrapplePoint* GrapplePointRef;
	AGrapplePoint* CurrentGrapplePoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grappling Params", meta = (AllowPrivateAccess = "true"))
	float DetectionRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grappling Params", meta = (AllowPrivateAccess = "true"))
	float GrappleThrowDistance = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grappling Params", meta = (AllowPrivateAccess = "true"))
	float SwingForce = 100000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > GrapplingObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ETraceTypeQuery> VisibleTraceType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* PandolFlowerDodgeMontage;

	#pragma endregion

	#pragma region Grappling

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* GrappleAir_Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* GrappleGround_Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* GrappleAir_Swing_Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* GrappleGround_Swing_Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Sound", meta = (AllowPrivateAccess = "true"))
	USoundBase* ThrowRope_Sound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Sound", meta = (AllowPrivateAccess = "true"))
	USoundBase* GrappleJump_Sound;

	#pragma endregion

	#pragma region GrapplingCurves

	/** Rope */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AirRopeLength_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* GroundRopeLength_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AirRopePosition_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* GroundRopePosition_Curve;

	/** Character Movement */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AirSpeed_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* GroundSpeed_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AirHeightOffset_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Grappling Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* GroundHeightOffset_Curve;

	#pragma endregion

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FVector CombatHandBoxExtent;

#pragma endregion

#pragma region FORCEINLINE_Functions

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE EPandolFlowerState GetPandolFlowerState() const { return PandolFlowerState; }
	FORCEINLINE float GetDetectionRadius() const { return DetectionRadius; }
	FORCEINLINE float GetGrappleThrowDistance() const { return GrappleThrowDistance; }

#pragma endregion


};
