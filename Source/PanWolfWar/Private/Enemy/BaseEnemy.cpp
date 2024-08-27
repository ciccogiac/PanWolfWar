#include "Enemy/BaseEnemy.h"

#include "Components/WidgetComponent.h"
#include <Enemy/BaseAIController.h>
#include "MotionWarpingComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include <PanWolfWar/PanWolfWarCharacter.h>

#include "Components/CombatComponent.h"
#include "Components/CapsuleComponent.h"

#include "UserWidgets/BaseEnemyWidget.h"

#include "Perception/AISense_Damage.h"

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;

	EnemyTargetWidget = CreateDefaultSubobject<UWidgetComponent>(*FString::Printf(TEXT("PlayerVisibleWidget")));
	if (EnemyTargetWidget)
	{
		EnemyTargetWidget->SetWidgetSpace(EWidgetSpace::Screen);
		EnemyTargetWidget->SetVisibility(false);
		EnemyTargetWidget->SetupAttachment(GetRootComponent());
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

void ABaseEnemy::SetTargetVisibility(bool NewVisibility)
{
	if (NewVisibility && !CanBeTargeted())
		return;
	EnemyTargetWidget->SetVisibility(NewVisibility);
}

bool ABaseEnemy::CanBeTargeted()
{
	return !bDied;
}

void ABaseEnemy::SetInvulnerability(bool NewInvulnerability)
{
}

bool ABaseEnemy::IsUnderAttack()
{
	return false;
}

void ABaseEnemy::SetUnderAttack()
{
	return ;
}

FRotator ABaseEnemy::GetDesiredDodgeRotation()
{
	return FRotator();
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

void ABaseEnemy::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(FindEnemies_TimerHandle);
}

bool ABaseEnemy::IsCombatActorAlive()
{
	return bDied;
}

float ABaseEnemy::PerformAttack()
{
	/*UAnimInstance* AnimIstance = GetMesh()->GetAnimInstance();
	if (!AnimIstance) return 0.f;
	if (AnimIstance->IsAnyMontagePlaying() || !AttackMontage) return 0.f;*/

	if (MotionWarping && CombatTarget)
	{
		CombatComponent->RotateToClosestEnemy(CombatTarget);

		MotionWarping->AddOrUpdateWarpTargetFromComponent(FName("CombatTarget"), CombatTarget->GetRootComponent(), FName(NAME_None), true);
	}

	/*float Duration = AnimIstance->Montage_Play(AttackMontage,1.f,EMontagePlayReturnType::Duration);
	return Duration;*/

	CombatComponent->PerformAttack(EAttackType::EAT_LightAttack_Right);

	return 1.f;
}

void ABaseEnemy::FindNearestAI()
{
	if (bDied || !bSeen) return;

	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector();
	TArray<FHitResult> Hit ;
	TArray<TEnumAsByte<EObjectTypeQuery>> Objects;
	Objects.Add(EObjectTypeQuery::ObjectTypeQuery3);
	
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Player);

	bool bBlocked = UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, End,1500.f, Objects, false, ActorsToIgnore, EDrawDebugTrace::None, Hit, true);

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
	//if (IsAlive() && Hitter)
	//{
	//	//DirectionalHitReact(Hitter->GetActorLocation()); 
	//	FName Section = IHitInterface::DirectionalHitReact(GetOwner(), Hitter->GetActorLocation());
	//	//Debug::Print(TEXT("Hit From : ") + Section.ToString());
	//	PlayHitReactMontage(Section);
	//	
	//}
	//else
	//{
	//	FName Section = IHitInterface::DirectionalHitReact(GetOwner(), Hitter->GetActorLocation());
	//	GetMesh()->SetSimulatePhysics(true);
	//	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//	ApplyHitReactionPhisicsVelocity(Section);
	//}
	//	
	//CombatComponent->PlayHitSound(ImpactPoint);
	//CombatComponent->SpawnHitParticles(ImpactPoint);


	const FRotator NewFaceRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Hitter->GetActorLocation());
	SetActorRotation(NewFaceRotation);
	
	/*HitReact_Montages*/
	if (HitReact_Montages.Num() == 0) return;
	const int32 RandIndex = UKismetMathLibrary::RandomIntegerInRange(0, HitReact_Montages.Num()-1);
	UAnimMontage* HitReact_Montage = HitReact_Montages[RandIndex];

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReact_Montage)
	{
		AnimInstance->Montage_Play(HitReact_Montage);
		GetMesh()->SetScalarParameterValueOnMaterials(FName("HitFxSwitch"), 1.f);

		GetWorld()->GetTimerManager().ClearTimer(HitPause_TimerHandle);
		UGameplayStatics::SetGlobalTimeDilation(this, 0.1f);
		GetWorld()->GetTimerManager().SetTimer(HitPause_TimerHandle, [this, Hitter]() {this->HitPause(Hitter); }, 0.025f, false);
	}

}

void ABaseEnemy::HitPause(AActor* Hitter)
{
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
	
	Cast<APlayerController>(Cast<ACharacter>(Hitter)->GetController())->ClientStartCameraShake(CameraShake_Wolf);
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

		
		GetMesh()->SetSimulatePhysics(true);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Die();
		FTimerHandle Die_TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(Die_TimerHandle, [this]() {this->Destroy(); }, 5.f, false);

		EnemyWidgetComponent->SetVisibility(false);
	}
	else
		BaseAIController->ReportDamageEvent(this, DamageCauser, DamageAmount, GetActorLocation());

		
	return DamageAmount;
}

void ABaseEnemy::ApplyHitReactionPhisicsVelocity(FName HitPart)
{
	FVector NewVel;

	if (HitPart == FName("FromFront"))
		NewVel = GetActorForwardVector() * (-4000.f);
	else if (HitPart == FName(" FromBack"))
		NewVel = GetActorForwardVector() * (4000.f);
	else if (HitPart == FName("FromRight") )
		NewVel = GetActorRightVector() * (-4000.f);
	else if (HitPart == FName(" FromLeft"))
		NewVel = GetActorRightVector() * (4000.f);

	GetMesh()->SetPhysicsLinearVelocity(NewVel, false, FName("pelvis"));
}