

#pragma once

#include "CoreMinimal.h"
#include "Components/TransformationCharacterComponent.h"
#include "Components/ActorComponent.h"
#include "PanWolfComponent.generated.h"

class UInputAction;
class UAnimMontage;
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
class PANWOLFWAR_API UPanWolfComponent : public UTransformationCharacterComponent
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
	void InstantBlock();
	void UnBlock();
	void SuccesfulBlock(AActor* Attacker);
	bool IsWolfValidBlock(AActor* InAttacker);

	void ClearAllTimer() override;

	UFUNCTION(BlueprintCallable)
	void ReturnToBlockFromAttack();

	UAnimMontage* GetPanWolfHitReactMontage();
	void OnWolfHitReactMontageEnded();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	void AddShield();
	void RemoveShield();

	void LeftBlock();
	void RightBlock();

	void ResetPerfectBlock();

	UFUNCTION()
	void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:


	bool bHitted = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* PanWolfDodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* PanWolf_HitReactMontage;


	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UAnimMontage* PanWolf_BlockMontage;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	USoundBase* Block_Sound;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UNiagaraSystem* BlockEffectNiagara;

	UPROPERTY(EditDefaultsOnly, Category = Combat)
	UNiagaraSystem* PerfectBlockEffectNiagara;

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
	float PerfectBlockTimer = 0.15f;

	float BlockActivatedTime = 0.f;
	bool bIsPerfectBlock = false;

	FTimerHandle PerfectBlock_TimerHandle;


	bool bIsBlocking = false;
	bool bIsBlockingReact = false;
		
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "PanWolf State ")
	EPanWolfState PanWolfState = EPanWolfState::EPWS_PanWolf;

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
	FORCEINLINE bool IsBlocking() const { return PanWolfState == EPanWolfState::EPWS_Blocking; }
	FORCEINLINE bool IsBlockingCharged() const { return bIsBlocking; }
};
