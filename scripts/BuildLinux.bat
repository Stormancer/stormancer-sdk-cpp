if "%1" == "rebuild" (
	set BuildType="Rebuild"
) else (
	set BuildType="Build"
)

msbuild stormancer-sdk-cpp-141.sln /t:Restore
msbuild stormancer-sdk-cpp-141.sln /t:stormancer-sdk-cpp-lib-Linux:%BuildType% /p:Configuration=Release /p:Platform=Linux-x64
msbuild stormancer-sdk-cpp-141.sln /t:stormancer-sdk-cpp-lib-Linux:%BuildType% /p:Configuration=Debug /p:Platform=Linux-x64
