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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTransformationStateChanged, ETransformationState, NewState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UTransformationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTransformationComponent();

	void SelectRightTransformation();
	void SelectLeftTransformation();
	void SelectUPTransformation();
	void ApplyTrasformation();
	void AnnulTrasnformation();

	void SetTransformation(ETransformationState NewTransformationState, ETransformationState PreviousTransformationState);

	void SetCanRegenFlower(bool CanRegenFlower);
	void SetCanRegenBird(bool CanRegenBird);

	bool AddItemStamina(ETransformationObjectTypes TransformationItemType,float Value);

	UFUNCTION(BlueprintCallable)
	void SelectDesiredTransformation(int32 TransformationState_ID);

	void InitializeTransformationUI(UPandoUIComponent* _PandoUIComponent);

	UPROPERTY(BlueprintAssignable, Category = "Transformation")
	FOnTransformationStateChanged OnTransformationStateChanged;

protected:
	virtual void BeginPlay() override;

private:
	bool CanTrasform(const int32 NewTransformation_ID);
	void ExecuteTransformation(ETransformationState NewTransformationState );
	void HandleComponentActivation(ETransformationState NewTransformationState, ETransformationState PreviousTransformationState);

	//void ConsumingBeer();

	void ConsumingTransformation(ETransformationState TransfomingState);

#pragma region Variables

	bool bBirdConsuptionStopped = false;
	bool bFlowerConsuptionStopped = false;

	UPandoUIComponent* PandoUIComponent;

	// Timer handle
	FTimerHandle Transformation_TimerHandle;
	FTimerHandle RegenFlower_TimerHandle;
	FTimerHandle RegenBird_TimerHandle;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation State ", meta = (AllowPrivateAccess = "true"))
	ETransformationState CurrentTransformationState = ETransformationState::ETS_None;

	int32 DesiredTransformationState_ID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation State ", meta = (AllowPrivateAccess = "true"))
	TArray<ETransformationState>  PossibleTransformationState;

	class APanWolfWarCharacter* PanWolfWarCharacter;

	class UAttributeComponent* Attributes;
	class UInteractComponent* InteractComponent;



#pragma endregion

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AnnulTransformationAction;
	
	FORCEINLINE ETransformationState GetCurrentTransformationState() const { return CurrentTransformationState; }
	FORCEINLINE void SetFlowerConsuptionStopped(bool NewValue)  {  bFlowerConsuptionStopped = NewValue; }
	
};
