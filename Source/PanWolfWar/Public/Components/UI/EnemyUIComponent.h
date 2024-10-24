#pragma once

#include "CoreMinimal.h"
#include "Components/UI/PawnUIComponent.h"
#include "EnemyUIComponent.generated.h"

class UPanWarWidgetBase;

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPercentChangedDelegate, float, NewPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionUIDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoolValueChangedDelegate, bool, NewValue);


UCLASS()
class PANWOLFWAR_API UEnemyUIComponent : public UPawnUIComponent
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable)
	FOnPercentChangedDelegate OnCurrentAwarenessChanged;

	UPROPERTY(BlueprintAssignable)
	FOnBoolValueChangedDelegate OnAttackingStateChanged;

	UPROPERTY(BlueprintAssignable)
	FOnActionUIDelegate OnAttackingCanceled;

	UPROPERTY(BlueprintAssignable)
	FOnVisibilityChangedDelegate OnAssassinationIconChanged;

	UFUNCTION(BlueprintCallable)
	void RegisterEnemyDrawnWidget(UPanWarWidgetBase* InWidgetToRegister);

	UFUNCTION(BlueprintCallable)
	void RemoveEnemyDrawnWidgetsIfAny();

private:
	TArray<UPanWarWidgetBase*> EnemyDrawnWidgets;

};
