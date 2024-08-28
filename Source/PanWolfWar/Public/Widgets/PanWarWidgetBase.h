#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PanWarWidgetBase.generated.h"

class UPandoUIComponent;
class UEnemyUIComponent;

UCLASS()
class PANWOLFWAR_API UPanWarWidgetBase : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Owning Pando UI Component Initialized"))
	void BP_OnOwningPandoUIComponentInitialized(UPandoUIComponent* OwningHeroUIComponent);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Owning Enemy UI Component Initialized"))
	void BP_OnOwningEnemyUIComponentInitialized(UEnemyUIComponent* OwningEnemyUIComponent);

public:
	UFUNCTION(BlueprintCallable)
	void InitEnemyCreatedWidget(AActor* OwningEnemyActor);
	
};
