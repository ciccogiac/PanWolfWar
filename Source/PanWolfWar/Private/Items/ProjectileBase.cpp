#include "Items/ProjectileBase.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PanWarFunctionLibrary.h"
#include "Interfaces/CombatInterface.h"
#include "Interfaces/HitInterface.h"
#include "Kismet/GameplayStatics.h"

#include "PanWolfWar/DebugHelper.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectileCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ProjectileCollisionBox"));
	SetRootComponent(ProjectileCollisionBox);
	ProjectileCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	ProjectileCollisionBox->OnComponentHit.AddUniqueDynamic(this, &ThisClass::OnProjectileHit);
	ProjectileCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnProjectileBeginOverlap);

	ProjectileNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraComponent"));
	ProjectileNiagaraComponent->SetupAttachment(GetRootComponent());

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->InitialSpeed = 700.f;
	ProjectileMovementComp->MaxSpeed = 900.f;
	ProjectileMovementComp->Velocity = FVector(1.f, 0.f, 0.f);
	ProjectileMovementComp->ProjectileGravityScale = 0.f;

	InitialLifeSpan = 4.f;

}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (ProjectileDamagePolicy == EProjectileDamagePolicy::OnBeginOverlap)
	{
		ProjectileCollisionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
	
}

void AProjectileBase::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	BP_OnSpawnProjectileHitFX(Hit.ImpactPoint);

	APawn* HitPawn = Cast<APawn>(OtherActor);

	if (!HitPawn || !UPanWarFunctionLibrary::IsTargetPawnHostile(GetInstigator(), HitPawn))
	{
		Destroy();
		return;
	}

	bool bIsValidBlock = false;

	bool bIsPlayerBlocking = false;

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(HitPawn);
	if (CombatInterface)
	{
		bIsPlayerBlocking = CombatInterface->IsBlocking();
	}


	if (bIsPlayerBlocking)
	{
		bIsValidBlock = UPanWarFunctionLibrary::IsValidBlock(this, HitPawn);
	}

	if (bIsValidBlock && CombatInterface)
	{
		/*CombatInterface->SuccesfulBlock(GetInstigator());*/
		CombatInterface->SuccesfulBlock(this);
	}
	else
	{
		HandleApplyProjectileDamage(HitPawn);
	}

	Destroy();
}

void AProjectileBase::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}

void AProjectileBase::HandleApplyProjectileDamage(APawn* InHitPawn)
{
	const float TargetDefensePower = Cast<ICombatInterface>(InHitPawn)->GetDefensePower();

	const float FinalDamage = BaseDamage / TargetDefensePower ;
	UGameplayStatics::ApplyDamage(InHitPawn, FinalDamage, GetInstigator()->GetController(), GetInstigator(), UDamageType::StaticClass());

	IHitInterface* HitInterface = Cast<IHitInterface>(InHitPawn);
	if (HitInterface)
	{
		/*HitInterface->GetHit(GetActorLocation(), GetInstigator());*/
		HitInterface->GetHit(GetActorLocation(), this);
	}
}