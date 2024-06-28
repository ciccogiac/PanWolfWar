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
class UMotionWarpingComponent;
class UInputMappingContext;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UPandolFlowerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPandolFlowerComponent();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



public:

	void Hook();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HookAction;

private:
	UPROPERTY()
	UAnimInstance* OwningPlayerAnimInstance;

	ACharacter* CharacterOwner;
	APanWolfWarCharacter* PanWolfCharacter;
	UCameraComponent* FollowCamera;
	UMotionWarpingComponent* MotionWarpingComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* PandolFlowerMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	bool ShowDebugTrace = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<ETraceTypeQuery> HookingTraceType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > HookableObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	float SearchHook_Radius = 366.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	float SearchObjectHookable_Radius = 33.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	float Min_HookingDistance = 200.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Params", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AFlowerCable> BP_FlowerCable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* ThrowFlowerCable_Montage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hooking Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* HookJump_Montage;


	AFlowerCable* FlowerCable;

	FHitResult FirstTrace;
	FHitResult SecondTrace;
	bool bDidHit = false;

	bool bIsHooking = false;
	//bool isHookingState = false;

	FVector  Hook_TargetLocation;
	FRotator Hook_TargetRotation;

	void SearchHookingPoint();
	bool SearchHookableObject();
	void PlayMontage(UAnimMontage* MontageToPlay);
	void SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition, const FRotator& InTargetRotation = FRotator::ZeroRotator);

	UFUNCTION()
	void OnFlowerNotifyStarted(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	UFUNCTION()
	void OnFlowerMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:

	UFUNCTION(BlueprintCallable, Category = "Hooking")
	void StartHooking();

	UFUNCTION(BlueprintCallable, Category = "Hooking")
	void EndHooking();

	UFUNCTION(BlueprintCallable, Category = "Hooking")
	//FORCEINLINE bool GetIsHooking() const { return bIsHooking && !isHookingState; }
	FORCEINLINE bool GetIsHooking() const { return bIsHooking; }
};
