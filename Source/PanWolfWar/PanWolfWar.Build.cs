// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using UnrealBuildTool.Rules;

public class PanWolfWar : ModuleRules
{
	public PanWolfWar(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "MotionWarping" , "Niagara" , "CableComponent" , "AIModule" , "AnimGraphRuntime" , "NavigationSystem" , "MoviePlayer", "GameplayAbilities" });
	}
}
