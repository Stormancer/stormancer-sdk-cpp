if "%1" == "rebuild" (
	set BuildType=":Rebuild"
) else (
	set BuildType=""
)

msbuild stormancer-sdk-cpp-141.sln /t:Restore
msbuild stormancer-sdk-cpp-141.sln /t:stormancer-sdk-cpp-lib-Linux%BuildType% /p:Configuration=Release /p:Platform=Linux-x64 /p:LinuxProps=%cd%\build-utils\platforms\linux\UnrealEngine4.22\LinuxUE422.props
msbuild stormancer-sdk-cpp-141.sln /t:stormancer-sdk-cpp-lib-Linux%BuildType% /p:Configuration=Debug /p:Platform=Linux-x64 /p:LinuxProps=%cd%\build-utils\platforms\linux\UnrealEngine4.22\LinuxUE422.props
