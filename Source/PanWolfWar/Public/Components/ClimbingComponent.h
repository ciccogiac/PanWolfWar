// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbingComponent.generated.h"

class UCharacterMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PANWOLFWAR_API UClimbingComponent : public UActorComponent
{
	GENERATED_BODY()


#pragma region PublicFunctions

public:

	UClimbingComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool ToggleClimbing();

#pragma endregion

#pragma region ProtectedFunctions

protected:

	virtual void BeginPlay() override;

#pragma endregion

#pragma region PrivateFunctions

private:


	void StartClimbing();
	void StopClimbing();

	bool TryClimbing();
	void ClimbCapsuleTrace(FHitResult &outHit);
	bool CheckNormalContact(FVector_NetQuantizeNormal ImpactNormal);
	bool FindClimbablePoint(FHitResult hit);

	void SnapMovementToClimbableSurface(float DeltaTime);

#pragma endregion

#pragma region PrivateVariables

private:
	AActor* ActorOwner;
	ACharacter* CharacterOwner;
	UCharacterMovementComponent* MovementComponent;

	#pragma region ClimbBPVariables

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Movement: Climbing", meta = (AllowPrivateAccess = "true"))
	TArray<TEnumAsByte<EObjectTypeQuery> > ClimbableObjectTypes;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	bool ShowDebugTrace = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	float Radius = 50.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	float HalfHeight = 75.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	float ForwardOffset = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Climb Params", meta = (AllowPrivateAccess = "true"))
	float MaxClimbSpeed = 100.f;




	#pragma endregion

	#pragma region ClimbCoreVariables

	bool bIsClimbing;
	FVector CurrentClimbableSurfaceLocation;
	FVector CurrentClimbableSurfaceNormal;

	#pragma endregion

#pragma endregion

#pragma region FORCEINLINE_functions

public:

	UFUNCTION(BlueprintCallable, Category = "Climbing")
	FORCEINLINE bool IsClimbing() const { return bIsClimbing; }

	FORCEINLINE FVector GetClimbableSurfaceNormal() const { return CurrentClimbableSurfaceNormal; }

#pragma endregion


};
