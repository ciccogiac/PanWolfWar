#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GrapplePointWidget.generated.h"


UCLASS()
class PANWOLFWAR_API UGrapplePointWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	class UImage* Background_Image;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (BindWidget))
	class UImage* Filling_Image;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* NodeUse ;


};
