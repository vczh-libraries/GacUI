param(
    [string]$Mode = $null,
    [string]$Executable = $null,
    [string]$Configuration = $null,
    [string]$Platform = $null
)

if ([string]::IsNullOrEmpty($Mode)) {
    throw "\$Mode parameter is required."
}
if ([string]::IsNullOrEmpty($Executable)) {
    throw "\$Executable parameter is required."
}

if (($Mode -ne "CLI") -and ($Mode -ne "UnitTest")) {
    throw "Invalid mode: $Mode. Allowed values are CLI or UnitTest."
}

if (($Configuration -ne $null) -ne ($Platform -ne $null)) {
    throw "Configuration and Platform parameters should be set or unset at the same time."
}
if ($Configuration -ne $null) {
    if (($Configuration -ne "Debug") -and ($Configuration -ne "Release")) {
        throw "Invalid configuration: $Configuration. Allowed values are Debug or Release."
    }
    if (($Platform -ne "x64") -and ($Platform -ne "Win32")) {
        throw "Invalid platform: $Platform. Allowed values are x64 or Win32."
    }
}

. $PSScriptRoot\copilotShared.ps1

if ($Mode -eq "UnitTest") {
  # Remove log files
  $logFile = "$PSScriptRoot\Execute.log"
  $logFileUnfinished = "$logFile.unfinished"
  Remove-Item -Path $logFile, $logFileUnfinished -Force -ErrorAction SilentlyContinue
}

# Ensure the executable name has .exe extension
if ($Executable.EndsWith(".exe")) {
    throw "\$Executable parameter should not include the .exe extension: $Executable"
}
$executableName = $Executable + ".exe"

# Find the solution folder by looking for *.sln files
$solutionFolder = GetSolutionDir

# Find the file with the latest modification time
if ($Configuration -ne $null) {
  $latestFile = GetLatestModifiedExecutable $solutionFolder $executableName
} else {
  $latestFile = GetSpecifiedExecutable $solutionFolder $executableName $Configuration $Platform
}
Write-Host "Selected $executableName`: $($latestFile.Path) (Modified: $($latestFile.LastWriteTime))"

# Try to read debug arguments from the corresponding .vcxproj.user file
$debugArgs = GetDebugArgs $solutionFolder $latestFile $Executable

# Execute the selected executable with debug arguments and save output to log file
if ($Mode -eq "UnitTest") {
  $commandLine = "`"$($latestFile.Path)`" /C $debugArgs"
  & { $commandLine; & cmd.exe /S /C $commandLine 2>&1 } | Tee-Object -FilePath $logFileUnfinished
  Rename-Item -Path $logFileUnfinished -NewName $logFile -Force
} else {
  $commandLine = "`"$($latestFile.Path)`" $debugArgs"
  & { $commandLine; & cmd.exe /S /C $commandLine 2>&1 }
}
exit $LASTEXITCODE
