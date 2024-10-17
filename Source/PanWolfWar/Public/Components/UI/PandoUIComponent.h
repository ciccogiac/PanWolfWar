#pragma once

#include "CoreMinimal.h"
#include "Components/UI/PawnUIComponent.h"
#include "PandoUIComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransformationStateChangedDelegate, ETransformationState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetActorChangedDelegate, AActor*, NewTarget);

UCLASS()
class PANWOLFWAR_API UPandoUIComponent : public UPawnUIComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable)
	FOnPercentChangedDelegate OnCurrentBeerPercentChanged;

	UPROPERTY(BlueprintAssignable)
	FOnCounterChangedDelegate OnCurrentBeerCounterChanged;
	
	UPROPERTY(BlueprintAssignable)
	FOnPercentChangedDelegate OnCurrentFlowerPercentChanged;

	UPROPERTY(BlueprintAssignable)
	FOnVisibilityChangedDelegate OnFlowerIconVisibilityChanged;

	UPROPERTY(BlueprintAssignable)
	FOnVisibilityChangedDelegate OnBirdIconVisibilityChanged;

	UPROPERTY(BlueprintAssignable)
	FOnTransformationStateChangedDelegate OnTransformationStateChangedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnTargetActorChangedDelegate OnTargetActorChangedDelegate;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	FORCEINLINE void ChangeTransformationStateWidgets(ETransformationState NewState) { OnTransformationStateChangedDelegate.Broadcast(NewState); }
	
};
