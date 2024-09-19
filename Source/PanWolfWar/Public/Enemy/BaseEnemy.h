#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Engine/TargetPoint.h>
#include "Interfaces/CombatInterface.h"
#include "Interfaces/HitInterface.h"
#include "Interfaces/PawnUIInterface.h"
#include "ScalableFloat.h"
#include "BaseEnemy.generated.h"


class UBehaviorTree;
class UCombatComponent;
class UEnemyCombatComponent;
class UWidgetComponent;
class UBaseEnemyWidget;
class UEnemyUIComponent;
class UEnemyAttributeComponent;
class UBoxComponent;
class UAssassinableComponent;



UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_Default UMETA(DisplayName = "Default"),
	EES_Strafing UMETA(DisplayName = "Strafing"),
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
	FScalableFloat BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	FScalableFloat StoneSpawnChance;

	// Costruttore di default
	FEnemyInitStats()
		: MaxHealth(1),AttackPower(1), DefensePower(1), BaseDamage(1), StoneSpawnChance(1)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeath);

UCLASS()
class PANWOLFWAR_API ABaseEnemy : public ACharacter, public ICombatInterface, public IHitInterface , public IPawnUIInterface
{
	GENERATED_BODY()

public:
	FOnEnemyDeath OnEnemyDeath;

	ABaseEnemy();



	UFUNCTION(BlueprintCallable)
	virtual void SetEnemyAware(bool NewVisibility);

	virtual UPawnCombatComponent* GetCombatComponent() const override;
	virtual void SetInvulnerability(bool NewInvulnerability) override;
	virtual FRotator GetDesiredDodgeRotation() override;
	virtual bool IsCombatActorAlive() override;
	virtual float PerformAttack() override;
	virtual bool IsUnderAttack() override;
	virtual void SetUnderAttack() override;
	virtual float GetDefensePower() override;
	virtual void OnDeathEnter() override;
	virtual bool IsBlocking() override;
	virtual void SuccesfulBlock(AActor* Attacker) override;
	virtual void FireProjectile() override;
	virtual float GetHealthPercent() override;
	virtual void AssassinationKilled() override;

	//HitInterface
	virtual void GetHit(const FVector& ImpactPoint, AActor* Hitter) override;

	//~ Begin IPawnUIInterface Interface.
	virtual UPawnUIComponent* GetPawnUIComponent() const override;
	virtual UEnemyUIComponent* GetEnemyUIComponent() const override;
	//~ End IPawnUIInterface Interface

	void SetAssassinationWidgetVisibility(bool bNewVisibility);
	void SetCollisionBoxAssassination(ECollisionEnabled::Type NewCollision);
	void AssassinationInitializeDie();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	virtual void Die();



	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	void PlayHitReactMontage(const FName& SectionName);
	bool IsAlive();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnEnemyDie_Event();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_FireProjectile();

#if WITH_EDITOR
	//~ Begin UObject Interface.
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface
#endif

private:	

	void InitEnemyStats();

	void FindNearestAI();
	void ApplyHitReactionPhisicsVelocity(FName HitPart);
	void EnableAssassination();
	void EnableHandToHandCombat();

protected:
	bool bDied = false;
	bool bSeen = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	FEnemyInitStats EnemyInitStats;

	/** Under Attack */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Under Attack", meta = (AllowPrivateAccess = "true"))
	float UnderAttack_Time = 6.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Under Attack", meta = (AllowPrivateAccess = "true"))
	float GetHitFX_Time = 0.3f;

	bool bIsUnderAttack = false;

	FTimerHandle UnderAttack_TimerHandle;
	FTimerHandle GetHitFX_TimerHandle;


	EEnemyState EnemyState;

	ACharacter* Player;
	UAnimInstance* AnimInstance;

	FTimerHandle FindEnemies_TimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAcces = "true"))
	class UMotionWarpingComponent* MotionWarping;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UEnemyUIComponent* EnemyUIComponent;
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UEnemyCombatComponent* EnemyCombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UAssassinableComponent* AssassinableComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* EnemyHealthBarWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBaseEnemyWidget* BaseEnemyWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UEnemyAttributeComponent* EnemyAttributeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sliding", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	double WarpTargetDistance = 75.f;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TArray<UAnimMontage*> HitReact_Montages;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TArray<UAnimMontage*> Death_Montages;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* Death_Sound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | Assassination")
	bool bEnableAssassination = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | Assassination", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* AssassinationBoxComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | Assassination", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* AssassinationWidgetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | Assassination", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Assassination_Preview_Mesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | HandToHand")
	bool bEnableHandToHandCombat = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | HandToHand")
	UBoxComponent* LeftHandCollisionBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | HandToHand", meta = (EditCondition = "bEnableHandToHandCombat"))
	FName LeftHandCollisionBoxAttachBoneName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat | HandToHand")
	UBoxComponent* RightHandCollisionBox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat | HandToHand", meta = (EditCondition = "bEnableHandToHandCombat"))
	FName RightHandCollisionBoxAttachBoneName;



public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
	TArray<ATargetPoint*> PatrolPoints;

public:
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsDead() const { return bDied; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetCombatTarget(AActor* Target) { CombatTarget = Target; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AActor* GetCombatTarget() const { return CombatTarget; }

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
};
