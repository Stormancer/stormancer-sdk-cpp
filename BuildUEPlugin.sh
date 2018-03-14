mkdir -p UE4/source_build
cp -rf UE4/stormancerPlugin UE4/source_build
cp -rf UE4/source/StormancerPlugin.Build.cs UE4/source_build/stormancerPlugin/Source/Stormancer
mkdir -p UE4/source_build/stormancerPlugin/3rdParty/Stormancer
rsync -ru output/* UE4/source_build/stormancerPlugin/3rdParty/Stormancer
