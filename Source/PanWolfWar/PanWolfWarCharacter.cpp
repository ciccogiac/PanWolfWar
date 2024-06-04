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

#include "Components/ClimbingComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "DebugHelper.h"

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

	ClimbingComponent = CreateDefaultSubobject<UClimbingComponent>(TEXT("ClimbingComponent"));

}

void APanWolfWarCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	AddMappingContext(DefaultMappingContext, 0);

	// Delegates Binding 

	if (ClimbingComponent)
	{
		ClimbingComponent->OnEnterClimbStateDelegate.BindUObject(this, &ThisClass::OnPlayerEnterClimbState);
		ClimbingComponent->OnExitClimbStateDelegate.BindUObject(this, &ThisClass::OnPlayerExitClimbState);
	}

}

#pragma endregion


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
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APanWolfWarCharacter::JumpClimbTrace);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APanWolfWarCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APanWolfWarCharacter::Look);

		// Climbing

		EnhancedInputComponent->BindAction(ClimbMoveAction, ETriggerEvent::Triggered, this, &APanWolfWarCharacter::ClimbMove);
		EnhancedInputComponent->BindAction(ClimbMoveAction, ETriggerEvent::Completed, this, &APanWolfWarCharacter::ClimbMoveEnd);

		EnhancedInputComponent->BindAction(ClimbAction, ETriggerEvent::Started, this, &APanWolfWarCharacter::Climb);

		EnhancedInputComponent->BindAction(ClimbJumpAction, ETriggerEvent::Started, this, &APanWolfWarCharacter::ClimbJump);

		EnhancedInputComponent->BindAction(ClimbDownAction, ETriggerEvent::Started, this, &APanWolfWarCharacter::ClimbDownActivate);
		EnhancedInputComponent->BindAction(ClimbDownAction, ETriggerEvent::Completed, this, &APanWolfWarCharacter::ClimbDownDeActivate);
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
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APanWolfWarCharacter::JumpClimbTrace()
{
	if (!ClimbingComponent->IsClimbing() && !ClimbingComponent->TryClimbing()) { ClimbingComponent->ToggleClimbing();  ACharacter::Jump(); }
}

void APanWolfWarCharacter::ClimbMove(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{		
		ClimbingComponent->LedgeMove(Get8DirectionVector(MovementVector));
	}

}

void APanWolfWarCharacter::ClimbMoveEnd(const FInputActionValue& Value)
{
	ClimbingComponent->SetClimbDirection(0.f);
	ClimbingComponent->SetJumpSaved(false);
}

void APanWolfWarCharacter::ClimbJump()
{
	if (ClimbingComponent->GetJumpSaved())
	{
		ClimbingComponent->TryJumping();
	}
	else
		ClimbingComponent->TryClimbUpon();
}

void APanWolfWarCharacter::Climb()
{
	ClimbingComponent->ToggleClimbing();	
}

void APanWolfWarCharacter::ClimbDownActivate()
{
	ClimbingComponent->SetClimbDown(true);
}

void APanWolfWarCharacter::ClimbDownDeActivate()
{
	ClimbingComponent->SetClimbDown(false);
}


#pragma endregion

FVector2D APanWolfWarCharacter::Get8DirectionVector(const FVector2D& InputVector)
{
	FVector2D Normalized = InputVector.GetSafeNormal();
	float Angle = FMath::Atan2(Normalized.Y, Normalized.X);
	// Aggiungere 2PI se l'angolo è negativo
	if (Angle < 0)
	{
		Angle += 2.0f * PI;
	}

	// Convertire l'angolo in gradi
	float AngleDegrees = FMath::RadiansToDegrees(Angle);

	//// Determinare la direzione in base all'angolo
	if (AngleDegrees <= 30.f || AngleDegrees > 330.f)
	{
		return FVector2D(1.0f, 0.0f);  // Right
	}
	else if (AngleDegrees <= 70.f)
	{
		return FVector2D(1.0f, 1.0f);  // Up-Right
	}
	else if (AngleDegrees <= 110.f)
	{
		return FVector2D(0.0f, 1.0f);  // Up
	}
	else if (AngleDegrees <= 150.f)
	{
		return FVector2D(-1.0f, 1.0f); // Up-Left
	}
	else if (AngleDegrees <= 210.f)
	{
		return FVector2D(-1.0f, 0.0f); // Left
	}
	else if (AngleDegrees <= 250.f)
	{
		return FVector2D(-1.0f, -1.0f); // Down-Left
	}
	else if (AngleDegrees <= 290.f)
	{
		return FVector2D(0.0f, -1.0f); // Down
	}
	else if (AngleDegrees <= 330.f)
	{
		return FVector2D(1.0f, -1.0f); // Down-Right
	}

	return FVector2D(0.0f, 0.0f);  // Default, should not be reached
}

//////////////////////////////////////////////////////////////////////////
// Components Delegates

#pragma region Climbing Delegates

void APanWolfWarCharacter::OnPlayerEnterClimbState()
{
	AddMappingContext(ClimbingMappingContext, 1);
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void APanWolfWarCharacter::OnPlayerExitClimbState()
{
	RemoveMappingContext(ClimbingMappingContext);
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

#pragma endregion



