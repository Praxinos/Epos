// IDDN.FR.001.220036.001.S.P.2021.000.00000
// EPOS is subject to copyright © laws and is the legal and intellectual property of Praxinos,Inc - Year of publishing 2022

using System.IO;
using UnrealBuildTool;

public class EposNamingConvention : ModuleRules
{
    public EposNamingConvention(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
            }
            );

        PrivateIncludePaths.AddRange(
            new string[] {
                // ... add other private include paths required here ...
            }
            );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                // ... add other public dependencies that you statically link with here ...
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                // [RUNTIME]
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                // [RUNTIME] custom engine dependencies
                "CinematicCamera",
                "LevelSequence",
                "MovieScene",
                "DeveloperSettings",
                // [RUNTIME] custom plugin dependencies
                "EposActors",
                "EposMovieScene",
                "EposSequence",
                "EposTracks",
                "EposNote",

                // [EDITOR]
                // [EDITOR] custom engine dependencies
                "MovieSceneTools",
                "EditorScriptingUtilities",
                "PropertyEditor",
                "EditorStyle",
                // [EDITOR] custom plugin dependencies
            }
            );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );
    }
}
