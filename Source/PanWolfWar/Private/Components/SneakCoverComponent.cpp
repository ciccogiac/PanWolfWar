#include "Components/SneakCoverComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <PanWolfWar/PanWolfWarCharacter.h>

#include "InputActionValue.h"

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
	Debug::Print(TEXT("Move Cover"));

	if (IsCovering)
	{
		FHitResult hit = DoWalltrace();
		if (!hit.bBlockingHit) return;

		SetCharRotation(hit.Normal);
		SetCharLocation();


		FVector2D MovementVector = Value.Get<FVector2D>().GetSafeNormal();

		
		const FVector MoveDirection = FVector( hit.Normal.Y, -hit.Normal.X, 0.f);
		CoverDirection = MovementVector.X > 0.25f ?
			 1.f : (MovementVector.X < -0.25f ? -1.f : 0.f);
		CharacterOwner->AddMovementInput(MoveDirection, CoverDirection);
		
	}
}

void USneakCoverComponent::StartCover()
{
	Debug::Print(TEXT("Start Cover"));

	FHitResult hit = DoWalltrace();
	if (!hit.bBlockingHit) return;

	SetCharRotation(hit.Normal);

	//const FVector ImpactNormal = hit.Normal;
	//const float FixCharRot = UKismetMathLibrary::MakeRotFromX(ImpactNormal).Yaw + 180.f;
	//CharacterOwner->SetActorRotation(FRotator(0.f, FixCharRot, 0.f));

	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = false;
	CharacterOwner->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 100.f;

	IsCovering = true;
	Activate();
	PanWolfCharacter->AddMappingContext(SneakCoverMappingContext, 2);
}

void USneakCoverComponent::StopCover()
{
	Debug::Print(TEXT("Stop Cover"));

	IsCovering = false;
	Deactivate();
	PanWolfCharacter->RemoveMappingContext(SneakCoverMappingContext);

	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = true;
	CharacterOwner->GetCharacterMovement()->bUseControllerDesiredRotation = true;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = 500.f;
}

const FHitResult USneakCoverComponent::DoWalltrace()
{
	const FVector Start = CharacterOwner->GetActorLocation();
	FHitResult hit;
	TArray<TEnumAsByte<EObjectTypeQuery> > WorldStaticObjectTypes;
	WorldStaticObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery1);

	UKismetSystemLibrary::CapsuleTraceSingleForObjects(this, Start, Start, 41, 30, WorldStaticObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, hit, true);
	return hit;
}

void USneakCoverComponent::SetCharRotation(const FVector ImpactNormal)
{
	const float FixCharRot = UKismetMathLibrary::MakeRotFromX(ImpactNormal).Yaw + 180.f;
	CharacterOwner->SetActorRotation(FRotator(0.f, FixCharRot, 0.f));
}

void USneakCoverComponent::SetCharLocation()
{
	const FVector NewLocation = CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * (0.5f);
	CharacterOwner->SetActorLocation(FVector(NewLocation.X, NewLocation.Y, CharacterOwner->GetActorLocation().Z));
}

