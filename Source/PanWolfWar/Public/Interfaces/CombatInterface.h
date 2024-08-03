#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

//UINTERFACE(MinimalAPI, Blueprintable)
UINTERFACE(MinimalAPI,BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class UCombatInterface : public UInterface
{
	GENERATED_BODY()

};


class PANWOLFWAR_API ICombatInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	virtual void ActivateCollision(FString CollisionPart) = 0;
	UFUNCTION(BlueprintCallable)
	virtual void DeactivateCollision(FString CollisionPart) = 0;

};
