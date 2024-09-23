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

#pragma region EngineFunctions

UPanWolfComponent::UPanWolfComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
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

	bIsBlocking = false;
	bIsBlockingReact = false;
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
	{
		if (PanWolfState == EPanWolfState::EPWS_Blocking)
			OwningPlayerAnimInstance->Montage_Stop(0.25, PanWolf_BlockMontage);

		if (PanWolfState == EPanWolfState::EPWS_PanWolf && bIsBlocking) 
		{
			CombatComponent->ResetAttack();
			return;
		}

			/*UnBlock();*/

		CombatComponent->PerformAttack(EAttackType::EAT_LightAttack);
	}
		

}

void UPanWolfComponent::HeavyAttack()
{
	if (!CombatComponent) return;
	if (bIsPerfectBlock)
		CombatComponent->Counterattack();
	else
	{
		if (PanWolfState == EPanWolfState::EPWS_Blocking)
			OwningPlayerAnimInstance->Montage_Stop(0.25, PanWolf_BlockMontage);
			/*UnBlock();*/

		if (PanWolfState == EPanWolfState::EPWS_PanWolf && bIsBlocking)
		{
			CombatComponent->ResetAttack();
			return;
		}

		CombatComponent->PerformAttack(EAttackType::EAT_HeavyAttack);
	}

}


#pragma endregion

#pragma region Block

void UPanWolfComponent::Block()
{
	Debug::Print(TEXT("Block"));
	/*if (PanWolfState != EPanWolfState::EPWS_PanWolf || !OwningPlayerAnimInstance) return;*/
	if (PanWolfState == EPanWolfState::EPWS_Blocking || !OwningPlayerAnimInstance) return;

	BlockActivatedTime = UGameplayStatics::GetTimeSeconds(this);
	bIsBlocking = true;

	if (PanWolfState == EPanWolfState::EPWS_Dodging) return;

	PanWolfState = EPanWolfState::EPWS_Blocking;

	OwningPlayerAnimInstance->Montage_Play(PanWolf_BlockMontage);
	OwningPlayerAnimInstance->Montage_JumpToSection(FName("Idle"), PanWolf_BlockMontage);

	/*AddShield();*/
}

void UPanWolfComponent::InstantBlock()
{
	BlockActivatedTime = UGameplayStatics::GetTimeSeconds(this);
	bIsBlocking = true;

	PanWolfState = EPanWolfState::EPWS_Blocking;

	OwningPlayerAnimInstance->Montage_Play(PanWolf_BlockMontage);
	OwningPlayerAnimInstance->Montage_JumpToSection(FName("Idle"), PanWolf_BlockMontage);
}

void UPanWolfComponent::LeftBlock()
{
	Debug::Print(TEXT("LeftBlock"));
	//OwningPlayerAnimInstance->Montage_Play(PanWolf_LeftBlockMontage);
	OwningPlayerAnimInstance->Montage_JumpToSection(FName("Left"), PanWolf_BlockMontage);
	bIsBlockingReact = true;
}

void UPanWolfComponent::RightBlock()
{
	Debug::Print(TEXT("RightBlock"));
	//OwningPlayerAnimInstance->Montage_Play(PanWolf_RightBlockMontage);
	OwningPlayerAnimInstance->Montage_JumpToSection(FName("Right"), PanWolf_BlockMontage);
	bIsBlockingReact = true;
}

void UPanWolfComponent::UnBlock()
{
	bIsBlocking = false;
	bIsBlockingReact = false;
	OwningPlayerAnimInstance->Montage_Stop(0.25, PanWolf_BlockMontage);
	/*RemoveShield();*/
}

void UPanWolfComponent::SuccesfulBlock(AActor* Attacker)
{
	bIsPerfectBlock = (UGameplayStatics::GetTimeSeconds(this) - BlockActivatedTime) < PerfectBlockTime ? true : false;

	if (Attacker)
	{

		FVector ForwardVectorOwner = CharacterOwner->GetActorForwardVector();
		FVector ForwardVectorAttacker = Attacker->GetActorForwardVector();


		float DotProduct = FVector::DotProduct(ForwardVectorOwner, ForwardVectorAttacker);
		float AngleInRadians = FMath::Acos(DotProduct);
		float AngleInDegrees = FMath::RadiansToDegrees(AngleInRadians);

		/*UE_LOG(LogTemp, Warning, TEXT("L'angolo tra i due attori è: %f gradi"), AngleInDegrees);*/

		FVector CrossProduct = FVector::CrossProduct(ForwardVectorOwner, ForwardVectorAttacker);
		if (CrossProduct.Z < 0 && AngleInDegrees < 135.f)
		{
			UE_LOG(LogTemp, Warning, TEXT("Attacker è a destra di CharacterOwner"));
			RightBlock();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Attacker è a sinistra di CharacterOwner"));
			LeftBlock();
		}

		const FRotator BlockRotation = UKismetMathLibrary::FindLookAtRotation(CharacterOwner->GetActorLocation(), Attacker->GetActorLocation());
		CharacterOwner->SetActorRotation(BlockRotation);

		//const FVector Force = (CharacterOwner->GetActorForwardVector() * -1.f) * BlockRepulsionForce;
		//CharacterOwner->GetCharacterMovement()->AddForce(Force);

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

		UGameplayStatics::SetGlobalTimeDilation(this, 0.2f);
		/*GetWorld()->GetTimerManager().SetTimer(PerfectBlock_TimerHandle, [this]() {UGameplayStatics::SetGlobalTimeDilation(this, 1.f); }, PerfectBlockTimer, false);*/
		GetWorld()->GetTimerManager().SetTimer(PerfectBlock_TimerHandle, [this]() {this->ResetPerfectBlock(); }, PerfectBlockTimer, false);
	}

}

void UPanWolfComponent::ResetPerfectBlock()
{
	bIsPerfectBlock = false;
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
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

void UPanWolfComponent::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == PanWolfDodgeMontage)
	{
		PanWolfCharacter->EndDodge();

		if(PanWolfState!= EPanWolfState::EPWS_Blocking)
			PanWolfState = EPanWolfState::EPWS_PanWolf;

		/*if (bIsBlocking)
			Block();*/
		
	}

	else if (Montage == PanWolf_HitReactMontage)
	{
		CharacterOwner->GetMesh()->SetScalarParameterValueOnMaterials(FName("HitFxSwitch"), 0.f);

		if (bIsBlocking)
			Block();
	}

	else if (Montage == PanWolf_BlockMontage)
	{
		if (PanWolfState == EPanWolfState::EPWS_Dodging) return;

		if (bIsBlockingReact)
		{
			bIsBlockingReact = false;
			/*if (OwningPlayerAnimInstance->Montage_IsPlaying(PanWolf_HitReactMontage))*/
			if (bInterrupted ||OwningPlayerAnimInstance->Montage_IsPlaying(PanWolf_HitReactMontage))
			{
				PanWolfState = EPanWolfState::EPWS_PanWolf;
				return;
			}
			OwningPlayerAnimInstance->Montage_Play(PanWolf_BlockMontage);
			OwningPlayerAnimInstance->Montage_JumpToSection(FName("Idle"), PanWolf_BlockMontage);

			return;
		}

		PanWolfState = EPanWolfState::EPWS_PanWolf;
		/*RemoveShield();

		if (UGameplayStatics::GetGlobalTimeDilation(this) != 1.f)
			UGameplayStatics::SetGlobalTimeDilation(this, 1.f);*/
	}

	else if (bIsBlocking)
	{
		Debug::Print(TEXT("Return to block"));
		Block();
	}

}
