#include "Components/PanWolfComponent.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/Combat/PandoCombatComponent.h"

#include "PanWolfWar/DebugHelper.h"
#include "Components/TargetingComponent.h"

#include "GameFramework/SpringArmComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include <NiagaraFunctionLibrary.h>
#include "NiagaraComponent.h"

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

	PanWolfState = EPanWolfState::EPWS_PanWolf;

	PanWolfCharacter->SetTransformationCharacter(SkeletalMeshAsset, Anim);

	CharacterOwner->GetCharacterMovement()->JumpZVelocity = 800.f;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(110.f);
	CharacterOwner->GetCapsuleComponent()->SetCapsuleRadius(85.f);
	CharacterOwner->GetMesh()->AddLocalOffset(FVector(15.f, -20.f, -10.f));

	
	PanWolfCharacter->GetCameraBoom()->TargetArmLength = 400.f;
	CharacterOwner->GetCharacterMovement()->bWantsToCrouch = false;

	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();

	if (OwningPlayerAnimInstance)
	{
		CombatComponent->SetCombatEnabled(OwningPlayerAnimInstance, ETransformationCombatType::ETCT_PanWolf);
		OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UPanWolfComponent::OnMontageEnded);
	}

	PanWolfCharacter->SetCollisionHandBoxExtent(CombatHandBoxExtent);
}

void UPanWolfComponent::Deactivate()
{
	Super::Deactivate();

	CharacterOwner->GetCharacterMovement()->JumpZVelocity = 500.f;
	CharacterOwner->GetCapsuleComponent()->SetCapsuleRadius(35.f);
	CombatComponent->ResetAttack();

	PanWolfCharacter->RemoveMappingContext(PanWolfMappingContext);

	PanWolfState = EPanWolfState::EPWS_PanWolf;
}

void UPanWolfComponent::Block()
{

	if (PanWolfState != EPanWolfState::EPWS_PanWolf || !OwningPlayerAnimInstance) return;

	BlockActivatedTime = UGameplayStatics::GetTimeSeconds(this);

	PanWolfState = EPanWolfState::EPWS_Blocking;
	OwningPlayerAnimInstance->Montage_Play(PanWolf_BlockMontage);

	AddShield();
}

void UPanWolfComponent::UnBlock()
{

	OwningPlayerAnimInstance->Montage_Stop(0.25, PanWolf_BlockMontage);
	RemoveShield();
}

void UPanWolfComponent::SuccesfulBlock(AActor* Attacker)
{


	bIsPerfectBlock = (UGameplayStatics::GetTimeSeconds(this) - BlockActivatedTime ) < PerfectBlockTime ? true : false;

	if(Attacker)
	{
		const FRotator BlockRotation = UKismetMathLibrary::FindLookAtRotation(CharacterOwner->GetActorLocation(),Attacker->GetActorLocation());
		CharacterOwner->SetActorRotation(BlockRotation);

		const FVector Force = (CharacterOwner->GetActorForwardVector() * -1.f) * BlockRepulsionForce;
		CharacterOwner->GetCharacterMovement()->AddForce(Force);

		//Da Provare
		//NewVel = GetActorForwardVector() * (-4000.f);
		//GetMesh()->SetPhysicsLinearVelocity(NewVel, false, FName("pelvis"));
	}



	if (ShieldBlock_Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShieldBlock_Sound, CharacterOwner->GetActorLocation());

	}

	UNiagaraFunctionLibrary::SpawnSystemAttached(BlockShieldNiagara, CharacterOwner->GetMesh(), FName("ShieldSocket"), FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);

	if (bIsPerfectBlock)
	{
		Debug::Print(TEXT("PerfectBlock"));
		//JumpToFinisher?

		UNiagaraFunctionLibrary::SpawnSystemAttached(PerfectBlockShieldNiagara, CharacterOwner->GetMesh(), FName("ShieldSocket"), CharacterOwner->GetActorForwardVector() * 30.f, UKismetMathLibrary::MakeRotFromX(CharacterOwner->GetActorForwardVector()), EAttachLocation::KeepRelativeOffset, true);
	
		UGameplayStatics::SetGlobalTimeDilation(this,0.2f);
		/*GetWorld()->GetTimerManager().SetTimer(PerfectBlock_TimerHandle, [this]() {UGameplayStatics::SetGlobalTimeDilation(this, 1.f); }, PerfectBlockTimer, false);*/
		GetWorld()->GetTimerManager().SetTimer(PerfectBlock_TimerHandle, [this]() {this->ResetPerfectBlock(); }, PerfectBlockTimer, false);
	}

}

void UPanWolfComponent::ResetPerfectBlock()
{
	bIsPerfectBlock = false;
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
}

void UPanWolfComponent::Jump()
{
	//if (IsPlayingMontage_ExcludingBlendOut()) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	CharacterOwner->Jump();
}

void UPanWolfComponent::Dodge()
{
	if (!PanWolfCharacter->CanPerformDodge() || (PanWolfState != EPanWolfState::EPWS_PanWolf)) return;
	if (!PanWolfDodgeMontage || !OwningPlayerAnimInstance) return;

	PanWolfState = EPanWolfState::EPWS_Dodging;

	/*const FVector CachedRollingDirection = CharacterOwner->GetCharacterMovement()->GetLastInputVector().GetSafeNormal();
	const FRotator TargetRotation = UKismetMathLibrary::MakeRotFromX(CachedRollingDirection);
	PanWolfCharacter->SetMotionWarpTarget(FName("RollingDirection"), FVector::ZeroVector, TargetRotation);*/

	PanWolfCharacter->StartDodge();
	OwningPlayerAnimInstance->Montage_Play(PanWolfDodgeMontage);
}

void UPanWolfComponent::LightAttack()
{

	if (!CombatComponent) return;

	if (bIsPerfectBlock)
		CombatComponent->Counterattack();
	else
		CombatComponent->PerformAttack(EAttackType::EAT_LightAttack);

}

void UPanWolfComponent::HeavyAttack()
{
	if (!CombatComponent) return;
	if (bIsPerfectBlock)
		CombatComponent->Counterattack();
	else
		CombatComponent->PerformAttack(EAttackType::EAT_HeavyAttack);
}

void UPanWolfComponent::BeginPlay()
{
	Super::BeginPlay();	

	CombatComponent = Cast<UPandoCombatComponent>(PanWolfCharacter->GetCombatComponent());
}

void UPanWolfComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPanWolfComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == PanWolfDodgeMontage)
	{
		PanWolfCharacter->EndDodge();
		PanWolfState = EPanWolfState::EPWS_PanWolf;
	}

	else if (Montage == PanWolf_HitReactMontage)
	{
		CharacterOwner->GetMesh()->SetScalarParameterValueOnMaterials(FName("HitFxSwitch"), 0.f);
	}

	else if (Montage == PanWolf_BlockMontage)
	{
		PanWolfState = EPanWolfState::EPWS_PanWolf;
		RemoveShield();

		if(UGameplayStatics::GetGlobalTimeDilation(this) != 1.f)
			UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
	}
}

void UPanWolfComponent::AddShield()
{
	if (AddShield_Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, AddShield_Sound, CharacterOwner->GetActorLocation());

	}

	Shield_NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(ShieldNiagara, CharacterOwner->GetMesh(), FName("ShieldSocket"), FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, false);
}

void UPanWolfComponent::RemoveShield()
{
	if (Shield_NiagaraComp)
		Shield_NiagaraComp->DestroyComponent();
}