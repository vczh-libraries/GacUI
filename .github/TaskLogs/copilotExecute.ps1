param(
    [Parameter(Mandatory=$true)]
    [string]$Executable
)

# Ensure the executable name has .exe extension
if (-not $Executable.EndsWith(".exe")) {
    $executableName = $Executable + ".exe"
} else {
    $executableName = $Executable
}

# Find the solution folder by looking for *.sln files
$currentDir = Get-Location
$solutionFolder = $null

while ($currentDir -ne $null) {
    $solutionFiles = Get-ChildItem -Path $currentDir.Path -Filter "*.sln" -ErrorAction SilentlyContinue
    if ($solutionFiles.Count -gt 0) {
        $solutionFolder = $currentDir.Path
        Write-Host "Found solution folder: $solutionFolder"
        break
    }
    $currentDir = $currentDir.Parent
}

if ($solutionFolder -eq $null) {
    throw "Could not find a solution folder (*.sln file) in current directory or any parent directories."
}

# Define configuration to path mappings
$configToPathMap = @{
    "Debug|Win32" = "$solutionFolder\Debug\$executableName"
    "Release|Win32" = "$solutionFolder\Release\$executableName"
    "Debug|x64" = "$solutionFolder\x64\Debug\$executableName"
    "Release|x64" = "$solutionFolder\x64\Release\$executableName"
}

# Find existing files and get their modification times with configuration info
$existingFiles = @()
foreach ($config in $configToPathMap.Keys) {
    $path = $configToPathMap[$config]
    if (Test-Path $path) {
        $fileInfo = Get-Item $path
        $existingFiles += [PSCustomObject]@{
            Path = $path
            Configuration = $config
            LastWriteTime = $fileInfo.LastWriteTime
        }
    }
}

# Find the file with the latest modification time
if ($existingFiles.Count -gt 0) {
    $latestFile = $existingFiles | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    Write-Host "Selected $executableName`: $($latestFile.Path) (Modified: $($latestFile.LastWriteTime))"

    # Try to read debug arguments from the corresponding .vcxproj.user file
    $userProjectFile = "$solutionFolder\$Executable\$Executable.vcxproj.user"
    $debugArgs = ""

    if (Test-Path $userProjectFile) {
        try {
            [xml]$userProjectXml = Get-Content $userProjectFile
            $configPlatform = $latestFile.Configuration
            
            if ($configPlatform) {
                # Find the PropertyGroup with the matching Condition
                $propertyGroup = $userProjectXml.Project.PropertyGroup | Where-Object { 
                    $_.Condition -eq "'`$(Configuration)|`$(Platform)'=='$configPlatform'" 
                }
                
                if ($propertyGroup -and $propertyGroup.LocalDebuggerCommandArguments) {
                    $debugArgs = $propertyGroup.LocalDebuggerCommandArguments
                    Write-Host "Found debug arguments: $debugArgs"
                }
            }
        }
        catch {
            Write-Host "Warning: Could not read debug arguments from $userProjectFile"
        }
    } else {
		Write-Host "Failed to find $userProjectFile"
	}

    # Execute the selected executable with debug arguments using cmd to handle argument parsing
    $commandLine = "/C $debugArgs"
    $startInfo = New-Object System.Diagnostics.ProcessStartInfo
    $startInfo.FileName = $latestFile.Path
    $startInfo.Arguments = $commandLine
    $startInfo.UseShellExecute = $false
    $process = [System.Diagnostics.Process]::Start($startInfo)
    $process.WaitForExit()
    exit $process.ExitCode
} else {
    throw "No $executableName files found in any of the expected locations."
}