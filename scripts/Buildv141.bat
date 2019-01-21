.nuget\nuget.exe restore stormancer-sdk-cpp-141.sln
msbuild stormancer-sdk-cpp-141.sln /t:stormancer-sdk-cpp-lib /p:Configuration=Release /p:Platform=x64
msbuild stormancer-sdk-cpp-141.sln /t:stormancer-sdk-cpp-lib /p:Configuration=Debug /p:Platform=x64
