function GetCDBPath {
    if ($env:CDBPATH -eq $null) {
        $MESSAGE_1 = "You have to add an environment variable named CDBPATH and set its value to the path of cdb.exe, e.g.:"
        $MESSAGE_2 = "C:\Program Files (x86)\Windows Kits\10\Debuggers\x64\cdb.exe"
        $MESSAGE_3 = "To get cdb.exe, you need to install WDK via VS Installer (Individual Components page) followed by Windows WDK."
        throw "$MESSAGE_1\r\n$MESSAGE_2\r\n$MESSAGE_3"
    }
    return $env:CDBPATH
}

function GetSolutionDir {
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
    return $solutionFolder
}

function GetLatestModifiedExecutable($solutionFolder, $executableName) {
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

    if ($existingFiles.Count -eq 0) {
        throw "No $executableName files found in any of the expected locations."
    }

    return $existingFiles | Sort-Object LastWriteTime -Descending | Select-Object -First 1
}

function GetDebugArgs($solutionFolder, $latestFile, $executable) {
    $userProjectFile = "$solutionFolder\$executable\$executable.vcxproj.user"
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
                    return $debugArgs
                }
            }
        }
        catch {
            Write-Host "Warning: Could not read debug arguments from $userProjectFile"
        }
    } else {
    	Write-Host "Failed to find $userProjectFile"
    }
    return ""
}