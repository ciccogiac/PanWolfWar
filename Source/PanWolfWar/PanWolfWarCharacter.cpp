// Copyright Epic Games, Inc. All Rights Reserved.

#include "PanWolfWarCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "MotionWarpingComponent.h"
#include "Components/ClimbingComponent.h"
#include "Components/AttributeComponent.h"

#include "DebugHelper.h"


#include "Actors/InteractableObject.h"
#include "Components/InteractComponent.h"

#include "Components/TransformationComponent.h"

#include "NiagaraComponent.h"

#include "Components/PandolfoComponent.h"
#include "Components/PanWolfComponent.h"
#include "Components/PandolFlowerComponent.h"
#include "Components/PanBirdComponent.h"

#include "Engine/SkinnedAssetCommon.h"

#include "Components/KiteComponent.h"
#include "Components/SneakCoverComponent.h"

#include "Components/WidgetComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// APanWolfWarCharacter

#pragma region EngineFunctions

APanWolfWarCharacter::APanWolfWarCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create a Niagara Components
	NiagaraTransformation = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraTransformation"));
	NiagaraTransformation->SetupAttachment(GetMesh());

	NiagaraApplyTransformationEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraApplyTransformationEffect"));
	NiagaraApplyTransformationEffect->SetupAttachment(GetMesh());

	// Create a Actor Components
	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComp"));

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	InteractComponent = CreateDefaultSubobject<UInteractComponent>(TEXT("InteractComponent"));
	TransformationComponent = CreateDefaultSubobject<UTransformationComponent>(TEXT("TransformationComponent"));

	PandolfoComponent = CreateDefaultSubobject<UPandolfoComponent>(TEXT("PandolfoComponent"));
	PanWolfComponent = CreateDefaultSubobject<UPanWolfComponent>(TEXT("PanWolfComponent"));
	PandolFlowerComponent = CreateDefaultSubobject<UPandolFlowerComponent>(TEXT("PandolFlowerComponent"));
	PanBirdComponent = CreateDefaultSubobject<UPanBirdComponent>(TEXT("PanBirdComponent"));

	PlayerHidingWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerHidingWidget"));
	if (PlayerHidingWidget)
	{
		PlayerHidingWidget->SetWidgetSpace(EWidgetSpace::Screen);
		PlayerHidingWidget->SetVisibility(false);
		PlayerHidingWidget->SetupAttachment(GetRootComponent());
	}


	PlayerSeenWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PlayerSeenWidget"));
	if (PlayerSeenWidget)
	{
		PlayerSeenWidget->SetWidgetSpace(EWidgetSpace::Screen);
		PlayerSeenWidget->SetVisibility(false);
		PlayerSeenWidget->SetupAttachment(GetRootComponent());
	}
}

void APanWolfWarCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();


	AddMappingContext(DefaultMappingContext, 0);

}

#pragma endregion

void APanWolfWarCharacter::SetTransformationCharacter(TObjectPtr<USkeletalMesh> SkeletalMeshAsset, TSubclassOf<UAnimInstance> Anim)
{

	GetMesh()->SetSkeletalMeshAsset(SkeletalMeshAsset);

	if (SkeletalMeshAsset)
	{
		GetMesh()->SetSkeletalMesh(SkeletalMeshAsset);

		// Aggiorna i materiali
		const TArray<FSkeletalMaterial>& NewMaterials = SkeletalMeshAsset->GetMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < NewMaterials.Num(); ++MaterialIndex)
		{
			GetMesh()->SetMaterial(MaterialIndex, NewMaterials[MaterialIndex].MaterialInterface);
		}

	}

	GetMesh()->SetAnimInstanceClass(Anim);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()));

}

//////////////////////////////////////////////////////////////////////////
// Input

#pragma region Input

void APanWolfWarCharacter::AddMappingContext(UInputMappingContext* MappingContextToAdd, int32 Priority)
{
	if (!MappingContextToAdd) return;

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContextToAdd, Priority);
		}
	}

}

void APanWolfWarCharacter::RemoveMappingContext(UInputMappingContext* MappingContextToRemove)
{
	if (!MappingContextToRemove) return;

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(MappingContextToRemove);
		}
	}
}

void APanWolfWarCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{

		#pragma region DefaultAction

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APanWolfWarCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APanWolfWarCharacter::Look);

		// Jumping
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Interact
		EnhancedInputComponent->BindAction(InteractComponent->InteractAction, ETriggerEvent::Started, InteractComponent, &UInteractComponent::Interact);
		EnhancedInputComponent->BindAction(InteractComponent->InteractMoveAction, ETriggerEvent::Triggered, InteractComponent, &UInteractComponent::InteractMove);

		//Transformation
		EnhancedInputComponent->BindAction(TransformationComponent->AnnulTransformationAction, ETriggerEvent::Started, TransformationComponent, &UTransformationComponent::AnnulTrasnformation);

		#pragma endregion

		#pragma region Pandolfo

		//PandolfoAction
		EnhancedInputComponent->BindAction(PandolfoComponent->Pandolfo_JumpAction, ETriggerEvent::Started, PandolfoComponent, &UPandolfoComponent::Jump);
		EnhancedInputComponent->BindAction(PandolfoComponent->Pandolfo_CrouchAction, ETriggerEvent::Completed, PandolfoComponent, &UPandolfoComponent::Crouch);
		EnhancedInputComponent->BindAction(PandolfoComponent->Pandolfo_SlidingAction, ETriggerEvent::Completed, PandolfoComponent, &UPandolfoComponent::Sliding);
		EnhancedInputComponent->BindAction(PandolfoComponent->Pandolfo_AssassinAction, ETriggerEvent::Completed, PandolfoComponent, &UPandolfoComponent::Assassination);

		// Climbing
		EnhancedInputComponent->BindAction(PandolfoComponent->GetClimbingComponent()->ToggleClimbAction, ETriggerEvent::Started, PandolfoComponent->GetClimbingComponent(), &UClimbingComponent::ToggleClimbing);
		EnhancedInputComponent->BindAction(PandolfoComponent->GetClimbingComponent()->ClimbMoveAction, ETriggerEvent::Triggered, PandolfoComponent->GetClimbingComponent(), &UClimbingComponent::ClimbMove);
		EnhancedInputComponent->BindAction(PandolfoComponent->GetClimbingComponent()->ClimbMoveAction, ETriggerEvent::Completed, PandolfoComponent->GetClimbingComponent(), &UClimbingComponent::ClimbMoveEnd);
		EnhancedInputComponent->BindAction(PandolfoComponent->GetClimbingComponent()->ClimbJumpAction, ETriggerEvent::Started, PandolfoComponent->GetClimbingComponent(), &UClimbingComponent::ClimbJump);
		EnhancedInputComponent->BindAction(PandolfoComponent->GetClimbingComponent()->ClimbDownAction, ETriggerEvent::Started, PandolfoComponent->GetClimbingComponent(), &UClimbingComponent::ClimbDownActivate);
		EnhancedInputComponent->BindAction(PandolfoComponent->GetClimbingComponent()->ClimbDownAction, ETriggerEvent::Completed, PandolfoComponent->GetClimbingComponent(), &UClimbingComponent::ClimbDownDeActivate);

		// Transformation
		EnhancedInputComponent->BindAction(PandolfoComponent->TransformationSelectUPAction, ETriggerEvent::Started, TransformationComponent, &UTransformationComponent::SelectUPTransformation);
		EnhancedInputComponent->BindAction(PandolfoComponent->TransformationSelectRightAction, ETriggerEvent::Started, TransformationComponent, &UTransformationComponent::SelectRightTransformation);
		EnhancedInputComponent->BindAction(PandolfoComponent->TransformationSelectLeftAction, ETriggerEvent::Started, TransformationComponent, &UTransformationComponent::SelectLeftTransformation);

		// SneakCover
		EnhancedInputComponent->BindAction(PandolfoComponent->GetSneakCoverComponent()->SneakCoverMoveAction, ETriggerEvent::Triggered, PandolfoComponent->GetSneakCoverComponent(), &USneakCoverComponent::CoverMove);
		EnhancedInputComponent->BindAction(PandolfoComponent->GetSneakCoverComponent()->JumpCoverAction, ETriggerEvent::Started, PandolfoComponent->GetSneakCoverComponent(), &USneakCoverComponent::JumpCover);

		// Kite
		EnhancedInputComponent->BindAction(PandolfoComponent->GetKiteComponent()->KiteMoveAction, ETriggerEvent::Triggered, PandolfoComponent->GetKiteComponent(), &UKiteComponent::KiteMove);	
		EnhancedInputComponent->BindAction(PandolfoComponent->GetKiteComponent()->KiteJumpAction, ETriggerEvent::Started, PandolfoComponent->GetKiteComponent(), &UKiteComponent::KiteJump);
		#pragma endregion

		#pragma region PandolFlower

		EnhancedInputComponent->BindAction(PandolFlowerComponent->MoveAction, ETriggerEvent::Triggered, PandolFlowerComponent, &UPandolFlowerComponent::Move);
		EnhancedInputComponent->BindAction(PandolFlowerComponent->HookAction, ETriggerEvent::Started, PandolFlowerComponent, &UPandolFlowerComponent::Hook);
		EnhancedInputComponent->BindAction(PandolFlowerComponent->JumpAction, ETriggerEvent::Started, PandolFlowerComponent, &UPandolFlowerComponent::Jump);

		#pragma endregion

		#pragma region PanBird

		EnhancedInputComponent->BindAction(PanBirdComponent->BirdMoveAction, ETriggerEvent::Triggered, PanBirdComponent, &UPanBirdComponent::Move);

		#pragma endregion


	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

#pragma endregion

#pragma region InputCallback

void APanWolfWarCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APanWolfWarCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>() ;

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APanWolfWarCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (PandolfoComponent->IsActive())
	{		
		PandolfoComponent->HandleLand();
	}
}

void APanWolfWarCharacter::Falling()
{
	//Debug::Print(TEXT("Start Falling"));
	if (PandolfoComponent->IsActive())
	{
		PandolfoComponent->HandleFalling();
	}
}

#pragma endregion


//////////////////////////////////////////////////////////////////////////
// Interfaces

#pragma region Interfaces

bool APanWolfWarCharacter::SetOverlappingObject(AInteractableObject* InteractableObject, bool bEnter)
{
	if (InteractableObject->ActorHasTag(FName("Pandolfo_Object")) && !PandolfoComponent->IsActive()) return false;
	if (InteractableObject->ActorHasTag(FName("PanWolf_Object")) && !PanWolfComponent->IsActive()) return false;
	if (InteractableObject->ActorHasTag(FName("PandolFlower_Object")) && !PandolFlowerComponent->IsActive()) return false;
	if (InteractableObject->ActorHasTag(FName("PanBird_Object")) && !PanBirdComponent->IsActive()) return false;

	return InteractComponent->SetOverlappingObject(InteractableObject, bEnter);
}



#pragma endregion


void APanWolfWarCharacter::SetMotionWarpTarget(const FName& InWarpTargetName, const FVector& InTargetPosition, const FRotator& InTargetRotation)
{
	if (!MotionWarpingComponent) return;

	if (InTargetRotation != FRotator::ZeroRotator)
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(InWarpTargetName, InTargetPosition, InTargetRotation);
	else
		MotionWarpingComponent->AddOrUpdateWarpTargetFromLocation(InWarpTargetName, InTargetPosition);
}


void APanWolfWarCharacter::SetIsHiding(bool Value, bool DoCrouchCheck)
{
	if (Value && !EnemyAware.IsEmpty())
		return;

	if (DoCrouchCheck && Value && !GetMovementComponent()->IsCrouching())
		return;

	bIsHiding = Value;
	PlayerHidingWidget->SetVisibility(bIsHiding);
}

void APanWolfWarCharacter::AddEnemyAware(AActor* Enemy)
{	
	EnemyAware.AddUnique(Enemy);
	if (EnemyAware.Num() == 1)
	{
		PlayerSeenWidget->SetVisibility(true);
	}
		
}

void APanWolfWarCharacter::RemoveEnemyAware(AActor* Enemy)
{	
	EnemyAware.Remove(Enemy);
	if (EnemyAware.IsEmpty())
	{
		PlayerSeenWidget->SetVisibility(false);
	}
}
		
