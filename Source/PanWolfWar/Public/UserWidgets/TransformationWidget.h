// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"


#include <PanWolfWar/CharacterStates.h>

#include "TransformationWidget.generated.h"


USTRUCT(BlueprintType)
struct FTransformationImage
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyStruct")
	UTexture2D* Transformation_Texture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MyStruct")
	FString Transformation_Name;
};

UCLASS()
class PANWOLFWAR_API UTransformationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SelectTransformation(const ETransformationState SelectedState);
	void SetTransformation(const ETransformationState SelectedState);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TransformationMap")
	TMap<ETransformationState, FTransformationImage> TransformationMap;
	

protected:

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Transformation_Text;

	UPROPERTY(meta = (BindWidget))
	class UImage* Transformation_Image;

	UPROPERTY(meta = (BindWidget))
	class UImage* Arrow_Left;

	UPROPERTY(meta = (BindWidget))
	class UImage* Arrow_Right;

private:
	FTransformationImage* GetTransformationInfo(const ETransformationState SelectedState);
	
};
