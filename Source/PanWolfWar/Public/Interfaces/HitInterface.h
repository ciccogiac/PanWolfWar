#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitInterface.generated.h"


UINTERFACE(MinimalAPI)
class UHitInterface : public UInterface
{
	GENERATED_BODY()
};


class PANWOLFWAR_API IHitInterface
{
	GENERATED_BODY()

public:
	virtual void GetHit(const FVector& ImpactPoint, AActor* Hitter) = 0;
	static FName DirectionalHitReact(AActor* InAttacker, AActor* InVictim);
};
