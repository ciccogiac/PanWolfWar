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


void USneakCoverComponent::CoverMove(const FInputActionValue& Value)
{

	if (IsCovering)
	{
		FVector2D MovementVector = Value.Get<FVector2D>().GetSafeNormal();



		CoverDirection = MovementVector.X > 0.25f ?
			1.f : (MovementVector.X < -0.25f ? -1.f : 0.f);

		if (CoverDirection == 0.f && MovementVector.Y > 0.25f)
			CoverDirection = LastCoverDirection;
		else if (CoverDirection == 0.f && MovementVector.Y < -0.25f)
			CoverDirection = -LastCoverDirection;
		else
		LastCoverDirection = CoverDirection;

		const FHitResult hit = DoWalltrace(40.f, CoverDirection);
		if (!hit.bBlockingHit)
		{
			Debug::Print(TEXT("Esco"));
			ExitCover();
			return;
		}

		



		const FVector MoveDirection = FVector(hit.Normal.Y, -hit.Normal.X, 0.f);
		//CoverDirection = MovementVector.Y > 0.25f ?
		//	1.f : (MovementVector.Y < -0.25f ? -1.f : CoverDirection);

		/*if (MovementVector.Y > 0.25f)
			JumpCover();*/

		

		//FindWall to Block Movement
		const FVector CrouchStart = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 17.5f;
		//const FVector CrouchEnd = CrouchStart + MoveDirection * CoverDirection * 37.5f * 2;
		FHitResult Crouchhit;
		//UKismetSystemLibrary::LineTraceSingle(this, CrouchStart, CrouchEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, Crouchhit, true);
		//if (Crouchhit.bBlockingHit)
		//	return;

		//Find NoFloor To block Movement
		//const FVector FloorStart = CrouchStart + MoveDirection * CoverDirection * 30.f + CharacterOwner->GetActorForwardVector() * 10.f;
		const FVector FloorStart = CrouchStart + CharacterOwner->GetActorRightVector() * CoverDirection * 30.f ;
		const FVector FloorEnd = FloorStart - CharacterOwner->GetActorUpVector() * (CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 25.f);
		UKismetSystemLibrary::SphereTraceSingle(this, FloorStart, FloorEnd,15.f, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, Crouchhit, true);
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

	FHitResult hit = DoWalltrace(40.f);
	if (!hit.bBlockingHit) { return;}

	SetCharRotation(hit.Normal);

	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = false;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 100.f;

	IsCovering = true;
	PanWolfCharacter->AddMappingContext(SneakCoverMappingContext, 2);

	PandolfoComponent->PandolfoState = EPandolfoState::EPS_Covering;

	LastCoverDirection = 0.f;
}

void USneakCoverComponent::EnterCover()
{
	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = false;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 100.f;

	IsCovering = true;
	PanWolfCharacter->AddMappingContext(SneakCoverMappingContext, 2);

	PandolfoComponent->PandolfoState = EPandolfoState::EPS_Covering;

	LastCoverDirection = 0.f;

	SetCharRotation(SavedAttachNormal);
	SetCharLocation(SavedAttachPoint, SavedAttachNormal);

	CharacterOwner->EnableInput(CharacterOwner->GetLocalViewingPlayerController());

}

bool USneakCoverComponent::CanEnterCover(const FVector StartPoint)
{
	const FVector Start = StartPoint + CharacterOwner->GetActorForwardVector() * 35.f ;
	const FVector End = Start + CharacterOwner->GetActorForwardVector() * 0.1f;
	FHitResult hit;

	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, 40.f, WorldStaticObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, hit, true);
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

	if (CharacterOwner->bIsCrouched)
	{
		return;
	}

	int N_SpaceIterarion = 6;
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
				UKismetSystemLibrary::LineTraceSingle(this, CapsuleSpaceStart, CapsuleSpaceEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, CapsuleSpaceHit, true);
				if (!CapsuleSpaceHit.bBlockingHit || CapsuleSpaceHit.bStartPenetrating)
				{
					isSpace = false;
					break;
				}
					
			}

			if (!isSpace)
				break;
			
			if ( i == N_SpaceIterarion - 1)
			{				

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

	PandolfoComponent->PandolfoState = EPandolfoState::EPS_Pandolfo;
}

const FHitResult USneakCoverComponent::DoWalltrace(float TraceRadius, float Direction)
{
	const FVector Start = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 35.f + CharacterOwner->GetActorRightVector() * Direction * 20.f;
	const FVector End = Start + CharacterOwner->GetActorForwardVector() * 0.1f;
	FHitResult hit;

	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, TraceRadius, WorldStaticObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, hit, true);
	return hit;
}

bool USneakCoverComponent::CheckCanTurn(const FVector TurnPoint)
{
	const FVector End = TurnPoint + CharacterOwner->GetActorRightVector() * 0.1f;
	FHitResult hit;

	UKismetSystemLibrary::SphereTraceSingleForObjects(this, TurnPoint, End, 20.f, WorldStaticObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, hit, true);
	SetCharRotation(hit.Normal);
	SetCharLocation(hit.ImpactPoint, hit.Normal);
	return false;
}

bool USneakCoverComponent::CheckCrouchHeight(const float Direction)
{

	const FVector CrouchStart = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 17.5f;
	const FVector CrouchEnd = CrouchStart + CharacterOwner->GetActorRightVector() * Direction * 37.5f * 2;
	FHitResult Crouchhit;
	UKismetSystemLibrary::LineTraceSingle(this, CrouchStart, CrouchEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, Crouchhit, true);
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

		UKismetSystemLibrary::LineTraceSingle(this, Start, End, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, hit, true);

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

void USneakCoverComponent::SetCharRotation(const FVector ImpactNormal)
{
	const float FixCharRot = UKismetMathLibrary::MakeRotFromX(ImpactNormal).Yaw + 180.f;
	//CharacterOwner->SetActorRotation(FRotator(0.f, FixCharRot, 0.f));

	FRotator NewRotation = UKismetMathLibrary::RInterpTo(CharacterOwner->GetActorRotation(), FRotator(0.f, FixCharRot, 0.f), GetWorld()->GetDeltaSeconds(), 1.5f);
	CharacterOwner->SetActorRotation(NewRotation);
}

void USneakCoverComponent::SetCharLocation(const FVector HitLocation, const FVector HitNormal)
{
	 FVector NewLocation = HitLocation + UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(HitNormal)) * 45.f;
	//CharacterOwner->SetActorLocation(FVector(NewLocation.X, NewLocation.Y, CharacterOwner->GetActorLocation().Z));

	//FRotator Rotator = FRotator(Rotation.Roll, Rotation.Yaw - 180, Rotation.Roll);

	 NewLocation = UKismetMathLibrary::VInterpTo(CharacterOwner->GetActorLocation(), NewLocation, GetWorld()->GetDeltaSeconds(), 1.f);
	//FRotator NewRotation = UKismetMathLibrary::RInterpTo(CharacterOwner->GetActorRotation(), Rotator, GetWorld()->GetDeltaSeconds(), 5.f);

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

