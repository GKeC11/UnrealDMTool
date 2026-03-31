// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class DMToolBox : ModuleRules
{
	public DMToolBox(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
				
			}
		);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				
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


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
			}
		);
	}
}