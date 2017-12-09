using UnrealBuildTool;
using System.Collections.Generic;

public class OutcastEditorTarget : TargetRules
{
	public OutcastEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "Outcast" } );
	}
}
