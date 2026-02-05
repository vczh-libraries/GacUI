param(
    [Parameter(Mandatory=$true)]
    [string]$Executable
)

. $PSScriptRoot\copilotShared.ps1

# Remove log files
$logFile = "$PSScriptRoot\Execute.log"
$logFileUnfinished = "$logFile.unfinished"
Remove-Item -Path $logFile, $logFileUnfinished -Force -ErrorAction SilentlyContinue

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

# Execute the selected executable with debug arguments and save output to log file
$commandLine = "`"$($latestFile.Path)`" /C $debugArgs"
& { $commandLine; & cmd.exe /S /C $commandLine 2>&1 } | Tee-Object -FilePath $logFileUnfinished
Rename-Item -Path $logFileUnfinished -NewName $logFile -Force
exit $LASTEXITCODE
