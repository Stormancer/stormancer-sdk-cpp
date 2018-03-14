if not exist "UE4\source_build" mkdir UE4\source_build
xcopy UE4\stormancerPlugin UE4\source_build /E
xcopy output UE4\source_build\stormancerPlugin\3rdParty\Stormancer /E
xcopy UE4\source\StormancerPlugin.Build.cs UE4\source_build\stormancerPlugin\Source\Stormancer /y