if "%1" == "rebuild" (
	set BuildType="Rebuild"
) else (
	set BuildType="Build"
)

msbuild stormancer-sdk-cpp-141.sln /t:Restore
msbuild stormancer-sdk-cpp-141.sln /t:stormancer-sdk-cpp-lib:%BuildType% /p:Configuration=Release /p:Platform=x64
msbuild stormancer-sdk-cpp-141.sln /t:stormancer-sdk-cpp-lib:%BuildType% /p:Configuration=Debug /p:Platform=x64
