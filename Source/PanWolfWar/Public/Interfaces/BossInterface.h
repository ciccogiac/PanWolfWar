#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BossInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UBossInterface : public UInterface
{
	GENERATED_BODY()
};


class PANWOLFWAR_API IBossInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	virtual void SummonsEnemies() = 0;
	UFUNCTION(BlueprintCallable)
	virtual void SummonsEnemies_Notify() = 0;
};
