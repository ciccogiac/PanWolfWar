// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstances/PanWarBaseEnemyAnimInstance.h"
#include "Enemy/BaseEnemy.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPanWarBaseEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (OwningCharacter)
	{
		OwningEnemyCharacter = Cast<ABaseEnemy>(OwningCharacter);
	}
}

void UPanWarBaseEnemyAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	//bShouldMove = (GroundSpeed > 3.f) && bHasAcceleration;

	////if (!IsEnemyInState(EEnemyState::EES_Default)) return;

	//const float LastYaw = OwningMovementComponent->GetLastUpdateRotation().Yaw;
	//RotationIdle = LastRotationIdle - LastYaw;
	//LastRotationIdle = LastYaw;

}

bool UPanWarBaseEnemyAnimInstance::IsEnemyInState(EEnemyState StateToCheck) const
{
	if (OwningEnemyCharacter)
	{
		return OwningEnemyCharacter->IsEnemyStateEqualTo(StateToCheck);
	}

	return false;

}
