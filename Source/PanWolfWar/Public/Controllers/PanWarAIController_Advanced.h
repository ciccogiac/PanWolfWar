#pragma once

#include "CoreMinimal.h"
#include "Controllers/PanWarAIController.h"
#include "Interfaces/CharacterInterface.h"
#include "PanWarAIController_Advanced.generated.h"

class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;
class ABaseEnemy;

UCLASS()
class PANWOLFWAR_API APanWarAIController_Advanced : public APanWarAIController
{
	GENERATED_BODY()

public:
	APanWarAIController_Advanced(const FObjectInitializer& ObjectInitializer);

	virtual void OnPossess(APawn* InPawn) override;
	UFUNCTION()
	void OnPossessedPawnDeath();

protected:

	virtual void OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;
	void SetNewTargetActor(AActor* Actor);
	void SetBlackboardTargetActor( AActor* Actor);
	UFUNCTION(BlueprintCallable)
	void NotifyNearbyAllies(AActor* DetectedActor);

protected:

	ABaseEnemy* OwnerBaseEnemy;
	ICharacterInterface* CharacterInterface;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAISenseConfig_Hearing* AISenseConfig_Hearing;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UAISenseConfig_Damage* AISenseConfig_Damage;

	

private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI ", meta = (AllowPrivateAccess = "true"))
	float MinRangeToConvalidateLost = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI | AlliesResearch", meta = (AllowPrivateAccess = "true"))
	bool ShowDebugTrace = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI | AlliesResearch", meta = (AllowPrivateAccess = "true"))
	FVector AlliesResearch_BoxExtent = FVector(2000.f,2000.f,500.f);
	
};
