

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PanWolfComponent.generated.h"

class APanWolfWarCharacter;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UPandoCombatComponent;

UENUM(BlueprintType)
enum class EPanWolfState : uint8
{
	EPWS_PanWolf UMETA(DisplayName = "PanWolf"),
	EPWS_Dodging UMETA(DisplayName = "Dodging")
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

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

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
		
public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HeavyAttackAction;

public:
	FORCEINLINE UAnimMontage* GetPanWolfHitReactMontage() const { return PanWolf_HitReactMontage; }
};
