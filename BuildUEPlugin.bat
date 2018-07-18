if not exist "UE4\source_build" mkdir UE4\source_build
xcopy /E /I /Y UE4\stormancerPlugin UE4\source_build\stormancerPlugin
xcopy /E /I /Y output UE4\source_build\stormancerPlugin\3rdParty\Stormancer
xcopy /Y /I UE4\source\StormancerPlugin.Build.cs UE4\source_build\stormancerPlugin\Source\Stormancer