// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interfaces/InteractInterface.h"
#include <PanWolfWar/CharacterStates.h>

#include "TransformationComponent.generated.h"

class UNiagaraSystem;
class UPandolFlowerComponent;
class UInputAction;
class UPandoUIComponent;
class UPandolfoComponent;
class UPanWolfComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransformationStateChanged, ETransformationState, NewState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UTransformationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTransformationComponent();

	void ClearAllTimer();

	void SelectPandolfoTransformation();
	void SelectFlowerTransformation();
	void SelectWolfTransformation();
	void SelectBirdTransformation();

	UFUNCTION(BlueprintCallable)
	void SelectDesiredTransformation(const ETransformationState DesiredTransformationState);

	void SetTransformation(const ETransformationState NewTransformationState,const ETransformationState PreviousTransformationState);

	void SetCanRegenFlower(bool CanRegenFlower);
	void SetCanRegenBird(bool CanRegenBird);

	bool AddItemStamina(ETransformationObjectTypes TransformationItemType,float Value);


	void InitializeTransformationUI(UPandoUIComponent* _PandoUIComponent);


	UPROPERTY(BlueprintAssignable, Category = "Transformation")
	FOnTransformationStateChanged OnTransformationStateChanged;

protected:
	virtual void BeginPlay() override;

private:
	bool CanTrasform(const ETransformationState NewDesiredTransformationState);
	void ExecuteTransformation(ETransformationState NewTransformationState );
	void HandleComponentActivation(ETransformationState NewTransformationState, ETransformationState PreviousTransformationState);

	//void ConsumingBeer();

	void ConsumingTransformation(ETransformationState TransfomingState);

#pragma region Variables

	bool bBirdConsuptionStopped = false;
	bool bFlowerConsuptionStopped = false;

	UPandoUIComponent* PandoUIComponent;
	UPandolfoComponent* PandolfoComponent;
	UPanWolfComponent* PanWolfComponent;
	// Timer handle
	FTimerHandle Transformation_TimerHandle;
	FTimerHandle RegenFlower_TimerHandle;
	FTimerHandle RegenBird_TimerHandle;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation State ", meta = (AllowPrivateAccess = "true"))
	ETransformationState CurrentTransformationState = ETransformationState::ETS_None;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation State ", meta = (AllowPrivateAccess = "true"))
	TArray<ETransformationState>  PossibleTransformationState;

	class APanWolfWarCharacter* PanWolfWarCharacter;

	class UAttributeComponent* Attributes;
	class UInteractComponent* InteractComponent;



#pragma endregion

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* SelectPandolfoTransformationAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* SelectFlowerTransformationAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* SelectWolfTransformationAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input| Transformation")
	UInputAction* SelectBirdTransformationAction;
	
	FORCEINLINE ETransformationState GetCurrentTransformationState() const { return CurrentTransformationState; }
	FORCEINLINE void SetFlowerConsuptionStopped(bool NewValue)  {  bFlowerConsuptionStopped = NewValue; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetPossibleTransformationState(TArray<ETransformationState> _PossibleTransformationState)  {  PossibleTransformationState = _PossibleTransformationState; }
};
