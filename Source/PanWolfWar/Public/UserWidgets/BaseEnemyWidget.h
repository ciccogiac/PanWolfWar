#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseEnemyWidget.generated.h"


UCLASS()
class PANWOLFWAR_API UBaseEnemyWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetHealthBarPercent(float Percent);

protected:

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;
};
