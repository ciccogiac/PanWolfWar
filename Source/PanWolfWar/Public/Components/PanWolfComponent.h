

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PanWolfComponent.generated.h"

class APanWolfWarCharacter;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UCombatComponent;

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

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:

	void ResetLightAttackComboCount();
	void ResetHeavyAttackComboCount();

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

private:
	APanWolfWarCharacter* PanWolfCharacter;
	ACharacter* CharacterOwner;
	UCombatComponent* CombatComponent;
	UAnimInstance* OwningPlayerAnimInstance;

	int32 CurrentLightAttackComboCount = 1;
	int32 CurrentHeavyAttackComboCount = 1;
	bool bJumpToFinisher = false;
	FTimerHandle ComboLightCountReset_TimerHandle;
	FTimerHandle ComboHeavyCountReset_TimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages", meta = (AllowPrivateAccess = "true"))
	TMap<int32,UAnimMontage*> LightAttackMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack Montages", meta = (AllowPrivateAccess = "true"))
	TMap<int32, UAnimMontage*> HeavyAttackMontages;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMesh> SkeletalMeshAsset;

	UPROPERTY(Category = Character, EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> Anim;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* PanWolfMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dodge", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* PandolWolfDodgeMontage;
		
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HeavyAttackAction;

};
