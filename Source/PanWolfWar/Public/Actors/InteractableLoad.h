#pragma once

#include "CoreMinimal.h"
#include "Actors/InteractableObject.h"
#include "InteractableLoad.generated.h"

class UInteractionLoadWidget;

UCLASS()
class PANWOLFWAR_API AInteractableLoad : public AInteractableObject
{
	GENERATED_BODY()
	
public:
	AInteractableLoad();

	virtual bool Interact(ACharacter* _CharacterOwner = nullptr) override;

	UFUNCTION(BlueprintImplementableEvent)
	void Interaction(float Percent);

protected:
	virtual void BeginPlay() override;

	virtual void BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
	virtual void BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;

private:

	void DecreasePercentage();

private:
	bool bFirstInteraction = true;
	bool bLoadFull = false;

	float Percentage = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction Values", meta = (AllowPrivateAccess = "true"))
	float InteractPercent = 5.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction Values", meta = (AllowPrivateAccess = "true"))
	float DecreasePercent = 1.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction Values", meta = (AllowPrivateAccess = "true"))
	float DecreaseTime = 0.2f;

	FTimerHandle Percentage_TimerHandle;

	UInteractionLoadWidget* InteractionLoadWidget;
};
