param([ValidateSet('Invalid','Local','Concurrent','Remote','Release','All')][string]$Suite = 'All')
$ErrorActionPreference = 'Stop'
$repository = Split-Path $PSScriptRoot -Parent
$wrapper = Join-Path $repository '.github/Scripts/copilotExecute.ps1'
$active = [Collections.Generic.List[object]]::new()
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class PortTestWindows {
 public delegate bool Callback(IntPtr w,IntPtr p);
 [DllImport("user32.dll")] public static extern bool EnumWindows(Callback callback,IntPtr p);
 [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr w,out uint p);
 [DllImport("user32.dll",CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr w,StringBuilder s,int n);
 [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr w,uint m,IntPtr a,IntPtr b);
 public static bool Crash(int process) { bool crash=false; EnumWindows((w,p)=>{ uint id; GetWindowThreadProcessId(w,out id); var s=new StringBuilder(512); GetWindowText(w,s,512); if(id==process && s.ToString()=="Microsoft Visual C++ Runtime Library") crash=true; return true;},IntPtr.Zero); return crash; }
 public static void Close(int process) { EnumWindows((w,p)=>{uint id;GetWindowThreadProcessId(w,out id);if(id==process)PostMessage(w,16,IntPtr.Zero,IntPtr.Zero);return true;},IntPtr.Zero); }
 public static string Showcase(int process) { string handle=null; EnumWindows((w,p)=>{uint id;GetWindowThreadProcessId(w,out id);var title=new StringBuilder(512);GetWindowText(w,title,512);if(id==process && title.ToString()=="Complete Control Showcase")handle="0x"+w.ToInt64().ToString("X");return true;},IntPtr.Zero);return handle; }
}
'@
function Start-App([string]$name, [string]$arguments, [int]$port, [switch]$Invalid, [string]$configuration = 'Debug') {
    $solution = if ($name -eq 'UiaListApp') { Join-Path $repository 'Tools/UiaList' } else { Join-Path $PSScriptRoot 'GacUISrc' }
    $file = Join-Path $solution "$name/$name.vcxproj.user"
    $saved = if (Test-Path -LiteralPath $file) { [IO.File]::ReadAllBytes($file) } else { $null }
    [xml]$xml = if ($saved) { [Text.Encoding]::UTF8.GetString($saved) } else { '<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003" />' }
    $condition = "'`$(Configuration)|`$(Platform)'=='$configuration|x64'"
    foreach ($group in $xml.DocumentElement.ChildNodes) {
        if ($group.Condition -eq $condition) { foreach ($old in @($group.ChildNodes | Where-Object LocalName -EQ 'LocalDebuggerCommandArguments')) { $null = $group.RemoveChild($old) } }
    }
    $group = $xml.CreateElement('PropertyGroup',$xml.DocumentElement.NamespaceURI); $group.SetAttribute('Condition',$condition)
    $argument = $xml.CreateElement('LocalDebuggerCommandArguments',$xml.DocumentElement.NamespaceURI); $argument.InnerText = $arguments
    $null = $group.AppendChild($argument); $null = $xml.DocumentElement.AppendChild($group); $xml.Save($file)
    $before = @(Get-Process $name -ErrorAction SilentlyContinue | ForEach-Object Id)
    $launcher = Start-Process powershell.exe -WindowStyle Hidden -WorkingDirectory $solution -ArgumentList @('-NoProfile','-File',$wrapper,'-Mode','CLI','-Executable',$name,'-Configuration',$configuration,'-Platform','x64') -RedirectStandardOutput "$env:TEMP/ports-$name.stdout.txt" -RedirectStandardError "$env:TEMP/ports-$name.stderr.txt" -PassThru
    $endpointName = if ($name -eq 'RemotingTest_Rendering_Win32') { 'RemotingTest_Rendering_Native' } else { $name }
    $run = [pscustomobject]@{ Name=$name; Launcher=$launcher; Process=$null; File=$file; Saved=$saved; Endpoint="http://localhost:$port/Automation/$endpointName" }
    $active.Add($run)
    if ($Invalid) {
        if (!$launcher.WaitForExit(15000)) { throw "$name did not reject $arguments" }
        if ($launcher.ExitCode -eq 0) { throw "$name accepted $arguments" }
        if (@(Get-Process $name -ErrorAction SilentlyContinue | Where-Object Id -NotIn $before).Count) { throw "$name left a process after rejection" }
        return $run
    }
    $deadline = [DateTime]::UtcNow.AddSeconds(30)
    do {
        if ($launcher.HasExited) { throw "$name exited early: $($launcher.ExitCode)" }
        $found = @(Get-Process $name -ErrorAction SilentlyContinue | Where-Object Id -NotIn $before)
        if ($found.Count -eq 1) { $run.Process = $found[0]; return $run }
        Start-Sleep -Milliseconds 100
    } while ([DateTime]::UtcNow -lt $deadline)
    throw "Could not identify owned $name process"
}
function Ready($run, [string]$path = 'Controls') {
    $deadline = [DateTime]::UtcNow.AddSeconds(45)
    do {
        if ($run.Process.HasExited) { throw "$($run.Name) exited before readiness: $($run.Process.ExitCode)" }
        if ([PortTestWindows]::Crash($run.Process.Id)) { throw "Native runtime dialog in $($run.Name), PID $($run.Process.Id)" }
        try { $state = Invoke-RestMethod "$($run.Endpoint)/$path" -TimeoutSec 15; if ($state) { return $state } } catch { Start-Sleep -Milliseconds 100 }
    } while ([DateTime]::UtcNow -lt $deadline)
    throw "Endpoint not ready: $($run.Endpoint)/$path"
}
function Input($run,[string]$command) {
    $response = Invoke-WebRequest "$($run.Endpoint)/IO" -Method Post -Headers @{'Content-Type'='application/json; charset=utf8'} -SkipHeaderValidation -Body ([Text.Encoding]::UTF8.GetBytes($command)) -TimeoutSec 15
    if ($response.Content -ne 'Queued') { throw "IO rejected: $($response.Content)" }
}
function Restore($run) {
    if ($null -ne $run.Saved) { [IO.File]::WriteAllBytes($run.File,$run.Saved) }
    elseif (Test-Path -LiteralPath $run.File) { Remove-Item -LiteralPath $run.File }
    $null = $active.Remove($run)
}
function Stopped($run, [int]$expectedExit = 0) {
    if (!$run.Launcher.WaitForExit(30000)) { throw "$($run.Name) did not shut down" }
    if ($run.Launcher.ExitCode -ne $expectedExit) { throw "$($run.Name) exit $($run.Launcher.ExitCode), expected $expectedExit" }
    $responding = $false
    try { $null = Invoke-WebRequest "$($run.Endpoint)/Controls" -TimeoutSec 2; $responding=$true } catch { }
    if ($responding) { throw "$($run.Name) endpoint survived" }
    Restore $run
}
try {
    $locals = 'CppTest','CppTest_Metaonly','CppTest_Reflection','GacUI_Host','Playground','UiaListApp'
    if ($Suite -in 'Invalid','All') {
        foreach ($name in @($locals) + @('CppTest_Rvm','RemotingTest_Core','RemotingTest_Rendering_Win32')) {
            $base = switch ($name) { 'CppTest_Rvm' { '/Pipe' } 'RemotingTest_Core' { '/Pipe /FCT' } 'RemotingTest_Rendering_Win32' { '/Pipe' } default { '' } }
            foreach ($bad in '/AsPort:','/AsPort:x','/AsPort:0','/AsPort:65536','/AsPort:8888 /AsPort:8890') {
                Write-Host "TEST $name rejects $bad"
                $run=Start-App $name "$base $bad" 0 -Invalid; Restore $run
            }
        }
    }
    if ($Suite -in 'Local','All') {
        foreach ($name in $locals) { foreach ($port in 8888,8901) {
            Write-Host "TEST $name / automation port $port"
            $arguments = if ($port -eq 8888) { '' } else { "/AsPort:$port" }
            $run=Start-App $name $arguments $port; $null=Ready $run; Input $run '!Exit'; Stopped $run
        } }
    }
    if ($Suite -in 'Concurrent','All') {
        $inspectorDriver = Join-Path $repository 'Tools/UiaList/Verification/Inspector.ps1'
        for ($cycle=1; $cycle -le 2; $cycle++) {
            Write-Host "TEST concurrent endpoints / cycle $cycle / hosted, ordinary, inspector"
            $hosted=Start-App CppTest '/AsPort:8888' 8888
            $ordinary=Start-App CppTest_Metaonly '/AsPort:8890' 8890
            $inspector=Start-App UiaListApp '/AsPort:8891' 8891
            $null=Ready $hosted; $null=Ready $ordinary; $null=Ready $inspector
            # Initial discovery can precede the other applications showing their HWNDs.
            & $inspectorDriver -AsPort 8891 -ClickLabel 'Refresh' -Button | Out-Null
            foreach ($target in @($hosted,$ordinary)) {
                $handle=[PortTestWindows]::Showcase($target.Process.Id)
                if (!$handle) { throw "Missing owned showcase HWND for $($target.Name)" }
                Write-Host "TEST inspector / $($target.Name), PID $($target.Process.Id), HWND $handle"
                & $inspectorDriver -AsPort 8891 -Command '!MouseMove:600,400' | Out-Null
                & $inspectorDriver -AsPort 8891 -Command '!MouseWheelUp:1000' | Out-Null
                & $inspectorDriver -AsPort 8891 -ClickLabel "*Complete Control Showcase*$handle*" -DoubleClick -Scroll | Out-Null
                $deadline=[DateTime]::UtcNow.AddSeconds(60)
                do {
                    $ready=@(& $inspectorDriver -AsPort 8891 -Find 'Ready*' | ConvertFrom-Json | Where-Object Text -Like 'Ready*')
                    if ($ready.Count) { break }
                    Start-Sleep -Milliseconds 100
                } while ([DateTime]::UtcNow -lt $deadline)
                if (!$ready.Count) { throw 'Inspector did not finish reading target tree' }
                & $inspectorDriver -AsPort 8891 -ClickLabel 'Nodes' | Out-Null
                $nodeCaption='Complete Control Showcase (IUIAutomationElement*'
                $deadline=[DateTime]::UtcNow.AddSeconds(60)
                do {
                    $shown=@(& $inspectorDriver -AsPort 8891 -Find $nodeCaption | ConvertFrom-Json | Where-Object Text -Like $nodeCaption)
                    if ($shown.Count) { break }
                    Start-Sleep -Milliseconds 100
                } while ([DateTime]::UtcNow -lt $deadline)
                if (!$shown.Count) { throw 'Inspector did not show its Nodes tab' }
                & $inspectorDriver -AsPort 8891 -ClickLabel $nodeCaption -Scroll | Out-Null
                & $inspectorDriver -AsPort 8891 -Command '!KeyPress:Enter' | Out-Null
                $deadline=[DateTime]::UtcNow.AddSeconds(60)
                do {
                    $detailState=Ready $inspector
                    if (@($detailState.SubWindows).Count) { break }
                    Start-Sleep -Milliseconds 100
                } while ([DateTime]::UtcNow -lt $deadline)
                if (!@($detailState.SubWindows).Count) { throw 'Inspector did not open target properties' }
                do {
                    $ready=@(& $inspectorDriver -AsPort 8891 -Window 1 -Find 'Ready*' | ConvertFrom-Json | Where-Object Text -Like 'Ready*')
                    if ($ready.Count) { break }
                    Start-Sleep -Milliseconds 100
                } while ([DateTime]::UtcNow -lt $deadline)
                if (!$ready.Count) { throw 'Inspector did not finish target properties' }
                & $inspectorDriver -AsPort 8891 -Window 1 -ClickLabel 'Actions' | Out-Null
                if (!@(& $inspectorDriver -AsPort 8891 -Window 1 -Find '*IUIAutomationWindowPattern*' | ConvertFrom-Json | Where-Object Text -Like '*IUIAutomationWindowPattern*').Count) { throw 'Missing Window action section' }
                Write-Host "PASS inspector / $($target.Name) native Window actions"
                & $inspectorDriver -AsPort 8891 -Window 1 -Command '!Exit' | Out-Null
                & $inspectorDriver -AsPort 8891 -ClickLabel 'Processes' | Out-Null
            }
            foreach ($target in @($hosted,$ordinary)) {
                $port=if($target.Name -eq 'CppTest'){8888}else{8890}
                & powershell.exe -NoProfile -Mta -File (Join-Path $PSScriptRoot 'UIA_Showcase.ps1') -Application $target.Name -AsPort $port -ClientProcessId $target.Process.Id -Scenario Concurrent
                if ($LASTEXITCODE -ne 0) { throw "Concurrent UIA actions failed for $($target.Name)" }
            }
            $null=Ready $hosted; $null=Ready $ordinary; $null=Ready $inspector
            Input $inspector '!Exit'; Stopped $inspector
            Input $ordinary '!Exit'; Stopped $ordinary
            Input $hosted '!Exit'; Stopped $hosted
        }
    }
    if ($Suite -in 'Remote','All') {
        foreach ($case in @(@('Pipe',8902,8888),@('Pipe',8888,8889),@('Http',8902,8889),@('MiniHttp',8888,8889),@('MiniHttp',8902,8889))) {
            $transport=$case[0]; $port=$case[1]; $rendererPort=$case[2]
            Write-Host "TEST Core /$transport automation=$port; renderer=$rendererPort; protocol=8888"
            $coreArguments = "/$transport /FCT"
            if ($port -ne 8888) { $coreArguments += " /AsPort:$port" }
            $core=Start-App RemotingTest_Core $coreArguments $port
            $rendererArguments = if ($rendererPort -eq 8888) { "/$transport" } else { "/$transport /AsPort:$rendererPort" }
            $renderer=Start-App RemotingTest_Rendering_Win32 $rendererArguments $rendererPort
            $null=Ready $core; $null=Ready $renderer Dom
            if ($transport -ne 'Pipe' -and !(Get-NetTCPConnection -LocalPort 8888 -State Listen -ErrorAction SilentlyContinue)) { throw 'Missing fixed protocol listener on 8888' }
            Input $core '!Exit'; Stopped $renderer; Stopped $core
        }
        $hostPath=Join-Path $PSScriptRoot 'GacUISrc/x64/Debug/RemotingTest_RvmHost.exe'
        foreach ($transport in 'Pipe','Http','MiniHttp','Cli') { foreach ($port in 8888,8902) {
            Write-Host "TEST CppTest_Rvm /$transport automation=$port; RVM protocol=8888"
            $arguments=if($transport -eq 'Cli'){'/Cli:"' + $hostPath + '"'}else{"/$transport"}
            if ($port -ne 8888) { $arguments += " /AsPort:$port" }
            $rvm=Start-App CppTest_Rvm $arguments $port
            $hostRun=if($transport -ne 'Cli'){Start-App RemotingTest_RvmHost "/$transport" 0}else{$null}
            $null=Ready $rvm
            if ($transport -in 'Http','MiniHttp' -and !(Get-NetTCPConnection -LocalPort 8888 -State Listen -ErrorAction SilentlyContinue)) { throw 'Missing RVM protocol listener' }
            Input $rvm '!Exit'; Stopped $rvm
            # ViewModelHostClient deliberately uses _Exit(1) when its requester disconnects.
            if($hostRun){Stopped $hostRun 1}
        } }
        Write-Host 'TEST Core /MiniHttp /RVMT with stdio host and split automation'
        $core=Start-App RemotingTest_Core ('/MiniHttp /RVMT /AsPort:8902 /Cli:"' + $hostPath + '"') 8902
        $renderer=Start-App RemotingTest_Rendering_Win32 '/MiniHttp /AsPort:8889' 8889
        $null=Ready $core; $null=Ready $renderer Dom; Input $core '!Exit'; Stopped $renderer; Stopped $core
    }
    if ($Suite -in 'Release','All') {
        Write-Host 'TEST Release UiaList / no automation listener'
        $run=Start-App UiaListApp '/AsPort:8901' 8901 -configuration Release
        Start-Sleep -Seconds 3
        if ($run.Process.HasExited -or [PortTestWindows]::Crash($run.Process.Id)) { throw 'Release inspector failed startup' }
        $response=$false; try { $null=Invoke-WebRequest "$($run.Endpoint)/Controls" -TimeoutSec 2; $response=$true } catch { }
        if($response){throw 'Release inspector exposed an endpoint'}
        [PortTestWindows]::Close($run.Process.Id); Stopped $run
    }
    Write-Host "PASS automation ports / $Suite"
} finally {
    foreach ($run in @($active.ToArray())) {
        if ($run.Process -and !$run.Process.HasExited) { $run.Process.Kill() }
        if (!$run.Launcher.HasExited) { $run.Launcher.Kill() }
        Restore $run
    }
}
