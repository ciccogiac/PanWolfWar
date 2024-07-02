// Fill out your copyright notice in the Description page of Project Settings.


#include "UserWidgets/TransformationWidget.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"


void UTransformationWidget::SetTransformation(const ETransformationState SelectedState)
{

	FTransformationImage* TransformationImage = GetTransformationInfo(SelectedState);
	if (!TransformationImage) return;

	if (Transformation_Text)
	{
		const FText Text = FText::FromString(TransformationImage->Transformation_Name);
		Transformation_Text->SetText(Text);
	}

	if (Transformation_Image)
	{
		Transformation_Image->SetBrushFromTexture(TransformationImage->Transformation_Texture);
	}
}

FTransformationImage* UTransformationWidget::GetTransformationInfo(const ETransformationState SelectedState)
{
	return TransformationMap.Find(SelectedState);
}
