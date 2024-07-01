#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PandolfoComponent.generated.h"

class UInputMappingContext;
class APanWolfWarCharacter;
class UClimbingComponent;
class UInputAction;
class UCapsuleComponent;
class USpringArmComponent;

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

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	ACharacter* CharacterOwner;
	APanWolfWarCharacter* PanWolfCharacter;
	UCapsuleComponent* Capsule;
	USpringArmComponent* CameraBoom;


	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> Anim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* PandolfoMappingContext;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UClimbingComponent* ClimbingComponent;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* Pandolfo_JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* TransformationSelectRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* TransformationSelectLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* TransformationApply;

public:
	FORCEINLINE UClimbingComponent* GetClimbingComponent()  const { return ClimbingComponent; }
	
};
