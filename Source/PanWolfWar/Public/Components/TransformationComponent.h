// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include <PanWolfWar/CharacterStates.h>

#include "TransformationComponent.generated.h"


class UInputAction;
class UTransformationWidget;
class UNiagaraSystem;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UTransformationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTransformationComponent();

	void SelectRightTransformation();
	void SelectLeftTransformation();
	void ApplyTrasformation();

	void SelectDesiredTransformation();

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransformationMap")
	//TMap<ETransformationState, FTransformationImage> TransformationMap;

protected:
	virtual void BeginPlay() override;

private:
	bool CanTrasform();
	

private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation State ", meta = (AllowPrivateAccess = "true"))
	ETransformationState CurrentTransformationState = ETransformationState::ETS_Pandolfo;

	int DesiredTransformationState_ID = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation State ", meta = (AllowPrivateAccess = "true"))
	TArray<ETransformationState>  PossibleTransformationState;

	
	UTransformationWidget* TransformationWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Transformation State ", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UTransformationWidget> TransformationWidgetClass;

	class APanWolfWarCharacter* PanWolfWarCharacter;

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






public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Transformation")
	UInputAction* TransformationSelectRightAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Transformation")
	UInputAction* TransformationSelectLeftAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input Transformation")
	UInputAction* TransformationApply;

	FORCEINLINE ETransformationState GetCurrentState() const { return CurrentTransformationState; }
	FORCEINLINE bool IsInTransformingState() const { return CurrentTransformationState != ETransformationState::ETS_Pandolfo; }
		
};
