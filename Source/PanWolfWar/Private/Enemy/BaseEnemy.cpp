#include "Enemy/BaseEnemy.h"

#include "Components/WidgetComponent.h"
#include <Enemy/BaseAIController.h>
#include "MotionWarpingComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include <PanWolfWar/PanWolfWarCharacter.h>

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

void ABaseEnemy::SetPlayerVisibility(bool NewVisibility)
{
	bSeen = NewVisibility;
	PlayerVisibleWidget->SetVisibility(NewVisibility);

	if (bSeen)
	{
		FindNearestAI();
		GetWorld()->GetTimerManager().SetTimer(FindEnemies_TimerHandle, [this]() {this->FindNearestAI(); }, 3.f, true);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(FindEnemies_TimerHandle);
	}
		
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	BaseAIController = Cast<ABaseAIController>(GetController());
	Player =  UGameplayStatics::GetPlayerCharacter(this, 0 );
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

void ABaseEnemy::FindNearestAI()
{
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector();
	TArray<FHitResult> Hit;
	TArray<TEnumAsByte<EObjectTypeQuery>> Objects;
	Objects.Add(EObjectTypeQuery::ObjectTypeQuery3);
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Player);

	bool bBlocked = UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, End,1500.f, Objects, false, ActorsToIgnore, EDrawDebugTrace::ForDuration, Hit, true);

	if (!bBlocked || Hit.Num() <= 0) return;

	for (size_t i = 0; i < Hit.Num() ; i++)
	{
		FHitResult EnemyHit = Hit[i];
		if (EnemyHit.bBlockingHit)
		{
			//Debug::Print(TEXT("Enemy: ") + EnemyHit.GetActor()->GetName());
			ABaseEnemy* Enemy = Cast<ABaseEnemy>(EnemyHit.GetActor());
			if (!Enemy || Enemy->bSeen) continue;
			Enemy->BaseAIController->FindNewEnemy(Player);
		}
			
	}


}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

