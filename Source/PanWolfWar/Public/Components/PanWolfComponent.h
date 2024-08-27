

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PanWolfComponent.generated.h"

class APanWolfWarCharacter;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UPandoCombatComponent;

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
	APanWolfWarCharacter* PanWolfCharacter;
	ACharacter* CharacterOwner;
	UPandoCombatComponent* CombatComponent;
	UAnimInstance* OwningPlayerAnimInstance;

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
