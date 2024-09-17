#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnCombatComponent.generated.h"

class APanWarWeaponBase;
class UBoxComponent;

UENUM(BlueprintType)
enum class EToggleDamageType : uint8
{
	CurrentEquippedWeapon,
	LeftHand,
	RightHand
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UPawnCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPawnCombatComponent();

	void InitializeCombatStats(float _BaseDamage ,float _AttackPower , float _DefensePower);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EquipWeapon(APanWarWeaponBase* InWeaponToRegister);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	APanWarWeaponBase* GetCurrentEquippedWeapon() const;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ToggleWeaponCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType = EToggleDamageType::CurrentEquippedWeapon);

	void EnableHandToHandCombat(UBoxComponent* _LeftHandCollisionBox, UBoxComponent* _RightHandCollisionBox);

	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);

	UFUNCTION(BlueprintCallable)
	virtual void ResetAttack();

protected:
	virtual void BeginPlay() override;
	virtual bool ExecuteHitActor(FHitResult& Hit);
	virtual float CalculateFinalDamage(float BaseDamage, float TargetDefensePower);

private:

	void ToggleCurrentEquippedWeaponCollision(bool bShouldEnable);
	void ToggleBodyCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType);
	void BoxCollisionTrace(EToggleDamageType ToggleDamageType);
	void ApplyDamageToActorHit(AActor* DamagedActor, float BaseDamage, AController* EventInstigator, AActor* DamageCauser, TSubclassOf<UDamageType> DamageTypeClass);

protected:
	ACharacter* CharacterOwner;
	UAnimInstance* OwningPlayerAnimInstance = nullptr;

	FTimerHandle WeaponCollision_TimerHandle;
	FTimerHandle RightHandCollision_TimerHandle;
	FTimerHandle LeftHandCollision_TimerHandle;

	UPROPERTY(EditAnywhere, Category = Category = "Combat | FX")
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = "Combat | FX")
	UParticleSystem* HitParticles;

	TArray<AActor*> AlreadyHitActor = TArray<AActor*>();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Collision Trace", meta = (AllowPrivateAccess = "true"))
	bool ShowDebugTrace = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats", meta = (AllowPrivateAccess = "true"))
	float BaseAttackDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats", meta = (AllowPrivateAccess = "true"))
	float AttackPower = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Stats", meta = (AllowPrivateAccess = "true"))
	float DefensePower = 1.f;
	 
	bool CachedUnblockableAttack = false;

	

private:

	APanWarWeaponBase* CurrentEquippedWeapon;
	UBoxComponent* LeftHandCollisionBox;
	UBoxComponent* RightHandCollisionBox;

public:
	FORCEINLINE float GetDefensePower() const { return DefensePower; }
};
