#include "Enemy/AssassinableComponent.h"
#include <NiagaraFunctionLibrary.h>
#include "GameFramework/Character.h"
#include "Interfaces/CharacterInterface.h"
#include "Components/PandolfoComponent.h"
#include "Components/PandolFlowerComponent.h"
#include "Enemy/BaseEnemy.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"

UAssassinableComponent::UAssassinableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UAssassinableComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ACharacter>(GetOwner());	
	EnemyOwner = Cast<ABaseEnemy>(CharacterOwner);

}

void UAssassinableComponent::BoxCollisionEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
	/*if(CharacterOwner->ActorHasTag(FName("Dead"))) return;*/
	if (!EnemyOwner || !EnemyOwner->IsCombatActorAlive() || EnemyOwner->IsEnemyAware()) return;

	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (!CharacterInterface) return;

		UPandolfoComponent* PandolfoComponent = CharacterInterface->GetPandolfoComponent();
		UPandolFlowerComponent* PandolFlowerComponent = CharacterInterface->GetPandolFlowerComponent();

		if (!PandolfoComponent || (!PandolfoComponent->IsActive() && !PandolFlowerComponent->IsActive())) return;

		PandolfoComponent->SetAssassinableEnemy(EnemyOwner);
		EnemyOwner->SetAssassinationWidgetVisibility(true);
		MarkAsTarget(true);
	}
}

void UAssassinableComponent::BoxCollisionExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	/*if (CharacterOwner->ActorHasTag(FName("Dead"))) return;*/
	if (!EnemyOwner || !EnemyOwner->IsCombatActorAlive()) return;

	if (OtherActor->Implements<UCharacterInterface>())
	{
		ICharacterInterface* CharacterInterface = Cast<ICharacterInterface>(OtherActor);
		if (!CharacterInterface) return;

		UPandolfoComponent* PandolfoComponent = CharacterInterface->GetPandolfoComponent();
		UPandolFlowerComponent* PandolFlowerComponent = CharacterInterface->GetPandolFlowerComponent();

		if (!PandolfoComponent || (!PandolfoComponent->IsActive() && !PandolFlowerComponent->IsActive())) return;

		PandolfoComponent->SetAssassinableEnemy(nullptr);
		EnemyOwner->SetAssassinationWidgetVisibility(false);
		MarkAsTarget(false);
	}
}

void UAssassinableComponent::MarkAsTarget(bool IsTargeted)
{
	const float EmissiveMultiplier = UKismetMathLibrary::SelectFloat(1.0, 0.f, IsTargeted);
	CharacterOwner->GetMesh()->SetScalarParameterValueOnMaterials(FName("HitFxSwitch"), EmissiveMultiplier);
}

void UAssassinableComponent::Assassinated(int32 AssassinationIndex, UPandolfoComponent* _PandolfoComponent, bool AirAssassination)
{
	UAnimMontage* AssassinationMontage = nullptr;;

	if (AirAssassination)
	{
		AssassinationMontage = AirAssassinDeathMontage;
	}
	else
	{
		if (AssassinationIndex == 0 || AssassinationIndex > AssassinationMontage_Map.Num()) return;
		AssassinationMontage = *AssassinationMontage_Map.Find(AssassinationIndex);
	}

	if (!AssassinationMontage ) return;

	UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return;

	EnemyOwner->InitializeEnemyDeath();
	CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (AirAssassination)
		AirAssassinated();

	OwningPlayerAnimInstance->Montage_Play(AssassinationMontage);


	if (_PandolfoComponent)
		_PandolfoComponent->SetAssassinableEnemy(nullptr);
	if (AirAssassination && _PandolfoComponent)
	{
		_PandolfoComponent->SetAssassinableEnemy(nullptr);
		_PandolfoComponent->SetAssassinableAirEnemy(nullptr);
	}


	EnemyOwner->SetAssassinationWidgetVisibility(false);
	MarkAsTarget(false);
}

void UAssassinableComponent::AirAssassinated()
{
	CharacterOwner->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ACharacter* Player = EnemyOwner->GetPlayer(); 
	if (!Player) return;
	const FRotator NewRotation = FRotator(0.f, UKismetMathLibrary::FindLookAtRotation(CharacterOwner->GetActorLocation(), Player->GetActorLocation()).Yaw, 0.f);
	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(CharacterOwner->GetCapsuleComponent(), CharacterOwner->GetActorLocation(), NewRotation, false, false, 0.05, false, EMoveComponentAction::Move, LatentInfo);
}

void UAssassinableComponent::Killed()
{
	PlayBloodEffect();
	CharacterOwner->GetMesh()->SetSimulatePhysics(true);
	UGameplayStatics::PlayWorldCameraShake(this, AssassinationCameraShake, CharacterOwner->GetActorLocation(), 4444.f, 4444.f);
}

void UAssassinableComponent::PlayBloodEffect()
{
	UNiagaraFunctionLibrary::SpawnSystemAttached(BloodEffect_Niagara, CharacterOwner->GetMesh(), BloodEffect_SocketName, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false);
}