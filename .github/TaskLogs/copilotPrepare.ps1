# Prepare Copilot workspace files

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

Write-Host "Copilot preparation completed."