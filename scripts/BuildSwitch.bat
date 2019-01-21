.nuget\nuget.exe restore stormancer-sdk-cpp-nx.sln
msbuild stormancer-sdk-cpp-nx.sln /t:stormancer-sdk-cpp-nx /p:Configuration=Release /p:Platform=NX64
msbuild stormancer-sdk-cpp-nx.sln /t:stormancer-sdk-cpp-nx /p:Configuration=Debug /p:Platform=NX64
