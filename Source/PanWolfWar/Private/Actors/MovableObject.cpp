// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/MovableObject.h"

#include "PhysicsEngine/PhysicsConstraintComponent.h"

#include "Kismet/KismetMathLibrary.h"

#include "PanWolfWar/DebugHelper.h"

// Sets default values
AMovableObject::AMovableObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;



	PhysicsConstraintComponent = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("PhysicsConstraintComponent"));
	PhysicsConstraintComponent->SetupAttachment(StaticMesh);

	FConstrainComponentPropName a;
	a.ComponentName = StaticMesh->GetFName();
	PhysicsConstraintComponent->ComponentName1 = a;
	PhysicsConstraintComponent->SetLinearXLimit(ELinearConstraintMotion::LCM_Free,0.f);
	PhysicsConstraintComponent->SetLinearYLimit(ELinearConstraintMotion::LCM_Free, 0.f);
	PhysicsConstraintComponent->SetLinearZLimit(ELinearConstraintMotion::LCM_Free, 0.f);
	PhysicsConstraintComponent->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked,45.f);
	PhysicsConstraintComponent->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Locked, 45.f);
	PhysicsConstraintComponent->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Locked, 45.f);


}

// Called when the game starts or when spawned
void AMovableObject::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AMovableObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMovableObject::SetObjectlocation(bool bForwardDirection, float DirectionSpeed)
{
	const FVector ForwardVector = StaticMesh->GetForwardVector();
	const FVector RightVector = StaticMesh->GetRightVector();
	const FVector DirectionVector = UKismetMathLibrary::SelectVector(ForwardVector, RightVector, bForwardDirection);

	const FVector CurrentLocation = StaticMesh->GetRelativeLocation();
	const FVector TargetLocation = CurrentLocation + DirectionVector * DirectionSpeed;

	const FVector InterpolatedPosition = UKismetMathLibrary::VInterpTo(CurrentLocation, TargetLocation, GetWorld()->GetDeltaSeconds(), 4.f);

	StaticMesh->SetRelativeLocation(InterpolatedPosition);
}

void AMovableObject::Interact(bool bStartInteraction)
{
	Debug::Print(TEXT("Interact"), FColor::Cyan, 1);
	StaticMesh->SetSimulatePhysics(bStartInteraction);
	/*if (bStartInteraction)
	{
		StaticMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	}
	else
	{
		StaticMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	}*/
}



