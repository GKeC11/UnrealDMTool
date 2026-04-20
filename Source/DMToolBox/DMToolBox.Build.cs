// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DMToolBox : ModuleRules
{
	public DMToolBox(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"CommonUI",
			}
		);


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"DeveloperSettings",
				"UMG",
				"CommonUI",
				"GameplayTags",
				"EnhancedInput",
				"Puerts",
				"JsEnv",
				"GameplayMessageRuntime",
			}
		);
	}
}