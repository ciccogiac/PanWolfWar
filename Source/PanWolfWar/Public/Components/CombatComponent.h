#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class UAnimMontage;

UENUM(BlueprintType)
enum class EAttackType2 : uint8
{
	EAT_LightAttack_Right UMETA(DisplayName = "LightAttack_Right"),
	EAT_LightAttack_Left UMETA(DisplayName = "LightAttack_Left"),
	EAT_HeavyAttack UMETA(DisplayName = "HeavyAttack")
};

USTRUCT(BlueprintType)
struct FCollisionPart2Struct
{
	GENERATED_USTRUCT_BODY()

	FCollisionPart2Struct()
	{
	}

	UPrimitiveComponent* CollisionMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName StartSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EndSocketName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TraceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();

	void SetCombatEnabled(bool CombatEnabled);

	void PerformAttack(EAttackType2 AttackType);

	UFUNCTION(BlueprintCallable)
	void ContinueAttack();
	UFUNCTION(BlueprintCallable)
	void ResetAttack();


	UFUNCTION(BlueprintCallable)
	void ActivateCollision(FString CollisionPart);

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision(FString CollisionPart);

	UFUNCTION(BlueprintCallable)
	void SetCollisionCharacterPartMesh();

	UFUNCTION(BlueprintCallable)
	void SetCollisionWeaponPartMesh(UPrimitiveComponent* Mesh);

	void SetBlockingState(bool EnableBlocking);

	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);

	const AActor* GetClosestEnemy();
	const bool GetEnemyDirection(const AActor* ClosestEnemy);
	void RotateToClosestEnemy(const AActor* ClosestEnemy);

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	TArray<UAnimMontage*> GetAttackMontages(EAttackType2 AttackType);
	void TraceLoop();

	bool ActorIsSameType(AActor* OtherActor);
	void ExecuteGetHit(FHitResult& Hit);

	

private:
	ACharacter* CharacterOwner;

	bool bCombatEnabled = false;
	bool bAttackSaved = false;
	EAttackType2 SavedAttackType;
	bool bIsBlocking = false;
	int32 AttackCount = 0;
		
	bool bIsAttacking = false;


	FTimerHandle Collision_TimerHandle;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* HitParticles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages", meta = (AllowPrivateAccess = "true"))
	TArray<UAnimMontage*> Right_LightAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages", meta = (AllowPrivateAccess = "true"))
	TArray<UAnimMontage*> Left_LightAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages", meta = (AllowPrivateAccess = "true"))
	TArray<UAnimMontage*> HeavyAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision Parts", meta = (AllowPrivateAccess = "true"))
	TMap<FString, FCollisionPart2Struct> CollisionStructs;
	
	FCollisionPart2Struct* ActiveCollisionPart = nullptr;
	TArray<AActor*> AlreadyHitActor = TArray<AActor*>();
public:
	FORCEINLINE bool IsCombatEnabled() const { return bCombatEnabled; }
	FORCEINLINE bool IsBlocking() const { return bIsBlocking; }
	FORCEINLINE bool IsAttacking() const { return bIsAttacking; }
};

