// Fill out your copyright notice in the Description page of Project Settings.

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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UPandolFlowerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPandolFlowerComponent();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	void Move(const FInputActionValue& Value);
	void Hook();
	void Jump();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;





private:

	void CheckForGrapplePoint();
	void ActivateGrapplePoint(AActor* DetectedActor);
	void DeactivateGrapplePoint();
	void MoveRope();
	void GrapplingMovement();

	void PlayMontage(UAnimMontage* MontageToPlay);

	UFUNCTION()
	void OnFlowerNotifyStarted(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION()
	void OnFlowerMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION(BlueprintCallable)
	void RopeVisibility(bool NewVisibility);

	UFUNCTION(BlueprintCallable)
	void ResetMovement();

	UFUNCTION(BlueprintCallable)
	void ThrowRope();

	UFUNCTION(BlueprintCallable)
	void StartGrapplingMovement();

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;


private:
	UPROPERTY()
	UAnimInstance* OwningPlayerAnimInstance;

	ACharacter* CharacterOwner;
	APanWolfWarCharacter* PanWolfCharacter;
	UCameraComponent* FollowCamera;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> Anim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation Materials ", meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* Pandolflower_Niagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* PandolFlowerMappingContext;



	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	bool ShowDebugTrace = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ETraceTypeQuery> HookingTraceType;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AFlowerCable> BP_FlowerCable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* GrappleAir_Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* GrappleGround_Montage;


	AFlowerCable* FlowerCable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking ", meta = (AllowPrivateAccess = "true"))
	UStaticMesh* FlowerCable_EndMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking ", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* EndCable;

	bool bInGrapplingAnimation = false;
	bool bMovingWithGrapple = false;
	FVector GrapplingDestination;
	float RopeBaseLenght;
	FVector StartingPosition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	float DetectionRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	float GrappleThrowDistance = 1200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > GrapplingObjectTypes;
	
	AGrapplePoint* GrapplePointRef;
	AGrapplePoint* CurrentGrapplePoint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	USoundBase* ThrowRope_Sound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	USoundBase* GrappleJump_Sound;

	#pragma region Curves

	/** Rope */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AirRopeLength_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* GroundRopeLength_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AirRopePosition_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* GroundRopePosition_Curve;

	/** Character Movement */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AirSpeed_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* GroundSpeed_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* AirHeightOffset_Curve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params| Curves", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* GroundHeightOffset_Curve;

	#pragma endregion


public:
	FORCEINLINE float GetDetectionRadius() const { return DetectionRadius; }
	FORCEINLINE float GetGrappleThrowDistance() const { return GrappleThrowDistance; }
};
