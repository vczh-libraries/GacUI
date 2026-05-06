# Remember Copilot investigation notes

param(
    [switch]$Earliest,
    [Parameter(Position=0)]
    [ValidateRange(1, [int]::MaxValue)]
    [int]$Count = 1
)

if ($Earliest) {
    $learningRoot = Resolve-Path -LiteralPath "$PSScriptRoot\..\Learning"
    $earliestFolders = @(Get-ChildItem -LiteralPath $learningRoot -Directory |
        Sort-Object -Property Name |
        Select-Object -First $Count)

    if ($earliestFolders.Count -lt $Count) {
        Write-Output "Requested $Count backups and $($earliestFolders.Count) found:"
    }

    $earliestFolders | ForEach-Object {
        Write-Output $_.FullName
    }
    exit 0
}

$filePath = "$PSScriptRoot\..\TaskLogs\Copilot_Investigate.md"

if (Test-Path $filePath) {
    $timestamp = Get-Date -Format "yyyy-MM-dd-HH-mm-ss"
    $backupFolder = "$PSScriptRoot\..\Learning\$timestamp"
    $fileName = Split-Path $filePath -Leaf
    $destinationPath = Join-Path $backupFolder $fileName

    Write-Host "Creating backup folder: $backupFolder"
    New-Item -ItemType Directory -Path $backupFolder -Force | Out-Null

    Write-Host "Backing up $fileName to Learning folder..."
    Copy-Item -Path $filePath -Destination $destinationPath -Force

    Write-Host "Deleting $fileName..."
    Remove-Item -Path $filePath -Force
}

Write-Host "Copilot remember completed."
