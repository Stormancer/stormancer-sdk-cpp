Stormancer SDK C++  
  
Configuration help  

For adding RakNet and the Stormancer SDK to your environment path (in visual studio only)  
Go to Project Properties > Configuration Properties > Debugging  
Select All Configurations and All Platforms on top of the property window.  
Change the Environment option  
PATH=$(SolutionDir)RakNet\Lib\DLL\Lib;$(SolutionDir)bin;%PATH%
