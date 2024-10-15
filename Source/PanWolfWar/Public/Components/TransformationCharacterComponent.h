#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAssets/TransformationData/DataAsset_TransformationDataBase.h"
#include "TransformationCharacterComponent.generated.h"

class APanWolfWarCharacter;
class UCapsuleComponent;
class USpringArmComponent;
class UPandoCombatComponent;
class UCharacterMovementComponent;
class UTargetingComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UTransformationCharacterComponent : public UActorComponent
{
	GENERATED_BODY()
		
public:
	UTransformationCharacterComponent();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	bool CheckCapsuleSpace();

	virtual void PlayHardLandMontage();

	float GetCombatDistanceRange(ECombatAttackRange CombatAttackRange);

protected:
	virtual void BeginPlay() override;


	UFUNCTION()
	virtual void OnHardLandMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
	FTransformationCharacterData TransformationCharacterData;

	ACharacter* CharacterOwner;
	APanWolfWarCharacter* PanWolfCharacter;
	UCharacterMovementComponent*  MovementComponent;
	UCapsuleComponent* Capsule;
	USpringArmComponent* CameraBoom;
	UPandoCombatComponent* CombatComponent;
	UTargetingComponent* TargetingComponent;

	UAnimInstance* OwningPlayerAnimInstance;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HardLandMontage;

	bool bCanCrouch = true;

	bool bIsHardLanding = false;

public:
	FORCEINLINE void SetTransformationCharacterData(FTransformationCharacterData _TransformationCharacterData) { TransformationCharacterData = _TransformationCharacterData; }

	FORCEINLINE float GetTargetArmLength() const { return TransformationCharacterData.TargetArmLength;}
	FORCEINLINE float GetCapsuleRadius() const { return TransformationCharacterData.CapsuleRadius; }
	FORCEINLINE float GetCapsuleHalfHeight() const { return TransformationCharacterData.CapsuleHalfHeight; }
	FORCEINLINE float GetMaxWalkSpeed() const { return TransformationCharacterData.MaxWalkSpeed; }
	FORCEINLINE float GetMaxWalkSpeedCrouched() const { return TransformationCharacterData.MaxWalkSpeedCrouched; }
	FORCEINLINE float GetFallDamageDivisor() const { return TransformationCharacterData.FallDamageDivisor; }
	FORCEINLINE float GetFootStepLoudness() const { return TransformationCharacterData.FootStepLoudness; }
	FORCEINLINE float GetFootStepMaxRange() const { return TransformationCharacterData.FootStepMaxRange; }

};
