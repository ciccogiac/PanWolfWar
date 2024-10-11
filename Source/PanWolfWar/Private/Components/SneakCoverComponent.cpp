#include "Components/SneakCoverComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <PanWolfWar/PanWolfWarCharacter.h>

#include "InputActionValue.h"

#include "Components/CapsuleComponent.h"

#include "Components/PandolfoComponent.h"

#include "TimerManager.h"
#include "GameFramework/SpringArmComponent.h"

#pragma region EngineFunctions

USneakCoverComponent::USneakCoverComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
}

void USneakCoverComponent::Activate(bool bReset)
{
	Super::Activate();
}

void USneakCoverComponent::Deactivate()
{
	Super::Deactivate();
}

void USneakCoverComponent::BeginPlay()
{
	Super::BeginPlay();

	if (PanWolfCharacter)
	{
		PandolfoComponent = PanWolfCharacter->GetPandolfoComponent();
		Capsule = PanWolfCharacter->GetCapsuleComponent();
		CameraBoom = PanWolfCharacter->GetCameraBoom();
		MovementComponent = CharacterOwner->GetCharacterMovement();
	}


}

void USneakCoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

#pragma endregion

#pragma region Narrow

void USneakCoverComponent::StartNarrow(const FVector StartLocation, const FVector NarrowPosition)
{
	//Debug::Print(TEXT("StartNarrow"));
	if (!bIsNarrowing && !MovementComponent->IsCrouching() && CanEnterCover(StartLocation, true))
	{
		if (!StartNarrowMontage) return;
		UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
		if (!OwningPlayerAnimInstance) return;
		if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

		bIsNarrowing = true;
		PandolfoComponent->PandolfoState = EPandolfoState::EPS_Covering;

		FRotator R = UKismetMathLibrary::MakeRotFromX(-SavedAttachNormal);
		PanWolfCharacter->SetMotionWarpTarget(FName("PositionNarrowPoint"), FVector(NarrowPosition.X, NarrowPosition.Y, CharacterOwner->GetActorLocation().Z), R);
		PanWolfCharacter->SetMotionWarpTarget(FName("StartNarrowPoint"), FVector(SavedAttachPoint.X, SavedAttachPoint.Y, CharacterOwner->GetActorLocation().Z), R);


		Capsule->SetCapsuleRadius(22.f);
		CharacterOwner->DisableInput(CharacterOwner->GetLocalViewingPlayerController());
		CameraBoom->bUsePawnControlRotation = false;
		/*CameraBoom->SetRelativeRotation(FRotator(-10.f, 90.f, 0.f));
		CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 45.f));*/

		FLatentActionInfo LatentInfo;
		LatentInfo.CallbackTarget = this;
		UKismetSystemLibrary::MoveComponentTo(CameraBoom, FVector(0.f, -45.f, 45.f), FRotator(-10.f, 90.f, 0.f), true, true, 1.5f, true, EMoveComponentAction::Move, LatentInfo);

		CameraBoom->TargetArmLength = 200.f;
		OwningPlayerAnimInstance->Montage_Play(StartNarrowMontage);

		FOnMontageEnded StartNarrowMontageEndedDelegate;
		StartNarrowMontageEndedDelegate.BindUObject(this, &USneakCoverComponent::OnStartNarrowMontageEnded);
		OwningPlayerAnimInstance->Montage_SetEndDelegate(StartNarrowMontageEndedDelegate, StartNarrowMontage);
	}
}

void USneakCoverComponent::StopNarrow(const FVector EndLocation, const FVector EndDirection)
{
	//Debug::Print(TEXT("StopNarrow"));
	if (!bIsNarrowing) return;



	if (!ExitNarrowMontage) return;
	UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
	if (!OwningPlayerAnimInstance) return;
	if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;
	FRotator R = UKismetMathLibrary::MakeRotFromX(EndDirection.GetSafeNormal());
	PanWolfCharacter->SetMotionWarpTarget(FName("EndNarrowPoint"), FVector(EndLocation.X, EndLocation.Y, CharacterOwner->GetActorLocation().Z), R);

	CameraBoom->bUsePawnControlRotation = true;
	//PanWolfCharacter->GetCameraBoom()->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	//CameraBoom->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	UKismetSystemLibrary::MoveComponentTo(CameraBoom, FVector::ZeroVector, CameraBoom->GetComponentRotation(), true, true, 1.5f, true, EMoveComponentAction::Move, LatentInfo);

	CameraBoom->TargetArmLength = PandolfoComponent->GetTargetArmLength();
	OwningPlayerAnimInstance->Montage_Play(ExitNarrowMontage);

	FOnMontageEnded StartNarrowMontageEndedDelegate;
	StartNarrowMontageEndedDelegate.BindUObject(this, &USneakCoverComponent::OnExitNarrowMontageEnded);
	OwningPlayerAnimInstance->Montage_SetEndDelegate(StartNarrowMontageEndedDelegate, ExitNarrowMontage);
}

void USneakCoverComponent::OnStartNarrowMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	EnterCover(false);
}

void USneakCoverComponent::OnExitNarrowMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	Capsule->SetCapsuleRadius(PandolfoComponent->GetCapsuleRadius());
	bIsNarrowing = false;
	ExitCover();
}

#pragma endregion

#pragma region Cover

void USneakCoverComponent::ActivateWallSearch()
{
	GetWorld()->GetTimerManager().SetTimer(WallSearch_TimerHandle, this, &USneakCoverComponent::StartCover, 0.1f, true);
}

void USneakCoverComponent::DeactivateWallSearch()
{
	GetWorld()->GetTimerManager().ClearTimer(WallSearch_TimerHandle);
}

void USneakCoverComponent::CoverMove(const FInputActionValue& Value)
{

	if (IsCovering)
	{
		FVector2D MovementVector = Value.Get<FVector2D>().GetSafeNormal();



		

		if (!bIsNarrowing) 
		{
			CoverDirection = MovementVector.X > 0.5f ?
				1.f : (MovementVector.X < -0.5f ? -1.f : 0.f);

			if (CoverDirection == 0.f && MovementVector.Y > 0.5f)
				CoverDirection = LastCoverDirection;
			else if (CoverDirection == 0.f && MovementVector.Y < -0.5f)
				CoverDirection = -LastCoverDirection;
			else
				LastCoverDirection = CoverDirection;
		}

		else
		{
			CoverDirection = MovementVector.X > 0.85f ?
				1.f : (MovementVector.X < -0.85f ? -1.f : 0.f);

			if (CoverDirection == 0.f && MovementVector.Y > 0.15f)
				CoverDirection = 1.f;
			else if (CoverDirection == 0.f && MovementVector.Y < -0.15f)
				CoverDirection = -1;
			else
				LastCoverDirection = CoverDirection;
		}
		
		if (CoverDirection == 0.f) return;
		

		const FHitResult hit = DoWalltrace(40.f, CoverDirection);
		if (!hit.bBlockingHit)
		{
			Debug::Print(TEXT("Esco"));
			ExitCover();
			return;
		}


		const FVector MoveDirection = FVector(hit.Normal.Y, -hit.Normal.X, 0.f);	

		//FindWall to Block Movement
		const FVector CrouchStart = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 17.5f;
		FHitResult Crouchhit;
		EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
		const FVector FloorStart = CrouchStart + CharacterOwner->GetActorRightVector() * CoverDirection * 30.f ;
		const FVector FloorEnd = FloorStart - CharacterOwner->GetActorUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + 25.f);
		UKismetSystemLibrary::SphereTraceSingle(this, FloorStart, FloorEnd,15.f, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, Crouchhit, true);
		if (!Crouchhit.bBlockingHit)
			return;



		if (!CheckCrouchHeight(CoverDirection))
			return;



		CharacterOwner->AddMovementInput(MoveDirection, CoverDirection);

		
		SetCharRotation(hit.Normal);
		SetCharLocation(hit.ImpactPoint, hit.Normal);
		
	}
}

void USneakCoverComponent::StartCover()
{
	//Debug::Print(TEXT("Start Cover"));

	FHitResult hit = DoWalltrace(35.f);
	if (!hit.bBlockingHit) { return;}

	DeactivateWallSearch();

	SetCharRotation(hit.ImpactNormal,true);
	SetCharLocation(hit.ImpactPoint, hit.ImpactNormal, true);

	MovementComponent->bOrientRotationToMovement = false;
	MovementComponent->MaxWalkSpeed = 100.f;
	MovementComponent->MaxWalkSpeedCrouched = 100.f;

	IsCovering = true;
	PanWolfCharacter->AddMappingContext(SneakCoverMappingContext, 2);

	PandolfoComponent->PandolfoState = EPandolfoState::EPS_Covering;

	LastCoverDirection = 1.f;
}

void USneakCoverComponent::EnterCover(const bool SetLocRot)
{
	MovementComponent->bOrientRotationToMovement = false;
	MovementComponent->MaxWalkSpeed = 90.f;
	MovementComponent->MaxWalkSpeedCrouched = 80.f;

	IsCovering = true;
	PanWolfCharacter->AddMappingContext(SneakCoverMappingContext, 2);

	PandolfoComponent->PandolfoState = EPandolfoState::EPS_Covering;

	//LastCoverDirection = 0.f;
	if (SetLocRot)
	{
		SetCharRotation(SavedAttachNormal, true);
		SetCharLocation(SavedAttachPoint, SavedAttachNormal, true);
	}

	LastCoverDirection = 1.f;
	CharacterOwner->EnableInput(CharacterOwner->GetLocalViewingPlayerController());
}

bool USneakCoverComponent::CanEnterCover(const FVector StartPoint,bool bNarrowCover)
{
	const FVector ActorForward = bNarrowCover ? FVector(0.f, 0.f, 0.f) : CharacterOwner->GetActorForwardVector() * 35.f ;
	const FVector Start = StartPoint + ActorForward;
	const FVector ActorEndForward = bNarrowCover ?  FVector(0.f, 0.f, 0.1f) : CharacterOwner->GetActorForwardVector() * 0.1f;
	const FVector End = Start + ActorEndForward;
	FHitResult hit;
	EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, 40.f, WorldStaticObjectTypes, false, TArray<AActor*>(), DebugTrace, hit, true);
	
	if (hit.bBlockingHit)
	{
		SavedAttachNormal = hit.Normal;
		SavedAttachPoint = hit.ImpactPoint;
	}
		
	return hit.bBlockingHit;
}

void USneakCoverComponent::StopCover()
{
	//Debug::Print(TEXT("Stop Cover"));

	/*if (CharacterOwner->bIsCrouched)
	{
		return;
	}*/

	//Make a new Check lines for crouched situation

	int N_SpaceIterarion = 6;
	EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
	FVector EndPositionRight;
	FVector EndPositionLeft;
		//Find Space To exit cover
		for (size_t i = 0; i < N_SpaceIterarion; i++)
		{
			FVector SpaceStart = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 17.5f - CharacterOwner->GetActorForwardVector() * 20.f * i;
			bool isSpace = true;
			for (size_t t = 0; t < 5; t++)
			{
				FVector CapsuleSpaceStart = SpaceStart - CharacterOwner->GetActorRightVector() * 30.f * 2  + CharacterOwner->GetActorRightVector() * 30.f * t;
				FVector CapsuleSpaceEnd = CapsuleSpaceStart - CharacterOwner->GetActorUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + 25.f);
				FHitResult CapsuleSpaceHit;
				UKismetSystemLibrary::LineTraceSingle(this, CapsuleSpaceStart, CapsuleSpaceEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, CapsuleSpaceHit, true);
				if (!CapsuleSpaceHit.bBlockingHit || CapsuleSpaceHit.bStartPenetrating)
				{
					isSpace = false;
					break;
				}
				if (i == 4 && t == 4) EndPositionRight = CapsuleSpaceHit.ImpactPoint;
				if (i == 4 && t == 0) EndPositionLeft = CapsuleSpaceHit.ImpactPoint;
			}

			if (!isSpace)
				break;
			
			if ( i == N_SpaceIterarion - 1)
			{				
				UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
				if (ExitCoverMontage && OwningPlayerAnimInstance && !OwningPlayerAnimInstance->IsAnyMontagePlaying())
				{
					const FVector EndPosition = CoverDirection >= 0.f ? EndPositionRight : EndPositionLeft;
					PanWolfCharacter->SetMotionWarpTarget(FName("StopCoverPoint"), EndPosition, CharacterOwner->GetActorRotation() + FRotator(0.f,90.f * CoverDirection,0.f ));
					OwningPlayerAnimInstance->Montage_Play(ExitCoverMontage);
				}
					
				ExitCover();
				return;								

			}
		}
		
}

void USneakCoverComponent::ExitCover()
{
	IsCovering = false;
	PanWolfCharacter->RemoveMappingContext(SneakCoverMappingContext);

	MovementComponent->bOrientRotationToMovement = true;
	MovementComponent->MaxWalkSpeed = PandolfoComponent->GetMaxWalkSpeed();
	MovementComponent->MaxWalkSpeedCrouched = PandolfoComponent->GetMaxWalkSpeedCrouched();

	PandolfoComponent->PandolfoState = EPandolfoState::EPS_Pandolfo;
}

const FHitResult USneakCoverComponent::DoWalltrace(float TraceRadius, float Direction)
{
	const FVector Start = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 35.f + CharacterOwner->GetActorRightVector() * Direction * 20.f;
	const FVector End = Start + CharacterOwner->GetActorForwardVector() * 0.1f;
	FHitResult hit;
	EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, TraceRadius, WorldStaticObjectTypes, false, TArray<AActor*>(), DebugTrace, hit, true);
	return hit;
}

bool USneakCoverComponent::CheckCanTurn(const FVector TurnPoint)
{
	const FVector End = TurnPoint + CharacterOwner->GetActorRightVector() * 0.1f;
	FHitResult hit;
	EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	UKismetSystemLibrary::SphereTraceSingleForObjects(this, TurnPoint, End, 20.f, WorldStaticObjectTypes, false, TArray<AActor*>(), DebugTrace, hit, true);
	SetCharRotation(hit.Normal);
	SetCharLocation(hit.ImpactPoint, hit.Normal);
	return false;
}

bool USneakCoverComponent::CheckCrouchHeight(const float Direction)
{

	const FVector CrouchStart = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 17.5f;
	const FVector CrouchEnd = CrouchStart + CharacterOwner->GetActorRightVector() * Direction * 37.5f * 2;
	FHitResult Crouchhit;
	EDrawDebugTrace::Type DebugTrace = ShowDebugTrace ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
	UKismetSystemLibrary::LineTraceSingle(this, CrouchStart, CrouchEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, Crouchhit, true);
	if (Crouchhit.bBlockingHit)
	{
		//Check if can turn
		CheckCanTurn(Crouchhit.ImpactPoint);
		return false;
	}

	if (!MovementComponent->IsCrouching() && !MovementComponent->bWantsToCrouch)
	{
		FVector loc; FRotator Rot;
		CharacterOwner->GetActorEyesViewPoint(loc,Rot);
		const FVector Start = loc;
		const FVector End = Start + CharacterOwner->GetActorRightVector()* Direction * 37.5f;
		FHitResult hit;

		UKismetSystemLibrary::LineTraceSingle(this, Start, End, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, hit, true);

		if (hit.bBlockingHit)
		{
			MovementComponent->bWantsToCrouch = true;
			return true;			
		}
			
	}

	else if (MovementComponent->IsCrouching() && MovementComponent->bWantsToCrouch)
	{
		//Debug::Print(TEXT("TryUnCrouch"));
		MovementComponent->bWantsToCrouch = false;
		return true;
	}

	return true;
}

void USneakCoverComponent::SetCharRotation(const FVector ImpactNormal, bool Istantaneus)
{
	FRotator NewRotation = FRotator(0.f, UKismetMathLibrary::MakeRotFromX(ImpactNormal).Yaw + 180.f, 0.f) ;

	NewRotation = Istantaneus ? NewRotation : UKismetMathLibrary::RInterpTo(CharacterOwner->GetActorRotation(), NewRotation, GetWorld()->GetDeltaSeconds(), 1.5f);
	CharacterOwner->SetActorRotation(NewRotation);
}

void USneakCoverComponent::SetCharLocation(const FVector HitLocation, const FVector HitNormal, bool Istantaneus)
{
	 FVector NewLocation = HitLocation + UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(HitNormal)) * 45.f;
	 NewLocation = Istantaneus ? NewLocation :  UKismetMathLibrary::VInterpTo(CharacterOwner->GetActorLocation(), NewLocation, GetWorld()->GetDeltaSeconds(), 1.f);

	CharacterOwner->SetActorLocation(NewLocation);
}

void USneakCoverComponent::JumpCover()
{
	//Debug::Print(TEXT("JumpCover"));
	if (!MovementComponent->IsCrouching() && PandolfoComponent->TryClimbOrMantle())
	{
		ExitCover();
	}
}

#pragma endregion