#include "Enemy/BaseEnemy.h"
#include "Components/WidgetComponent.h"
#include "MotionWarpingComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Kismet/GameplayStatics.h"
#include "Components/Combat/EnemyCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "UserWidgets/BaseEnemyWidget.h"
#include "Components/UI/EnemyUIComponent.h"
#include "Widgets/PanWarWidgetBase.h"
#include "Components/EnemyAttributeComponent.h"
#include "Perception/AISense_Damage.h"
#include "Components/BoxComponent.h"
#include "PanWarFunctionLibrary.h"
#include "Enemy/AssassinableComponent.h"

#include "PanWolfWar/DebugHelper.h"

#pragma region EngineFunctions

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("Motion Warping"));
	EnemyCombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("EnemyCombatComponent"));
	EnemyUIComponent = CreateDefaultSubobject<UEnemyUIComponent>("EnemyUIComponent");
	EnemyAttributeComponent = CreateDefaultSubobject<UEnemyAttributeComponent>("EnemyAttributeComponent");
	AssassinableComponent = CreateDefaultSubobject<UAssassinableComponent>("AssassinableComponent");

	EnemyHealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyHealthBarWidgetComponent"));
	EnemyHealthBarWidgetComponent->SetupAttachment(GetMesh());

	EnemyAwarenessBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyAwarenessBarWidgetComponent"));
	EnemyAwarenessBarWidgetComponent->SetupAttachment(GetMesh()); 

	LeftHandCollisionBox = CreateDefaultSubobject<UBoxComponent>("LeftHandCollisionBox");
	LeftHandCollisionBox->SetupAttachment(GetMesh());
	LeftHandCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RightHandCollisionBox = CreateDefaultSubobject<UBoxComponent>("RightHandCollisionBox");
	RightHandCollisionBox->SetupAttachment(GetMesh());
	RightHandCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LeftHandCollisionBox->SetActive(false);
	RightHandCollisionBox->SetActive(false);

	EnableHandToHandCombat();

	AssassinationBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("AssassinationBoxComponent"));
	AssassinationBoxComponent->SetupAttachment(GetMesh());
	AssassinationBoxComponent->bHiddenInGame = true;
	AssassinationBoxComponent->SetLineThickness(2.f);
	AssassinationBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AssassinationWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(*FString::Printf(TEXT("AssassinationWidgetComponent")));
	AssassinationWidgetComponent->SetupAttachment(GetRootComponent());
	AssassinationWidgetComponent->SetVisibility(false);

	Assassination_Preview_Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Assassination_Preview_Mesh"));
	Assassination_Preview_Mesh->SetupAttachment(GetMesh());
	Assassination_Preview_Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Assassination_Preview_Mesh->bHiddenInGame = true;

	EnableAssassination();

	Tags.Add(FName("Enemy"));
}

void ABaseEnemy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, bEnableHandToHandCombat))
	{
		EnableHandToHandCombat();
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, LeftHandCollisionBoxAttachBoneName))
	{
		LeftHandCollisionBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, LeftHandCollisionBoxAttachBoneName);
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, RightHandCollisionBoxAttachBoneName))
	{
		RightHandCollisionBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, RightHandCollisionBoxAttachBoneName);
	}

	if (PropertyChangedEvent.GetMemberPropertyName() == GET_MEMBER_NAME_CHECKED(ThisClass, bEnableAssassination))
	{
		EnableAssassination();
	}
	
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay(); 

	Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	AnimInstance = GetMesh()->GetAnimInstance();


	InitEnemyStats();


	if (UPanWarWidgetBase* HealthWidget = Cast<UPanWarWidgetBase>(EnemyHealthBarWidgetComponent->GetUserWidgetObject()))
	{
		HealthWidget->InitEnemyCreatedWidget(this);
		EnemyUIComponent->OnCurrentHealthChanged.Broadcast(EnemyAttributeComponent->GetHealthPercent());
	}

	if (UPanWarWidgetBase* AwarenessWidget = Cast<UPanWarWidgetBase>(EnemyAwarenessBarWidgetComponent->GetUserWidgetObject()))
	{
		AwarenessWidget->InitEnemyCreatedWidget(this);
		EnemyUIComponent->OnCurrentAwarenessChanged.Broadcast(0.f);
	}

	if (bEnableHandToHandCombat && EnemyCombatComponent)
	{
		EnemyCombatComponent->EnableHandToHandCombat(LeftHandCollisionBox, RightHandCollisionBox);
	}

	EnemyState = EEnemyState::EES_Default;

	if (bEnableAssassination && AssassinableComponent)
	{
		AssassinationBoxComponent->OnComponentBeginOverlap.AddDynamic(AssassinableComponent, &UAssassinableComponent::BoxCollisionEnter);
		AssassinationBoxComponent->OnComponentEndOverlap.AddDynamic(AssassinableComponent, &UAssassinableComponent::BoxCollisionExit);
	}

}

#pragma endregion

#pragma region initialization

void ABaseEnemy::InitEnemyStats()
{
	int32 CurrentGameDifficulty = UPanWarFunctionLibrary::GetCurrentGameDifficulty(this);

	float MaxHealth = EnemyInitStats.MaxHealth.GetValueAtLevel(CurrentGameDifficulty);
	float StoneSpawnChance = EnemyInitStats.StoneSpawnChance.GetValueAtLevel(CurrentGameDifficulty);
	EnemyAttributeComponent->InitializeAttributeStats(MaxHealth, StoneSpawnChance);

	float AttackPower = EnemyInitStats.AttackPower.GetValueAtLevel(CurrentGameDifficulty);
	float DefensePower = EnemyInitStats.DefensePower.GetValueAtLevel(CurrentGameDifficulty);
	float BaseDamage = EnemyInitStats.BaseDamage.GetValueAtLevel(CurrentGameDifficulty);
	EnemyCombatComponent->InitializeCombatStats(BaseDamage, AttackPower, DefensePower);

}

void ABaseEnemy::EnableAssassination()
{
	if (bEnableAssassination)
	{
		AssassinationBoxComponent->SetVisibility(true);
		AssassinationWidgetComponent->SetVisibility(true);
		Assassination_Preview_Mesh->SetVisibility(true);
		SetCollisionBoxAssassination(ECollisionEnabled::QueryOnly);
		Tags.Add(FName("Assassinable"));
	}
	else
	{
		AssassinationBoxComponent->SetVisibility(false);
		AssassinationWidgetComponent->SetVisibility(false);
		Assassination_Preview_Mesh->SetVisibility(false);
		SetCollisionBoxAssassination(ECollisionEnabled::NoCollision);
		Tags.Remove(FName("Assassinable"));
	}
}

void ABaseEnemy::EnableHandToHandCombat()
{
	if (bEnableHandToHandCombat)
	{
		LeftHandCollisionBox->SetVisibility(true);
		RightHandCollisionBox->SetVisibility(true);

	}
	else
	{
		LeftHandCollisionBox->SetVisibility(false);
		RightHandCollisionBox->SetVisibility(false);
	}
}


#pragma endregion

#pragma region CombatInterface

UPawnCombatComponent* ABaseEnemy::GetCombatComponent() const
{
	return EnemyCombatComponent;
}

void ABaseEnemy::SetInvulnerability(bool NewInvulnerability)
{
}

FRotator ABaseEnemy::GetDesiredDodgeRotation()
{
	return FRotator();
}

bool ABaseEnemy::IsCombatActorAlive()
{
	return IsEnemyAlive();
}

float ABaseEnemy::PerformAttack()
{

	if (!EnemyCombatComponent) return 0.f;
	EnemyCombatComponent->PerformAttack();

	return 1.f;
}

bool ABaseEnemy::IsUnderAttack()
{
	return bIsUnderAttack;
}

void ABaseEnemy::SetUnderAttack()
{
	return;
}

float ABaseEnemy::GetDefensePower()
{
	return EnemyCombatComponent->GetDefensePower();
}

void ABaseEnemy::OnDeathEnter()
{
	GetMesh()->bPauseAnims = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EnemyUIComponent->RemoveEnemyDrawnWidgetsIfAny();

	OnEnemyDie_Event();
}

bool ABaseEnemy::IsBlocking()
{
	return false;
}

void ABaseEnemy::SuccesfulBlock(AActor* Attacker)
{
}

void ABaseEnemy::FireProjectile()
{
	BP_FireProjectile();
}

float ABaseEnemy::GetHealthPercent()
{
	if (EnemyAttributeComponent)
		return EnemyAttributeComponent->GetHealthPercent();

	return -1.f;
}

void ABaseEnemy::AssassinationKilled()
{
	AssassinableComponent->Killed();
}

#pragma endregion

#pragma region HitDamage&Death

//HitInterface
void ABaseEnemy::GetHit(const FVector& ImpactPoint, AActor* Hitter)
{
	if (!IsEnemyAlive() || !Hitter) return;

	if (HitReact_Montages.Num() != 0)
	{
		const FRotator NewFaceRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Hitter->GetActorLocation());
		SetActorRotation(NewFaceRotation);

		const int32 RandIndex = UKismetMathLibrary::RandomIntegerInRange(0, HitReact_Montages.Num() - 1);
		UAnimMontage* HitReact_Montage = HitReact_Montages[RandIndex];

		if (AnimInstance && HitReact_Montage)
		{
			AnimInstance->Montage_Play(HitReact_Montage);

		}
	}

	GetMesh()->SetScalarParameterValueOnMaterials(FName("HitFxSwitch"), 1.f);

	bIsUnderAttack = true;
	GetWorld()->GetTimerManager().SetTimer(UnderAttack_TimerHandle, [this]() {this->bIsUnderAttack = false; }, UnderAttack_Time, false);
	GetWorld()->GetTimerManager().SetTimer(GetHitFX_TimerHandle, [this]() {this->GetMesh()->SetScalarParameterValueOnMaterials(FName("HitFxSwitch"), 0.f); }, GetHitFX_Time, false);

	//EnemyCombatComponent->PlayHitSound(ImpactPoint);
	/*EnemyCombatComponent->SpawnHitParticles(ImpactPoint);*/

}

bool ABaseEnemy::IsEnemyAlive()
{
	return EnemyAttributeComponent->IsAlive() && !bDied && !ActorHasTag(FName("Dead"));
}

float ABaseEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	EnemyAttributeComponent->ReceiveDamage(DamageAmount);
	EnemyUIComponent->OnCurrentHealthChanged.Broadcast(EnemyAttributeComponent->GetHealthPercent());

	if (!EnemyAttributeComponent->IsAlive() && !bDied)
	{
		HandleEnemyDeath();

	}

	else if (DamageAmount > 0.0f && IsEnemyAlive())
	{
		// Notifica manuale dell'evento di danno al sistema di percezione AI
		UAISense_Damage::ReportDamageEvent(this, this, DamageCauser, DamageAmount, GetActorLocation(), GetActorLocation());
	}

	return DamageAmount;
}

void ABaseEnemy::InitializeEnemyDeath()
{
	bDied = true;
	Tags.Add(FName("Dead"));
	OnEnemyDeath.Broadcast();
}

void ABaseEnemy::HandleEnemyDeath()
{
	InitializeEnemyDeath();

	if (Death_Montages.Num() != 0)
	{
		const int32 RandIndex = UKismetMathLibrary::RandomIntegerInRange(0, Death_Montages.Num() - 1);
		UAnimMontage* Death_Montage = Death_Montages[RandIndex];

		if (AnimInstance && Death_Montage)
		{
			AnimInstance->Montage_Play(Death_Montage);
		}
	}
	if (Death_Sound)
		UGameplayStatics::PlaySoundAtLocation(this, Death_Sound, GetActorLocation());


	GetWorld()->GetTimerManager().ClearTimer(UnderAttack_TimerHandle);
}

#pragma endregion

#pragma region Assassination

void ABaseEnemy::SetAssassinationWidgetVisibility(bool bNewVisibility)
{
	AssassinationWidgetComponent->SetVisibility(bNewVisibility);
}

void ABaseEnemy::SetCollisionBoxAssassination(ECollisionEnabled::Type NewCollision)
{
	AssassinationBoxComponent->SetCollisionEnabled(NewCollision);
}


#pragma endregion

void ABaseEnemy::SetEnemyAware(bool NewVisibility)
{
	/*Debug::Print(TEXT("ChangeVisibility"));*/
	bSeen = NewVisibility;
	//PlayerVisibleWidget->SetVisibility(NewVisibility);

	//if (bSeen && !bDied)
	//{

	//	EnemyHealthBarWidgetComponent->SetVisibility(true);
	//}
	//else
	//{
	//	EnemyHealthBarWidgetComponent->SetVisibility(false);
	//}
		
}

void ABaseEnemy::UpdateCurrentEnemyAwareness(float Percent)
{
	EnemyUIComponent->OnCurrentAwarenessChanged.Broadcast(Percent);
}

//~ Begin IPawnUIInterface Interface.
UPawnUIComponent* ABaseEnemy::GetPawnUIComponent() const
{
	return EnemyUIComponent;
}
//~ End IPawnUIInterface Interface