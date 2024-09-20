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
#include "Components/Combat/PandoCombatComponent.h"
#include "Components/TargetingComponent.h"

#include "NiagaraComponent.h"

#include "Components/PandolfoComponent.h"
#include "Components/PanWolfComponent.h"
#include "Components/PandolFlowerComponent.h"
#include "Components/PanBirdComponent.h"

#include "Engine/SkinnedAssetCommon.h"

#include "Components/KiteComponent.h"
#include "Components/SneakCoverComponent.h"

#include "Components/WidgetComponent.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#include "Components/BoxComponent.h"

#include "GameModes/PanWarSurvivalGameMode.h"

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

	HidingAssassinBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("HidingAssassinBox"));
	HidingAssassinBoxComponent->SetupAttachment(RootComponent);
	HidingAssassinBoxComponent->bHiddenInGame = true;
	HidingAssassinBoxComponent->SetLineThickness(2.f);
	HidingAssassinBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	PandoCombatComponent = CreateDefaultSubobject<UPandoCombatComponent>(TEXT("PandoCombatComponent"));

	TargetingComponent = CreateDefaultSubobject<UTargetingComponent>(TEXT("TargetingComponent"));

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

	/** MetaHuman */

	Torso = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Torso"));
	Torso->SetupAttachment(GetMesh());

	Face = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Face"));
	Face->SetupAttachment(GetMesh());

	Feets = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Feets"));
	Feets->SetupAttachment(GetMesh());

	Legs = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Legs"));
	Legs->SetupAttachment(GetMesh());

	Torso->SetLeaderPoseComponent(GetMesh());
	Feets->SetLeaderPoseComponent(GetMesh());
	Legs->SetLeaderPoseComponent(GetMesh());

	SetMetaHumanVisibility(false);

	LeftHandCollisionBox = CreateDefaultSubobject<UBoxComponent>("LeftHandCollisionBox");
	LeftHandCollisionBox->SetupAttachment(GetMesh());
	LeftHandCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RightHandCollisionBox = CreateDefaultSubobject<UBoxComponent>("RightHandCollisionBox");
	RightHandCollisionBox->SetupAttachment(GetMesh());
	RightHandCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	LeftHandCollisionBox->SetActive(false);
	RightHandCollisionBox->SetActive(false);

	LeftHandCollisionBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Hand_L_Combat"));
	RightHandCollisionBox->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("Hand_R_Combat"));

}

void APanWolfWarCharacter::SetMetaHumanVisibility(bool bVisible)
{
	Torso->SetVisibility(bVisible);
	Feets->SetVisibility(bVisible);
	Legs->SetVisibility(bVisible);
	Face->SetVisibility(bVisible);
}

void APanWolfWarCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();


	AddMappingContext(DefaultMappingContext, 0);

	if (PandoCombatComponent)
	{
		PandoCombatComponent->EnableHandToHandCombat(LeftHandCollisionBox, RightHandCollisionBox);
	}
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

void APanWolfWarCharacter::SetCollisionHandBoxExtent(FVector Extent)
{
	LeftHandCollisionBox->SetBoxExtent(Extent);
	RightHandCollisionBox->SetBoxExtent(Extent);
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

		//Targeting
		EnhancedInputComponent->BindAction(TargetingComponent->TargetLockAction, ETriggerEvent::Started, TargetingComponent, &UTargetingComponent::ToggleLock);

		EnhancedInputComponent->BindAction(TargetingComponent->SwitchTargetAction, ETriggerEvent::Triggered, TargetingComponent, &UTargetingComponent::SwitchTargetTriggered);
		EnhancedInputComponent->BindAction(TargetingComponent->SwitchTargetAction, ETriggerEvent::Completed, TargetingComponent, &UTargetingComponent::SwitchTargetCompleted);

		#pragma endregion

		#pragma region Pandolfo

		//PandolfoAction
		EnhancedInputComponent->BindAction(PandolfoComponent->Pandolfo_JumpAction, ETriggerEvent::Started, PandolfoComponent, &UPandolfoComponent::Jump);
		EnhancedInputComponent->BindAction(PandolfoComponent->Pandolfo_CrouchAction, ETriggerEvent::Completed, PandolfoComponent, &UPandolfoComponent::Crouch);
		EnhancedInputComponent->BindAction(PandolfoComponent->DodgeAction, ETriggerEvent::Started, PandolfoComponent, &UPandolfoComponent::Dodge);
		EnhancedInputComponent->BindAction(PandolfoComponent->Pandolfo_SlidingAction, ETriggerEvent::Completed, PandolfoComponent, &UPandolfoComponent::Sliding);
		EnhancedInputComponent->BindAction(PandolfoComponent->Pandolfo_GlideAction, ETriggerEvent::Triggered, PandolfoComponent, &UPandolfoComponent::TryGliding);
		EnhancedInputComponent->BindAction(PandolfoComponent->Pandolfo_AssassinAction, ETriggerEvent::Completed, PandolfoComponent, &UPandolfoComponent::Assassination);
		EnhancedInputComponent->BindAction(PandolfoComponent->LightAttackAction, ETriggerEvent::Started, PandolfoComponent, &UPandolfoComponent::LightAttack);

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
		EnhancedInputComponent->BindAction(PandolfoComponent->GetKiteComponent()->KiteExitAction, ETriggerEvent::Started, PandolfoComponent->GetKiteComponent(), &UKiteComponent::KiteExit);
		#pragma endregion

		#pragma region PandolFlower

		EnhancedInputComponent->BindAction(PandolFlowerComponent->MoveAction, ETriggerEvent::Triggered, PandolFlowerComponent, &UPandolFlowerComponent::Move);
		EnhancedInputComponent->BindAction(PandolFlowerComponent->HookAction, ETriggerEvent::Started, PandolFlowerComponent, &UPandolFlowerComponent::Hook);
		EnhancedInputComponent->BindAction(PandolFlowerComponent->JumpAction, ETriggerEvent::Started, PandolFlowerComponent, &UPandolFlowerComponent::Jump);
		EnhancedInputComponent->BindAction(PandolFlowerComponent->DodgeAction, ETriggerEvent::Started, PandolFlowerComponent, &UPandolFlowerComponent::Dodge);
		EnhancedInputComponent->BindAction(PandolFlowerComponent->PandolFlower_CrouchAction, ETriggerEvent::Started, PandolFlowerComponent, &UPandolFlowerComponent::Crouch);
		EnhancedInputComponent->BindAction(PandolFlowerComponent->PandolFlower_HideAction, ETriggerEvent::Started, PandolFlowerComponent, &UPandolFlowerComponent::Hide);	
		EnhancedInputComponent->BindAction(PandolFlowerComponent->PandolFlower_AssassinAction, ETriggerEvent::Started, PandolFlowerComponent, &UPandolFlowerComponent::Assassination);
		EnhancedInputComponent->BindAction(PandolFlowerComponent->LightAttackAction, ETriggerEvent::Started, PandolFlowerComponent, &UPandolFlowerComponent::LightAttack);
		#pragma endregion

		#pragma region PanBird

		EnhancedInputComponent->BindAction(PanBirdComponent->BirdMoveAction, ETriggerEvent::Triggered, PanBirdComponent, &UPanBirdComponent::Move);

		#pragma endregion

		#pragma region PanWolf
		EnhancedInputComponent->BindAction(PanWolfComponent->JumpAction, ETriggerEvent::Started, PanWolfComponent, &UPanWolfComponent::Jump);
		EnhancedInputComponent->BindAction(PanWolfComponent->DodgeAction, ETriggerEvent::Started, PanWolfComponent, &UPanWolfComponent::Dodge);
		EnhancedInputComponent->BindAction(PanWolfComponent->LightAttackAction, ETriggerEvent::Started, PanWolfComponent, &UPanWolfComponent::LightAttack);
		EnhancedInputComponent->BindAction(PanWolfComponent->HeavyAttackAction, ETriggerEvent::Started, PanWolfComponent, &UPanWolfComponent::HeavyAttack);
		EnhancedInputComponent->BindAction(PanWolfComponent->BlockAction, ETriggerEvent::Started, PanWolfComponent, &UPanWolfComponent::Block);
		EnhancedInputComponent->BindAction(PanWolfComponent->BlockAction, ETriggerEvent::Completed, PanWolfComponent, &UPanWolfComponent::UnBlock);
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

bool APanWolfWarCharacter::CanPerformDodge()
{
	if (!Attributes->IsAlive() || GetCharacterMovement()->IsFalling()) return false;
	return true;
}

FRotator APanWolfWarCharacter::GetDesiredDodgeRotation()
{
	if (UKismetMathLibrary::EqualEqual_VectorVector(GetCharacterMovement()->GetLastInputVector(), FVector::ZeroVector, 0.001f)) return GetActorRotation();
	return UKismetMathLibrary::MakeRotFromX(GetLastMovementInputVector());

}

void APanWolfWarCharacter::StartDodge()
{
	if (TargetingComponent->IsTargeting())
	{	
		//TargetingComponent->Deactivate();
		TargetingComponent->SetIsDodging(true);
		/*TargetingComponent->DisableLock();*/
	}
}

void APanWolfWarCharacter::EndDodge()
{
	PandoCombatComponent->ResetAttack();
	if (TargetingComponent->IsTargeting())
		TargetingComponent->SetIsDodging(false);
		//TargetingComponent->Activate();
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

void APanWolfWarCharacter::ConsumeStone(float StoneValue, EStoneTypes StoneType)
{
	switch (StoneType)
	{
	case EStoneTypes::EST_HealingStone:
		Attributes->AddHealth(StoneValue);
		break;
	case EStoneTypes::EST_BeerStone:
		break;
	default:
		break;
	}
}

void APanWolfWarCharacter::SetInvulnerability(bool NewInvulnerability)
{
	bIsInvulnerable = NewInvulnerability;
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

	if (bIsHiding)
	{
		GetMesh()->SetScalarParameterValueOnMaterials(FName("Emissive Multiplier"), 10.f);
		HidingAssassinBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
		
	else
	{
		GetMesh()->SetScalarParameterValueOnMaterials(FName("Emissive Multiplier"), 0.f);
		HidingAssassinBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
		

}


void APanWolfWarCharacter::AddEnemyAware(AActor* Enemy)
{	
	if (!Enemy) return;

	EnemyAware.AddUnique(Enemy);
	if (EnemyAware.Num() == 1)
	{
		PlayerSeenWidget->SetVisibility(true);
	}
		
}

void APanWolfWarCharacter::RemoveEnemyAware(AActor* Enemy)
{	
	if (!Enemy) return;

	EnemyAware.Remove(Enemy);
	if (EnemyAware.IsEmpty())
	{
		PlayerSeenWidget->SetVisibility(false);
	}
}
		

#pragma region Combat

void APanWolfWarCharacter::GetHit(const FVector& ImpactPoint, AActor* Hitter)
{
	if (!IsAlive() || !Hitter || bIsInvulnerable) return;
	
	FName Section = IHitInterface::DirectionalHitReact(Hitter , GetOwner() );
	PlayHitReactMontage(Section);

	PandoCombatComponent->ResetAttack();
	PandoCombatComponent->PlayHitSound(ImpactPoint);
	PandoCombatComponent->SpawnHitParticles(ImpactPoint);
	
}

void APanWolfWarCharacter::SetUnderAttack()
{
	bIsUnderAttack = true;
	GetWorld()->GetTimerManager().SetTimer(UnderAttack_TimerHandle, [this]() {this->ResetUnderAttack(); }, 2.f, false);
}

bool APanWolfWarCharacter::IsUnderAttack()
{
	return bIsUnderAttack;
}

void APanWolfWarCharacter::ResetUnderAttack()
{
	bIsUnderAttack = false;
}

void APanWolfWarCharacter::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	UAnimMontage* ReactMontage = nullptr;
	GetHitReactMontage(ReactMontage);
	if (!ReactMontage) return;

	if (AnimInstance->Montage_IsPlaying(ReactMontage)) return;

	AnimInstance->Montage_Play(ReactMontage);
	AnimInstance->Montage_JumpToSection(SectionName, ReactMontage);

	GetMesh()->SetScalarParameterValueOnMaterials(FName("HitFxSwitch"), 1.f);
}

void APanWolfWarCharacter::GetHitReactMontage(UAnimMontage*& ReactMontage)
{
	switch (TransformationComponent->GetCurrentTransformationState())
	{
	case ETransformationState::ETS_Pandolfo:
		if (PandolfoComponent->IsActive()) { ReactMontage = Pandolfo_HitReactMontage; }
		break;
	case ETransformationState::ETS_PanWolf:
		if (PanWolfComponent->IsActive()) { ReactMontage = PanWolfComponent->GetPanWolfHitReactMontage(); }
		break;
	case ETransformationState::ETS_PanFlower:
		if (PandolFlowerComponent->IsActive()) { ReactMontage = PandolFlower_HitReactMontage; }
		break;
	case ETransformationState::ETS_PanBird:
		if (PanBirdComponent->IsActive()) { ReactMontage = PanBird_HitReactMontage; }
		break;
	default:
		break;
	}

}

bool APanWolfWarCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}

bool APanWolfWarCharacter::IsCombatActorAlive()
{
	return IsAlive();
}

float APanWolfWarCharacter::PerformAttack()
{
	return 0.0f;
}

float APanWolfWarCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!Attributes) return 0.f;

	//const float Damage = DamageAmount / GetDamageDivisor();
	Attributes->ReceiveDamage(DamageAmount);
	if (!Attributes->IsAlive())
		Die();

	return DamageAmount;
}


void APanWolfWarCharacter::Die()
{
	Tags.Add(FName("Dead"));
	TransformationComponent->SelectDesiredTransformation(0);
	PandolfoComponent->ClearAllTimer();
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AGameModeBase* GameModeBase = UGameplayStatics::GetGameMode(this);
	if (GameModeBase)
	{
		APanWarSurvivalGameMode* PanWarSurvivalGameMode = Cast<APanWarSurvivalGameMode>(GameModeBase);
		if(PanWarSurvivalGameMode)
			PanWarSurvivalGameMode->OnSurvivalGameModeChanged(EPanWarSurvivalGameModeState::PlayerDied);
	}

	if (APlayerController* PlayerController = GetLocalViewingPlayerController())
	{
		PlayerController->SetInputMode(FInputModeUIOnly());
		PlayerController->bShowMouseCursor = true;
	}
	

	FTimerHandle Die_TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(Die_TimerHandle, [this]() {this->Destroy(); }, 5.f, false);
}

float APanWolfWarCharacter::GetDefensePower()
{
	return PandoCombatComponent->GetDefensePower();
}

void APanWolfWarCharacter::OnDeathEnter()
{
	return ;
}

#pragma endregion

UPawnUIComponent* APanWolfWarCharacter::GetPawnUIComponent() const
{
	return nullptr;
}

bool APanWolfWarCharacter::IsBlocking()
{
	if (TransformationComponent->GetCurrentTransformationState() == ETransformationState::ETS_PanWolf && PanWolfComponent->IsActive())
	{
		return PanWolfComponent->IsBlocking();
	}

	return false;
}

void APanWolfWarCharacter::SuccesfulBlock(AActor* Attacker)
{
	if (TransformationComponent->GetCurrentTransformationState() == ETransformationState::ETS_PanWolf && PanWolfComponent->IsActive())
	{
		return PanWolfComponent->SuccesfulBlock(Attacker);
	}
}

float APanWolfWarCharacter::GetHealthPercent()
{
	if(Attributes)
		return Attributes->GetHealthPercent();

	return -1.f;
}

UPawnCombatComponent* APanWolfWarCharacter::GetCombatComponent() const
{
	return PandoCombatComponent;
}