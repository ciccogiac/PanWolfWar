#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionLoadWidget.generated.h"


UCLASS()
class PANWOLFWAR_API UInteractionLoadWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetKeyInteractionText(const FString String);
	void SetInteractionBarPercent(float Percent);

protected:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InteractionKey_Text;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* InteractionProgressBar;
};
