# Prepare Copilot review files

$taskLogsDir = "$PSScriptRoot\..\TaskLogs"

# Delete all Copilot_Review_Finished_*.md files
Remove-Item -Path "$taskLogsDir\Copilot_Review_Finished_*.md" -Force -ErrorAction SilentlyContinue

# Rename all Copilot_Review_Writing_*.md files to Copilot_Review_Finished_*.md
Get-ChildItem -Path "$taskLogsDir\Copilot_Review_Writing_*.md" -ErrorAction SilentlyContinue | ForEach-Object {
    $newName = $_.Name -replace '^Copilot_Review_Writing_', 'Copilot_Review_Finished_'
    Rename-Item -Path $_.FullName -NewName $newName
}
