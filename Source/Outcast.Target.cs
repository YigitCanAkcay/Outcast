using UnrealBuildTool;
using System.Collections.Generic;

public class OutcastTarget : TargetRules
{
	public OutcastTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;

		ExtraModuleNames.AddRange( new string[] { "Outcast" } );
	}
}
