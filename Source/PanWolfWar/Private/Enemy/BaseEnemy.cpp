#include "Enemy/BaseEnemy.h"

#include "Components/WidgetComponent.h"
#include <Enemy/BaseAIController.h>
#include "MotionWarpingComponent.h"

#include "PanWolfWar/DebugHelper.h"

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

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("Motion Warping"));
}

void ABaseEnemy::SetPlayerVisibilityWidget(bool NewVisibility)
{
	bSeen = NewVisibility;
	PlayerVisibleWidget->SetVisibility(NewVisibility);
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	BaseAIController = Cast<ABaseAIController>(GetController());
}

void ABaseEnemy::Die()
{
	BaseAIController->Die();
	bDied = true;
}

float ABaseEnemy::PerformAttack()
{
	UAnimInstance* AnimIstance = GetMesh()->GetAnimInstance();
	if (!AnimIstance) return 0.f;
	if (AnimIstance->IsAnyMontagePlaying() || !AttackMontage) return 0.f;

	if (MotionWarping && CombatTarget)
	{
		MotionWarping->AddOrUpdateWarpTargetFromComponent(FName("CombatTarget"), CombatTarget->GetRootComponent(), FName(NAME_None), true);
	}

	float Duration = AnimIstance->Montage_Play(AttackMontage,1.f,EMontagePlayReturnType::Duration);
	
	return Duration;

}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

