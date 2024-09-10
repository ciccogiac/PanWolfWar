#pragma once

UENUM(BlueprintType)
enum class EPanWarGameDifficulty : uint8
{
	Easy,
	Normal,
	Hard,
	VeryHard
};

UENUM()
enum class EPanWarCountDownActionInput : uint8
{
	Start,
	Cancel
};

UENUM()
enum class EPanWarCountDownActionOutput : uint8
{
	Updated,
	Completed,
	Cancelled
};

UENUM(BlueprintType)
enum class EPanWarInputMode : uint8
{
	GameOnly,
	UIOnly
};