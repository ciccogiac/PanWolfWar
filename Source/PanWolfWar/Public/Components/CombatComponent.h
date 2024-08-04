#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class UAnimMontage;

UENUM(BlueprintType)
enum class EAttackType : uint8
{
	EAT_LightAttack UMETA(DisplayName = "LightAttack"),
	EAT_HeavyAttack UMETA(DisplayName = "HeavyAttack")
};

USTRUCT(BlueprintType)
struct FCollisionPartStruct
{
	GENERATED_USTRUCT_BODY()

	FCollisionPartStruct()
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

	void PerformAttack(EAttackType AttackType);

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

protected:

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	TArray<UAnimMontage*> GetAttackMontages(EAttackType AttackType);
	void TraceLoop();

	bool ActorIsSameType(AActor* OtherActor);
	void ExecuteGetHit(FHitResult& Hit);

private:
	ACharacter* CharacterOwner;

	bool bCombatEnabled = false;
	bool bAttackSaved = false;
	bool bIsBlocking = false;
	int32 AttackCount = 0;
		
	bool bIsAttacking = false;


	FTimerHandle Collision_TimerHandle;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* HitParticles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages", meta = (AllowPrivateAccess = "true"))
	TArray<UAnimMontage*> LightAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages", meta = (AllowPrivateAccess = "true"))
	TArray<UAnimMontage*> HeavyAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision Parts", meta = (AllowPrivateAccess = "true"))
	TMap<FString, FCollisionPartStruct> CollisionStructs;
	
	FCollisionPartStruct* ActiveCollisionPart = nullptr;
	TArray<AActor*> AlreadyHitActor = TArray<AActor*>();
public:
	FORCEINLINE bool IsCombatEnabled() const { return bCombatEnabled; }
	FORCEINLINE void SetCombatEnabled(bool CombatEnabled) { bCombatEnabled = CombatEnabled; }
	FORCEINLINE bool IsBlocking() const { return bIsBlocking; }
};

