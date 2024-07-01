// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include <PanWolfWar/CharacterStates.h>

#include "TransformationComponent.generated.h"


class UTransformationWidget;
class UNiagaraSystem;
class UPandolFlowerComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UTransformationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTransformationComponent();

	void SelectRightTransformation();
	void SelectLeftTransformation();
	void ApplyTrasformation();

	void SetTransformation(ETransformationState NewTransformationState, ETransformationState PreviousTransformationState);

	void SetCanRegenFlower(bool Value);
	void SetCanRegenBird(bool Value);

	UFUNCTION(BlueprintCallable)
	void SelectDesiredTransformation(int32 TransformationState_ID);

protected:
	virtual void BeginPlay() override;

private:
	bool CanTrasform(const int32 NewTransformation_ID);
	void ExecuteTransformation(ETransformationState NewTransformationState );
	void HandleComponentActivation(ETransformationState NewTransformationState, ETransformationState PreviousTransformationState);

	//void ConsumingBeer();

	void ConsumingTransformation(ETransformationState TransfomingState);

	void RegenFlower();
	void RegenBird();

#pragma region Variables

	bool bCanRegenFlower = false;
	bool bCanRegenBird = false;

	// Timer handle
	FTimerHandle Transformation_TimerHandle;
	FTimerHandle RegenFlower_TimerHandle;
	FTimerHandle RegenBird_TimerHandle;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation State ", meta = (AllowPrivateAccess = "true"))
	ETransformationState CurrentTransformationState = ETransformationState::ETS_None;

	int32 DesiredTransformationState_ID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation State ", meta = (AllowPrivateAccess = "true"))
	TArray<ETransformationState>  PossibleTransformationState;

	
	UTransformationWidget* TransformationWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation State ", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UTransformationWidget> TransformationWidgetClass;

	class APanWolfWarCharacter* PanWolfWarCharacter;

	class UAttributeComponent* Attributes;
	class UInteractComponent* InteractComponent;



#pragma endregion

	
};
