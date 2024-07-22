#pragma once

#include "CoreMinimal.h"
#include "Actors/InteractableObject.h"
#include "InteractableLoad.generated.h"


UCLASS()
class PANWOLFWAR_API AInteractableLoad : public AInteractableObject
{
	GENERATED_BODY()
	
public:
	AInteractableLoad();

	virtual bool Interact(ACharacter* _CharacterOwner = nullptr) override;

protected:
	virtual void BeginPlay() override;

private:

	void DecreasePercentage();

private:
	bool bFirstInteraction = true;

	float Percentage = 0.f;
	float InteractPercent = 5.f;
	float DecreasePercent = 2.5f;
	float DecreaseTime = 1.f;

	FTimerHandle Percentage_TimerHandle;
};
