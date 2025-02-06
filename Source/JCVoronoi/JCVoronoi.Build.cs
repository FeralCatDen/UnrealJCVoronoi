// Copyright Feral Cat Den, LLC. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class JCVoronoi : ModuleRules
{
	public JCVoronoi(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateIncludePaths.Add("JCVoronoi/Private");

		PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
			}
		);
	}
}
