param([int]$Cycles = 25, [switch]$CloseWithDeepTree, [ValidateRange(1,65535)][int]$AsPort = 8888, [int]$FixtureProcessId)

$ErrorActionPreference = 'Stop'
$driver = Join-Path $PSScriptRoot 'Inspector.ps1'
. $driver -AsPort $AsPort -Find '<no caption>' | Out-Null
if (!$FixtureProcessId) {
    $fixtures=@(Get-Process UiaFixture)
    if ($fixtures.Count -ne 1) { throw 'Supply -FixtureProcessId when captions are ambiguous.' }
    $FixtureProcessId=$fixtures[0].Id
}
& $driver -AsPort $AsPort -ClickLabel Processes | Out-Null
& $driver -AsPort $AsPort -ClickLabel Refresh | Out-Null

function Wait-Tree([string]$expected, [switch]$AllowNoSelection) {
    $deadline = [DateTime]::UtcNow.AddMinutes(3)
    do {
        Assert-InspectorAlive
        try { $state = Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15 }
        catch { Assert-InspectorAlive; throw }
        $labels = @(Get-TextNodes $state.MainWindow.composition)
        if (@($labels | Where-Object { $_.Control -eq 'Label' -and ($_.Text -like $expected -or ($AllowNoSelection -and $_.Text -like 'Select a process, then click a window*')) }).Count) { return }
        Start-Sleep -Milliseconds 250
    } while ([DateTime]::UtcNow -lt $deadline)
    throw "Tree did not reach '$expected'."
}

# Open the fixture's large/deep window before starting. Each iteration repeats
# the original crash trigger: publish the complete deep tree, then replace it.
Wait-Tree 'Ready*' -AllowNoSelection
& $driver -AsPort $AsPort -SelectProcessId $FixtureProcessId | Out-Null
for ($cycle = 1; $cycle -le $Cycles; $cycle++) {
    $started = [DateTime]::UtcNow
    & $driver -AsPort $AsPort -ClickLabel Processes | Out-Null
    & $driver -AsPort $AsPort -ClickLabel '*10,000 siblings and 1,000 levels*' | Out-Null
    Wait-Tree 'Ready (11053)'
    & $driver -AsPort $AsPort -ClickLabel Nodes | Out-Null
    & $driver -AsPort $AsPort -ClickLabel Processes | Out-Null
    & $driver -AsPort $AsPort -ClickLabel '*synthetic providers*' | Out-Null
    Wait-Tree 'Ready (51)'
    [pscustomobject]@{Cycle=$cycle; RawNodes=11053; ReplacementNodes=51; Seconds=([DateTime]::UtcNow-$started).TotalSeconds} | ConvertTo-Json -Compress
}

if ($CloseWithDeepTree) {
    & $driver -AsPort $AsPort -ClickLabel Processes | Out-Null
    & $driver -AsPort $AsPort -ClickLabel '*10,000 siblings and 1,000 levels*' | Out-Null
    Wait-Tree 'Ready (11053)'
    & $driver -AsPort $AsPort -Command '!Exit'
    # Under CDB, confirm the debuggee's exit event and Debug leak output.
}
