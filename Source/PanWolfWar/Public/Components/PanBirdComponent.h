// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "PanBirdComponent.generated.h"


class APanWolfWarCharacter;
class UInputMappingContext;
struct FInputActionValue;
class UCapsuleComponent;
class USpringArmComponent;
class UInputAction;

class USkeletalMesh;
struct FSkeletalMaterial;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UPanBirdComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPanBirdComponent();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;


	void Move(const FInputActionValue& Value);


protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:
	APanWolfWarCharacter* PanWolfCharacter;
	UCapsuleComponent* Capsule;
	USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* PanBirdMappingContext;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> Anim;

		
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BirdMoveAction;
};
