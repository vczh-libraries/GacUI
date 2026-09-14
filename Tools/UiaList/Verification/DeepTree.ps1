param([int]$Cycles = 25, [switch]$CloseWithDeepTree)

$ErrorActionPreference = 'Stop'
$driver = Join-Path $PSScriptRoot 'Inspector.ps1'
. $driver -Find '<no caption>' | Out-Null

function Wait-Tree([string]$expected) {
    $deadline = [DateTime]::UtcNow.AddMinutes(3)
    do {
        Assert-InspectorAlive
        try { $state = Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15 }
        catch { Assert-InspectorAlive; throw }
        $labels = @(Get-TextNodes $state.MainWindow.composition)
        if (@($labels | Where-Object { $_.Control -eq 'Label' -and $_.Text -like $expected }).Count) { return }
        Start-Sleep -Milliseconds 250
    } while ([DateTime]::UtcNow -lt $deadline)
    throw "Tree did not reach '$expected'."
}

# Open the fixture's large/deep window before starting. Each iteration repeats
# the original crash trigger: publish the complete deep tree, then replace it.
for ($cycle = 1; $cycle -le $Cycles; $cycle++) {
    $started = [DateTime]::UtcNow
    & $driver -ClickLabel Processes | Out-Null
    & $driver -ClickLabel '*10,000 siblings and 1,000 levels*' -DoubleClick | Out-Null
    Wait-Tree 'Ready (11053)'
    & $driver -ClickLabel Nodes | Out-Null
    & $driver -ClickLabel Processes | Out-Null
    & $driver -ClickLabel '*synthetic providers*' -DoubleClick | Out-Null
    Wait-Tree 'Ready (51)'
    [pscustomobject]@{Cycle=$cycle; RawNodes=11053; ReplacementNodes=51; Seconds=([DateTime]::UtcNow-$started).TotalSeconds} | ConvertTo-Json -Compress
}

if ($CloseWithDeepTree) {
    & $driver -ClickLabel Processes | Out-Null
    & $driver -ClickLabel '*10,000 siblings and 1,000 levels*' -DoubleClick | Out-Null
    Wait-Tree 'Ready (11053)'
    & $driver -Command '!Exit'
    # Under CDB, confirm the debuggee's exit event and Debug leak output.
}
