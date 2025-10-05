# Prepare Copilot workspace files

param(
    [switch]$Backup
)

# Create or override the markdown files with the specified content
$filesToOverride = @{
    "Copilot_Planning.md" = "# !!!PLANNING!!!"
    "Copilot_Execution.md" = "# !!!EXECUTION!!!"
    "Copilot_Task.md" = "# !!!TASK!!!"
}

# Create or override the markdown files with the specified content
$filesToCreate = @{
    "Copilot_Scrum.md" = "# !!!SCRUM!!!"
    "Copilot_KB.md" = "# !!!KNOWLEDGE BASE!!!"
}

# Backup each markdown file for learning
$filesToBackup = @()
foreach ($file in $filesToOverride.GetEnumerator()) {
    $filePath = "$PSScriptRoot\$($file.Key)"
    if (Test-Path $filePath) {
        $filesToBackup += $filePath
    }
}

if ($filesToBackup.Count -gt 0) {
    $timestamp = Get-Date -Format "yyyy-MM-dd-HH-mm-ss"
    $backupFolder = "$PSScriptRoot\..\Learning\$timestamp"
    
    Write-Host "Creating backup folder: $backupFolder"
    New-Item -ItemType Directory -Path $backupFolder -Force | Out-Null
    
    foreach ($filePath in $filesToBackup) {
        $fileName = Split-Path $filePath -Leaf
        $destinationPath = Join-Path $backupFolder $fileName
        Write-Host "Backing up $fileName to Learning folder..."
        Copy-Item -Path $filePath -Destination $destinationPath -Force
    }
}

if ($Backup) {
    # Delete all files in $filesToOverride
    foreach ($file in $filesToOverride.GetEnumerator()) {
        $filePath = "$PSScriptRoot\$($file.Key)"
        if (Test-Path $filePath) {
            Write-Host "Deleting $($file.Key)..."
            Remove-Item -Path $filePath -Force
        }
    }
} else {
    # Create each markdown file with the specified content
    foreach ($file in $filesToOverride.GetEnumerator()) {
        $filePath = "$PSScriptRoot\$($file.Key)"
        Write-Host "Creating/overriding $($file.Key)..."
        $file.Value | Out-File -FilePath $filePath -Encoding UTF8
    }

    # Create files only if they don't exist
    foreach ($file in $filesToCreate.GetEnumerator()) {
        $filePath = "$PSScriptRoot\$($file.Key)"
        if (-not (Test-Path $filePath)) {
            Write-Host "Creating $($file.Key)..."
            $file.Value | Out-File -FilePath $filePath -Encoding UTF8
        } else {
            Write-Host "Skipping $($file.Key) (already exists)..."
        }
    }
}

Write-Host "Copilot preparation completed."