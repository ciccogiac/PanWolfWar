#pragma once

UENUM(BlueprintType)
enum class ETransformationState : uint8
{
	ETS_Pandolfo UMETA(DisplayName = "Pandolfo"),
	ETS_Transforming UMETA(DisplayName = "Transforming"),
	ETS_PanWolf UMETA(DisplayName = "PanWolf"),
	ETS_PanFlower UMETA(DisplayName = "PanFlower"),
	ETS_PanBird UMETA(DisplayName = "PanBird"),
	ETS_None UMETA(DisplayName = "None"),
};

UENUM(BlueprintType)
enum class ECombatAttackRange : uint8
{
	ECAR_UnderAttackDistanceRange UMETA(DisplayName = "UnderAttackDistanceRange"),
	ECAR_MeleeAttackDistanceRange UMETA(DisplayName = "MeleeAttackDistanceRange"),
	ECAR_MeleeAttackDistance UMETA(DisplayName = "MeleeAttackDistance")
};