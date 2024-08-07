#include "Components/KiteComponent.h"

#include "PanWolfWar/DebugHelper.h"
#include <PanWolfWar/PanWolfWarCharacter.h>
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "InputActionValue.h"
#include "Actors/KiteBoard.h"

UKiteComponent::UKiteComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;


	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
}

void UKiteComponent::KiteExit()
{
}

void UKiteComponent::KiteMove(const FInputActionValue& Value)
{
	if (!KiteBoard) return;


	FVector2D MovementVector = Value.Get<FVector2D>().GetSafeNormal();

	const float ForceY = UKismetMathLibrary::MapRangeClamped(MovementVector.Y, -1, 1, -KiteMoveForce, KiteMoveForce);

	//KiteBoard->GetBoardMesh()->AddForce(KiteBoard->GetBoardMesh()->GetForwardVector() * ForceY);

	KiteBoard->GetBoardMesh()->AddForce(CharacterOwner->GetActorForwardVector() * ForceY);

	//DrawDebugLine(GetWorld(), KiteBoard->GetBoardMesh()->GetComponentLocation() + KiteBoard->GetBoardMesh()->GetForwardVector(), KiteBoard->GetBoardMesh()->GetComponentLocation() + KiteBoard->GetBoardMesh()->GetForwardVector() * 50.f,FColor::Magenta, true);

	if (MovementVector.X == 0.f) return;

	const float X = MovementVector.Y >= 0.f ? MovementVector.X : -MovementVector.X;
	KiteBoard->GetBoardMesh()->AddWorldRotation(FRotator(0.f, X, 0.f));
}

void UKiteComponent::KiteJump()
{
	if (!KiteBoard) return;
	if (FMath::Abs(KiteBoard->GetVelocity().Z) > 10.f) return;
	
	KiteBoard->GetBoardMesh()->AddImpulse(CharacterOwner->GetActorUpVector() * KiteMoveForce/2.f);
}

void UKiteComponent::Activate(bool bReset)
{
	Super::Activate();

	PanWolfCharacter->AddMappingContext(KiteMappingContext, 2);
	CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);

	if (!KiteBoard) return;
	KiteBoard->EnableSeaPhysics();
	KiteBoard->GetBoardMesh()->SetSimulatePhysics(true);
}

void UKiteComponent::Deactivate()
{
	Super::Deactivate();

	PanWolfCharacter->RemoveMappingContext(KiteMappingContext);
	CharacterOwner->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
}

void UKiteComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UKiteComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

