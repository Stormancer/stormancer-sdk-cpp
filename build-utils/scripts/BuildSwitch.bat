if "%1" == "rebuild" (
	set BuildType=":Rebuild"
) else (
	set BuildType=""
)

msbuild stormancer-sdk-cpp-nx.sln /t:Restore
msbuild stormancer-sdk-cpp-nx.sln /t:stormancer-sdk-cpp-nx%BuildType% /p:Configuration=Release /p:Platform=NX64
msbuild stormancer-sdk-cpp-nx.sln /t:stormancer-sdk-cpp-nx%BuildType% /p:Configuration=Debug /p:Platform=NX64
