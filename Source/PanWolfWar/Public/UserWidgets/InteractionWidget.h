#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionWidget.generated.h"


UCLASS()
class PANWOLFWAR_API UInteractionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetKeyInteractionText(const FString String);

protected:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* InteractionKey_Text;

	
};
