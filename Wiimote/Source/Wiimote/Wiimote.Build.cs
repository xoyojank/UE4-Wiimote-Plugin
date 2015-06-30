// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class Wiimote : ModuleRules
	{
		public Wiimote(TargetInfo Target)
		{

            PublicIncludePaths.AddRange(
                new string[] {
                    "Wiimote/Public"
					// ... add public include paths required here ...
				}
                );

			PrivateIncludePaths.AddRange(
				new string[] {
					"Wiimote/Private",
					// ... add other private include paths required here ...
				}
				);

            PublicDependencyModuleNames.AddRange(
                new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"InputDevice",
					"InputCore",
					"WiiUse",
					// ... add other public dependencies that you statically link with here ...
				}
                );

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"InputDevice",
					// ... add private dependencies that you statically link with here ...
				}
				);

            DynamicallyLoadedModuleNames.AddRange(
                new string[]
				{                  
                    "WiiUse",
					// ... add any modules that your module loads dynamically here ...
				}
                );


            if (UEBuildConfiguration.bBuildEditor == true)
            {
                //@TODO: Needed for the triangulation code used for sprites (but only in editor mode)
                //@TOOD: Try to move the code dependent on the triangulation code to the editor-only module
              //  PrivateDependencyModuleNames.AddRange(new string [] {"UnrealEd","ContentBrowser"});
            }
		}
	}
}
