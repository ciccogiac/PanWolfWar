// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimInstances/PanWarBaseEnemyAnimInstance.h"
#include "Enemy/BaseEnemy.h"

void UPanWarBaseEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (OwningCharacter)
	{
		OwningEnemyCharacter = Cast<ABaseEnemy>(OwningCharacter);
	}
}

bool UPanWarBaseEnemyAnimInstance::IsEnemyInState(EEnemyState StateToCheck) const
{
	if (OwningEnemyCharacter)
	{
		return OwningEnemyCharacter->IsEnemyStateEqualTo(StateToCheck);
	}

	return false;

}
