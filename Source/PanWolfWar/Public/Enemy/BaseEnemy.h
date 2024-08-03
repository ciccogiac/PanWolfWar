#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Engine/TargetPoint.h>
#include "Interfaces/CombatInterface.h"
#include "BaseEnemy.generated.h"


class UBehaviorTree;
class UCombatComponent;


UCLASS()
class PANWOLFWAR_API ABaseEnemy : public ACharacter, public ICombatInterface
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

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	virtual void Die();

	UFUNCTION(BlueprintCallable)
	float PerformAttack();

private:	

	void FindNearestAI();

protected:
	bool bDied = false;
	bool bSeen = false;
	ACharacter* Player;

	FTimerHandle FindEnemies_TimerHandle;

	class ABaseAIController* BaseAIController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAcces = "true"))
	class UMotionWarpingComponent* MotionWarping;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* BehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat)
	AActor* CombatTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination Components", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PlayerVisibleWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sliding", meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AllowPrivateAccess = "true"))
	double WarpTargetDistance = 75.f;

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
