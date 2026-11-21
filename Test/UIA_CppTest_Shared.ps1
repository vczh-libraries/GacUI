param(
    [ValidateSet('CppTest','CppTest_Metaonly','CppTest_Reflection','GacUI_Host','Playground')][string]$Application,
    [ValidateRange(1,65535)][int]$AsPort,
    [ValidateSet('Win32','x64')][string]$Platform = 'x64',
    [int]$ClientProcessId = 0,
    [ValidateRange(0,15)][int]$EditorSopStartStep = 0,
    [switch]$EditorSopResumeAtImages,
    [string]$BusyClick = '',
    [ValidateSet('All','List','Grid','Text','Refresh','Calendar','Layout','ShowcaseSkins','LocalizedLabels','Responsive','ResponsiveWindow','ResponsivePalette','Walk','Window','Concurrent','Review','Review2','Review3','Transitions','Lifetime','RawClock','RawClockHttp','ThemeProperty','RawClockPassive','RawClockListener','Touching','Persistence','DocumentExchange','DocumentStyles','GalleryLabels','DocumentContextMenu','DocumentEditorSop','DocumentEditorCommands','PointRegression','TouchingList','FileDialogMutations')][string]$Scenario = 'All',
    [switch]$HostedFixture,
    [switch]$GdiFixture,
    [switch]$Gdi,
    [string]$ExchangeDocument = '',
    [ValidateRange(0,5)][int]$ClockSwitch = 0,
    [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
$repository = Split-Path $PSScriptRoot -Parent
$solution = Join-Path $PSScriptRoot 'GacUISrc'
if ($ClientProcessId) {
    if ($Scenario -eq 'Lifetime') {
        Add-Type -Path (Join-Path $PSScriptRoot 'UIA_Lifetime.cs')
        try { [GacUILifetimeTests]::Run($ClientProcessId, $AsPort, [bool]$HostedFixture) }
        catch { [Console]::Error.WriteLine($_.Exception.ToString()); exit 1 }
        exit 0
    }
    Add-Type -AssemblyName UIAutomationClient, UIAutomationTypes, WindowsBase, System.Web.Extensions
    Add-Type -Path (Join-Path $PSScriptRoot 'UIA_CppTest_Shared.cs') -ReferencedAssemblies UIAutomationClient, UIAutomationTypes, WindowsBase, System.Web.Extensions, System.Xml, System.Net.Http
    [GacUIShowcaseTests]::ClockSwitch = $ClockSwitch
    [GacUIShowcaseTests]::BusyCommand = $BusyClick
    [GacUIShowcaseTests]::ExchangeDocument = $ExchangeDocument
    [GacUIShowcaseTests]::PersistenceOutput = Join-Path $PSScriptRoot 'Output/Injection'
    [GacUIShowcaseTests]::FileDialogPlayground = Join-Path $PSScriptRoot 'FileDialogPlayground'
    if ($Scenario -eq 'FileDialogMutations') { [GacUIShowcaseTests]::PersistenceOutput = [GacUIShowcaseTests]::FileDialogPlayground }
    [GacUIShowcaseTests]::EditorSopStartStep = $EditorSopStartStep
    [GacUIShowcaseTests]::EditorSopResumeAtImages = [bool]$EditorSopResumeAtImages
    try { [GacUIShowcaseTests]::Run($ClientProcessId, ($Application -eq 'CppTest' -or $HostedFixture), $Scenario, $AsPort, $Application) }
    catch { [Console]::Error.WriteLine($_.Exception.ToString()); exit 1 }
    exit 0
}
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class ShowcaseWindows {
    delegate bool Callback(IntPtr window, IntPtr parameter);
    [DllImport("user32.dll")] static extern bool EnumWindows(Callback callback, IntPtr parameter);
    [DllImport("user32.dll")] static extern bool EnumChildWindows(IntPtr parent, Callback callback, IntPtr parameter);
    [DllImport("user32.dll")] static extern int GetDlgCtrlID(IntPtr child);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern IntPtr SendMessageTimeout(IntPtr window, uint message, IntPtr wParam, StringBuilder text, uint flags, uint timeout, out IntPtr result);
    [DllImport("user32.dll")] static extern bool PostMessage(IntPtr window, uint message, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr window, out uint process);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] static extern int GetWindowText(IntPtr window, StringBuilder text, int length);
    public static bool Crashed(int target) {
        bool crashed = false;
        EnumWindows((window, parameter) => { uint process; GetWindowThreadProcessId(window, out process); if (process == target) { var title = new StringBuilder(512); GetWindowText(window, title, title.Capacity); if (title.ToString() == "Microsoft Visual C++ Runtime Library") crashed = true; } return true; }, IntPtr.Zero);
        return crashed;
    }
    public static string DismissRuntimeError(int target) {
        var report = new StringBuilder();
        EnumWindows((window, parameter) => {
            uint owner; GetWindowThreadProcessId(window, out owner);
            var title = new StringBuilder(256);
            if (owner != target || GetWindowText(window, title, title.Capacity) == 0 || title.ToString() != "Microsoft Visual C++ Runtime Library") return true;
            IntPtr abort = IntPtr.Zero, ok = IntPtr.Zero;
            EnumChildWindows(window, (child, unused) => {
                var text = new StringBuilder(4096); IntPtr result;
                SendMessageTimeout(child, 13, new IntPtr(text.Capacity), text, 2, 1000, out result);
                if (text.Length > 0) report.AppendLine(text.ToString());
                if (GetDlgCtrlID(child) == 3) abort = child;
                if (GetDlgCtrlID(child) == 1) ok = child;
                return true;
            }, IntPtr.Zero);
            if (abort != IntPtr.Zero || ok != IntPtr.Zero)
                PostMessage(abort != IntPtr.Zero ? abort : ok, 0x00F5, IntPtr.Zero, IntPtr.Zero);
            else
                PostMessage(window, 0x0010, IntPtr.Zero, IntPtr.Zero);
            return false;
        }, IntPtr.Zero);
        return report.ToString();
    }
}
'@
function Assert-Target {
    if ($owned.HasExited) { throw "Target unexpectedly exited: $($owned.ExitCode)" }
    if ([ShowcaseWindows]::Crashed($owned.Id)) { throw "Target PID $($owned.Id) has a blocking native runtime error dialog." }
}
if (!$SkipBuild) {
    Write-Host "TEST $Application / build Debug $Platform"
    Push-Location $solution
    try { & (Join-Path $repository '.github/Scripts/copilotBuild.ps1') -Configuration Debug -Platform $Platform }
    finally { Pop-Location }
    if ($LASTEXITCODE -ne 0) { throw 'Showcase build failed.' }
    $buildLog = Get-Content (Join-Path $repository '.github/Scripts/Build.log') -Raw
    if ($buildLog -notmatch 'Build succeeded\.') { throw 'Completed build log does not report success.' }
}

$userFile = Join-Path $solution "$Application/$Application.vcxproj.user"
$original = if (Test-Path -LiteralPath $userFile) { [IO.File]::ReadAllBytes($userFile) } else { $null }
$launcher = $null
$owned = $null
$client = $null
$listener = $null
$listenerReady = $null
$listenerStart = $null
try {
    if ($original) {
        $originalText = [Text.Encoding]::UTF8.GetString($original).TrimStart([char]0xFEFF)
        [xml]$settings = $originalText
    }
    else { [xml]$settings = '<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003" />' }
    # The execution wrapper reads this configuration; preserve all other user settings.
    $group = $settings.CreateElement('PropertyGroup', $settings.DocumentElement.NamespaceURI)
    $group.SetAttribute('Condition', "'`$(Configuration)|`$(Platform)'=='Debug|$Platform'")
    foreach ($existing in $settings.DocumentElement.ChildNodes) {
        if ($existing.Condition -eq $group.Condition) {
            foreach ($oldArgument in @($existing.ChildNodes | Where-Object LocalName -EQ 'LocalDebuggerCommandArguments')) { $null = $existing.RemoveChild($oldArgument) }
        }
    }
    $argument = $settings.CreateElement('LocalDebuggerCommandArguments', $settings.DocumentElement.NamespaceURI)
    $argument.InnerText = "/AsPort:$AsPort"
    if ($HostedFixture) { $argument.InnerText += ' /UiaHosted' }
    if ($GdiFixture) { $argument.InnerText += ' /UiaGdi' }
    if ($Gdi) { $argument.InnerText += ' /Gdi' }
    $null = $group.AppendChild($argument)
    $null = $settings.DocumentElement.AppendChild($group)
    $settings.Save($userFile)
    $before = @(Get-Process $Application -ErrorAction SilentlyContinue | ForEach-Object Id)
    Write-Host "TEST $Application / launch /AsPort:$AsPort"
    $wrapper = Join-Path $repository '.github/Scripts/copilotExecute.ps1'
    $launcher = Start-Process powershell.exe -WindowStyle Hidden -WorkingDirectory $solution -ArgumentList @('-NoProfile','-File',"`"$wrapper`"",'-Mode','CLI','-Executable',$Application,'-Configuration','Debug','-Platform',$Platform) -PassThru
    # Windows PowerShell needs a retained handle to read ExitCode after exit.
    $launcherHandle = $launcher.Handle
    $deadline = [DateTime]::UtcNow.AddSeconds(30)
    do {
        if ($launcher.HasExited) { throw "Launcher exited early: $($launcher.ExitCode)" }
        $candidates = @(Get-Process $Application -ErrorAction SilentlyContinue | Where-Object Id -NotIn $before)
        if ($candidates.Count -eq 1) { $owned = $candidates[0]; break }
        Start-Sleep -Milliseconds 100
    } while ([DateTime]::UtcNow -lt $deadline)
    if (!$owned) { throw 'Could not uniquely identify the launched process.' }
    $ownedHandle = $owned.Handle
    $endpoint = "http://localhost:$AsPort/Automation/$Application"
    do {
        Assert-Target
        try {
            $state = Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 15
            if ($state.MainWindow) { break }
            Start-Sleep -Milliseconds 100
        }
        catch { Assert-Target; Start-Sleep -Milliseconds 100 }
    } while ([DateTime]::UtcNow -lt $deadline)
    if (!$state.MainWindow) { throw 'Automation endpoint did not become ready.' }
    # A separate windowless MTA process provides a hard deadline even if a provider blocks.
    $clientArguments = @('-NoProfile','-Mta','-File',"`"$PSCommandPath`"",'-Application',$Application,'-AsPort',$AsPort,'-ClientProcessId',$owned.Id,'-Scenario',$Scenario,'-EditorSopStartStep',$EditorSopStartStep,'-ClockSwitch',$ClockSwitch)
    if ($EditorSopResumeAtImages) { $clientArguments += '-EditorSopResumeAtImages' }
    if ($ExchangeDocument) { $clientArguments += @('-ExchangeDocument', "`"$ExchangeDocument`"") }
    if ($Scenario -eq 'Review2') {
        function Find-Busy($node) {
            if ($node.elementText -eq 'Busy') { return $node.bounds }
            foreach ($child in $node.children) { $found = Find-Busy $child; if ($found) { return $found } }
        }
        $bounds = Find-Busy $state.MainWindow.composition
        if (!$bounds) { throw 'Missing Busy button in automation layout.' }
        $clientArguments += @('-BusyClick', ('!LeftClick:{0},{1}' -f [int](($bounds.x1+$bounds.x2)/2), [int](($bounds.y1+$bounds.y2)/2)))
    }
    if ($HostedFixture) { $clientArguments += '-HostedFixture' }
    if ($Scenario -eq 'RawClockPassive') {
        $listenerStart = [Threading.EventWaitHandle]::new($false, 'ManualReset', "GacUI.Clock.Start.$($owned.Id)")
        $listenerReady = [Threading.EventWaitHandle]::new($false, 'ManualReset', "GacUI.Clock.Ready.$($owned.Id)")
        $listenerArguments = $clientArguments | ForEach-Object { if ($_ -eq 'RawClockPassive') { 'RawClockListener' } else { $_ } }
        $listenerLog = Join-Path $env:TEMP "$Application-clock-listener"
        $listener = Start-Process powershell.exe -WindowStyle Hidden -ArgumentList $listenerArguments -RedirectStandardOutput "$listenerLog.stdout.txt" -RedirectStandardError "$listenerLog.stderr.txt" -PassThru
        $listenerHandle = $listener.Handle
    }
    $client = Start-Process powershell.exe -WindowStyle Hidden -ArgumentList $clientArguments -RedirectStandardOutput (Join-Path $env:TEMP "$Application-uia.stdout.txt") -RedirectStandardError (Join-Path $env:TEMP "$Application-uia.stderr.txt") -PassThru
    $clientHandle = $client.Handle
    $lastCount = 0
    # Full showcase coverage includes repeated layout rebuilds and palette replacement.
    $deadline = [DateTime]::UtcNow.AddMinutes($(if ($Scenario -in @('DocumentEditorSop','DocumentEditorCommands')) { 90 } else { 30 }))
    do {
        $lines = @(Get-Content (Join-Path $env:TEMP "$Application-uia.stdout.txt") -ErrorAction SilentlyContinue)
        $lines | Select-Object -Skip $lastCount | Write-Host
        $lastCount = $lines.Count
        if ($client.WaitForExit(500)) { break }
        if ($listener -and $listener.HasExited -and $listener.ExitCode -ne 0) { throw "Passive listener failed. $(Get-Content "$listenerLog.stderr.txt" -Raw)" }
        # Window.Close can finish before the client's final exit check returns.
        # Keep both exit codes authoritative instead of rejecting that ordering.
        if ($owned.HasExited) {
            if (!$client.WaitForExit(30000)) { throw "Target exited with code $($owned.ExitCode) before the UIA client completed." }
            break
        }
        if ([ShowcaseWindows]::Crashed($owned.Id)) {
            if ($Scenario -in @('RawClockHttp','ThemeProperty','RawClockPassive')) {
                Write-Host ([ShowcaseWindows]::DismissRuntimeError($owned.Id))
                if (!$owned.WaitForExit(5000)) {
                    Write-Host "Stopping the owned target after dismissing its runtime error dialog."
                    $owned.Kill()
                    $owned.WaitForExit()
                }
                Write-Host "Target exit after runtime error: $($owned.ExitCode)"
                $null = $client.WaitForExit(5000)
                Get-Content (Join-Path $env:TEMP "$Application-uia.stdout.txt") | Select-Object -Skip $lastCount | Write-Host
            }
            throw "Target PID $($owned.Id) has a blocking native runtime error dialog."
        }
    } while ([DateTime]::UtcNow -lt $deadline)
    if (!$client.HasExited) { $client.Kill(); throw 'MTA UIA test deadline exceeded.' }
    Get-Content (Join-Path $env:TEMP "$Application-uia.stdout.txt") | Select-Object -Skip $lastCount | Write-Host
    if ($client.ExitCode -ne 0) { throw "UIA client exit: $($client.ExitCode). $(Get-Content (Join-Path $env:TEMP "$Application-uia.stderr.txt") -Raw)" }
    if (!$launcher.WaitForExit(30000)) { throw 'Normal UIA Window.Close did not stop the launched application.' }
    if ($launcher.ExitCode -ne 0) { throw "Application exit: $($launcher.ExitCode)" }
    if ($listener) {
        if (!$listener.WaitForExit(30000) -or $listener.ExitCode -ne 0) { throw "Passive listener failed. $(Get-Content "$listenerLog.stderr.txt" -Raw)" }
        Get-Content "$listenerLog.stdout.txt" | Write-Host
    }
    if (Get-Process -Id $owned.Id -ErrorAction SilentlyContinue) { throw 'Owned process survived shutdown.' }
    $stillListening = $false
    try { $null = Invoke-RestMethod "$endpoint/Controls" -TimeoutSec 2; $stillListening = $true } catch { }
    if ($stillListening) { throw 'Owned endpoint survived shutdown.' }
    Write-Host "PASS $Application / normal shutdown and endpoint release"
}
finally {
    if ($listener -and !$listener.HasExited) { $listener.Kill() }
    if ($listenerReady) { $listenerReady.Dispose() }
    if ($listenerStart) { $listenerStart.Dispose() }
    if ($client -and !$client.HasExited) { $client.Kill() }
    if ($owned -and !$owned.HasExited) { $owned.Kill() }
    if ($launcher -and !$launcher.HasExited) { $launcher.Kill() }
    if ($original) { [IO.File]::WriteAllBytes($userFile, $original) }
    elseif (Test-Path -LiteralPath $userFile) { Remove-Item -LiteralPath $userFile }
}
