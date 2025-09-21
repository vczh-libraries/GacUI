# Find the solution folder by looking for *.sln files
$currentDir = Get-Location
$solutionFile = $null

while ($currentDir -ne $null) {
    $solutionFiles = Get-ChildItem -Path $currentDir.Path -Filter "*.sln" -ErrorAction SilentlyContinue
    if ($solutionFiles.Count -gt 0) {
        $solutionFile = "$($currentDir.Path)\$($solutionFiles[0])"
        Write-Host "Found solution file: $solutionFile"
        break
    }
    $currentDir = $currentDir.Parent
}

if ($solutionFile -eq $null) {
    throw "Could not find a solution file (*.sln file) in current directory or any parent directories."
}

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

# Execute msbuild
$commandLine = "/c $cmd_arguments"
$startInfo = New-Object System.Diagnostics.ProcessStartInfo
$startInfo.FileName = $env:ComSpec
$startInfo.Arguments = $commandLine
$startInfo.UseShellExecute = $false
$process = [System.Diagnostics.Process]::Start($startInfo)
$process.WaitForExit()
exit $process.ExitCode