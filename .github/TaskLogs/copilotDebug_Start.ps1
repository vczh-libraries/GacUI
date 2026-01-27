param(
    [Parameter(Mandatory=$true)]
    [string]$Executable
)

. $PSScriptRoot\copilotShared.ps1

# Ensure the executable name has .exe extension
if ($Executable.EndsWith(".exe")) {
    throw "\$Executable parameter should not include the .exe extension: $Executable"
}
$executableName = $Executable + ".exe"

# Find the solution folder by looking for *.sln files
$solutionFolder = GetSolutionDir

# Find the file with the latest modification time
$latestFile = GetLatestModifiedExecutable $solutionFolder $executableName
Write-Host "Selected $executableName`: $($latestFile.Path) (Modified: $($latestFile.LastWriteTime))"

# Try to read debug arguments from the corresponding .vcxproj.user file
$debugArgs = GetDebugArgs $solutionFolder $latestFile $Executable

$cdbpath = GetCDBPath
$commandLine = "`"$($cdbpath)`" -server npipe:pipe=VlppUnitTest -cf `"$PSScriptRoot\copilotDebug_Init.txt`" -o `"$($latestFile.Path)`" $debugArgs"
echo $commandLine
cmd.exe /S /C $commandLine
