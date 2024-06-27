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


#include "CharacterActor/FlowerCable.h"
#include "CableComponent.h"

#include "MotionWarpingComponent.h"

UPandolFlowerComponent::UPandolFlowerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CharacterOwner = Cast<ACharacter>(GetOwner());
}

void UPandolFlowerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
	if(PanWolfCharacter)
	{
		FollowCamera = PanWolfCharacter->GetFollowCamera();
		MotionWarpingComponent = PanWolfCharacter->GetMotionWarpingComponent();
	}

	OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (OwningPlayerAnimInstance)
	{
		OwningPlayerAnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UPandolFlowerComponent::OnFlowerNotifyStarted);
		OwningPlayerAnimInstance->OnMontageEnded.AddDynamic(this, &UPandolFlowerComponent::OnFlowerMontageEnded);
	}

	FlowerCable = GetWorld()->SpawnActorDeferred<AFlowerCable>(BP_FlowerCable, CharacterOwner->GetActorTransform());

	FlowerCable->CableComponent->SetAttachEndToComponent(CharacterOwner->GetMesh(),FName("hand_r"));
}

void UPandolFlowerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Debug::Print(TEXT("Hello"));
	if (!bIsHooking)
	{
		SearchHookingPoint();
	}
	
}

void UPandolFlowerComponent::Hook()
{
	if (bIsHooking) return;

	if (!bDidHit) { Debug::Print(TEXT("Noo Hooking Point Find")); return; }

	if (!SearchHookableObject()) return;

	bIsHooking = true;
	//isHookingState = false;
	//Face to HookingTarget
	const FRotator NewRotation = FRotator(CharacterOwner->GetActorRotation().Pitch, Hook_TargetRotation.Yaw, CharacterOwner->GetActorRotation().Roll);
	CharacterOwner->SetActorRotation(NewRotation);
	CharacterOwner->GetCharacterMovement()->DisableMovement();

	PlayMontage(ThrowFlowerCable_Montage);

}

void UPandolFlowerComponent::SearchHookingPoint()
{
	const FVector CameraForward =  FollowCamera->GetForwardVector();
	const FVector CameraLocation = FollowCamera->GetComponentLocation();
	const FVector ActorUP = CharacterOwner->GetActorUpVector();

	const FVector Start = CameraLocation + CameraForward *820.f + ActorUP * 44.f;
	const FVector End   = CameraLocation + CameraForward * 1555.f + ActorUP * 333.f;

	UKismetSystemLibrary::SphereTraceSingle(this ,Start, End, SearchHook_Radius, HookingTraceType,false, TArray<AActor*>(),EDrawDebugTrace::None, FirstTrace,true);

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
	const FVector Start = FirstTrace.ImpactPoint + FVector(0.f,0.f,33.f);
	const FVector End = FirstTrace.ImpactPoint + FVector(0.f, 0.f, -150.f);

	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, SearchObjectHookable_Radius, HookableObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::None, SecondTrace, true);

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

void UPandolFlowerComponent::PlayMontage(UAnimMontage* MontageToPlay)
{
	if (!MontageToPlay) return;
	if (!OwningPlayerAnimInstance) return;
	//if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) { return;}

	OwningPlayerAnimInstance->Montage_Play(MontageToPlay);
}

void UPandolFlowerComponent::SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition, const FRotator& InTargetRotation)
{
	if (!MotionWarpingComponent) return;

	if (InTargetRotation != FRotator::ZeroRotator)
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(InWarpTargetName, InTargetPosition, InTargetRotation);
	else
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(InWarpTargetName, InTargetPosition);
}

void UPandolFlowerComponent::OnFlowerNotifyStarted(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if (NotifyName == FName("StartHooking"))
	{
		FlowerCable->CableComponent->bAttachStart = true;

		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		float OverTime = FVector::Distance(Hook_TargetLocation, CharacterOwner->GetActorLocation()) / 2500.f;
		UKismetSystemLibrary::MoveComponentTo(FlowerCable->CableComponent, Hook_TargetLocation, Hook_TargetRotation, true, false, OverTime, true, EMoveComponentAction::Move, LatentInfo);
	
	}

}

void UPandolFlowerComponent::OnFlowerMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ThrowFlowerCable_Montage)
	{
		StartHooking();
	}

	if (Montage == HookJump_Montage)
	{
		EndHooking();
	}
}

void UPandolFlowerComponent::StartHooking()
{
	const FVector TargetLocation =  SecondTrace.ImpactPoint + FVector(0.f, 0.f, 120.f);
	//DrawDebugPoint(GetWorld(), TargetLocation, 5.f, FColor::Magenta,false,10.f);

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	//FRotator Rotator = FRotator(0.f, ClimbRotation.Yaw, 0.f);
	float OverTime = FVector::Distance(TargetLocation, CharacterOwner->GetActorLocation()) / 1000.f;
	UKismetSystemLibrary::MoveComponentTo(CharacterOwner->GetCapsuleComponent(), TargetLocation, Hook_TargetRotation, true, false, OverTime, true, EMoveComponentAction::Move, LatentInfo);

	//SetMotionWarpTarget(FName("HookTarget"), TargetLocation, Hook_TargetRotation);

	PlayMontage(HookJump_Montage);
	//isHookingState = true;
}

void UPandolFlowerComponent::EndHooking()
{
	bIsHooking = false;
	//isHookingState = false;
	CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	FlowerCable->CableComponent->bAttachStart = false;
}

