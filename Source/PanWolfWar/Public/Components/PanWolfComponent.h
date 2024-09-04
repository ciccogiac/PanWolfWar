

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PanWolfComponent.generated.h"

class APanWolfWarCharacter;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UPandoCombatComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UENUM(BlueprintType)
enum class EPanWolfState : uint8
{
	EPWS_PanWolf UMETA(DisplayName = "PanWolf"),
	EPWS_Dodging UMETA(DisplayName = "Dodging"),
	EPWS_Blocking UMETA(DisplayName = "Blocking")
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UPanWolfComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPanWolfComponent();

	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	void Jump();
	void LightAttack();
	void HeavyAttack();
	void Dodge();

	void Block();
	void UnBlock();
	void SuccesfulBlock(AActor* Attacker);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	void AddShield();
	void RemoveShield();

	void ResetPerfectBlock();

private:
	APanWolfWarCharacter* PanWolfCharacter;
	ACharacter* CharacterOwner;
	UPandoCombatComponent* CombatComponent;
	UAnimInstance* OwningPlayerAnimInstance;

	EPanWolfState PanWolfState = EPanWolfState::EPWS_PanWolf;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> Anim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* PanWolfMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* PanWolfDodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* PanWolf_HitReactMontage;


	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* PanWolf_BlockMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	USoundBase* ShieldBlock_Sound;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UNiagaraSystem* BlockShieldNiagara;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UNiagaraSystem* PerfectBlockShieldNiagara;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	USoundBase* AddShield_Sound;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UNiagaraSystem* ShieldNiagara;

	UNiagaraComponent* Shield_NiagaraComp;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float BlockRepulsionForce = 2500000.f;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float PerfectBlockTime = 0.25f;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	float PerfectBlockTimer = 0.1f;

	float BlockActivatedTime = 0.f;
	bool bIsPerfectBlock = false;

	FTimerHandle PerfectBlock_TimerHandle;
		
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HeavyAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BlockAction;

public:
	FORCEINLINE UAnimMontage* GetPanWolfHitReactMontage() const { return PanWolf_HitReactMontage; }
	FORCEINLINE bool IsBlocking() const { return PanWolfState == EPanWolfState::EPWS_Blocking; }
};
