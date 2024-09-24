#include "Components/Combat/PawnCombatComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/CombatInterface.h"

#include "PanWarFunctionLibrary.h"

#include "PanWolfWar/DebugHelper.h"

#include "Items/Weapons/PanWarWeaponBase.h"
#include "Components/BoxComponent.h"

#pragma region EngineFunctions

UPawnCombatComponent::UPawnCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CharacterOwner = Cast<ACharacter>(GetOwner());
}

void UPawnCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPawnCombatComponent::InitializeCombatStats(float _BaseDamage, float _AttackPower, float _DefensePower)
{
	BaseAttackDamage = _BaseDamage;
	AttackPower = _AttackPower;
	DefensePower = _DefensePower;
}

#pragma endregion

#pragma region CollisionTrace

void UPawnCombatComponent::ToggleWeaponCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType)
{
	if (ToggleDamageType == EToggleDamageType::CurrentEquippedWeapon)
	{
		ToggleCurrentEquippedWeaponCollision(bShouldEnable);
	}

	else
	{
		ToggleBodyCollision(bShouldEnable, ToggleDamageType);
	}
}

void UPawnCombatComponent::BoxCollisionTrace(EToggleDamageType ToggleDamageType)
{
	UBoxComponent* CollisionBox = nullptr;

	switch (ToggleDamageType)
	{
	case EToggleDamageType::CurrentEquippedWeapon:
		CollisionBox = CurrentEquippedWeapon->GetWeaponCollisionBox();
		break;
	case EToggleDamageType::LeftHand:
		CollisionBox = LeftHandCollisionBox;
		break;
	case EToggleDamageType::RightHand:
		CollisionBox = RightHandCollisionBox;
		break;
	default:
		break;
	}

	check(CollisionBox);
	const FVector BoxExtent = CollisionBox->GetScaledBoxExtent();
	const FVector BoxLocation = CollisionBox->GetComponentLocation();
	const FRotator BoxRotation = CollisionBox->GetComponentRotation();

	// Esegui il Box Trace
	TArray<FHitResult> HitResults;
	TArray<TEnumAsByte<EObjectTypeQuery> > CombatObjectTypes;
	CombatObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	const bool bTraceHit = UKismetSystemLibrary::BoxTraceMultiForObjects(this,BoxLocation,BoxLocation + FVector(0.f,0.f,0.1f), BoxExtent, BoxRotation, CombatObjectTypes, false, AlreadyHitActor, DebugTraceType, HitResults, true);
	if (!bTraceHit) return;


	for (size_t i = 0; i < HitResults.Num(); i++)
	{
		FHitResult Hit = HitResults[i];
		if (!Hit.bBlockingHit) continue;

		AlreadyHitActor.Add(HitResults[i].GetActor());

		if (!UPanWarFunctionLibrary::IsTargetPawnHostile(Cast<APawn>(CharacterOwner), Cast<APawn>(Hit.GetActor()))) continue;

		//BlockingCheck
		bool bIsValidBlock = false;
		bool bIsPlayerBlocking = false;

		ICombatInterface* CombatInterface = Cast<ICombatInterface>(Hit.GetActor());
		if (CombatInterface)
			bIsPlayerBlocking = CombatInterface->IsBlocking();

		const bool bIsMyAttackUnblockable = CachedUnblockableAttack;

		if (bIsPlayerBlocking && !bIsMyAttackUnblockable)
		{
			/*bIsValidBlock = UPanWarFunctionLibrary::IsValidBlock(CharacterOwner, Hit.GetActor());*/
			bIsValidBlock = CombatInterface->IsValidBlock(CharacterOwner, Hit.GetActor());
		}

		if (bIsValidBlock && CombatInterface)
		{
			CombatInterface->SuccesfulBlock(CharacterOwner);
		}

		else
		{
			/*ApplyDamageToActorHit(Hit.GetActor(), ActiveCollisionPart->Damage, CharacterOwner->GetInstigator()->GetController(), CharacterOwner, UDamageType::StaticClass());*/
			ApplyDamageToActorHit(Hit.GetActor(), BaseAttackDamage, CharacterOwner->GetInstigator()->GetController(), CharacterOwner, UDamageType::StaticClass());
			ExecuteHitActor(Hit);
		}



	}
}

#pragma endregion

#pragma region HitHandler

void UPawnCombatComponent::ApplyDamageToActorHit(AActor* DamagedActor, float BaseDamage, AController* EventInstigator, AActor* DamageCauser, TSubclassOf<UDamageType> DamageTypeClass)
{
	/*UGameplayStatics::ApplyDamage(Hit.GetActor(), ActiveCollisionPart->Damage, CharacterOwner->GetInstigator()->GetController(), CharacterOwner, UDamageType::StaticClass());*/
	const float TargetDefensePower = Cast<ICombatInterface>(DamagedActor)->GetDefensePower();

	const float FinalDamage = CalculateFinalDamage(BaseDamage, TargetDefensePower);
	UGameplayStatics::ApplyDamage(DamagedActor, FinalDamage, EventInstigator, DamageCauser, DamageTypeClass);
}

float UPawnCombatComponent::CalculateFinalDamage(float BaseDamage, float TargetDefensePower)
{
	return (BaseDamage * AttackPower / TargetDefensePower);

}

bool UPawnCombatComponent::ExecuteHitActor(FHitResult& Hit)
{
	IHitInterface* HitInterface = Cast<IHitInterface>(Hit.GetActor());
	if (HitInterface)
	{
		HitInterface->GetHit(Hit.ImpactPoint, CharacterOwner);
		return true;
	}

	return false;
}

void UPawnCombatComponent::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);

	}
}

void UPawnCombatComponent::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
}

#pragma endregion

#pragma region Weapon

void UPawnCombatComponent::ToggleCurrentEquippedWeaponCollision(bool bShouldEnable)
{
	if (!CurrentEquippedWeapon) return;

	if (bShouldEnable)
	{
		AlreadyHitActor.Empty();
		GetWorld()->GetTimerManager().SetTimer(WeaponCollision_TimerHandle, [this]() {this->BoxCollisionTrace(EToggleDamageType::CurrentEquippedWeapon); }, 0.001f, true);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(WeaponCollision_TimerHandle);
	}
}

void UPawnCombatComponent::EquipWeapon(APanWarWeaponBase* InWeaponToRegister)
{
	check(InWeaponToRegister);
	check(CurrentEquippedWeapon != InWeaponToRegister);

	CurrentEquippedWeapon = InWeaponToRegister;

}

APanWarWeaponBase* UPawnCombatComponent::GetCurrentEquippedWeapon() const
{
	return CurrentEquippedWeapon;
}

#pragma endregion

#pragma region HandToHand

void UPawnCombatComponent::EnableHandToHandCombat(UBoxComponent* _LeftHandCollisionBox, UBoxComponent* _RightHandCollisionBox)
{
	LeftHandCollisionBox = _LeftHandCollisionBox;
	RightHandCollisionBox = _RightHandCollisionBox;
}

void UPawnCombatComponent::ToggleBodyCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType)
{
	AlreadyHitActor.Empty();

	switch (ToggleDamageType)
	{
	case EToggleDamageType::LeftHand:
		bShouldEnable ?
			GetWorld()->GetTimerManager().SetTimer(LeftHandCollision_TimerHandle, [this]() {this->BoxCollisionTrace(EToggleDamageType::LeftHand); }, 0.001f, true)
			:
			GetWorld()->GetTimerManager().ClearTimer(LeftHandCollision_TimerHandle);
		break;

	case EToggleDamageType::RightHand:
		bShouldEnable ?
			GetWorld()->GetTimerManager().SetTimer(RightHandCollision_TimerHandle, [this]() {this->BoxCollisionTrace(EToggleDamageType::RightHand); }, 0.001f, true)
			:
			GetWorld()->GetTimerManager().ClearTimer(RightHandCollision_TimerHandle);

		break;

	default:
		break;
	}

}

#pragma endregion



void UPawnCombatComponent::ResetAttack()
{

}



