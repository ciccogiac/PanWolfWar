#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Engine/TargetPoint.h>
#include "Interfaces/CombatInterface.h"
#include "Interfaces/HitInterface.h"
#include "Interfaces/PawnUIInterface.h"
#include "ScalableFloat.h"
#include "BaseEnemy.generated.h"

class UEnemyCombatComponent;
class UWidgetComponent;
class UPawnUIComponent;
class UEnemyUIComponent;
class UEnemyAttributeComponent;
class UBoxComponent;
class UAssassinableComponent;
class UNiagaraSystem;


UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_Default UMETA(DisplayName = "Default"),
	EES_Strafing UMETA(DisplayName = "Strafing"),
	EES_Blocking UMETA(DisplayName = "Blocking"),
	EES_Stunned UMETA(DisplayName = "Stunned"),
	EES_Dodging UMETA(DisplayName = "Dodging")
};

USTRUCT(BlueprintType)
struct FEnemyInitStats
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	FScalableFloat MaxHealth;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	FScalableFloat AttackPower;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	FScalableFloat DefensePower;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	FScalableFloat BlockPower;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	FScalableFloat BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	FScalableFloat StoneSpawnChance;

	// Costruttore di default
	FEnemyInitStats()
		: MaxHealth(1),AttackPower(1), DefensePower(1), BaseDamage(1), StoneSpawnChance(1)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyDeath, ABaseEnemy*, Enemy);

UCLASS()
class PANWOLFWAR_API ABaseEnemy : public ACharacter, public ICombatInterface, public IHitInterface , public IPawnUIInterface
{
	GENERATED_BODY()

#pragma region Functions

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnEnemyDeath OnEnemyDeath;

	ABaseEnemy();

	UFUNCTION(BlueprintCallable)
	virtual void SetEnemyAware(bool NewVisibility);

	//~ Begin ICombatInterface Interface.
	virtual UPawnCombatComponent* GetCombatComponent() const override;
	virtual void SetInvulnerability(bool NewInvulnerability) override;
	virtual FRotator GetDesiredDodgeRotation() override;
	virtual bool IsCombatActorAlive() override;
	virtual void AttackWarning() override;
	virtual void CancelAttack() override;
	virtual float PerformAttack() override;
	virtual bool IsUnderAttack() override;
	virtual void SetUnderAttack() override;
	virtual float GetDefensePower() override;
	virtual void OnDeathEnter() override;
	virtual bool IsBlocking() override;
	virtual bool IsBlockingAttackRecently() override;
	virtual void SuccesfulBlock(AActor* Attacker) override;
	virtual void FireProjectile() override;
	virtual float GetHealthPercent() override;
	virtual void AssassinationKilled() override;
	virtual void Block() override;
	virtual void UnBlock() override;
	virtual bool IsValidBlock(AActor* InAttacker, AActor* InDefender) override;
	virtual void ShortStunned();
	virtual void LongStunned();
	virtual bool IsStunned() override;
	virtual bool IsDodging() override;
	//~ End ICombatInterface Interface

	//HitInterface
	virtual void GetHit(const FVector& ImpactPoint, AActor* Hitter) override;


	//Assassination
	void SetAssassinationWidgetVisibility(bool bNewVisibility);
	void SetCollisionBoxAssassination(ECollisionEnabled::Type NewCollision);
	void InitializeEnemyDeath();

	//~ Begin IPawnUIInterface Interface.
	virtual UPawnUIComponent* GetPawnUIComponent() const override;
	//~ End IPawnUIInterface Interface

	void UpdateCurrentEnemyAwareness(float Percent);

	UFUNCTION()
	void OnPlayerAttack(AActor* Attacker);  // Funzione che si attiva quando il giocatore attacca

protected:
	virtual void BeginPlay() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_FireProjectile();

	//~ Begin EnemyDeath
	void HandleEnemyDeath();
	bool IsEnemyAlive();

	UFUNCTION(BlueprintImplementableEvent)
	void OnEnemyDie_Event();
	//~ End EnemyDeath

#if WITH_EDITOR
	//~ Begin UObject Interface.
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif

private:	
	//~ Begin EnemyInitialization
	void InitEnemyStats();
	void EnableAssassination();
	void EnableHandToHandCombat();
	//~ End EnemyInitialization

	UFUNCTION()
	void OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnShortStunnedMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void UnStunned();
	bool CanBlockPlayerAttack(AActor* Attacker);

	void Dodge(FName Direction);
	FName GetDodgeDirection(AActor* Attacker);
	bool IsDirectionClear(const FVector& Direction);

	UFUNCTION()
	void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

#pragma endregion

#pragma region Variables

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TArray<ATargetPoint*> PatrolPoints;

protected:

	bool bDied = false;
	bool bSeen = false;
	bool bHitted = false;
	bool bIsUnderAttack = false;
	bool bIsBlockingAttackRecently = false;

	EEnemyState EnemyState;
	ACharacter* Player;
	UAnimInstance* AnimInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug", meta = (AllowPrivateAccess = "true"))
	bool ShowDebugTrace = false;

#pragma region Initialization

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	FEnemyInitStats EnemyInitStats;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | Assassination")
	bool bEnableAssassination = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | HandToHand")
	bool bEnableHandToHandCombat = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | HandToHand", meta = (EditCondition = "bEnableHandToHandCombat"))
	FName LeftHandCollisionBoxAttachBoneName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | HandToHand", meta = (EditCondition = "bEnableHandToHandCombat"))
	FName RightHandCollisionBoxAttachBoneName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | Assassination")
	bool bEnableBlockAttack = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | Assassination")
	bool bEnableDodge = false;


#pragma endregion

#pragma region Components

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAcces = "true"))
	class UMotionWarpingComponent* MotionWarping;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UEnemyUIComponent* EnemyUIComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UEnemyCombatComponent* EnemyCombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UAssassinableComponent* AssassinableComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* EnemyCombatBaseWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* EnemyAwarenessBarWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UEnemyAttributeComponent* EnemyAttributeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Assassination", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* AssassinationBoxComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | Assassination", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* AssassinationWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | Assassination", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Assassination_Preview_Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | HandToHand")
	UBoxComponent* LeftHandCollisionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | HandToHand")
	UBoxComponent* RightHandCollisionBox;

#pragma endregion

#pragma region Timer

	FTimerHandle UnderAttack_TimerHandle;
	FTimerHandle GetHitFX_TimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float UnderAttack_Time = 3.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float GetHitFX_Time = 0.3f;

#pragma endregion

#pragma region Montages

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TArray<UAnimMontage*> HitReact_Montages;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TArray<UAnimMontage*> Death_Montages;


	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* Death_Sound;

#pragma endregion

#pragma region Block

	UPROPERTY(EditDefaultsOnly, Category = Block)
	float PerfectBlockTime = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = Block)
	float PerfectBlockTimer = 0.15f;

	float BlockActivatedTime = 0.f;
	bool bIsPerfectBlock = false;

	FTimerHandle PerfectBlock_TimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EnemyBlockMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EnemyShortStunnedMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EnemyStunnedMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Block Montages", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* EnemyDodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = Block)
	USoundBase* Block_Sound;

	UPROPERTY(EditDefaultsOnly, Category = Block)
	UNiagaraSystem* BlockEffectNiagara;

	UPROPERTY(EditDefaultsOnly, Category = Block)
	UNiagaraSystem* PerfectBlockEffectNiagara;

	FTimerHandle UnBlock_TimerHandle;
	float UnBlockTime = 1.2f;

	FTimerHandle BlockAttackRecently_TimerHandle;
	float BlockAttackRecentlyTime = 1.5f;

	FTimerHandle UnStunned_TimerHandle;
	/*float UnStunnedShortTime = 3.f;*/
	float UnStunnedLongTime = 10.f;

	bool bIsLongStunned = false;


	UPROPERTY(EditDefaultsOnly, Category = Block)
	float MaxBlockDistance = 310.0f;

	UPROPERTY(EditDefaultsOnly, Category = Block)
	float MaxBlockAngle = 65.0f;

	UPROPERTY(EditDefaultsOnly, Category = Block)
	float MaxPrevisionBlockAngle = 65.0f;

	UPROPERTY(EditDefaultsOnly, Category = Block)
	float SuccessChanceBlockMin = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = Block)
	float SuccessChanceBlockMax = 0.9f;

	UPROPERTY(EditDefaultsOnly, Category = Block)
	float SuccessChanceDodgeMin = 0.4f;

	UPROPERTY(EditDefaultsOnly, Category = Block)
	float SuccessChanceDodgeMax =0.75f;

#pragma endregion


#pragma endregion

#pragma region ForceinlineFunctions

public:

	FORCEINLINE bool IsEnemyAware() const { return bSeen; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetEnemyState(EEnemyState NewState) { EnemyState = NewState; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	FORCEINLINE EEnemyState GetEnemyState() const { return EnemyState; }

	UFUNCTION(BlueprintPure, meta = (BlueprintThreadSafe))
	FORCEINLINE bool IsEnemyStateEqualTo(EEnemyState StateToCheck) const { return EnemyState == StateToCheck; }

	FORCEINLINE UAssassinableComponent* GetAssassinableComponent()  const { return AssassinableComponent; }
	FORCEINLINE FTransform GetAssassinationTransform() const { return Assassination_Preview_Mesh->GetComponentTransform(); }
	FORCEINLINE ACharacter* GetPlayer() const { return Player; }

	//~ Begin IPawnUIInterface Interface.
	FORCEINLINE virtual UEnemyUIComponent* GetEnemyUIComponent() const override { return EnemyUIComponent; }
	//~ End IPawnUIInterface Interface

#pragma endregion

};
