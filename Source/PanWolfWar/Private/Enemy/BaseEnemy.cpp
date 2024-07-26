#include "Enemy/BaseEnemy.h"

#include "Components/WidgetComponent.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	PlayerVisibleWidget = CreateDefaultSubobject<UWidgetComponent>(*FString::Printf(TEXT("PlayerVisibleWidget")));
	if (PlayerVisibleWidget)
	{
		PlayerVisibleWidget->SetWidgetSpace(EWidgetSpace::Screen);
		PlayerVisibleWidget->SetVisibility(false);
		PlayerVisibleWidget->SetupAttachment(GetRootComponent());
	}
}

void ABaseEnemy::SetPlayerVisibilityWidget(bool NewVisibility)
{
	bSeen = NewVisibility;
	PlayerVisibleWidget->SetVisibility(NewVisibility);
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

