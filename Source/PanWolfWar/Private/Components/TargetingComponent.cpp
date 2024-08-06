#include "Components/TargetingComponent.h"

#include "PanWolfWar/DebugHelper.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include <PanWolfWar/PanWolfWarCharacter.h>
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include <float.h>

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

	if (!bIsTargeting || !TargetActor) { DisableLock(); return; }
	if (!CanTargetActor(TargetActor)) 
	{
		DisableLock();
		if(FindNearTarget(false))
			EnableLock();
		return; 
	}

	const FRotator CharacterRotation = CharacterOwner->GetActorRotation();
	const FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(CharacterOwner->GetActorLocation(), TargetActor->GetActorLocation());
	const float RotationYaw = UKismetMathLibrary::RInterpTo(CharacterRotation, LookRotation, DeltaTime, 5.f).Yaw;

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
	if (bIsTargeting)
	{
		if (FindNearTarget())
			EnableLock();
		else
			DisableLock();

	}
		
	else
		//if (FindTarget()) 
		if (FindNearTarget())
			EnableLock();
}

void UTargetingComponent::TryLock()
{
	if (FindNearTarget(false))
		EnableLock();
}

void UTargetingComponent::ForceUnLock()
{
	Debug::Print(TEXT("ForceUNlock"));
	if (bIsTargeting)
		DisableLock();
}

void UTargetingComponent::EnableLock()
{
	//Debug::Print(TEXT("Enable Lock"));


	bIsTargeting = true;
	TargetInterface->SetTargetVisibility(true);
	SetRotationMode(true);

	Activate();
}

void UTargetingComponent::DisableLock()
{
	//Debug::Print(TEXT("Disable Lock"));

	bIsTargeting = false;
	TargetInterface->SetTargetVisibility(false);
	TargetActor = nullptr;
	SetRotationMode(false);

	Deactivate();
}


bool UTargetingComponent::FindNearTarget(bool DoTraceCameraOriented)
{
	const FVector Start = CharacterOwner->GetActorLocation();
	FVector End; float Radius;

	if (DoTraceCameraOriented)
	{		
		End = Start + PanWolfCharacter->GetFollowCamera()->GetForwardVector() * 1250.f;
		Radius = 175.f;
	}
	else
	{
		End = Start + CharacterOwner->GetActorForwardVector();
		Radius = 300.f;
	}

	TArray<FHitResult> Hits;
	TArray<TEnumAsByte<EObjectTypeQuery> > CombatObjectTypes;
	CombatObjectTypes.Add(EObjectTypeQuery::ObjectTypeQuery3);
	TArray<AActor*> ActorsToIgnore = TArray<AActor*>();
	ActorsToIgnore.Add(CharacterOwner);
	if(TargetActor)
		ActorsToIgnore.Add(TargetActor);
	if (!UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, End, Radius, CombatObjectTypes, false, ActorsToIgnore, EDrawDebugTrace::None, Hits, true)) return false;

	
	
	float ClosestDistance = FLT_MAX;
	AActor* ClosestEnemy = nullptr;
	ITargetInterface* LoopTargetInterface = nullptr;
	ITargetInterface* ClosestTargetInterface = nullptr;

	for (size_t i = 0; i < Hits.Num(); i++)
	{
		AActor* LoopActor = Hits[i].GetActor();

		if (!LoopActor->Implements<UTargetInterface>()) continue;
		LoopTargetInterface = Cast<ITargetInterface>(LoopActor);
		if (!LoopTargetInterface) continue;

		//if (!CanTargetActor(LoopActor)) continue;

		const float LoopDistance = UKismetMathLibrary::Vector_Distance(CharacterOwner->GetActorLocation(), LoopActor->GetActorLocation());
		if (LoopDistance > 1250.f) continue;
		if (!LoopTargetInterface->CanBeTargeted()) continue;


		//LoopActor->GetActorLocation()
		const float Distance = UKismetMathLibrary::Vector_Distance(LoopActor->GetActorLocation(), Start);
		if (Distance < ClosestDistance && LoopActor!= TargetActor)
		{
			ClosestDistance = Distance;
			ClosestEnemy = LoopActor;
			ClosestTargetInterface = LoopTargetInterface;
		}
	}

	
	if (!ClosestEnemy || !ClosestTargetInterface) return false;

	if(bIsTargeting)
		DisableLock();

	TargetActor = ClosestEnemy;
	TargetInterface = ClosestTargetInterface;

	
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
	CharacterOwner->GetCharacterMovement()->MaxWalkSpeed = bTargetMode ? 350.f : 500.f;
}