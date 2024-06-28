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

	UFUNCTION(BlueprintCallable)
	void SelectDesiredTransformation(int32 TransformationState_ID);

protected:
	virtual void BeginPlay() override;

private:
	bool CanTrasform(const int32 NewTransformation_ID);
	void ExecuteTransformation(ETransformationState NewTransformationState , UMaterialInterface* Material1, UMaterialInterface* Material2,UNiagaraSystem* NiagaraTransformation = nullptr );
	void HandleComponentActivation(ETransformationState NewTransformationState, ETransformationState PreviousTransformationState);

	void ConsumingBeer();

#pragma region Variables

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





	#pragma region Transformation Materials

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation Materials ", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* Pandolfo_Material1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation Materials ", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* Pandolfo_Material2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation Materials ", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* Panwolf_Material1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation Materials ", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* Panwolf_Material2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation Materials ", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* Pandolflower_Material1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation Materials ", meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* Pandolflower_Material2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation Materials ", meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* Pandolflower_Niagara;

	#pragma endregion

#pragma endregion



#pragma region FORCEINLINE_functions
public:


	FORCEINLINE ETransformationState GetCurrentState() const { return CurrentTransformationState; }
	FORCEINLINE bool IsInTransformingState() const { return CurrentTransformationState != ETransformationState::ETS_Pandolfo; }

#pragma endregion


		
};
