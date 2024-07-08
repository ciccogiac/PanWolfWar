#include "Components/PandolfoComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Components/ClimbingComponent.h"

#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "GameFramework/Character.h"

UPandolfoComponent::UPandolfoComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);

	ClimbingComponent = CreateDefaultSubobject<UClimbingComponent>(TEXT("ClimbingComponent"));
}

void UPandolfoComponent::Activate(bool bReset)
{
	Super::Activate();

	PanWolfCharacter->AddMappingContext(PandolfoMappingContext, 1);


	Capsule->SetCapsuleRadius(35.f);
	Capsule->SetCapsuleHalfHeight(90.f);
	CameraBoom->TargetArmLength = 400.f;

	PanWolfCharacter->bUseControllerRotationPitch = false;
	PanWolfCharacter->bUseControllerRotationYaw = false;

	PanWolfCharacter->SetTransformationCharacter(SkeletalMeshAsset, Anim);

	PanWolfCharacter->GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Falling);
	PanWolfCharacter->GetCharacterMovement()->MaxFlySpeed = 0.f;

	ClimbingComponent->SetAnimationBindings();
}

void UPandolfoComponent::Deactivate()
{
	Super::Deactivate();

	PanWolfCharacter->RemoveMappingContext(PandolfoMappingContext);
	ClimbingComponent->Deactivate();
}

void UPandolfoComponent::BeginPlay()
{
	Super::BeginPlay();

	Capsule = PanWolfCharacter->GetCapsuleComponent();
	CameraBoom = PanWolfCharacter->GetCameraBoom();
}

void UPandolfoComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

const bool UPandolfoComponent::IsClimbing()
{
	return ClimbingComponent->IsClimbing();
}

void UPandolfoComponent::Jump()
{
	if (!ClimbingComponent->TryClimbing())
	{		
		CharacterOwner->Jump();
	}

	ClimbingComponent->Activate();

}
