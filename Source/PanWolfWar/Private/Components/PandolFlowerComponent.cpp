// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PandolFlowerComponent.h"

#include "PanWolfWar/DebugHelper.h"
#include "GameFramework/Character.h"
#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Camera/CameraComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Components/CapsuleComponent.h"
#include "NiagaraComponent.h"
#include "CharacterActor/FlowerCable.h"


#pragma region EngineFunctions

UPandolFlowerComponent::UPandolFlowerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
}

void UPandolFlowerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (PanWolfCharacter)
	{
		FollowCamera = PanWolfCharacter->GetFollowCamera();
	}

}

void UPandolFlowerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsHooking)
	{
		SearchHookingPoint();
	}

}

#pragma endregion

#pragma region ActivationComponent

void UPandolFlowerComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	PanWolfCharacter->AddMappingContext(PandolFlowerMappingContext, 1);

	PanWolfCharacter->SetTransformationCharacter(SkeletalMeshAsset, Anim);
	PanWolfCharacter->GetNiagaraTransformation()->SetAsset(Pandolflower_Niagara);

	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (OwningPlayerAnimInstance)
	{
		OwningPlayerAnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UPandolFlowerComponent::OnFlowerNotifyStarted);
		OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UPandolFlowerComponent::OnFlowerMontageEnded);
	}

	FlowerCable = GetWorld()->SpawnActor<AFlowerCable>(BP_FlowerCable, CharacterOwner->GetActorLocation(), CharacterOwner->GetActorRotation());
	FlowerCable->SetAttachEndCable(CharacterOwner->GetMesh(), FName("hand_r"));

}

void UPandolFlowerComponent::Deactivate()
{
	Super::Deactivate();

	PanWolfCharacter->RemoveMappingContext(PandolFlowerMappingContext);

	PanWolfCharacter->GetNiagaraTransformation()->SetAsset(nullptr);

	if (FlowerCable)
		FlowerCable->Destroy();
}

#pragma endregion

#pragma region Hooking

void UPandolFlowerComponent::Hook()
{
	if (!IsActive()) return;

	if (bIsHooking) return;

	if (!bDidHit) { Debug::Print(TEXT("Noo Hooking Point Find")); return; }

	if (!SearchHookableObject()) return;

	bIsHooking = true;
	//Face to HookingTarget
	const FRotator NewRotation = FRotator(CharacterOwner->GetActorRotation().Pitch, Hook_TargetRotation.Yaw, CharacterOwner->GetActorRotation().Roll);
	CharacterOwner->SetActorRotation(NewRotation);
	CharacterOwner->GetCharacterMovement()->DisableMovement();

	PlayMontage(ThrowFlowerCable_Montage);

}

void UPandolFlowerComponent::SearchHookingPoint()
{
	const FVector CameraForward = FollowCamera->GetForwardVector();
	const FVector CameraLocation = FollowCamera->GetComponentLocation();
	const FVector ActorUP = CharacterOwner->GetActorUpVector();

	const FVector Start = CameraLocation + CameraForward * 820.f + ActorUP * 44.f;
	const FVector End = CameraLocation + CameraForward * 1555.f + ActorUP * 333.f;

	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;

	UKismetSystemLibrary::SphereTraceSingle(this, Start, End, SearchHook_Radius, HookingTraceType, false, TArray<AActor*>(), DebugTraceType, FirstTrace, true);

	if (FirstTrace.bBlockingHit)
	{
		bDidHit = true;
		DrawDebugPoint(GetWorld(), FirstTrace.ImpactPoint, 25.f, FColor::White);
	}
	else
		bDidHit = false;
}

bool UPandolFlowerComponent::SearchHookableObject()
{
	const FVector Start = FirstTrace.ImpactPoint + FVector(0.f, 0.f, 33.f);
	const FVector End = FirstTrace.ImpactPoint + FVector(0.f, 0.f, -150.f);

	EDrawDebugTrace::Type DebugTraceType = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, SearchObjectHookable_Radius, HookableObjectTypes, false, TArray<AActor*>(), DebugTraceType, SecondTrace, true);

	if (SecondTrace.bBlockingHit)
	{

		if (UKismetMathLibrary::Vector_Distance(CharacterOwner->GetActorLocation(), SecondTrace.ImpactPoint) <= Min_HookingDistance)
		{
			Debug::Print(TEXT("Target Too Close"));
			return false;
		}
		Hook_TargetLocation = SecondTrace.ImpactPoint;
		Hook_TargetRotation = UKismetMathLibrary::FindLookAtRotation(CharacterOwner->GetActorLocation(), Hook_TargetLocation);
		return true;
	}
	return false;
}

void UPandolFlowerComponent::StartHooking()
{
	const FVector TargetLocation = SecondTrace.ImpactPoint + FVector(0.f, 0.f, 57.5f);
	DrawDebugPoint(GetWorld(), TargetLocation, 5.f, FColor::Magenta, false, 10.f);

	PanWolfCharacter->SetMotionWarpTarget(FName("HookTarget"), TargetLocation, Hook_TargetRotation);
	CharacterOwner->GetCharacterMovement()->MaxFlySpeed = 0.f;
	CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying, 0);

	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(45.f);
	PlayMontage(HookJump_Montage);

}

void UPandolFlowerComponent::EndHooking()
{
	bIsHooking = false;
	//isHookingState = false;
	CharacterOwner->GetCharacterMovement()->StopMovementImmediately();
	CharacterOwner->GetCapsuleComponent()->SetCapsuleHalfHeight(90.f);
	CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	FlowerCable->SetAttachStartCable(false);
}

#pragma endregion

#pragma region MontageSection

void UPandolFlowerComponent::PlayMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) return;
	if (!OwningPlayerAnimInstance) return;
	//if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) { return;}

	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
}

void UPandolFlowerComponent::OnFlowerNotifyStarted(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == FName("StartHooking"))
	{
		FlowerCable->HookCable(Hook_TargetLocation, Hook_TargetRotation, CharacterOwner->GetActorLocation());

	}

}

void UPandolFlowerComponent::OnFlowerMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ThrowFlowerCable_Montage)
	{
		StartHooking();
	}

	else if (Montage == HookJump_Montage)
	{
		EndHooking();
	}
}

#pragma endregion






