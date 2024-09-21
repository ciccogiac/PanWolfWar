#include "Components/TargetingComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "EnhancedInputSubsystems.h"
#include "Widgets/PanWarWidgetBase.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SizeBox.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

#pragma region EngineFunctions



UTargetingComponent::UTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
}

void UTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);	

	if (CurrentLockedActor && CurrentLockedActor->ActorHasTag("Dead"))
	{
		TryLockOnTarget();
	}

	/*if (!CurrentLockedActor || CurrentLockedActor->ActorHasTag("Dead") || CharacterOwner->ActorHasTag("Dead") )*/
	if (!CurrentLockedActor  || CharacterOwner->ActorHasTag("Dead"))
	{
		CancelTargetLockAbility();
		return;
	}


	SetTargetLockWidgetPosition();

	//const bool bShouldOverrideRotation =
	//	!UWarriorFunctionLibrary::NativeDoesActorHaveTag(GetHeroCharacterFromActorInfo(), WarriorGameplayTags::Player_Status_Rolling)
	//	&&
	//	!UWarriorFunctionLibrary::NativeDoesActorHaveTag(GetHeroCharacterFromActorInfo(), WarriorGameplayTags::Player_Status_Blocking);


	const bool bShouldOverrideRotation = !isDodging;

	if (bShouldOverrideRotation)
	{
		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(
			CharacterOwner->GetActorLocation(),
			CurrentLockedActor->GetActorLocation()
		);

		LookAtRot -= FRotator(TargetLockCameraOffsetDistance, 0.f, 0.f);

		const FRotator CurrentControlRot = CharacterOwner->GetLocalViewingPlayerController()->GetControlRotation();
		const FRotator TargetRot = FMath::RInterpTo(CurrentControlRot, LookAtRot, DeltaTime, TargetLockRotationInterpSpeed);

		CharacterOwner->GetLocalViewingPlayerController()->SetControlRotation(FRotator(TargetRot.Pitch, TargetRot.Yaw, 0.f));
		/*CharacterOwner->SetActorRotation(FRotator(0.f, TargetRot.Yaw, 0.f));*/

		const FRotator TargetRot2 = FMath::RInterpTo(CharacterOwner->GetActorRotation(), LookAtRot, DeltaTime, TargetLockRotationActorInterpSpeed);
		CharacterOwner->SetActorRotation(FRotator(0.f, TargetRot2.Yaw, 0.f));
	}
}

void UTargetingComponent::Activate(bool bReset)
{
	Super::Activate();
}

void UTargetingComponent::Deactivate()
{
	Super::Deactivate();

}

#pragma endregion

void UTargetingComponent::ToggleLock()
{

	if (!bIsTargeting)
		TryLockOnTarget();
	else
		DisableLock();
}


void UTargetingComponent::EnableLock()
{


	bIsTargeting = true;

	InitTargetLockMovement();
	InitTargetLockMappingContext();

	Activate();

	//Debug::Print(TEXT("EnableLock"));
}

void UTargetingComponent::DisableLock()
{

	bIsTargeting = false;

	ResetTargetLockMovement();
	CleanUp();
	ResetTargetLockMappingContext();

	Deactivate();

	/*Debug::Print(TEXT("DisableLock"));*/

}

void UTargetingComponent::SwitchTargetTriggered(const FInputActionValue& InputActionValue)
{
	SwitchDirection = InputActionValue.Get<FVector2D>();
}

void UTargetingComponent::SwitchTargetCompleted(const FInputActionValue& InputActionValue)
{
	GetAvailableActorsToLock();

	TArray<AActor*> ActorsOnLeft;
	TArray<AActor*> ActorsOnRight;
	AActor* NewTargetToLock = nullptr;

	GetAvailableActorsAroundTarget(ActorsOnLeft, ActorsOnRight);

	if (SwitchDirection.X <= 0.f)
	{
		NewTargetToLock = GetNearestTargetFromAvailableActors(ActorsOnLeft);
	}
	else
	{
		NewTargetToLock = GetNearestTargetFromAvailableActors(ActorsOnRight);
	}

	if (NewTargetToLock)
	{
		CurrentLockedActor = NewTargetToLock;
	}
}

void UTargetingComponent::TryLockOnTarget()
{
	GetAvailableActorsToLock();

	if (AvailableActorsToLock.IsEmpty())
	{
		CancelTargetLockAbility();
		return;
	}

	CurrentLockedActor = GetNearestTargetFromAvailableActors(AvailableActorsToLock);

	if (CurrentLockedActor)
	{
		DrawTargetLockWidget();

		SetTargetLockWidgetPosition();

		if(!bIsTargeting)
			EnableLock();
	}
	else
	{
		CancelTargetLockAbility();
	}
}

void UTargetingComponent::GetAvailableActorsToLock()
{
	AvailableActorsToLock.Empty();
	TArray<FHitResult> BoxTraceHits;

	UKismetSystemLibrary::BoxTraceMultiForObjects(
		CharacterOwner,
		CharacterOwner->GetActorLocation(),
		CharacterOwner->GetActorLocation() + CharacterOwner->GetActorForwardVector() * BoxTraceDistance,
		TraceBoxSize / 2.f,
		CharacterOwner->GetActorForwardVector().ToOrientationRotator(),
		BoxTraceChannel,
		false,
		TArray<AActor*>(),
		bShowPersistentDebugShape ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None,
		BoxTraceHits,
		true
	);

	for (const FHitResult& TraceHit : BoxTraceHits)
	{
		if (AActor* HitActor = TraceHit.GetActor())
		{
			if (HitActor != CharacterOwner && !HitActor->ActorHasTag("Dead"))
			{
				AvailableActorsToLock.AddUnique(HitActor);

			}
		}
	}
}

AActor* UTargetingComponent::GetNearestTargetFromAvailableActors(const TArray<AActor*>& InAvailableActors)
{
	float ClosestDistance = 0.f;
	return UGameplayStatics::FindNearestActor(CharacterOwner->GetActorLocation(), InAvailableActors, ClosestDistance);
}

void UTargetingComponent::GetAvailableActorsAroundTarget(TArray<AActor*>& OutActorsOnLeft, TArray<AActor*>& OutActorsOnRight)
{
	if (!CurrentLockedActor || AvailableActorsToLock.IsEmpty())
	{
		CancelTargetLockAbility();
		return;
	}

	const FVector PlayerLocation = CharacterOwner->GetActorLocation();
	const FVector PlayerToCurrentNormalized = (CurrentLockedActor->GetActorLocation() - PlayerLocation).GetSafeNormal();

	for (AActor* AvailableActor : AvailableActorsToLock)
	{
		if (!AvailableActor || AvailableActor == CurrentLockedActor) continue;

		const FVector PlayerToAvailableNormalized = (AvailableActor->GetActorLocation() - PlayerLocation).GetSafeNormal();

		const FVector CrossResult = FVector::CrossProduct(PlayerToCurrentNormalized, PlayerToAvailableNormalized);

		if (CrossResult.Z > 0.f)
		{
			OutActorsOnRight.AddUnique(AvailableActor);
		}
		else
		{
			OutActorsOnLeft.AddUnique(AvailableActor);
		}
	}
}

void UTargetingComponent::DrawTargetLockWidget()
{
	if (!DrawnTargetLockWidget)
	{
		checkf(TargetLockWidgetClass, TEXT("Forgot to assign a valid widget class in Blueprint"));

		DrawnTargetLockWidget = CreateWidget<UPanWarWidgetBase>(CharacterOwner->GetLocalViewingPlayerController(), TargetLockWidgetClass);

		check(DrawnTargetLockWidget);

		DrawnTargetLockWidget->AddToViewport();
	}
}

void UTargetingComponent::SetTargetLockWidgetPosition()
{
	if (!DrawnTargetLockWidget || !CurrentLockedActor)
	{
		CancelTargetLockAbility();
		return;
	}

	FVector2D ScreenPosition;
	UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		CharacterOwner->GetLocalViewingPlayerController(),
		CurrentLockedActor->GetActorLocation(),
		ScreenPosition,
		true
	);

	if (TargetLockWidgetSize == FVector2D::ZeroVector)
	{
		DrawnTargetLockWidget->WidgetTree->ForEachWidget(
			[this](UWidget* FoundWidget)
			{
				if (USizeBox* FoundSizeBox = Cast<USizeBox>(FoundWidget))
				{
					TargetLockWidgetSize.X = FoundSizeBox->GetWidthOverride();
					TargetLockWidgetSize.Y = FoundSizeBox->GetHeightOverride();
				}
			}
		);
	}

	ScreenPosition -= (TargetLockWidgetSize / 2.f);

	DrawnTargetLockWidget->SetPositionInViewport(ScreenPosition, false);
}

void UTargetingComponent::InitTargetLockMovement()
{
	CachedDefaultMaxWalkSpeed = CharacterOwner->GetCharacterMovement()->MaxWalkSpeed;

	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = TargetLockMaxWalkSpeed;
}

void UTargetingComponent::InitTargetLockMappingContext()
{
	const ULocalPlayer* LocalPlayer = CharacterOwner->GetLocalViewingPlayerController()->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	check(Subsystem)

		Subsystem->AddMappingContext(TargetLockMappingContext, 3);
}

void UTargetingComponent::CleanUp()
{
	AvailableActorsToLock.Empty();

	CurrentLockedActor = nullptr;

	if (DrawnTargetLockWidget)
	{
		DrawnTargetLockWidget->RemoveFromParent();
	}

	DrawnTargetLockWidget = nullptr;

	TargetLockWidgetSize = FVector2D::ZeroVector;

	CachedDefaultMaxWalkSpeed = 0.f;
}

void UTargetingComponent::ResetTargetLockMovement()
{
	if (CachedDefaultMaxWalkSpeed > 0.f)
	{
		CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = CachedDefaultMaxWalkSpeed;
	}
}

void UTargetingComponent::ResetTargetLockMappingContext()
{
	if (!CharacterOwner->GetLocalViewingPlayerController())
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = CharacterOwner->GetLocalViewingPlayerController()->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);

	check(Subsystem)

		Subsystem->RemoveMappingContext(TargetLockMappingContext);
}

void UTargetingComponent::CancelTargetLockAbility()
{
	DisableLock();
}
