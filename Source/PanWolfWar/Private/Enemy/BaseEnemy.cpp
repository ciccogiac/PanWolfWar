#include "Enemy/BaseEnemy.h"

#include "Components/WidgetComponent.h"
#include <Enemy/BaseAIController.h>
#include "MotionWarpingComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include <PanWolfWar/PanWolfWarCharacter.h>

#include "Components/CombatComponent.h"
#include "Components/CapsuleComponent.h"

#include "UserWidgets/BaseEnemyWidget.h"

#include "Perception/AISense_Damage.h"

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

	
	

	EnemyWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyWidgetComponent"));
	if (EnemyWidgetComponent)
	{
		EnemyWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
		EnemyWidgetComponent->SetVisibility(false);
		EnemyWidgetComponent->SetupAttachment(GetRootComponent());
		
	}

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("Motion Warping"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	Tags.Add(FName("Enemy"));
}

void ABaseEnemy::SetPlayerVisibility(bool NewVisibility)
{
	bSeen = NewVisibility;
	//PlayerVisibleWidget->SetVisibility(NewVisibility);

	if (bSeen && !bDied)
	{
		FindNearestAI();
		GetWorld()->GetTimerManager().SetTimer(FindEnemies_TimerHandle, [this]() {this->FindNearestAI(); }, 3.f, true);

		EnemyWidgetComponent->SetVisibility(true);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(FindEnemies_TimerHandle);

		EnemyWidgetComponent->SetVisibility(false);
	}
		
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	BaseAIController = Cast<ABaseAIController>(GetController());
	Player =  UGameplayStatics::GetPlayerCharacter(this, 0 );

	BaseEnemyWidget = Cast<UBaseEnemyWidget>(EnemyWidgetComponent->GetWidget());
}

void ABaseEnemy::Die()
{
	BaseAIController->Die();
	bDied = true;
	GetWorld()->GetTimerManager().ClearTimer(FindEnemies_TimerHandle);
}

float ABaseEnemy::PerformAttack()
{
	/*UAnimInstance* AnimIstance = GetMesh()->GetAnimInstance();
	if (!AnimIstance) return 0.f;
	if (AnimIstance->IsAnyMontagePlaying() || !AttackMontage) return 0.f;*/

	if (MotionWarping && CombatTarget)
	{
		MotionWarping->AddOrUpdateWarpTargetFromComponent(FName("CombatTarget"), CombatTarget->GetRootComponent(), FName(NAME_None), true);
	}

	/*float Duration = AnimIstance->Montage_Play(AttackMontage,1.f,EMontagePlayReturnType::Duration);
	return Duration;*/

	CombatComponent->PerformAttack(EAttackType::EAT_LightAttack);

	return 1.f;
}

void ABaseEnemy::FindNearestAI()
{
	if (bDied || !bSeen) return;

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
			if (!Enemy) continue;
			if (Enemy->bSeen) continue;
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



void ABaseEnemy::ActivateCollision(FString CollisionPart)
{
	CombatComponent->ActivateCollision(CollisionPart);
}

void ABaseEnemy::DeactivateCollision(FString CollisionPart)
{
	CombatComponent->DeactivateCollision(CollisionPart);
}


void ABaseEnemy::GetHit(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && Hitter)
	{
		//DirectionalHitReact(Hitter->GetActorLocation()); 
		FName Section = IHitInterface::DirectionalHitReact(GetOwner(), Hitter->GetActorLocation());
		//Debug::Print(TEXT("Hit From : ") + Section.ToString());
		PlayHitReactMontage(Section);
	}
	//else Die();

	CombatComponent->PlayHitSound(ImpactPoint);
	CombatComponent->SpawnHitParticles(ImpactPoint);
}

void ABaseEnemy::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

bool ABaseEnemy::IsAlive()
{
	//return Attributes && Attributes->IsAlive();
	return !bDied;
}

float ABaseEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	/*if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}*/

	
	Health = FMath::Clamp(Health - DamageAmount, 0.f, 100.f);
	BaseEnemyWidget->SetHealthBarPercent(Health/100.f);
	
	if (Health <= 0)
	{
		Die();
		GetMesh()->SetSimulatePhysics(true);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FTimerHandle Die_TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(Die_TimerHandle, [this]() {this->Destroy(); }, 5.f, false);

		EnemyWidgetComponent->SetVisibility(false);
	}
	else
		BaseAIController->ReportDamageEvent(this, DamageCauser, DamageAmount, GetActorLocation());

		
	return DamageAmount;
}
