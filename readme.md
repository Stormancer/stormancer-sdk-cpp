Stormancer SDK C++  
  
Configuration help  

Install the C++ REST SDK by searching casablanca in the nuget packages online library.  

For adding RakNet and the Stormancer SDK to your environment path (in visual studio only)  
Go to Project Properties > Configuration Properties > Debugging  
Select All Configurations and All Platforms on top of the property window.  
Change the Environment option and add this line :  
PATH=$(SolutionDir)RakNet\Lib\DLL\Lib;$(SolutionDir)bin;%PATH%

And don't forget to set Echo as startup project.  
