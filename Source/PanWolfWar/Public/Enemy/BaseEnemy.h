#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Engine/TargetPoint.h>
#include "Interfaces/CombatInterface.h"
#include "Interfaces/HitInterface.h"
#include "Interfaces/TargetInterface.h"
#include "BaseEnemy.generated.h"


class UBehaviorTree;
class UCombatComponent;
class UEnemyCombatComponent;
class UWidgetComponent;
class UBaseEnemyWidget;


UCLASS()
class PANWOLFWAR_API ABaseEnemy : public ACharacter, public ICombatInterface, public IHitInterface , public ITargetInterface
{
	GENERATED_BODY()

public:
	ABaseEnemy();

	UFUNCTION(BlueprintCallable)
	virtual void SetPlayerVisibility(bool NewVisibility);

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void ActivateCollision(FString CollisionPart) override;
	virtual void DeactivateCollision(FString CollisionPart) override;
	virtual void SetInvulnerability(bool NewInvulnerability) override;
	virtual FRotator GetDesiredDodgeRotation() override;
	virtual bool IsCombatActorAlive() override;
	virtual float PerformAttack() override;
	virtual bool IsUnderAttack() override;
	virtual void SetUnderAttack() override;

	//HitInterface
	virtual void GetHit(const FVector& ImpactPoint, AActor* Hitter) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	virtual void Die();



	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	void PlayHitReactMontage(const FName& SectionName);
	bool IsAlive();

	virtual void SetTargetVisibility(bool NewVisibility) override;
	virtual bool CanBeTargeted() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:	

	void FindNearestAI();
	void ApplyHitReactionPhisicsVelocity(FName HitPart);

protected:
	bool bDied = false;
	bool bSeen = false;
	float Health = 100.f;
	ACharacter* Player;

	FTimerHandle FindEnemies_TimerHandle;

	class ABaseAIController* BaseAIController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAcces = "true"))
	class UMotionWarpingComponent* MotionWarping;


	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UEnemyCombatComponent* EnemyCombatComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination Components", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* EnemyTargetWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* EnemyWidgetComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBaseEnemyWidget* BaseEnemyWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sliding", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	double WarpTargetDistance = 75.f;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	TArray<UAnimMontage*> HitReact_Montages;

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

	FORCEINLINE bool IsAware() const { return bSeen; }
};
