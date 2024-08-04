#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TargetInterface.generated.h"


UINTERFACE(MinimalAPI, BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UTargetInterface : public UInterface
{
	GENERATED_BODY()
};


class PANWOLFWAR_API ITargetInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void SetTargetVisibility(bool NewVisibility) = 0;

	UFUNCTION(BlueprintCallable)
	virtual bool CanBeTargeted() = 0;
};
