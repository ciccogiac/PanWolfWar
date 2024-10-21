#pragma once

UENUM(BlueprintType)
enum class EPanWarGameDifficulty : uint8
{
	Easy,
	Normal,
	Hard,
	VeryHard
};

UENUM(BlueprintType)
enum class EPanWarLevel : uint8
{
	MainMenuMap,
	SurvivalGameModeMap,
	Level_1,
	Level_2,
	Level_3,
	Level_4
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