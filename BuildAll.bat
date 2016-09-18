.nuget\nuget.exe restore stormancer-sdk-cpp-120.sln
msbuild stormancer-sdk-cpp-120.sln /p:Configuration=Release /p:Platform=x64
msbuild stormancer-sdk-cpp-120.sln /p:Configuration=Release /p:Platform=x86
msbuild stormancer-sdk-cpp-120.sln /p:Configuration=Debug /p:Platform=x64
msbuild stormancer-sdk-cpp-120.sln /p:Configuration=Debug /p:Platform=x86

@powershell -NoProfile -ExecutionPolicy Bypass -Command "Write-NugetPackage .\stormancer-120-static.autopkg"

