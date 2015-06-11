Stormancer SDK C++  

Documentation  
http://stormancer.github.io/stormancer-sdk-cpp/documentation/  


Configuration help  

NuGet packages to install :  
- C++ REST SDK (or cpprestsdk) (by searching casablanca)  
- Reactive Extensions for C++ (or rxcpp)  

Bundled libraries :  
- RakNet  
- MessagePack  

Configuring your client project :  
Add RakNet and the Stormancer SDK to your environment path (in visual studio only) :  
Go to Project Properties > Configuration Properties > Debugging  
Select All Configurations and All Platforms on top of the property window.  
Change the Environment option and add this line :  
PATH=$(SolutionDir)RakNet\Lib\DLL\Lib;$(SolutionDir)bin;%PATH%  

And don't forget to set Echo as startup project.  
