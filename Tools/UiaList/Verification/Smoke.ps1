param(
    [ValidateSet('x64','Win32')][string]$Platform = 'x64',
    [int]$Cycles = 1,
    [int]$FixtureProcessId
)

$ErrorActionPreference = 'Stop'
$product = Split-Path $PSScriptRoot -Parent
$repository = Split-Path (Split-Path $product -Parent) -Parent
$driver = Join-Path $PSScriptRoot 'Inspector.ps1'
$wrapper = Join-Path $repository '.github/Scripts/copilotExecute.ps1'
$endpoint = 'http://localhost:8888/Automation/UiaListApp'
if (!$FixtureProcessId) {
    $fixtures = @(Get-Process UiaFixture)
    if ($fixtures.Count -ne 1) { throw 'Supply -FixtureProcessId when more than one fixture is running.' }
    $FixtureProcessId = $fixtures[0].Id
}
$callLog = Join-Path $PSScriptRoot "UiaFixture-$FixtureProcessId.calls.txt"
if (!(Test-Path -LiteralPath $callLog)) { throw "Missing independent fixture log: $callLog" }
function Invoke-Count {
    return @(Get-Content -LiteralPath $callLog -Encoding Unicode | Where-Object { $_ -match 'target=101\s+WM_COMMAND\s+code=0\s' }).Count
}
function State {
    Assert-InspectorAlive
    try { return Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15 }
    catch { Assert-InspectorAlive; throw }
}
function Wait-Ready([int]$index, [string]$expected = 'Ready*') {
    $deadline = [DateTime]::UtcNow.AddSeconds(30)
    do {
        $s = State
        $windows = @($s.MainWindow) + @($s.SubWindows)
        if ($windows.Count -gt $index) {
            $labels = @(Get-TextNodes $windows[$index].composition)
            if (@($labels | Where-Object { $_.Control -eq 'Label' -and $_.Text -like $expected }).Count) { return }
        }
        Start-Sleep -Milliseconds 100
    } while ([DateTime]::UtcNow -lt $deadline)
    throw "Window $index did not reach '$expected'."
}
function Click([string]$label, [int]$index = 0, [switch]$Twice, [switch]$OnlyButton) {
    & $driver -Window $index -ClickLabel $label -DoubleClick:$Twice -Button:$OnlyButton -Scroll:($index -eq 1 -and $OnlyButton) | Out-Null
}
function Input([string]$command, [int]$index = 0) { & $driver -Window $index -Command $command | Out-Null }

for ($cycle = 1; $cycle -le $Cycles; $cycle++) {
    if (Get-Process UiaListApp -ErrorAction SilentlyContinue) { throw 'An inspector is already running.' }
    $before = Invoke-Count
    $started = [DateTime]::UtcNow
    $hostExecutable = (Get-Process -Id $PID).Path
    # The repository wrapper launches each ordinary Debug GUI run.
    $process = Start-Process -FilePath $hostExecutable -WindowStyle Hidden -WorkingDirectory $product -ArgumentList @('-NoProfile','-File',$wrapper,'-Mode','CLI','-Executable','UiaListApp','-Configuration','Debug','-Platform',$Platform) -PassThru
    $deadline = [DateTime]::UtcNow.AddSeconds(30)
    do {
        if ($process.HasExited) { throw "Inspector startup exited with $($process.ExitCode)." }
        try { $null = Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15; break } catch { Start-Sleep -Milliseconds 100 }
    } while ([DateTime]::UtcNow -lt $deadline)
    . $driver -Find '<no caption>' | Out-Null
    Click '*UiaFixture,*' -Twice
    Wait-Ready 0
    Click 'Nodes'
    Click 'Invoke counter (*' -Twice
    Wait-Ready 1
    Click 'UIA_NamePropertyId*' 1 -Twice
    $s = State
    if (@($s.SubWindows).Count -ne 2) { throw 'The read-only detail modal did not open.' }
    Input '!Exit' 2
    Click 'Actions' 1
    Click 'Invoke' 1 -OnlyButton
    Wait-Ready 1 'Completed'
    Wait-Ready 0
    if ((Invoke-Count) -ne $before + 1) { throw 'Invoke did not reach the independent native target exactly once.' }
    Input '!Exit' 1
    Click 'Refresh' -OnlyButton
    Wait-Ready 0
    Input '!Exit'
    if (!$process.WaitForExit(30000)) { Assert-InspectorAlive; throw 'Inspector did not finish normal shutdown.' }
    if ($process.ExitCode -ne 0) { throw "Inspector exited with $($process.ExitCode)." }
    if (Get-Process UiaListApp -ErrorAction SilentlyContinue) { throw 'Inspector child survived wrapper exit.' }
    [pscustomobject]@{Cycle=$cycle; Platform=$Platform; FixturePid=$FixtureProcessId; Seconds=([DateTime]::UtcNow-$started).TotalSeconds; InvokeCalls=1; ExitCode=$process.ExitCode} | ConvertTo-Json -Compress
}
