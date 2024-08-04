#include "Components/TargetingComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


#pragma region EngineFunctions



UTargetingComponent::UTargetingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.SetTickFunctionEnable(false);
	bAutoActivate = false;

	CharacterOwner = Cast<ACharacter>(GetOwner());
	PanWolfCharacter = Cast<APanWolfWarCharacter>(CharacterOwner);
}

void UTargetingComponent::BeginPlay()
{
	Super::BeginPlay();
	
}


void UTargetingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!IsTargeting || !TargetActor) { DisableLock(); return; }
	if (!CanTargetActor(TargetActor)) { DisableLock(); return; }

	const FRotator CharacterRotation = CharacterOwner->GetActorRotation();
	const FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(CharacterOwner->GetActorLocation(), TargetActor->GetActorLocation());
	const float RotationYaw = UKismetMathLibrary::RInterpTo(CharacterRotation, LookRotation, DeltaTime, 10.f).Yaw;

	const FRotator NewRotation = FRotator(CharacterRotation.Roll, RotationYaw, CharacterRotation.Pitch);
	CharacterOwner->SetActorRotation(NewRotation);
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
	if (IsTargeting)
		DisableLock();
	else
		EnableLock();
}

void UTargetingComponent::EnableLock()
{
	//Debug::Print(TEXT("Enable Lock"));

	if (!FindTarget()) return;

	IsTargeting = true;
	TargetInterface->SetTargetVisibility(true);
	SetRotationMode(true);

	Activate();
}

void UTargetingComponent::DisableLock()
{
	//Debug::Print(TEXT("Disable Lock"));

	IsTargeting = false;
	TargetInterface->SetTargetVisibility(false);
	TargetActor = nullptr;
	SetRotationMode(false);

	Deactivate();
}

bool UTargetingComponent::FindTarget()
{

	const FVector Start = CharacterOwner->GetActorLocation();
	const FVector End = PanWolfCharacter->GetFollowCamera()->GetForwardVector() * 1250.f;

	FHitResult Hit;
	TArray<TEnumAsByte<EObjectTypeQuery> > CombatObjectTypes;
	CombatObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);
	if (!UKismetSystemLibrary::SphereTraceSingleForObjects(this, Start, End, 125, CombatObjectTypes, false, TArray<AActor*>(), EDrawDebugTrace::ForDuration, Hit, true)) return false;

	AActor* FindActor = Hit.GetActor();

	if (!FindActor->Implements<UTargetInterface>()) return false;
	TargetInterface = Cast<ITargetInterface>(FindActor);
	if (!TargetInterface) return false;

	if (!CanTargetActor(FindActor)) return false;

	TargetActor = FindActor;
	return true;
}

bool UTargetingComponent::CanTargetActor(AActor* FindActor)
{
	const float Distance = UKismetMathLibrary::Vector_Distance(CharacterOwner->GetActorLocation(), FindActor->GetActorLocation());
	if (Distance > 1250.f) return false;

	if (!TargetInterface->CanBeTargeted()) return false;

	return true;
}

void UTargetingComponent::SetRotationMode(bool bTargetMode)
{
	CharacterOwner->GetCharacterMovement()->bOrientRotationToMovement = !bTargetMode;
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = bTargetMode ? 250.f : 500.f;
}