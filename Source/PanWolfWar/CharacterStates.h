#pragma once

UENUM(BlueprintType)
enum class ETransformationState : uint8
{
	ETS_Pandolfo UMETA(DisplayName = "Pandolfo"),
	ETS_Transforming UMETA(DisplayName = "Transforming"),
	ETS_PanWolf UMETA(DisplayName = "PanWolf"),
	ETS_PanFlower UMETA(DisplayName = "PanFlower"),
	ETS_None UMETA(DisplayName = "None"),
};