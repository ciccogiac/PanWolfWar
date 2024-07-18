#include "Components/SneakCoverComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <PanWolfWar/PanWolfWarCharacter.h>

#include "InputActionValue.h"

#include "Components/CapsuleComponent.h"

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
		const FHitResult hit = DoWalltrace();
		if (!hit.bBlockingHit)
		{
			StopCover();
			return;
		}

		

		FVector2D MovementVector = Value.Get<FVector2D>().GetSafeNormal();

		
		const FVector MoveDirection = FVector( hit.Normal.Y, -hit.Normal.X, 0.f);
		CoverDirection = MovementVector.X > 0.25f ?
			 1.f : (MovementVector.X < -0.25f ? -1.f : 0.f);
		CoverDirection = MovementVector.Y > 0.25f ?
			1.f : (MovementVector.Y < -0.25f ? -1.f : CoverDirection);


		const FVector CrouchStart = CharacterOwner->GetActorLocation();
		const FVector CrouchEnd = CrouchStart + MoveDirection * CoverDirection * 37.5f * 2;
		FHitResult Crouchhit;
		UKismetSystemLibrary::LineTraceSingle(this, CrouchStart, CrouchEnd, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, Crouchhit, true);
		if (Crouchhit.bBlockingHit)
			return;
		

		CheckCrouchHeight(MoveDirection * CoverDirection);

		CharacterOwner->AddMovementInput(MoveDirection, CoverDirection);

		SetCharRotation(hit.Normal);
		SetCharLocation(hit.ImpactPoint, hit.Normal);
		
	}
}

void USneakCoverComponent::StartCover()
{
	Debug::Print(TEXT("Start Cover"));

	FHitResult hit = DoWalltrace(40.f);
	if (!hit.bBlockingHit) return;

	SetCharRotation(hit.Normal);

	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = false;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 100.f;

	IsCovering = true;
	PanWolfCharacter->AddMappingContext(SneakCoverMappingContext, 2);
}

void USneakCoverComponent::StopCover()
{
	Debug::Print(TEXT("Stop Cover"));

	IsCovering = false;
	PanWolfCharacter->RemoveMappingContext(SneakCoverMappingContext);

	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = true;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 500.f;
}

const FHitResult USneakCoverComponent::DoWalltrace(float TraceRadius)
{
	const FVector Start = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * 35.f;
	const FVector End = Start + CharacterOwner->GetActorForwardVector() * 0.1f;
	FHitResult hit;
	TArray<TEnumAsByte<EObjectTypeQuery> > WorldStaticObjectTypes;
	WorldStaticObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);

	UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, TraceRadius, WorldStaticObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, hit, true);
	return hit;
}

void USneakCoverComponent::CheckCrouchHeight(const FVector Direction)
{
	if (!CharacterOwner->GetCharacterMovement()->IsCrouching() && !CharacterOwner->GetCharacterMovement()->bWantsToCrouch)
	{
		FVector loc; FRotator Rot;
		CharacterOwner->GetActorEyesViewPoint(loc,Rot);
		const FVector Start = loc;
		const FVector End = Start + Direction * 37.5f;
		FHitResult hit;
		TArray<TEnumAsByte<EObjectTypeQuery> > WorldStaticObjectTypes;
		WorldStaticObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);

		UKismetSystemLibrary::LineTraceSingle(this, Start, End, ETraceTypeQuery::TraceTypeQuery1, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, hit, true);

		if(hit.bBlockingHit)
			CharacterOwner->GetCharacterMovement()->bWantsToCrouch = true;
	}

	else if (CharacterOwner->GetCharacterMovement()->IsCrouching() && CharacterOwner->GetCharacterMovement()->bWantsToCrouch)
	{
		Debug::Print(TEXT("TryUnCrouch"));
		CharacterOwner->GetCharacterMovement()->bWantsToCrouch = false;
	}

}

void USneakCoverComponent::SetCharRotation(const FVector ImpactNormal)
{
	const float FixCharRot = UKismetMathLibrary::MakeRotFromX(ImpactNormal).Yaw + 180.f;
	CharacterOwner->SetActorRotation(FRotator(0.f, FixCharRot, 0.f));
}

void USneakCoverComponent::SetCharLocation(const FVector HitLocation, const FVector HitNormal)
{
	const FVector NewLocation = HitLocation + UKismetMathLibrary::GetForwardVector(UKismetMathLibrary::MakeRotFromX(HitNormal)) * 45.f;
	CharacterOwner->SetActorLocation(FVector(NewLocation.X, NewLocation.Y, CharacterOwner->GetActorLocation().Z));

	//FLatentActionInfo LatentInfo;
	//LatentInfo.CallbackTarget = this;
	//FRotator Rotator = FRotator(0.f, UKismetMathLibrary::MakeRotFromX(HitNormal).Yaw -180.f, 0.f);
	//const FVector LedgeLocation = FVector(NewLocation.X, NewLocation.Y, CharacterOwner->GetActorLocation().Z);
	//float OverTime = FVector::Distance(LedgeLocation, CharacterOwner->GetActorLocation()) / 2500000000.f;
	//UKismetSystemLibrary::MoveComponentTo(CharacterOwner->GetCapsuleComponent(), LedgeLocation, Rotator, true, false, OverTime, true, EMoveComponentAction::Move, LatentInfo);
}

