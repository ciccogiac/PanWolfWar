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

protected:
	virtual void BeginPlay() override;

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


public:
	FORCEINLINE void SetTransformationCharacterData(FTransformationCharacterData _TransformationCharacterData) { TransformationCharacterData = _TransformationCharacterData; }

	FORCEINLINE float GetTargetArmLength() const { return TransformationCharacterData.TargetArmLength;}
	FORCEINLINE float GetCapsuleRadius() const { return TransformationCharacterData.CapsuleRadius; }
	FORCEINLINE float GetCapsuleHalfHeight() const { return TransformationCharacterData.CapsuleHalfHeight; }
	FORCEINLINE float GetMaxWalkSpeed() const { return TransformationCharacterData.MaxWalkSpeed; }
	FORCEINLINE float GetMaxWalkSpeedCrouched() const { return TransformationCharacterData.MaxWalkSpeedCrouched; }

};
