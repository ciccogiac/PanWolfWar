#include "UserWidgets/InteractionLoadWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UInteractionLoadWidget::SetKeyInteractionText(const FString String)
{
	if (InteractionKey_Text)
	{
		const FText Text = FText::FromString(String);
		InteractionKey_Text->SetText(Text);
	}

}

void UInteractionLoadWidget::SetInteractionBarPercent(float Percent)
{
	if (InteractionProgressBar)
	{
		InteractionProgressBar->SetPercent(Percent);
	}
}
