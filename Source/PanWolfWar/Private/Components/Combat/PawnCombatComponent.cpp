#include "Components/Combat/PawnCombatComponent.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/CombatInterface.h"

#include "GenericTeamAgentInterface.h"

#include "PanWolfWar/DebugHelper.h"

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


#pragma endregion

#pragma region CollisionTrace

void UPawnCombatComponent::ActivateCollision(FString CollisionPart)
{
	ActiveCollisionPart = CollisionStructs.Find(CollisionPart);
	if (!ActiveCollisionPart) return;

	AlreadyHitActor.Empty();

	GetWorld()->GetTimerManager().SetTimer(Collision_TimerHandle, [this]() {this->TraceLoop(); }, 0.001f, true);
}

void UPawnCombatComponent::DeactivateCollision(FString CollisionPart)
{
	GetWorld()->GetTimerManager().ClearTimer(Collision_TimerHandle);
	ActiveCollisionPart = nullptr;
}

void UPawnCombatComponent::TraceLoop()
{
	if (!ActiveCollisionPart) return;

	//Debug::Print(TEXT("DoTrace"));

	const FVector Start = ActiveCollisionPart->CollisionMesh->GetSocketLocation(ActiveCollisionPart->StartSocketName);
	const FVector End = ActiveCollisionPart->CollisionMesh->GetSocketLocation(ActiveCollisionPart->EndSocketName);
	const float Radius = ActiveCollisionPart->TraceRadius;
	TArray<FHitResult> HitResults;
	TArray<TEnumAsByte<EObjectTypeQuery> > CombatObjectTypes;
	CombatObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);
	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	const bool bTraceHit = UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, End, Radius, CombatObjectTypes, false, AlreadyHitActor, DebugTraceType, HitResults, true);

	if (!bTraceHit) return;


	for (size_t i = 0; i < HitResults.Num(); i++)
	{
		FHitResult Hit = HitResults[i];
		if (!Hit.bBlockingHit) continue;

		AlreadyHitActor.Add(HitResults[i].GetActor());

		//if (ActorIsSameType(Hit.GetActor())) continue;
		if (!IsTargetPawnHostile(Cast<APawn>(CharacterOwner), Cast<APawn>(Hit.GetActor()))) continue;

		ApplyDamageToActorHit(Hit.GetActor(), ActiveCollisionPart->Damage, CharacterOwner->GetInstigator()->GetController(), CharacterOwner, UDamageType::StaticClass());
		ExecuteHitActor(Hit);

	}


}

bool UPawnCombatComponent::IsTargetPawnHostile(APawn* QueryPawn, APawn* TargetPawn)
{
	check(QueryPawn && TargetPawn);

	IGenericTeamAgentInterface* QueryTeamAgent = Cast<IGenericTeamAgentInterface>(QueryPawn->GetController());
	IGenericTeamAgentInterface* TargetTeamAgent = Cast<IGenericTeamAgentInterface>(TargetPawn->GetController());

	if (QueryTeamAgent && TargetTeamAgent)
	{
		return QueryTeamAgent->GetGenericTeamId() != TargetTeamAgent->GetGenericTeamId();
	}

	return false;
}

bool UPawnCombatComponent::ActorIsSameType(AActor* OtherActor)
{
	return (
		(CharacterOwner->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"))) ||
		(CharacterOwner->ActorHasTag(TEXT("Player")) && OtherActor->ActorHasTag(TEXT("Player")))
		);
	/*return CharacterOwner->ActorHasTag(TEXT("Enemy")) && OtherActor->ActorHasTag(TEXT("Enemy"));*/
}

void UPawnCombatComponent::ApplyDamageToActorHit(AActor* DamagedActor, float BaseDamage, AController* EventInstigator, AActor* DamageCauser, TSubclassOf<UDamageType> DamageTypeClass)
{
	/*UGameplayStatics::ApplyDamage(Hit.GetActor(), ActiveCollisionPart->Damage, CharacterOwner->GetInstigator()->GetController(), CharacterOwner, UDamageType::StaticClass());*/
	const float TargetDefensePower = Cast<ICombatInterface>(DamagedActor)->GetDefensePower();

	const float FinalDamage = CalculateFinalDamage(BaseDamage, TargetDefensePower);
	UGameplayStatics::ApplyDamage(DamagedActor, FinalDamage, EventInstigator, DamageCauser, DamageTypeClass);
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

float UPawnCombatComponent::CalculateFinalDamage(float BaseDamage,float TargetDefensePower)
{
	return (BaseDamage * AttackPower / TargetDefensePower);

	//return BaseDamage;
}

#pragma endregion

#pragma region SetMesh

void UPawnCombatComponent::SetCollisionCharacterPartMesh()
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

void UPawnCombatComponent::SetCollisionWeaponPartMesh(UPrimitiveComponent* Mesh)
{
	FCollisionPartStruct* CollisionPartStruct = CollisionStructs.Find(FString("MainWeapon"));
	if (!CollisionPartStruct) return;

	CollisionPartStruct->CollisionMesh = Mesh;
}

#pragma endregion

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

bool UPawnCombatComponent::IsPlayingMontage_ExcludingBlendOut()
{
	if (!OwningPlayerAnimInstance) return false;

	// Ottieni il montaggio corrente
	UAnimMontage* CurrentMontage = OwningPlayerAnimInstance->GetCurrentActiveMontage();

	if (CurrentMontage && OwningPlayerAnimInstance->Montage_IsPlaying(CurrentMontage))
	{
		float CurrentMontagePosition = OwningPlayerAnimInstance->Montage_GetPosition(CurrentMontage);
		float MontageBlendOutTime = CurrentMontage->BlendOut.GetBlendTime();
		float MontageDuration = CurrentMontage->GetPlayLength();

		if ((CurrentMontagePosition >= MontageDuration - MontageBlendOutTime))
		{
			return false;
		}
		else
			return true;
	}
	else
		return false;
}

void UPawnCombatComponent::ResetAttack()
{

}
