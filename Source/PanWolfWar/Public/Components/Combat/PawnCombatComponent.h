#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnCombatComponent.generated.h"

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
class PANWOLFWAR_API UPawnCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPawnCombatComponent();

	void SetCombatEnabled(bool CombatEnabled, UAnimInstance* PlayerAnimInstance);

	UFUNCTION(BlueprintCallable)
	virtual void ResetAttack();

	UFUNCTION(BlueprintCallable)
	void ActivateCollision(FString CollisionPart);

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision(FString CollisionPart);

	UFUNCTION(BlueprintCallable)
	void SetCollisionCharacterPartMesh();

	UFUNCTION(BlueprintCallable)
	void SetCollisionWeaponPartMesh(UPrimitiveComponent* Mesh);

	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);

protected:
	virtual void BeginPlay() override;
	bool IsPlayingMontage_ExcludingBlendOut();
	virtual bool ExecuteHitActor(FHitResult& Hit);

	virtual void HandleNewAnimInstance();

private:

	void TraceLoop();

	bool ActorIsSameType(AActor* OtherActor);


protected:
	ACharacter* CharacterOwner;
	UAnimInstance* OwningPlayerAnimInstance = nullptr;

	FTimerHandle Collision_TimerHandle;

	UPROPERTY(EditAnywhere, Category = Combat)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = Combat)
	UParticleSystem* HitParticles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Collision Parts", meta = (AllowPrivateAccess = "true"))
	TMap<FString, FCollisionPartStruct> CollisionStructs;

	FCollisionPartStruct* ActiveCollisionPart = nullptr;
	TArray<AActor*> AlreadyHitActor = TArray<AActor*>();
};
