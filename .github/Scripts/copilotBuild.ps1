. $PSScriptRoot\copilotShared.ps1

# Remove log file if it exists
$logFile = "$PSScriptRoot\Build.log"
if (Test-Path $logFile) {
    Remove-Item $logFile -Force
    Write-Host "Removed existing log file: $logFile"
}

# Find the solution folder by looking for *.sln files
$solutionFolder = GetSolutionDir
$solutionFile = "$solutionFolder\$((Get-ChildItem -Path $solutionFolder -Filter "*.sln" -ErrorAction SilentlyContinue)[0])"

$vsdevcmd = $env:VLPP_VSDEVCMD_PATH
if ($vsdevcmd -eq $null) {
    $MESSAGE_1 = "You have to add an environment variable named VLPP_VSDEVCMD_PATH and set its value to the path of VsDevCmd.bat, e.g.:"
    $MESSAGE_2 = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    throw "$MESSAGE_1\r\n$MESSAGE_2"
}

$Configuration = "Debug"
$Platform = "x64"
$msbuild_arguments = "MSBUILD `"$solutionFile`" /m:8 $rebuildControl /p:Configuration=`"$Configuration`";Platform=`"$Platform`""
$cmd_arguments = "`"`"$vsdevcmd`" & $msbuild_arguments"

# Execute msbuild with output to both console and log file
$logFile = "$PSScriptRoot\Build.log"
$commandLine = "/c $cmd_arguments"
$logFileUnfinished = "$logFile.unfinished"
& $env:ComSpec $commandLine 2>&1 | Tee-Object -FilePath $logFileUnfinished
Rename-Item -Path $logFileUnfinished -NewName $logFile -Force
exit $LASTEXITCODE