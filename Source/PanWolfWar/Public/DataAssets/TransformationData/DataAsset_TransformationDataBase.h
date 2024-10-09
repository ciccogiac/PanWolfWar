// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include <PanWolfWar/CharacterStates.h>
#include "DataAsset_TransformationDataBase.generated.h"

class UInputMappingContext;

USTRUCT(BlueprintType)
struct FTransformationCharacterData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "TransformationData")
	float MaxWalkSpeed;
	UPROPERTY(EditDefaultsOnly, Category = "TransformationData")
	float MaxWalkSpeedCrouched;
	UPROPERTY(EditDefaultsOnly, Category = "TransformationData")
	float CapsuleRadius;
	UPROPERTY(EditDefaultsOnly, Category = "TransformationData")
	float CapsuleHalfHeight;
	UPROPERTY(EditDefaultsOnly, Category = "TransformationData")
	float TargetArmLength;
	UPROPERTY(EditDefaultsOnly, Category = "TransformationData")
	float JumpZVelocity;
	UPROPERTY(EditDefaultsOnly, Category = "TransformationData")
	FVector CombatHandBoxExtent;

	

	UPROPERTY(EditDefaultsOnly, Category = "TransformationInput")
	UInputMappingContext* TransformationCharacterMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "TransformationMesh")
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(EditDefaultsOnly, Category = "TransformationMesh")
	TSubclassOf<UAnimInstance> Anim;

};
/**
 * 
 */
UCLASS()
class PANWOLFWAR_API UDataAsset_TransformationDataBase : public UDataAsset
{
	GENERATED_BODY()
	
public:
	FTransformationCharacterData GetTransformationCharacterData(ETransformationState TransformationState);

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TMap<ETransformationState,FTransformationCharacterData> TransformationsCharactersData;

};
