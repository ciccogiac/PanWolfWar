#pragma once

#include "CoreMinimal.h"
#include "BaseEnemy.h"
#include "AssassinableEnemy.generated.h"

class UNiagaraComponent;
class UAnimMontage;

UCLASS()
class PANWOLFWAR_API AAssassinableEnemy : public ABaseEnemy
{
	GENERATED_BODY()

public:
	AAssassinableEnemy();

	void Assassinated(UAnimMontage* AssassinatedMontage);

protected:
	virtual void BeginPlay() override;

	virtual void SetPlayerVisibilityWidget(bool NewVisibility) override;

	UFUNCTION()
	virtual void BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:

	UFUNCTION(BlueprintCallable)
	void Killed();

	UFUNCTION(BlueprintCallable)
	void Die();

private:

	//bool bDied = false;
	FTimerHandle Die_TimerHandle;
	class UPandolfoComponent* PandolfoComponent;

	/** Niagara */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Effects, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* Niagara_DieEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Effects, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* Niagara_BloodEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Assassination Components", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* BoxComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination Components", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* AssassinationWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* Assassin_Preview_Mesh;
	
	//UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination", meta = (AllowPrivateAccess = "true"))
	//UAnimMontage* AssassinationMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Assassination", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UCameraShakeBase> CameraShake;

public:
	FORCEINLINE FTransform GetAssassinationTransform() const { return Assassin_Preview_Mesh->GetComponentTransform(); }
};
