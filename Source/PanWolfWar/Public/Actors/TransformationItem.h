#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/InteractInterface.h"
#include "TransformationItem.generated.h"

class UNiagaraComponent;

UCLASS()
class PANWOLFWAR_API ATransformationItem : public AActor
{
	GENERATED_BODY()
	
public:	
	ATransformationItem();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact Params", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* BoxComponent;

	UFUNCTION()
	virtual void BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:

	void CollectObject();
	void ResetObject();

	// Timer handle
	FTimerHandle Item_TimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category =  Params, meta = (AllowPrivateAccess = "true"))
	ETransformationObjectTypes TransformationItemType = ETransformationObjectTypes::ETOT_PanWolf_Object;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Params, meta = (AllowPrivateAccess = "true"))
	float StaminaToAdd = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Params, meta = (AllowPrivateAccess = "true"))
	float SecondToRespawn = 10.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* NiagaraCollectObjectEffect;

};
