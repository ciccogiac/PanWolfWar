#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BaseAIController.generated.h"


UCLASS()
class PANWOLFWAR_API ABaseAIController : public AAIController
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintImplementableEvent)
	void Die();

protected:
	virtual ETeamAttitude::Type GetTeamAttitudeTowards( const AActor& other) const override;
};
