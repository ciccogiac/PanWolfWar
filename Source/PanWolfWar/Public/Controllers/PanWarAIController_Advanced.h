#pragma once

#include "CoreMinimal.h"
#include "Controllers/PanWarAIController.h"
#include "Interfaces/CharacterInterface.h"
#include "PanWarAIController_Advanced.generated.h"

class UAISenseConfig_Hearing;
class UAISenseConfig_Damage;

UCLASS()
class PANWOLFWAR_API APanWarAIController_Advanced : public APanWarAIController
{
	GENERATED_BODY()

public:
	APanWarAIController_Advanced(const FObjectInitializer& ObjectInitializer);

	virtual void OnPossess(APawn* InPawn) override;
	UFUNCTION()
	void OnPossessedPawnDeath(ABaseEnemy* Enemy);

protected:

	virtual void OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus) override;
	void SetNewTargetActor(AActor* Actor);
	void SetBlackboardTargetActor( AActor* Actor);
	UFUNCTION(BlueprintCallable)
	void NotifyNearbyAllies(AActor* DetectedActor);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:

	void IncrementAwareness(AActor* DetectedActor);
	void DecrementAwareness();

protected:

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

	FTimerHandle FoundTarget_TimerHandle;
	FTimerHandle LostTarget_TimerHandle;

	float Awareness = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI | Awareness", meta = (AllowPrivateAccess = "true"))
	float MaxAwarenessDistance = 1500.0f; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI | Awareness", meta = (AllowPrivateAccess = "true"))
	float AwarenessIncrementRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI | Awareness", meta = (AllowPrivateAccess = "true"))
	float AwarenessDecrementRate = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI | Awareness", meta = (AllowPrivateAccess = "true"))
	float FoundTarget_TimerLoop = 0.05f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI | Awareness", meta = (AllowPrivateAccess = "true"))
	float LostTarget_TimerLoop = 0.3f;
	
};
