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

	void SetCanRegenFlower(bool Value);
	void SetCanRegenBird(bool Value);

	bool AddItemStamina(ETransformationObjectTypes TransformationItemType,float Value);

	UFUNCTION(BlueprintCallable)
	void SelectDesiredTransformation(int32 TransformationState_ID);

	void InitializeTransformationUI(UPandoUIComponent* _PandoUIComponent);

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
};
