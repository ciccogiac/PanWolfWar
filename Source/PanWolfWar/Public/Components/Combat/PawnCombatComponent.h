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

UENUM(BlueprintType)
enum class EAttackState : uint8
{
	EAS_Nothing UMETA(DisplayName = "Nothing"),
	EAS_Attacking UMETA(DisplayName = "Attacking")
};

// Delegate multicast che sar� emesso quando viene effettuato un attacco
//DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPerformAttack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformAttack, AActor*, Attacker);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UPawnCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPawnCombatComponent();

	void InitializeCombatStats(float _BaseDamage ,float _AttackPower , float _DefensePower , float _BlockPower = 1);

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


	// Delegate che notifica l'inizio di un attacco
	//UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnPerformAttack OnPerformAttack;

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

	EAttackState AttackState = EAttackState::EAS_Nothing;

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

	float BaseAttackDamage = 10.f;
	float AttackPower = 1.f;
	float DefensePower = 1.f;
	float BlockPower = 1.f;
	 
	bool CachedUnblockableAttack = false;
	bool CachedStunnedAttack = false;

	

private:

	APanWarWeaponBase* CurrentEquippedWeapon;
	UBoxComponent* LeftHandCollisionBox;
	UBoxComponent* RightHandCollisionBox;

public:
	FORCEINLINE float GetDefensePower() const { return DefensePower; }
	FORCEINLINE float GetBlockPower() const { return BlockPower; }
	FORCEINLINE EAttackState GetAttackState() const { return AttackState; }
	FORCEINLINE bool IsAttacking() const { return AttackState == EAttackState::EAS_Attacking; }
};
