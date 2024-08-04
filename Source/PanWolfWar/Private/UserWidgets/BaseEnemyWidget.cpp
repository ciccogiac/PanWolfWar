#include "UserWidgets/BaseEnemyWidget.h"

#include "Components/ProgressBar.h"

void UBaseEnemyWidget::SetHealthBarPercent(float Percent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(Percent);
	}
}
