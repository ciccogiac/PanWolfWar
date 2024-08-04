#include "Components/CombatComponent.h"

#include "GameFramework/Character.h"

#include "PanWolfWar/DebugHelper.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"
#include "Kismet/GameplayStatics.h"


#pragma region EngineFunctions

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

#pragma endregion

#pragma region PerformAttacks

void UCombatComponent::PerformAttack(EAttackType AttackType)
{
	// Check If Can Perform Attack (ISDead , ISAttacking , IsInOtherState)

	if (bIsAttacking)
	{
		bAttackSaved = true;
		return;
	}

	TArray<UAnimMontage*> AttackMontages = GetAttackMontages(AttackType);
	if (!AttackMontages.IsValidIndex(AttackCount)) return;
	UAnimMontage* Selected_AttackMontage = AttackMontages[AttackCount];
	if (!Selected_AttackMontage) return;

	UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return;

	// Set State to Attacking
	bIsAttacking = true;

	OwningPlayerAnimInstance->Montage_Play(Selected_AttackMontage);


	AttackCount++;
	if (AttackCount >= AttackMontages.Num())
		AttackCount = 0;
}

void UCombatComponent::ContinueAttack()
{
	if (!bAttackSaved)
	{
		ResetAttack();
		return;
	}

	bAttackSaved = false;
	bIsAttacking = false;
	PerformAttack(EAttackType::EAT_LightAttack);
}

void UCombatComponent::ResetAttack()
{
	AttackCount = 0;
	bAttackSaved = false;
	bIsAttacking = false;
}

TArray<UAnimMontage*> UCombatComponent::GetAttackMontages(EAttackType AttackType)
{
	switch (AttackType)
	{
	case EAttackType::EAT_LightAttack:
		return LightAttackMontages;

	case EAttackType::EAT_HeavyAttack:
		return HeavyAttackMontages;

	default:
		break;
	}

	return TArray<UAnimMontage*>();
}

#pragma endregion

#pragma region CollisionTrace

void UCombatComponent::ActivateCollision(FString CollisionPart)
{
	//Debug::Print(TEXT("ActivateTrace: ") + CollisionPart);

	ActiveCollisionPart = CollisionStructs.Find(CollisionPart);
	if (!ActiveCollisionPart) return;

	AlreadyHitActor.Empty();

	GetWorld()->GetTimerManager().SetTimer(Collision_TimerHandle, [this]() {this->TraceLoop(); }, 0.001f, true);
}

void UCombatComponent::DeactivateCollision(FString CollisionPart)
{
	//Debug::Print(TEXT("DeactivateTrace: ") + CollisionPart);

	GetWorld()->GetTimerManager().ClearTimer(Collision_TimerHandle);
	ActiveCollisionPart = nullptr;
}

void UCombatComponent::TraceLoop()
{
	if (!ActiveCollisionPart) return;

	//Debug::Print(TEXT("DoTrace"));

	const FVector Start = ActiveCollisionPart->CollisionMesh->GetSocketLocation(ActiveCollisionPart->StartSocketName);
	const FVector End = ActiveCollisionPart->CollisionMesh->GetSocketLocation(ActiveCollisionPart->EndSocketName);
	const float Radius = ActiveCollisionPart->TraceRadius;
	TArray<FHitResult> HitResults;
	TArray<TEnumAsByte<EObjectTypeQuery> > CombatObjectTypes;
	CombatObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);
	const bool bTraceHit = UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, End, Radius, CombatObjectTypes, false, AlreadyHitActor, EDrawDebugTrace::None, HitResults, true);

	if (!bTraceHit) return;


	for (size_t i = 0; i < HitResults.Num(); i++)
	{
		FHitResult Hit = HitResults[i];
		if (!Hit.bBlockingHit) continue;

		AlreadyHitActor.Add(HitResults[i].GetActor());

		if (ActorIsSameType(Hit.GetActor())) continue;

		UGameplayStatics::ApplyDamage(Hit.GetActor(), ActiveCollisionPart->Damage, CharacterOwner->GetInstigator()->GetController(), CharacterOwner, UDamageType::StaticClass());
		ExecuteGetHit(Hit);

		//Debug::Print(TEXT("Hit: ") + Hit.GetActor()->GetName());
	}


}

bool UCombatComponent::ActorIsSameType(AActor* OtherActor)
{
	return CharacterOwner->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));
}

void UCombatComponent::ExecuteGetHit(FHitResult& Hit)
{
	IHitInterface* HitInterface = Cast<IHitInterface>(Hit.GetActor());
	if (HitInterface)
	{
		HitInterface->GetHit(Hit.ImpactPoint, CharacterOwner);
	}
}

#pragma endregion

#pragma region SetMesh

void UCombatComponent::SetCollisionCharacterPartMesh()
{
	TArray<FString> CharacterPart = TArray<FString>();
	CharacterPart.Add(FString("Hand_R"));
	CharacterPart.Add(FString("Hand_L"));
	CharacterPart.Add(FString("Foot_R"));
	CharacterPart.Add(FString("Foot_L"));

	for (size_t i = 0; i < 4; i++)
	{
		FCollisionPartStruct* CollisionPartStruct = CollisionStructs.Find(CharacterPart[i]);
		if (!CollisionPartStruct) continue;

		CollisionPartStruct->CollisionMesh = CharacterOwner->GetMesh();
	}

}

void UCombatComponent::SetCollisionWeaponPartMesh(UPrimitiveComponent* Mesh)
{
	FCollisionPartStruct* CollisionPartStruct = CollisionStructs.Find(FString("MainWeapon"));
	if (!CollisionPartStruct) return;

	CollisionPartStruct->CollisionMesh = Mesh;
}

#pragma endregion


void UCombatComponent::SetBlockingState(bool EnableBlocking)
{
	if (EnableBlocking == bIsBlocking || !bCombatEnabled) return;
	bIsBlocking = EnableBlocking;
}


void UCombatComponent::PlayHitSound(const FVector& ImpactPoint)
{
	if (HitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);

	}
}

void UCombatComponent::SpawnHitParticles(const FVector& ImpactPoint)
{
	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, ImpactPoint);
	}
}





