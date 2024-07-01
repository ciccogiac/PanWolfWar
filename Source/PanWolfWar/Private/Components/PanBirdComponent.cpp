#include "Components/PanBirdComponent.h"
#include <PanWolfWar/PanWolfWarCharacter.h>
#include "GameFramework/CharacterMovementComponent.h"

#include "InputActionValue.h"

#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "PanWolfWar/DebugHelper.h"

UPanBirdComponent::UPanBirdComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	PanWolfCharacter = Cast<APanWolfWarCharacter>(GetOwner());

}

void UPanBirdComponent::BeginPlay()
{
	Super::BeginPlay();

	Capsule = PanWolfCharacter->GetCapsuleComponent();
	CameraBoom = PanWolfCharacter->GetCameraBoom();
}

void UPanBirdComponent::Activate(bool bReset)
{
	Super::Activate();

	PanWolfCharacter->AddMappingContext(PanBirdMappingContext, 1);

	Capsule->SetCapsuleRadius(15.f);
	Capsule->SetCapsuleHalfHeight(20.f);
	CameraBoom->TargetArmLength = 300.f;

	PanWolfCharacter->bUseControllerRotationPitch = true;
	PanWolfCharacter->bUseControllerRotationYaw = true;

	PanWolfCharacter->SetTransformationCharacter(SkeletalMeshAsset, Anim);

	PanWolfCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Flying);
	PanWolfCharacter->GetCharacterMovement()->MaxFlySpeed = 500.f;

}

void UPanBirdComponent::Deactivate()
{
	Super::Deactivate();

	PanWolfCharacter->RemoveMappingContext(PanBirdMappingContext);
}




void UPanBirdComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPanBirdComponent::Move(const FInputActionValue& Value)
{

	FVector2D MovementVector = Value.Get<FVector2D>();
	const float DirectionValue = MovementVector.Y;
	//const float DirectionValue = Value.Get<float>();

	if (PanWolfCharacter->Controller != nullptr && (DirectionValue > 0.f))
	{

		FVector Forward = PanWolfCharacter->GetActorForwardVector();
		PanWolfCharacter->AddMovementInput(Forward, DirectionValue);
	}
}

