// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Interfaces/InteractInterface.h"
#include "Interfaces/CharacterInterface.h"
#include "Interfaces/CombatInterface.h"
#include "Interfaces/HitInterface.h"
#include "PanWolfWarCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UNiagaraComponent;

class UMotionWarpingComponent;
class UInteractComponent;
class UTransformationComponent;
class UAttributeComponent;
class UPandolfoComponent;
class UPanWolfComponent;
class UPandolFlowerComponent;
class UPanBirdComponent;
class UCombatComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game, Blueprintable)
class APanWolfWarCharacter : public ACharacter , public IInteractInterface , public ICharacterInterface, public ICombatInterface , public IHitInterface
{
	GENERATED_BODY()

#pragma region PublicFunctions

public:
	APanWolfWarCharacter();

	void AddMappingContext(UInputMappingContext* MappingContextToAdd, int32 Priority);
	void RemoveMappingContext(UInputMappingContext* MappingContextToRemove);
	void SetTransformationCharacter(TObjectPtr<USkeletalMesh> SkeletalMeshAsset, TSubclassOf<UAnimInstance> Anim);

	void SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition, const FRotator& InTargetRotation = FRotator::ZeroRotator);

	UFUNCTION(BlueprintCallable)
	void SetIsHiding(bool Value, bool DoCrouchCheck = true);

	//Combat Interface
	virtual void ActivateCollision(FString CollisionPart) override;
	virtual void DeactivateCollision(FString CollisionPart) override;

	//HitInterface
	virtual void GetHit(const FVector& ImpactPoint, AActor* Hitter) override;
	#pragma region InputCallback

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	#pragma endregion

#pragma endregion

#pragma region ProtectedFunctions

protected:

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay();

	virtual void Landed(const FHitResult& Hit) override;
	virtual void Falling() override;

	void PlayHitReactMontage(const FName& SectionName);
	void GetHitReactMontage(UAnimMontage*& ReactMontage);
	bool IsAlive();
	void Die();
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	float GetDamageDivisor();
#pragma endregion

#pragma region PrivateVariables

private:

	bool bIsHiding = false;
	TArray<AActor*> EnemyAware = TArray<AActor*>();

	#pragma region Components

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hiding Widget", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PlayerHidingWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hiding Widget", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PlayerSeenWidget;

	/** Niagara Transformation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* NiagaraTransformation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* NiagaraApplyTransformationEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UMotionWarpingComponent* MotionWarpingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UInteractComponent* InteractComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UTransformationComponent* TransformationComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UPandolfoComponent* PandolfoComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UPandolFlowerComponent* PandolFlowerComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UPanWolfComponent* PanWolfComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UPanBirdComponent* PanBirdComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UAttributeComponent* Attributes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	#pragma endregion

	#pragma region Combat Variables

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float Pandolfo_DamageDivisor = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float PanWolf_DamageDivisor = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float PandolFlower_DamageDivisor = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float PanBird_DamageDivisor = 10.f;

	#pragma region HitReact Montages

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* Pandolfo_HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* PandolFlower_HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* PanWolf_HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* PanBird_HitReactMontage;

	#pragma endregion

	#pragma endregion

	#pragma region InputAction

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* InteractableMappingContext;

	/** InputActions */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;


	#pragma endregion

#pragma endregion

#pragma region PrivateFunctions
private:



	//Interfaces

	virtual bool SetOverlappingObject(AInteractableObject* InteractableObject, bool bEnter = true) override;



#pragma endregion

#pragma region FORCEINLINE_functions

public:

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	
	FORCEINLINE virtual UAttributeComponent* GetAttributeComponent()  const override { return Attributes; }
	FORCEINLINE UInteractComponent* GetInteractComponent() const { return InteractComponent; }
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; }

	FORCEINLINE virtual UTransformationComponent* GetTransformationComponent()  const override { return TransformationComponent; } ;
	FORCEINLINE virtual UPandolfoComponent* GetPandolfoComponent() const override { return PandolfoComponent; }
	FORCEINLINE UPanWolfComponent* GetPanWolfComponent() const { return PanWolfComponent; }
	FORCEINLINE virtual UPandolFlowerComponent* GetPandolFlowerComponent() const override { return PandolFlowerComponent; }
	FORCEINLINE UPanBirdComponent* GetPanBirdComponent() const { return PanBirdComponent; }

	FORCEINLINE UNiagaraComponent* GetNiagaraTransformation() { return NiagaraTransformation; }
	FORCEINLINE UNiagaraComponent* GetNiagaraTransformationEffect() { return NiagaraApplyTransformationEffect; }


	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsHiding() const { return bIsHiding; }

	UFUNCTION(BlueprintCallable)
	void AddEnemyAware(AActor* Enemy);

	UFUNCTION(BlueprintCallable)
	void RemoveEnemyAware(AActor* Enemy);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE TArray<AActor*> GetEnemyAware() { return EnemyAware; }

#pragma endregion



};

