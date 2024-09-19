#include "Enemy/BaseEnemy.h"

#include "Components/WidgetComponent.h"
#include <Enemy/BaseAIController.h>
#include "MotionWarpingComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Kismet/GameplayStatics.h"
#include "Components/CombatComponent.h"
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

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("Motion Warping"));
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	EnemyCombatComponent = CreateDefaultSubobject<UEnemyCombatComponent>(TEXT("EnemyCombatComponent"));
	EnemyUIComponent = CreateDefaultSubobject<UEnemyUIComponent>("EnemyUIComponent");
	EnemyAttributeComponent = CreateDefaultSubobject<UEnemyAttributeComponent>("EnemyAttributeComponent");
	AssassinableComponent = CreateDefaultSubobject<UAssassinableComponent>("AssassinableComponent");

	EnemyHealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("EnemyHealthBarWidgetComponent"));
	EnemyHealthBarWidgetComponent->SetupAttachment(GetMesh());

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

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay(); 

	BaseAIController = Cast<ABaseAIController>(GetController());
	Player = UGameplayStatics::GetPlayerCharacter(this, 0);
	AnimInstance = GetMesh()->GetAnimInstance();


	InitEnemyStats();


	if (UPanWarWidgetBase* HealthWidget = Cast<UPanWarWidgetBase>(EnemyHealthBarWidgetComponent->GetUserWidgetObject()))
	{
		HealthWidget->InitEnemyCreatedWidget(this);
		EnemyUIComponent->OnCurrentHealthChanged.Broadcast(EnemyAttributeComponent->GetHealthPercent());
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

void ABaseEnemy::InitEnemyStats()
{
	int32 CurrentGameDifficulty = UPanWarFunctionLibrary::GetCurrentGameDifficulty(this);

	float MaxHealth =EnemyInitStats.MaxHealth.GetValueAtLevel(CurrentGameDifficulty);
	float StoneSpawnChance = EnemyInitStats.StoneSpawnChance.GetValueAtLevel(CurrentGameDifficulty);
	EnemyAttributeComponent->InitializeAttributeStats(MaxHealth, StoneSpawnChance);

	float AttackPower = EnemyInitStats.AttackPower.GetValueAtLevel(CurrentGameDifficulty);
	float DefensePower = EnemyInitStats.DefensePower.GetValueAtLevel(CurrentGameDifficulty);
	float BaseDamage = EnemyInitStats.BaseDamage.GetValueAtLevel(CurrentGameDifficulty);
	EnemyCombatComponent->InitializeCombatStats(BaseDamage, AttackPower, DefensePower);

}

UPawnUIComponent* ABaseEnemy::GetPawnUIComponent() const
{
	return EnemyUIComponent;
}

UEnemyUIComponent* ABaseEnemy::GetEnemyUIComponent() const
{
	return EnemyUIComponent;
}

void ABaseEnemy::SetInvulnerability(bool NewInvulnerability)
{
}

bool ABaseEnemy::IsUnderAttack()
{
	return bIsUnderAttack;
}

void ABaseEnemy::SetUnderAttack()
{
	return ;
}

FRotator ABaseEnemy::GetDesiredDodgeRotation()
{
	return FRotator();
}

void ABaseEnemy::SetEnemyAware(bool NewVisibility)
{
	/*Debug::Print(TEXT("ChangeVisibility"));*/
	bSeen = NewVisibility;
	//PlayerVisibleWidget->SetVisibility(NewVisibility);

	//if (bSeen && !bDied)
	//{
	//	/*FindNearestAI();
	//	GetWorld()->GetTimerManager().SetTimer(FindEnemies_TimerHandle, [this]() {this->FindNearestAI(); }, 3.f, true);*/

	//	EnemyHealthBarWidgetComponent->SetVisibility(true);
	//}
	//else
	//{
	//	/*GetWorld()->GetTimerManager().ClearTimer(FindEnemies_TimerHandle);*/

	//	EnemyHealthBarWidgetComponent->SetVisibility(false);
	//}
		
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

	if (!EnemyCombatComponent) return 0.f;
	EnemyCombatComponent->PerformAttack();

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

	if (!IsAlive() || !Hitter) return;

	/*HitReact_Montages*/
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

void ABaseEnemy::PlayHitReactMontage(const FName& SectionName)
{
	//UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
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

	
	/*Health = FMath::Clamp(Health - DamageAmount, 0.f, 100.f);
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

		
	return DamageAmount;*/

	//Health = FMath::Clamp(Health - DamageAmount, 0.f, 100.f);
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	EnemyAttributeComponent->ReceiveDamage(DamageAmount);
	EnemyUIComponent->OnCurrentHealthChanged.Broadcast(EnemyAttributeComponent->GetHealthPercent());

	if (!EnemyAttributeComponent->IsAlive() && !bDied)
	{
		bDied = true;
		Tags.Add(FName("Dead"));
		OnEnemyDeath.Broadcast();

		if (Death_Montages.Num() != 0)
		{
			const int32 RandIndex = UKismetMathLibrary::RandomIntegerInRange(0, Death_Montages.Num() - 1);
			UAnimMontage* Death_Montage = Death_Montages[RandIndex];

			if (AnimInstance && Death_Montage)
			{
				AnimInstance->Montage_Play(Death_Montage);
			}
		}
		if(Death_Sound)
			UGameplayStatics::PlaySoundAtLocation(this, Death_Sound, GetActorLocation());


		GetWorld()->GetTimerManager().ClearTimer(UnderAttack_TimerHandle);

		/*GetMesh()->SetSimulatePhysics(true);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		FTimerHandle Die_TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(Die_TimerHandle, [this]() {this->Destroy(); }, 5.f, false);*/
	}

	else if (DamageAmount > 0.0f)
	{
		// Notifica manuale dell'evento di danno al sistema di percezione AI
		UAISense_Damage::ReportDamageEvent(this, this, DamageCauser, DamageAmount, GetActorLocation(), GetActorLocation());
	}

	return DamageAmount;
}

void ABaseEnemy::OnDeathEnter()
{
	GetMesh()->bPauseAnims = true;
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EnemyUIComponent->RemoveEnemyDrawnWidgetsIfAny();

	OnEnemyDie_Event();
}

void ABaseEnemy::ApplyHitReactionPhisicsVelocity(FName HitPart)
{
	FVector NewVel;

	if (HitPart == FName("FromFront"))
		NewVel = GetActorForwardVector() * (-4000.f);
	else if (HitPart == FName(" FromBack"))
		NewVel = GetActorForwardVector() * (4000.f);
	else if (HitPart == FName("FromRight"))
		NewVel = GetActorRightVector() * (-4000.f);
	else if (HitPart == FName(" FromLeft"))
		NewVel = GetActorRightVector() * (4000.f);

	GetMesh()->SetPhysicsLinearVelocity(NewVel, false, FName("pelvis"));
}

float ABaseEnemy::GetDefensePower()
{
	return EnemyCombatComponent->GetDefensePower();
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

UPawnCombatComponent* ABaseEnemy::GetCombatComponent() const
{
	return EnemyCombatComponent;
}

void ABaseEnemy::SetAssassinationWidgetVisibility(bool bNewVisibility)
{
	AssassinationWidgetComponent->SetVisibility(bNewVisibility);
}

void ABaseEnemy::SetCollisionBoxAssassination(ECollisionEnabled::Type NewCollision)
{
	AssassinationBoxComponent->SetCollisionEnabled(NewCollision);
}

void ABaseEnemy::AssassinationInitializeDie()
{
	bDied = true;
	Tags.Add(FName("Dead"));
	//SetCollisionBoxAssassination(ECollisionEnabled::NoCollision);
}

void ABaseEnemy::AssassinationKilled()
{
	AssassinableComponent->Killed();
}