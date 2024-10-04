// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AssassinableComponent.generated.h"

class UNiagaraSystem;
class ABaseEnemy;
class UPandolfoComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UAssassinableComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAssassinableComponent();

	UFUNCTION()
	virtual void BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void MarkAsTarget(bool IsMarked);
	void Assassinated(int32 AssassinationIndex, UPandolfoComponent* _PandolfoComponent, bool AirAssassination = false);


	void Killed();

	UFUNCTION()
	void OnPlayerTransformationStateChanged(ETransformationState NewTransformationState);

protected:
	virtual void BeginPlay() override;

private:
	void PlayBloodEffect();
	void AirAssassinated();

		
private:
	ACharacter* CharacterOwner;
	ABaseEnemy* EnemyOwner;

	bool bCanBeAssassinated = false;
	bool bIsMarked = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination", meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* BloodEffect_Niagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination", meta = (AllowPrivateAccess = "true"))
	FName BloodEffect_SocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> AssassinationCameraShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination", meta = (AllowPrivateAccess = "true"))
	TMap<int32, UAnimMontage*> AssassinationMontage_Map;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* AirAssassinDeathMontage;
};
