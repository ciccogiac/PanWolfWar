#pragma once

#include "CoreMinimal.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "EnemyCombatComponent.generated.h"


UCLASS()
class PANWOLFWAR_API UEnemyCombatComponent : public UPawnCombatComponent
{
	GENERATED_BODY()

public:

	virtual void ResetAttack() override;
};
