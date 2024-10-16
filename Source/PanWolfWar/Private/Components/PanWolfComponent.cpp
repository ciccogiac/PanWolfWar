#include "Components/PanWolfComponent.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/Combat/PandoCombatComponent.h"

#include "PanWolfWar/DebugHelper.h"


#include "GameFramework/SpringArmComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include <NiagaraFunctionLibrary.h>
#include "NiagaraComponent.h"

#include "PanWarFunctionLibrary.h"
#include "Components/TargetingComponent.h"

#pragma region EngineFunctions

UPanWolfComponent::UPanWolfComponent()
{
	bCanCrouch = false;
}

void UPanWolfComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPanWolfComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

#pragma endregion

#pragma region ActivationSection

void UPanWolfComponent::Activate(bool bReset)
{
	Super::Activate();	

	PanWolfState = EPanWolfState::EPWS_PanWolf;

	CharacterOwner->GetMesh()->AddLocalOffset(FVector(15.f, -20.f, -10.f));

	if (OwningPlayerAnimInstance)
	{
		CombatComponent->SetCombatEnabled(OwningPlayerAnimInstance, ETransformationCombatType::ETCT_PanWolf);
	}

	bIsBlocking = false;
	bIsBlockingReact = false;
	bHitted = false;
	PanWolfCharacter->SetIsHiding(false);
}

void UPanWolfComponent::Deactivate()
{
	Super::Deactivate();

	bHitted = false;

	if (PanWolfState == EPanWolfState::EPWS_Dodging)
		PanWolfCharacter->EndDodge();

	PanWolfState = EPanWolfState::EPWS_PanWolf;
}

#pragma endregion

#pragma region Actions

void UPanWolfComponent::Jump()
{
	//if (IsPlayingMontage_ExcludingBlendOut()) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

	CharacterOwner->Jump();
}

void UPanWolfComponent::Dodge()
{

	if (!PanWolfCharacter->CanPerformDodge() || (PanWolfState == EPanWolfState::EPWS_Dodging)) return;
	if (!PanWolfDodgeMontage || !OwningPlayerAnimInstance) return;
	if (bHitted) return;

	if (OwningPlayerAnimInstance->Montage_IsPlaying(PanWolfDodgeMontage)) return;

	//Check if is returning to block position
	if (OwningPlayerAnimInstance->Montage_IsPlaying(PanWolf_BlockMontage))
	{
		float CurrentMontagePosition = OwningPlayerAnimInstance->Montage_GetPosition(PanWolf_BlockMontage);
		float MontageBlendInTime = PanWolf_BlockMontage->BlendIn.GetBlendTime();
		float MontageBlendOutTime = PanWolf_BlockMontage->BlendOut.GetBlendTime();
		float MontageDuration = PanWolf_BlockMontage->GetSectionLength(0);
		float BlendInOffset = 0.1f;

		if ((CurrentMontagePosition <= MontageBlendInTime ))
		{
			return ;
		}

	}


	PanWolfState = EPanWolfState::EPWS_Dodging;

	PanWolfCharacter->StartDodge();
	OwningPlayerAnimInstance->Montage_Play(PanWolfDodgeMontage);

	FOnMontageEnded DodgeMontageEndedDelegate;
	DodgeMontageEndedDelegate.BindUObject(this, &UPanWolfComponent::OnDodgeMontageEnded);
	OwningPlayerAnimInstance->Montage_SetEndDelegate(DodgeMontageEndedDelegate, PanWolfDodgeMontage);
}

void UPanWolfComponent::LightAttack()
{

	if (!CombatComponent) return; if (bHitted) return;

	if (bIsPerfectBlock)
	{
		GetWorld()->GetTimerManager().ClearTimer(PerfectBlock_TimerHandle);
		ResetPerfectBlock();
		OwningPlayerAnimInstance->Montage_Stop(0.25, PanWolf_BlockMontage);
		PanWolfState = EPanWolfState::EPWS_PanWolf;
		CombatComponent->ResetAttack();
		CombatComponent->Counterattack();

		return;
	}

	else
	{
		if (PanWolfState != EPanWolfState::EPWS_Blocking && !bIsBlocking)
		{
			CombatComponent->PerformAttack(EAttackType::EAT_LightAttack);
			return;
		}

		else if (PanWolfState == EPanWolfState::EPWS_Blocking && bIsBlocking)
		{
			if (CombatComponent->IsAttacking()) return;

			if (OwningPlayerAnimInstance->Montage_IsPlaying(PanWolf_BlockMontage))
			{
				float CurrentMontagePosition = OwningPlayerAnimInstance->Montage_GetPosition(PanWolf_BlockMontage);
				float MontageBlendInTime = PanWolf_BlockMontage->BlendIn.GetBlendTime();

				if ((CurrentMontagePosition <= MontageBlendInTime )) return;

			}
			OwningPlayerAnimInstance->Montage_Stop(0.25, PanWolf_BlockMontage);


			PanWolfState = EPanWolfState::EPWS_PanWolf;
			CombatComponent->PerformAttack(EAttackType::EAT_LightAttack);
		}
	}
}

void UPanWolfComponent::HeavyAttack()
{
	if (!CombatComponent) return; if (bHitted) return;

	if (bIsPerfectBlock)
	{
		GetWorld()->GetTimerManager().ClearTimer(PerfectBlock_TimerHandle);
		ResetPerfectBlock();
		OwningPlayerAnimInstance->Montage_Stop(0.25, PanWolf_BlockMontage);
		PanWolfState = EPanWolfState::EPWS_PanWolf;
		CombatComponent->ResetAttack();
		CombatComponent->Counterattack();

	}
	else
	{

		if (PanWolfState != EPanWolfState::EPWS_Blocking && !bIsBlocking)
		{
			CombatComponent->PerformAttack(EAttackType::EAT_HeavyAttack);
			return;
		}

		else if (PanWolfState == EPanWolfState::EPWS_Blocking && bIsBlocking)
		{
			if (CombatComponent->IsAttacking()) return;



			if (OwningPlayerAnimInstance->Montage_IsPlaying(PanWolf_BlockMontage))
			{
				float CurrentMontagePosition = OwningPlayerAnimInstance->Montage_GetPosition(PanWolf_BlockMontage);
				float MontageBlendInTime = PanWolf_BlockMontage->BlendIn.GetBlendTime();

				if ((CurrentMontagePosition <= MontageBlendInTime)) return;

			}
			OwningPlayerAnimInstance->Montage_Stop(0.25, PanWolf_BlockMontage);


			PanWolfState = EPanWolfState::EPWS_PanWolf;
			CombatComponent->PerformAttack(EAttackType::EAT_HeavyAttack);
		}

	}

}

#pragma endregion

#pragma region Block

void UPanWolfComponent::Block()
{
	if (bIsBlocking) return;

	bIsBlocking = true;
	bIsBlockingReact = false;
	BlockActivatedTime = UGameplayStatics::GetTimeSeconds(this);

	if (PanWolfState != EPanWolfState::EPWS_PanWolf || !OwningPlayerAnimInstance) return;
	if (bHitted) return;

	PanWolfState = EPanWolfState::EPWS_Blocking;
	CombatComponent->ResetAttack();

	OwningPlayerAnimInstance->Montage_Play(PanWolf_BlockMontage);
	OwningPlayerAnimInstance->Montage_JumpToSection(FName("Idle"), PanWolf_BlockMontage);

	FOnMontageEnded BlockMontageEndedDelegate;
	BlockMontageEndedDelegate.BindUObject(this, &UPanWolfComponent::OnBlockMontageEnded);
	OwningPlayerAnimInstance->Montage_SetEndDelegate(BlockMontageEndedDelegate, PanWolf_BlockMontage);

}

void UPanWolfComponent::UnBlock()
{
	if (!bIsBlocking) return;

	bIsBlocking = false;
	bIsBlockingReact = false;
	OwningPlayerAnimInstance->Montage_Stop(0.25, PanWolf_BlockMontage);

	if (PanWolfState == EPanWolfState::EPWS_Blocking)
	{
		PanWolfState = EPanWolfState::EPWS_PanWolf;

	}
}

void UPanWolfComponent::InstantBlock()
{
	if (!bIsBlocking) return;
	if (!OwningPlayerAnimInstance) return;

	bIsBlockingReact = false;

	PanWolfState = EPanWolfState::EPWS_Blocking;

	OwningPlayerAnimInstance->Montage_Play(PanWolf_BlockMontage);
	OwningPlayerAnimInstance->Montage_JumpToSection(FName("Idle"), PanWolf_BlockMontage);

	FOnMontageEnded BlockMontageEndedDelegate;
	BlockMontageEndedDelegate.BindUObject(this, &UPanWolfComponent::OnBlockMontageEnded);
	OwningPlayerAnimInstance->Montage_SetEndDelegate(BlockMontageEndedDelegate, PanWolf_BlockMontage);
}

void UPanWolfComponent::LeftBlock()
{
	OwningPlayerAnimInstance->Montage_JumpToSection(FName("Left"), PanWolf_BlockMontage);
	bIsBlockingReact = true;
}

void UPanWolfComponent::RightBlock()
{
	OwningPlayerAnimInstance->Montage_JumpToSection(FName("Right"), PanWolf_BlockMontage);
	bIsBlockingReact = true;
}

bool UPanWolfComponent::IsWolfValidBlock(AActor* InAttacker)
{
	check(InAttacker);

	FVector ForwardVectorOwner = CharacterOwner->GetActorForwardVector();
	FVector ForwardVectorAttacker = InAttacker->GetActorForwardVector();


	float DotProduct = FVector::DotProduct(ForwardVectorOwner, ForwardVectorAttacker);
	FVector CrossProduct = FVector::CrossProduct(ForwardVectorOwner, ForwardVectorAttacker);


	if (CrossProduct.Z > 0 && DotProduct > 0)
	{
		/*UE_LOG(LogTemp, Warning, TEXT("Attacker è a sinistra di CharacterOwner"));*/
		return false;
	}

	return true;
}

void UPanWolfComponent::ReturnToBlockFromAttack()
{
	InstantBlock();
	if(bIsBlocking)
		CombatComponent->ResetAttack();
}

void UPanWolfComponent::SuccesfulBlock(AActor* Attacker)
{
	bIsPerfectBlock = 
		(
		Attacker &&
		Attacker->Implements<UCombatInterface>() &&
		UGameplayStatics::GetTimeSeconds(this) - BlockActivatedTime < PerfectBlockTime 
		)	? true : false;

	if (Attacker)
	{

		FVector ForwardVectorOwner = CharacterOwner->GetActorForwardVector();
		FVector ForwardVectorAttacker = Attacker->GetActorForwardVector();


		float DotProduct = FVector::DotProduct(ForwardVectorOwner, ForwardVectorAttacker);
		float AngleInRadians = FMath::Acos(DotProduct);
		float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);

		FVector CrossProduct = FVector::CrossProduct(ForwardVectorOwner, ForwardVectorAttacker);
		if (CrossProduct.Z < 0 && AngleInDegrees < 135.f)
		{
			RightBlock();
		}
		else
		{
			LeftBlock();
		}

		FVector PushDirection = - CharacterOwner->GetActorForwardVector();
		float PushbackStrength = 350.f;
		CharacterOwner->LaunchCharacter(PushDirection * PushbackStrength, true, true);
	}

	if (Block_Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Block_Sound, CharacterOwner->GetActorLocation());

	}

	UNiagaraFunctionLibrary::SpawnSystemAttached(BlockEffectNiagara, CharacterOwner->GetMesh(), FName("ShieldSocket"), FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);

	if (bIsPerfectBlock)
	{

		PanWolfCharacter->SetInvulnerability(true);
		if (TargetingComponent && TargetingComponent->IsTargeting())
		{
			TargetingComponent->SetSwitchDirectionBlocked(true);
			TargetingComponent->ChangeTargetActor(Attacker);
		}

		UNiagaraFunctionLibrary::SpawnSystemAttached(PerfectBlockEffectNiagara, CharacterOwner->GetMesh(), FName("ShieldSocket"), CharacterOwner->GetActorForwardVector() * 30.f, UKismetMathLibrary::MakeRotFromX(CharacterOwner->GetActorForwardVector()), EAttachLocation::KeepRelativeOffset, true);

		UGameplayStatics::SetGlobalTimeDilation(this, 0.2f);
		GetWorld()->GetTimerManager().SetTimer(PerfectBlock_TimerHandle, [this]() {this->ResetPerfectBlock(); }, PerfectBlockTimer, false); 

		ICombatInterface* EnemyCombatInterface = Cast<ICombatInterface>(Attacker);
		if (EnemyCombatInterface)
			EnemyCombatInterface->ShortStunned();
	}

}

void UPanWolfComponent::ResetPerfectBlock()
{
	bIsPerfectBlock = false;
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);

	PanWolfCharacter->SetInvulnerability(false);
	if (TargetingComponent)
		TargetingComponent->SetSwitchDirectionBlocked(false);
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

#pragma endregion

#pragma region MontageSection

UAnimMontage* UPanWolfComponent::GetPanWolfHitReactMontage()
{
	bHitted = true;
	return PanWolf_HitReactMontage;
}

void UPanWolfComponent::OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;
	
	PanWolfCharacter->EndDodge();

	if (PanWolfState != EPanWolfState::EPWS_Blocking)
	{
		PanWolfState = EPanWolfState::EPWS_PanWolf;
	}

}

void UPanWolfComponent::OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!Montage) return;
	
	if (PanWolfState == EPanWolfState::EPWS_Blocking && bIsBlockingReact)
	{
		bIsBlockingReact = false;

		if (bHitted)
		{
			return;
		}
		OwningPlayerAnimInstance->Montage_Play(PanWolf_BlockMontage);
		OwningPlayerAnimInstance->Montage_JumpToSection(FName("Idle"), PanWolf_BlockMontage);

		FOnMontageEnded BlockMontageEndedDelegate;
		BlockMontageEndedDelegate.BindUObject(this, &UPanWolfComponent::OnBlockMontageEnded);
		OwningPlayerAnimInstance->Montage_SetEndDelegate(BlockMontageEndedDelegate, PanWolf_BlockMontage);

		return;
	}
	
}

void UPanWolfComponent::OnWolfHitReactMontageEnded()
{
	if (bIsBlocking)
		InstantBlock();
	else
		PanWolfState = EPanWolfState::EPWS_PanWolf;

	bHitted = false;

	if (!CombatComponent->IsAttacking())
		CombatComponent->ResetAttack();
}

#pragma endregion

void UPanWolfComponent::ClearAllTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(PerfectBlock_TimerHandle);
}