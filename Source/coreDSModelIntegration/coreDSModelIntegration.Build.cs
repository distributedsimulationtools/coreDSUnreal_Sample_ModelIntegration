/* Copyright (C) Consultants 2J's, Inc - operating under the name ds.tools
 * coreDS Unreal sample - Model Integration
 */

using UnrealBuildTool;

public class coreDSModelIntegration : ModuleRules
{
	public coreDSModelIntegration(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// No dependency on the coreDS modules on purpose: the model follows the
		// "loose coupling" approach of the Model Integration specification, so it
		// builds and runs with or without coreDS Unreal installed.
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });
	}
}
