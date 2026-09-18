param(
    [ValidateSet('x64','Win32')][string]$Platform = 'x64',
    [ValidateRange(1,65535)][int]$AsPort = 8888,
    [int]$Cycles = 1,
    [int]$FixtureProcessId,
    [switch]$GetterRegression
)

$ErrorActionPreference = 'Stop'
$product = Split-Path $PSScriptRoot -Parent
$repository = Split-Path (Split-Path $product -Parent) -Parent
$driver = Join-Path $PSScriptRoot 'Inspector.ps1'
$wrapper = Join-Path $repository '.github/Scripts/copilotExecute.ps1'
$endpoint = "http://localhost:$AsPort/Automation/UiaListApp"
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
    & $driver -AsPort $AsPort -Window $index -ClickLabel $label -DoubleClick:$Twice -Button:$OnlyButton -Scroll:($index -eq 0 -or ($index -eq 1 -and $OnlyButton)) | Out-Null
}
function Input([string]$command, [int]$index = 0) { & $driver -AsPort $AsPort -Window $index -Command $command | Out-Null }
function Synthetic-Count([string]$method, [int]$target = 1) {
    return @(Get-Content (Join-Path $PSScriptRoot "UiaFixture-$FixtureProcessId.synthetic.txt") -Encoding Unicode | Where-Object { $_ -match ("target=$target\s+" + [regex]::Escape($method) + "\s") }).Count
}
function Getter-Field([string]$getter, [string]$parameter) {
    & $driver -AsPort $AsPort -Window 1 -ClickLabel $getter -Scroll | Out-Null
    $composition = (State).SubWindows[0].composition
    $labels = @(Get-TextNodes $composition)
    $heading = @($labels | Where-Object Text -CEQ $getter)[0]
    $label = @($labels | Where-Object { $_.Text -like $parameter -and $_.Bounds.y1 -gt $heading.Bounds.y1 -and $_.Bounds.y1 -lt $heading.Bounds.y2 + 150 })[0]
    if (!$label) { throw "Missing parameter $parameter for $getter" }
    $pending = [Collections.Generic.Stack[object]]::new(); $pending.Push($composition)
    while ($pending.Count) {
        $node=$pending.Pop()
        if ($node.control -like '*TextBox*' -and $node.bounds.x1 -ge $label.Bounds.x2 -and $node.bounds.y1 -le $label.Bounds.y2 -and $node.bounds.y2 -ge $label.Bounds.y1) { return $node.bounds }
        foreach ($child in $node.children) { $pending.Push($child) }
    }
    throw "Missing text editor for $getter / $parameter"
}
function Edit-Getter($bounds, [string]$text) {
    Input ("!LeftClick:{0},{1}" -f [int](($bounds.x1+$bounds.x2)/2),[int](($bounds.y1+$bounds.y2)/2)) 1
    Input '!KeyPress:Ctrl+A' 1
    Input "!Type:$text" 1
    Input '!KeyPress:Enter' 1
}
function Wait-Getter([string]$getter, [string]$expected) {
    $deadline=[DateTime]::UtcNow.AddSeconds(30)
    do {
        $labels=@(Get-TextNodes (State).SubWindows[0].composition)
        $heading=@($labels | Where-Object Text -CEQ $getter)[0]
        if (@($labels | Where-Object { $_.Text -like $expected -and $_.Bounds.y1 -gt $heading.Bounds.y2 -and $_.Bounds.y1 -lt $heading.Bounds.y2+230 }).Count) { return }
        Start-Sleep -Milliseconds 100
    } while ([DateTime]::UtcNow -lt $deadline)
    throw "Getter $getter did not display $expected"
}

$userFile = Join-Path $product 'UiaListApp/UiaListApp.vcxproj.user'
$savedUser = if (Test-Path -LiteralPath $userFile) { [IO.File]::ReadAllBytes($userFile) } else { $null }
try {
    [xml]$settings = if ($savedUser) { [Text.Encoding]::UTF8.GetString($savedUser) } else { '<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003" />' }
    $group = $settings.CreateElement('PropertyGroup', $settings.DocumentElement.NamespaceURI)
    $group.SetAttribute('Condition', "'`$(Configuration)|`$(Platform)'=='Debug|$Platform'")
    foreach ($existing in $settings.DocumentElement.ChildNodes) {
        if ($existing.Condition -eq $group.Condition) {
            foreach ($oldArgument in @($existing.ChildNodes | Where-Object LocalName -EQ 'LocalDebuggerCommandArguments')) { $null = $existing.RemoveChild($oldArgument) }
        }
    }
    $argument = $settings.CreateElement('LocalDebuggerCommandArguments', $settings.DocumentElement.NamespaceURI)
    $argument.InnerText = "/AsPort:$AsPort"
    $null = $group.AppendChild($argument)
    $null = $settings.DocumentElement.AppendChild($group)
    $settings.Save($userFile)
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
    . $driver -AsPort $AsPort -Find '<no caption>' | Out-Null
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
    $s = State
    $readouts = @(Get-TextNodes $s.SubWindows[0].composition)
    if (!@($readouts | Where-Object { $_.Text -eq 'GetClickablePoint' -and $_.Control -ne 'Button' }).Count) { throw 'Pure getter has no direct readout.' }
    if (@($readouts | Where-Object { $_.Text -eq 'GetClickablePoint' -and $_.Control -eq 'Button' }).Count) { throw 'Pure getter still has a duplicate command button.' }
    if ((Invoke-Count) -ne $before) { throw 'Opening getter readouts accidentally invoked the target.' }
    Click 'Invoke' 1 -OnlyButton
    Wait-Ready 1 'Completed'
    Wait-Ready 0
    if ((Invoke-Count) -ne $before + 1) { throw 'Invoke did not reach the independent native target exactly once.' }
    Input '!Exit' 1
    if ($GetterRegression) {
        Write-Host 'TEST inspector / getter drafts, committed queries, references and stale detail results'
        Click 'Processes'
        Click '*synthetic providers*' -Twice
        Wait-Ready 0
        Click 'Nodes'
        Click 'Synthetic capabilities*' -Twice
        Wait-Ready 1
        Click 'Actions' 1
        $listening=Synthetic-Count 'StartListening'; $cancel=Synthetic-Count 'Cancel'
        Click 'StartListening' 1 -OnlyButton
        Wait-Ready 1 'Completed'
        Click 'Cancel' 1 -OnlyButton
        Wait-Ready 1 'Completed'
        if ((Synthetic-Count 'StartListening') -ne $listening+1 -or (Synthetic-Count 'Cancel') -ne $cancel+1) { throw 'Explicit listening commands did not execute exactly once' }
        $field=Getter-Field 'GetItem' 'row *'
        $calls=Synthetic-Count 'GetItem'
        Edit-Getter $field 'x'
        if ((Synthetic-Count 'GetItem') -ne $calls) { throw 'Invalid getter draft reached the provider' }
        Edit-Getter $field '0'
        Wait-Getter 'GetItem' 'IUIAutomationElement #*'
        if ((Synthetic-Count 'GetItem') -ne $calls+1) { throw 'Valid committed getter did not query exactly once' }
        $field=Getter-Field 'GetItemByName' 'name'
        Edit-Getter $field 'A1'
        Wait-Getter 'GetItemByName' 'IUIAutomationElement #*'
        Edit-Getter $field 'missing'
        Edit-Getter $field 'A1'
        Edit-Getter $field 'missing'
        Wait-Getter 'GetItemByName' 'No result*'
        Click 'References' 1
        $oldWindow=(State).SubWindows[0].windowId
        Click 'Synthetic capabilities IUIAutomationElement #*' 1 -Twice
        $deadline=[DateTime]::UtcNow.AddSeconds(30)
        while ((State).SubWindows[0].windowId -eq $oldWindow) {
            if ([DateTime]::UtcNow -gt $deadline) { throw 'Returned reference did not open a new detail window' }
            Start-Sleep -Milliseconds 100
        }
        Wait-Ready 1
        Click 'Actions' 1
        Click 'Providers' 1
        $field=Getter-Field 'GetItemByName' 'name'
        Edit-Getter $field 'A1'
        Input '!Exit' 1
        Click 'Synthetic document*' -Twice
        Wait-Ready 1
        Click 'Actions' 1
        $labels=@(Get-TextNodes (State).SubWindows[0].composition)
        if (!@($labels | Where-Object Text -Like '*IUIAutomationTextPattern*').Count) { throw 'Replacement detail did not retain its own target' }
        if (@($labels | Where-Object Text -CEQ 'GetItemByName').Count) { throw 'Stale getter changed replacement detail' }
        Click 'References' 1
        $deadline=[DateTime]::UtcNow.AddSeconds(30)
        while (!@((Get-TextNodes (State).SubWindows[0].composition) | Where-Object { $_.Text -like 'get_DocumentRange #*' -and $_.Bounds.y1 -gt 142 -and $_.Bounds.y2 -lt 560 }).Count) {
            if ([DateTime]::UtcNow -gt $deadline) { throw 'Document range reference was not found' }
            Input '!MouseMove:400,300' 1
            Input '!MouseWheelDown:6' 1
        }
        & $driver -AsPort $AsPort -Window 1 -ClickLabel 'get_DocumentRange #*' -DoubleClick -Scroll | Out-Null
        Click 'Text ranges' 1
        $clones=Synthetic-Count 'Range.Clone' 2
        Click 'Clone' 1 -OnlyButton
        Wait-Ready 1 'Completed'
        if ((Synthetic-Count 'Range.Clone' 2) -ne $clones+1) { throw 'Explicit text-range Clone did not execute exactly once' }
        Input '!Exit' 1
    }
    Click 'Refresh' -OnlyButton
    Wait-Ready 0
    Input '!Exit'
    if (!$process.WaitForExit(30000)) { Assert-InspectorAlive; throw 'Inspector did not finish normal shutdown.' }
    if ($process.ExitCode -ne 0) { throw "Inspector exited with $($process.ExitCode)." }
    if (Get-Process UiaListApp -ErrorAction SilentlyContinue) { throw 'Inspector child survived wrapper exit.' }
    [pscustomobject]@{Cycle=$cycle; Platform=$Platform; FixturePid=$FixtureProcessId; Seconds=([DateTime]::UtcNow-$started).TotalSeconds; InvokeCalls=1; ExitCode=$process.ExitCode} | ConvertTo-Json -Compress
}
} finally {
    if ($savedUser) { [IO.File]::WriteAllBytes($userFile, $savedUser) }
    elseif (Test-Path -LiteralPath $userFile) { Remove-Item -LiteralPath $userFile }
}
