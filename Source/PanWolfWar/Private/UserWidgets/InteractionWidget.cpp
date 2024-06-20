// Fill out your copyright notice in the Description page of Project Settings.


#include "UserWidgets/InteractionWidget.h"
#include "Components/TextBlock.h"

void UInteractionWidget::SetKeyInteractionText(const FString String)
{
	if (InteractionKey_Text)
	{
		const FText Text = FText::FromString(String);
		InteractionKey_Text->SetText(Text);
	}

}
