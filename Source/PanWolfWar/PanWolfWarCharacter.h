// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Interfaces/InteractInterface.h"
#include "Interfaces/CharacterInterface.h"
#include "Interfaces/CombatInterface.h"
#include "Interfaces/HitInterface.h"
#include "Interfaces/PawnUIInterface.h"
#include "PanWolfWarCharacter.generated.h"

class UPandoCombatComponent;

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

class UTargetingComponent;
class UTransformationCharacterComponent;

class UPanWarWidgetBase;

class UDataAsset_TransformationDataBase;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game, Blueprintable)
class APanWolfWarCharacter : public ACharacter , public IInteractInterface , public ICharacterInterface, public ICombatInterface , public IHitInterface ,  public IPawnUIInterface
{
	GENERATED_BODY()

#pragma region PublicFunctions

public:
	APanWolfWarCharacter();

	void AddMappingContext(UInputMappingContext* MappingContextToAdd, int32 Priority);
	void RemoveMappingContext(UInputMappingContext* MappingContextToRemove);
	void SetTransformationCharacter(TObjectPtr<USkeletalMesh> SkeletalMeshAsset, TSubclassOf<UAnimInstance> Anim);

	void SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition, const FRotator& InTargetRotation = FRotator::ZeroRotator);

	void SetMetaHumanVisibility(bool bVisible);
	void SetMetaHumanHitFX(float bVisible);
	void SetMetaHumanHideFX(float bVisible);

	void SetIsHiding(bool Value);

	virtual void SetIsInsideHideBox(bool Value, bool ForceCrouch = false) override;

	bool CanPerformDodge();
	virtual FRotator GetDesiredDodgeRotation() override;
	void StartDodge();
	UFUNCTION(BlueprintCallable)
	void EndDodge();

	//Combat Interface
	virtual UPawnCombatComponent* GetCombatComponent() const override;
	virtual void SetInvulnerability(bool NewInvulnerability) override;
	virtual bool IsCombatActorAlive() override;
	virtual float PerformAttack() override;
	virtual bool IsUnderAttack() override;
	virtual void SetUnderAttack() override;
	virtual float GetDefensePower() override;
	virtual void OnDeathEnter() override;
	virtual bool IsBlocking() override;
	virtual bool IsBlockingCharged() override;
	virtual void SuccesfulBlock(AActor* Attacker) override;
	virtual float GetHealthPercent() override;
	virtual bool IsValidBlock(AActor* InAttacker, AActor* InDefender) override;

	void SetCollisionHandBoxExtent(FVector Extent);

	void ResetUnderAttack() ;
	//HitInterface
	virtual void GetHit(const FVector& ImpactPoint, AActor* Hitter) override;

	//~ Begin IPawnUIInterface Interface.
	virtual UPawnUIComponent* GetPawnUIComponent() const override;
	virtual UPandoUIComponent* GetPandoUIComponent() const override;
	//~ End IPawnUIInterface Interface

	//CharacterInterface
	ETransformationState GetCurrentTransformationState() const override;

	void HandleTransformationChangedState();
	ETransformationState GetCurrentTransformationState();
	UTransformationCharacterComponent* GetCurrentTransformationCharacterComponent();


	virtual void SetOverlappingObject(AInteractableObject* InteractableObject, bool bEnter = true) override;

	#pragma region InputCallback

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	#pragma endregion

	void Interact();

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

#pragma endregion

#pragma region PrivateVariables

private:
	bool bIsHiding = false;
	bool bIsInvulnerable = false;
	bool bIsUnderAttack = false;
	bool bHitted = false;
	bool bIsInsideHideBox = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = crouch, meta = (AllowPrivateAccess = "true"))
	bool bIsForcedCrouch = false;

	FTimerHandle UnderAttack_TimerHandle;
	TArray<AActor*> EnemyAware = TArray<AActor*>();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TransformationCharacterData", meta = (AllowPrivateAccess = "true"))
	UDataAsset_TransformationDataBase* TransformationDataBase;

	#pragma region Components

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assassination Components", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* HidingAssassinBoxComponent;

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
	UPandoCombatComponent* PandoCombatComponent;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UTargetingComponent* TargetingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
	UPandoUIComponent* PandoUIComponent;

	UPROPERTY()
	UPanWarWidgetBase* PanWarOverlay;

	UPROPERTY(EditDefaultsOnly, Category = HUD)
	TSubclassOf<UPanWarWidgetBase> PanWarOverlayClass;

	//MetaHuman

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Torso;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Face;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Feets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MetaHuman", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Legs;

	#pragma endregion

	#pragma region Combat Variables

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | HandToHand", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* LeftHandCollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | HandToHand", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* RightHandCollisionBox;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float Pandolfo_DefensePower = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float PanWolf_DefensePower = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float PandolFlower_DefensePower = 4.f;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float PanBird_DefensePower = 10.f;

	#pragma region HitReact Montages

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* Pandolfo_HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* PandolFlower_HitReactMontage;

	//UPROPERTY(EditDefaultsOnly, Category = Combat)
	//UAnimMontage* PanWolf_HitReactMontage;

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

	UFUNCTION()
	void OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted);


	//Interfaces


	virtual void ConsumeStone(float StoneValue, EStoneTypes StoneType) override;


#pragma endregion

#pragma region FORCEINLINE_functions

public:

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	
	FORCEINLINE virtual UAttributeComponent* GetAttributeComponent()  const override { return Attributes; }
	FORCEINLINE UInteractComponent* GetInteractComponent() const { return InteractComponent; }
	FORCEINLINE virtual UTargetingComponent* GetTargetingComponent() const override { return TargetingComponent; }

	FORCEINLINE virtual UTransformationComponent* GetTransformationComponent()  const override { return TransformationComponent; } ;
	FORCEINLINE virtual UPandolfoComponent* GetPandolfoComponent() const override { return PandolfoComponent; }
	FORCEINLINE virtual UPanWolfComponent* GetPanWolfComponent() const override { return PanWolfComponent; }
	FORCEINLINE virtual UPandolFlowerComponent* GetPandolFlowerComponent() const override { return PandolFlowerComponent; }
	FORCEINLINE UPanBirdComponent* GetPanBirdComponent() const { return PanBirdComponent; }

	FORCEINLINE UNiagaraComponent* GetNiagaraTransformation() { return NiagaraTransformation; }
	//FORCEINLINE UNiagaraComponent* GetNiagaraTransformationEffect() { return NiagaraApplyTransformationEffect; }


	FORCEINLINE virtual bool IsHiding() const override { return bIsHiding; }
	FORCEINLINE bool IsInsideHideBox() const  { return bIsInsideHideBox; }
	FORCEINLINE bool IsForcedCrouch() const { return bIsForcedCrouch; }

	
	virtual void AddEnemyAware(AActor* Enemy) override;
	virtual void RemoveEnemyAware(AActor* Enemy) override;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE TArray<AActor*> GetEnemyAware() { return EnemyAware; }
	
	FORCEINLINE bool IsHitted() { return bHitted; }
#pragma endregion



};

