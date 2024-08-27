#include "Components/PanWolfComponent.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/CombatComponent.h"

#include "PanWolfWar/DebugHelper.h"
#include "Components/TargetingComponent.h"

#include "GameFramework/SpringArmComponent.h"

UPanWolfComponent::UPanWolfComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
}

void UPanWolfComponent::Activate(bool bReset)
{
	Super::Activate();



	PanWolfCharacter->AddMappingContext(PanWolfMappingContext, 1);

	PanWolfCharacter->SetTransformationCharacter(SkeletalMeshAsset, Anim);

	CharacterOwner->GetCharacterMovement()->JumpZVelocity = 800.f;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(110.f);
	CharacterOwner->GetCapsuleComponent()->SetCapsuleRadius(85.f);
	CharacterOwner->GetMesh()->AddLocalOffset(FVector(15.f, -20.f, -10.f));

	CombatComponent->SetCombatEnabled(true);

	PanWolfCharacter->GetCameraBoom()->TargetArmLength = 400.f;
	CharacterOwner->GetCharacterMovement()->bWantsToCrouch = false;

	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

	if (OwningPlayerAnimInstance)
	{
		OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UPanWolfComponent::OnMontageEnded);
	}
}

void UPanWolfComponent::Deactivate()
{
	Super::Deactivate();

	CharacterOwner->GetCharacterMovement()->JumpZVelocity = 500.f;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleRadius(35.f);

	CombatComponent->SetCombatEnabled(false);

	PanWolfCharacter->RemoveMappingContext(PanWolfMappingContext);
}

void UPanWolfComponent::Jump()
{
	//if (IsPlayingMontage_ExcludingBlendOut()) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	CharacterOwner->Jump();
}

void UPanWolfComponent::Dodge()
{
	if (!PanWolfCharacter->CanPerformDodge()) return;

	//if (PandolfoState != EPandolfoState::EPS_Pandolfo) return;

	if (!PandolWolfDodgeMontage) return;
	if (!OwningPlayerAnimInstance) return;
	//if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;
	//if (CharacterOwner->GetCharacterMovement()->GetLastInputVector().Length() < 0.5f) return;

	//CharacterOwner->DisableInput(CharacterOwner->GetLocalViewingPlayerController());
	//Debug::Print(TEXT("Dodge"));
	OwningPlayerAnimInstance->Montage_Play(PandolWolfDodgeMontage);
}

void UPanWolfComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{

	if (LightAttackMontages.FindKey(Montage))
	{
		if (bInterrupted) return;
		GetWorld()->GetTimerManager().SetTimer(ComboLightCountReset_TimerHandle, [this]() {this->ResetLightAttackComboCount(); }, 0.025f, false);
	}

	else if (HeavyAttackMontages.FindKey(Montage))
	{
		if (bInterrupted) return;
		GetWorld()->GetTimerManager().SetTimer(ComboHeavyCountReset_TimerHandle, [this]() {this->ResetHeavyAttackComboCount(); }, 0.025f, false);
	}
}

bool UPanWolfComponent::IsPlayingMontage_ExcludingBlendOut()
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

void UPanWolfComponent::LightAttack()
{

	/*if (CharacterOwner->GetMovementComponent()->IsFalling())
	{
		
		CombatComponent->PerformAttack(EAttackType::EAT_HeavyAttack);
		return;
	}*/

	//if (!CombatComponent->IsAttacking())
	//{
	//	//if (!PanWolfCharacter->GetTargetingComponent()->IsTargeting())
	//	//	PanWolfCharacter->GetTargetingComponent()->TryLock();

	//	const bool isTargeting = PanWolfCharacter->GetTargetingComponent()->IsActive() && PanWolfCharacter->GetTargetingComponent()->IsTargeting();
	//	const AActor* ClosestEnemy = isTargeting ? PanWolfCharacter->GetTargetingComponent()->GetTargetActor() : CombatComponent->GetClosestEnemy();
	//	//if (!ClosestEnemy) return;

	//	const bool EnemyDirection = CombatComponent->GetEnemyDirection(ClosestEnemy);
	//	CombatComponent->RotateToClosestEnemy(ClosestEnemy);
	//	EAttackType AttackType = EnemyDirection ? EAttackType::EAT_LightAttack_Right : EAttackType::EAT_LightAttack_Left;
	//	CombatComponent->PerformAttack(AttackType);
	//}
	//else
	//	CombatComponent->PerformAttack(EAttackType::EAT_LightAttack_Right);


	//

	//if (!OwningPlayerAnimInstance) return;

	//// Ottieni il montaggio corrente
	//UAnimMontage* CurrentMontage = OwningPlayerAnimInstance->GetCurrentActiveMontage();

	//if (CurrentMontage && OwningPlayerAnimInstance->Montage_IsPlaying(CurrentMontage))
	//{
	//	float CurrentMontagePosition = OwningPlayerAnimInstance->Montage_GetPosition(CurrentMontage);
	//	float MontageBlendOutTime = CurrentMontage->BlendOut.GetBlendTime();
	//	float MontageDuration = CurrentMontage->GetPlayLength();

	//	if (!(CurrentMontagePosition >= MontageDuration - MontageBlendOutTime))
	//	{
	//		return;
	//	}
	//}

	if (!OwningPlayerAnimInstance) return;
	if (IsPlayingMontage_ExcludingBlendOut()) return;


	GetWorld()->GetTimerManager().ClearTimer(ComboLightCountReset_TimerHandle);
	UAnimMontage* AttackMontage = *LightAttackMontages.Find(CurrentLightAttackComboCount);
	if (!AttackMontage) return;	
	
	OwningPlayerAnimInstance->Montage_Play(AttackMontage);

	if (CurrentLightAttackComboCount == LightAttackMontages.Num()) { ResetLightAttackComboCount(); return; }

	if (CurrentLightAttackComboCount == (LightAttackMontages.Num() - 1)) { bJumpToFinisher = true; }
	else ResetHeavyAttackComboCount();

	CurrentLightAttackComboCount++;

	

}

void UPanWolfComponent::ResetLightAttackComboCount()
{
	CurrentLightAttackComboCount = 1;
	/*CurrentHeavyAttackComboCount = 1;*/
	bJumpToFinisher = false;
}

void UPanWolfComponent::HeavyAttack()
{
	//CombatComponent->PerformAttack(EAttackType::EAT_HeavyAttack);

	//if (!OwningPlayerAnimInstance || OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	if (!OwningPlayerAnimInstance) return;
	if (IsPlayingMontage_ExcludingBlendOut()) return;

	GetWorld()->GetTimerManager().ClearTimer(ComboHeavyCountReset_TimerHandle);
	if (bJumpToFinisher) { CurrentHeavyAttackComboCount = HeavyAttackMontages.Num(); }
	UAnimMontage* AttackMontage = *HeavyAttackMontages.Find(CurrentHeavyAttackComboCount);
	if (!AttackMontage) return;

	OwningPlayerAnimInstance->Montage_Play(AttackMontage);

	ResetLightAttackComboCount();

	if (CurrentHeavyAttackComboCount == HeavyAttackMontages.Num()) { ResetHeavyAttackComboCount(); return; }


	CurrentHeavyAttackComboCount++;
}

void UPanWolfComponent::ResetHeavyAttackComboCount()
{
	CurrentHeavyAttackComboCount = 1;
	/*CurrentLightAttackComboCount = 1;*/
	bJumpToFinisher = false;
}

void UPanWolfComponent::BeginPlay()
{
	Super::BeginPlay();	

	CombatComponent = PanWolfCharacter->GetCombatComponent();
}

void UPanWolfComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

