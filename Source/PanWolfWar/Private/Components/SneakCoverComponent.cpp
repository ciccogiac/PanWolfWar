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
	PandolfoComponent = PanWolfCharacter->GetPandolfoComponent();
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
	if (!bIsNarrowing && !CharacterOwner->GetCharacterMovement()->IsCrouching() && CanEnterCover(StartLocation, true))
	{
		if (!StartNarrowMontage) return;
		UAnimInstance* OwningPlayerAnimInstance = CharacterOwner->GetMesh()->GetAnimInstance();
		if (!OwningPlayerAnimInstance) return;
		if (OwningPlayerAnimInstance->IsAnyMontagePlaying()) return;

		bIsNarrowing = true;
		FRotator R = UKismetMathLibrary::MakeRotFromX(-SavedAttachNormal);
		PanWolfCharacter->SetMotionWarpTarget(FName("PositionNarrowPoint"), FVector(NarrowPosition.X, NarrowPosition.Y, CharacterOwner->GetActorLocation().Z), R);
		PanWolfCharacter->SetMotionWarpTarget(FName("StartNarrowPoint"), FVector(SavedAttachPoint.X, SavedAttachPoint.Y, CharacterOwner->GetActorLocation().Z), R);


		CharacterOwner->GetCapsuleComponent()->SetCapsuleRadius(22.f);
		CharacterOwner->DisableInput(CharacterOwner->GetLocalViewingPlayerController());
		PanWolfCharacter->GetCameraBoom()->bUsePawnControlRotation = false;
		PanWolfCharacter->GetCameraBoom()->SetRelativeRotation(FRotator(-10.f, 90.f, 0.f));
		PanWolfCharacter->GetCameraBoom()->SetRelativeLocation(FVector(0.f, 0.f, 45.f));
		PanWolfCharacter->GetCameraBoom()->TargetArmLength = 250.f;
		OwningPlayerAnimInstance->Montage_Play(StartNarrowMontage);
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
	ExitCover();
	PanWolfCharacter->GetCameraBoom()->bUsePawnControlRotation = true;
	//PanWolfCharacter->GetCameraBoom()->SetRelativeRotation(FRotator(0.f, 0.f, 0.f));
	PanWolfCharacter->GetCameraBoom()->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
	PanWolfCharacter->GetCameraBoom()->TargetArmLength = 400.f;
	OwningPlayerAnimInstance->Montage_Play(ExitNarrowMontage);
}

void USneakCoverComponent::EnterNarrow()
{
	EnterCover(false);
}

void USneakCoverComponent::ExitNarrow()
{
	CharacterOwner->GetCapsuleComponent()->SetCapsuleRadius(35.f);
	bIsNarrowing = false;
}

#pragma endregion



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
		const FVector FloorEnd = FloorStart - CharacterOwner->GetActorUpVector() * (CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 25.f);
		UKismetSystemLibrary::SphereTraceSingle(this, FloorStart, FloorEnd,15.f, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, Crouchhit, true);
		if (!Crouchhit.bBlockingHit)
			return;


		/** Auto EXIT*/

		//int N_SpaceIterarion = 6;
		////Find Space To exit cover
		//for (size_t i = 0; i < N_SpaceIterarion; i++)
		//{
		//	FVector SpaceStart = CrouchStart  - CharacterOwner->GetActorForwardVector() * 20.f * i;
		//	bool isSpace = true;
		//	for (size_t t = 0; t < 6; t++)
		//	{
		//		FVector CapsuleSpaceStart = SpaceStart  + MoveDirection * CoverDirection * 40.f * t;
		//		FVector CapsuleSpaceEnd = CapsuleSpaceStart - CharacterOwner->GetActorUpVector() * (CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 25.f);
		//		FHitResult CapsuleSpaceHit;
		//		UKismetSystemLibrary::LineTraceSingle(this, CapsuleSpaceStart, CapsuleSpaceEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, CapsuleSpaceHit, true);
		//		if (!CapsuleSpaceHit.bBlockingHit || CapsuleSpaceHit.bStartPenetrating)
		//		{
		//			isSpace = false;
		//			break;
		//		}
		//			
		//	}

		//	if (!isSpace)
		//		break;
		//	
		//	if ( i == N_SpaceIterarion - 1)
		//	{
		//		
		//				StopCover();
		//				CharacterOwner->GetMovementComponent()->StopMovementImmediately();
		//				return;								

		//	}
		//}

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

	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = false;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 100.f;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeedCrouched = 100.f;

	IsCovering = true;
	PanWolfCharacter->AddMappingContext(SneakCoverMappingContext, 2);

	PandolfoComponent->PandolfoState = EPandolfoState::EPS_Covering;

	LastCoverDirection = 1.f;
}

void USneakCoverComponent::EnterCover(const bool SetLocRot)
{
	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = false;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 100.f;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeedCrouched = 100.f;

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
				FVector CapsuleSpaceEnd = CapsuleSpaceStart - CharacterOwner->GetActorUpVector() * (CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 25.f);
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

	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = true;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 500.f;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeedCrouched = 250.f;

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

	if (!CharacterOwner->GetCharacterMovement()->IsCrouching() && !CharacterOwner->GetCharacterMovement()->bWantsToCrouch)
	{
		FVector loc; FRotator Rot;
		CharacterOwner->GetActorEyesViewPoint(loc,Rot);
		const FVector Start = loc;
		const FVector End = Start + CharacterOwner->GetActorRightVector()* Direction * 37.5f;
		FHitResult hit;

		UKismetSystemLibrary::LineTraceSingle(this, Start, End, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), DebugTrace, hit, true);

		if (hit.bBlockingHit)
		{
			CharacterOwner->GetCharacterMovement()->bWantsToCrouch = true;
			return true;			
		}
			
	}

	else if (CharacterOwner->GetCharacterMovement()->IsCrouching() && CharacterOwner->GetCharacterMovement()->bWantsToCrouch)
	{
		//Debug::Print(TEXT("TryUnCrouch"));
		CharacterOwner->GetCharacterMovement()->bWantsToCrouch = false;
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
	if (!CharacterOwner->GetCharacterMovement()->IsCrouching() && PandolfoComponent->TryClimbOrMantle())
	{
		ExitCover();
	}
}

