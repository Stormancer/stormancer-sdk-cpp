param(
    [string]$DownloadPath = "https://github.com/Stormancer/Sample_DedicatedClientServer/releases/download",
    [string]$Version = "1.01", 
    [string]$FileName = "Stormancer_DCS_1.01.zip", 
    [string]$InstallPath = "C:\Workspace\Sample_DedicatedClientServer-Unreal\DedicatedServer\Plugins\UEStormancerPlugin\Resources\DCS",
    [string]$TempPath = "D:\Temp"
)

Write-Host "DownloadPath = $($DownloadPath)"
Write-Host "Version = $($Version)"
Write-Host "FileName = $($FileName)"
Write-Host "InstallPath = $($InstallPath)"
Write-Host "TempPath = $($TempPath)"

$TempFolder 
$TempFolder = $((Get-Date).ToString('u')) -replace ':','_'
$TempFolder = "$($TempPath)\$($TempFolder)"

New-Item -ItemType Directory -Path $TempFolder | Out-Null
#New-TemporaryFile | %{ rm $_; $TempFolder = $_; mkdir $_ } | Out-Null
$TempDownload = "$($TempFolder)\$($FileName)"


$FullDownloadName = "$($DownloadPath)/$($Version)/$($FileName)"

Function DownloadFile 
{
	Write-Host "Downloading $($FileName) ..."
	$client = New-Object System.Net.WebClient
	$client.DownloadFile($FullDownloadName, $TempDownload)
} 


DownloadFile

Write-Host "Expanding the archive ..."

#The archive is expand in the root so we need to install it in it's directory and then clean the empty directory
#Clean old Resources in InstallPath
if(Test-Path -Path $InstallPath) 
{
	#Maybe dangerous, just delete the old Resources of what we are searching
	Remove-Item $InstallPath -Force -recurse
}
$ExpandFolderName = (Get-Item $TempDownload ).Basename
$expandPath = "$($TempFolder)\$($ExpandFolderName)"
New-Item -ItemType Directory -Force -Path $expandPath | Out-Null
(new-object -com shell.application).NameSpace($expandPath).CopyHere((new-object -com shell.application).NameSpace($TempDownload).Items())

# Get expand Content path
if(!(test-path $InstallPath))
{
    New-Item -ItemType Directory -Force -Path $InstallPath | Out-Null
}
$versionFile = (Get-Item $expandPath ).Basename
New-Item -ItemType File -Force -Path "$($InstallPath)\$($versionFile)"  | Out-Null
#Get-ChildItem -Directory -Path $expandPath | New-Item -ItemType Directory -Force -Path Select-Object Fullname | Out-Null
Get-ChildItem -Path $expandPath | Copy-Item -Recurse -Destination $InstallPath
