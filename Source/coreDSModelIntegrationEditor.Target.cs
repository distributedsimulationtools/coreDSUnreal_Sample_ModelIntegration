/* Copyright (C) Consultants 2J's, Inc - operating under the name ds.tools
 * coreDS Unreal sample - Model Integration
 */

using UnrealBuildTool;
using System.Collections.Generic;

public class coreDSModelIntegrationEditorTarget : TargetRules
{
	public coreDSModelIntegrationEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("coreDSModelIntegration");
	}
}
