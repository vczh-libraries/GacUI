#requires -Version 7.0
# Shared process transport for the independent CLI verification drivers.
$ErrorActionPreference = 'Stop'

function Assert-Uia($Condition, [string]$Detail) {
    if (!$Condition) { throw $Detail }
}

function Assert-UiaEqual($Actual, $Expected, [string]$Detail = 'Values differ') {
    if ($null -eq $Actual -or $null -eq $Expected) {
        Assert-Uia ($null -eq $Actual -and $null -eq $Expected) $Detail
    } elseif ($Expected -is [System.Collections.IDictionary]) {
        Assert-Uia ($Actual -is [System.Collections.IDictionary] -and $Actual.Count -eq $Expected.Count) $Detail
        foreach ($key in $Expected.Keys) {
            Assert-Uia ($Actual.Contains($key)) "$Detail / missing $key"
            Assert-UiaEqual $Actual[$key] $Expected[$key] "$Detail / $key"
        }
    } elseif ($Expected -is [string] -or $Expected -is [bool]) {
        Assert-Uia ($Actual.GetType() -eq $Expected.GetType() -and $Actual -ceq $Expected) $Detail
    } elseif ($Expected -is [System.Collections.IEnumerable]) {
        Assert-Uia ($Actual -is [System.Collections.IEnumerable] -and $Actual.Count -eq $Expected.Count) $Detail
        for ($index = 0; $index -lt $Expected.Count; $index++) {
            Assert-UiaEqual $Actual[$index] $Expected[$index] "$Detail / $index"
        }
    } else {
        Assert-Uia ($Actual -eq $Expected) "$Detail (actual=$Actual, expected=$Expected)"
    }
}

function Start-UiaProcess {
    param([string]$Solution, [string]$Executable = 'UiaListCli', [string]$Configuration = 'Debug', [string]$Platform = 'x64')
    $repository = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
    $wrapper = (Join-Path $repository '.github/Scripts/copilotExecute.ps1').Replace("'", "''")
    $command = "& '$wrapper' -Mode CLI -Executable $Executable -Configuration $Configuration -Platform $Platform -Interactive 6>`$null"
    $start = [System.Diagnostics.ProcessStartInfo]::new()
    $start.FileName = (Get-Command pwsh -CommandType Application | Select-Object -First 1).Source
    $start.ArgumentList.Add('-NoProfile')
    $start.ArgumentList.Add('-EncodedCommand')
    $start.ArgumentList.Add([Convert]::ToBase64String([Text.Encoding]::Unicode.GetBytes($command)))
    $start.WorkingDirectory = $Solution
    $start.UseShellExecute = $false
    $start.CreateNoWindow = $true
    $start.RedirectStandardInput = $true
    $start.RedirectStandardOutput = $true
    $start.RedirectStandardError = $true
    $utf8 = [Text.UTF8Encoding]::new($false)
    $start.StandardInputEncoding = $utf8
    $start.StandardOutputEncoding = $utf8
    $start.StandardErrorEncoding = $utf8
    return [System.Diagnostics.Process]::Start($start)
}

class UiaCliClient {
    [System.Diagnostics.Process]$Process
    [object]$NextResponse
    [object]$ErrorText
    [string]$Output
    [int]$Count

    UiaCliClient([string]$Solution, [string]$Configuration, [string]$Platform) {
        $this.Process = Start-UiaProcess -Solution $Solution -Configuration $Configuration -Platform $Platform
        $this.NextResponse = $this.Process.StandardOutput.ReadLineAsync()
        $this.ErrorText = $this.Process.StandardError.ReadToEndAsync()
        $this.Output = Join-Path ([IO.Path]::GetTempPath()) "UiaListCli-$($this.Process.Id).jsonl"
        [IO.File]::WriteAllText($this.Output, '', [Text.UTF8Encoding]::new($false))
    }

    [object] ReadLine([int]$TimeoutMilliseconds) {
        if ($null -eq $this.NextResponse) { return $null }
        if (!$this.NextResponse.Wait($TimeoutMilliseconds)) { throw "Timed out waiting for CLI response; transcript: $($this.Output)" }
        $line = $this.NextResponse.GetAwaiter().GetResult()
        $this.NextResponse = if ($null -eq $line) { $null } else { $this.Process.StandardOutput.ReadLineAsync() }
        return $line
    }

    [object] Receive([string]$Command, [string]$InputLine, [bool]$ExpectedSuccess) {
        $raw = $this.ReadLine(120000)
        if ($null -eq $raw) { throw "CLI exited before response: $($this.ErrorText.GetAwaiter().GetResult())" }
        $response = ConvertFrom-Json -InputObject $raw -AsHashtable -Depth 100
        Assert-UiaEqual @($response.Keys | Sort-Object) @('command','error','generation','ok','result') 'Response envelope'
        Assert-UiaEqual $response.command (($Command -split '\s+')[0]) 'Response command'
        Assert-UiaEqual $response.ok $ExpectedSuccess "Command $Command; error=$($response.error | ConvertTo-Json -Compress)"
        Assert-Uia (($null -eq $response.error) -eq $ExpectedSuccess) 'Response error presence'
        $this.Count++
        $record = @{input=$InputLine; output=$response} | ConvertTo-Json -Compress -Depth 100
        [IO.File]::AppendAllText($this.Output, $record + "`n", [Text.UTF8Encoding]::new($false))
        return $response
    }

    [object] Send([string]$Command, [string]$Target, [object]$Arguments, [bool]$ExpectedSuccess) {
        $line = $Command
        if ($Target) { $line += " $Target" }
        if ($null -ne $Arguments) { $line += ' ' + (ConvertTo-Json -InputObject $Arguments -Compress -Depth 100) }
        $this.Process.StandardInput.WriteLine($line)
        $this.Process.StandardInput.Flush()
        return $this.Receive($Command, $line, $ExpectedSuccess)
    }

    [object] Send([string]$Command) { return $this.Send($Command, $null, $null, $true) }
    [object] Send([string]$Command, [string]$Target) { return $this.Send($Command, $Target, $null, $true) }
    [object] Send([string]$Command, [string]$Target, [object]$Arguments) { return $this.Send($Command, $Target, $Arguments, $true) }

    [void] Finish([bool]$ExpectedSuccess) {
        $this.Process.StandardInput.Close()
        if (!$this.Process.WaitForExit(30000)) { throw 'CLI did not exit within 30 seconds' }
        Assert-Uia (($this.Process.ExitCode -eq 0) -eq $ExpectedSuccess) "Unexpected CLI exit $($this.Process.ExitCode): $($this.ErrorText.GetAwaiter().GetResult())"
        Assert-Uia ($null -eq $this.ReadLine(5000)) 'Extra stdout response'
        Write-Host "PASS $($this.Count) commands; $($this.Output)"
        $this.Process.Dispose()
    }

    [void] Close([bool]$Eof) {
        if (!$Eof) { $null = $this.Send('Exit-Application') }
        $this.Finish($true)
    }
    [void] Close() { $this.Close($false) }
}

function New-UiaClient {
    param([string]$Solution = (Split-Path $PSScriptRoot -Parent), [string]$Configuration = 'Debug', [string]$Platform = 'x64')
    return [UiaCliClient]::new($Solution, $Configuration, $Platform)
}

function Get-UiaNode($Tree, [string]$Name) {
    $node = $Tree.nodes | Where-Object name -CEQ $Name | Select-Object -First 1
    Assert-Uia ($null -ne $node) "Missing node: $Name"
    return $node
}

function Read-UiaFixtureLog([string]$Path, [switch]$AsBytes) {
    $stream = [IO.File]::Open($Path, [IO.FileMode]::Open, [IO.FileAccess]::Read, [IO.FileShare]::ReadWrite)
    $buffer = [IO.MemoryStream]::new()
    try {
        $stream.CopyTo($buffer)
        if ($AsBytes) { return ,$buffer.ToArray() }
        return [Text.Encoding]::Unicode.GetString($buffer.ToArray())
    } finally {
        $buffer.Dispose()
        $stream.Dispose()
    }
}

function Get-UiaFixtureWindows($Client, [int]$FixtureProcessId) {
    $processes = $Client.Send('List-Process').result.processes
    $fixture = $processes | Where-Object pid -EQ $FixtureProcessId | Select-Object -First 1
    Assert-Uia ($null -ne $fixture) "Missing fixture PID $FixtureProcessId"
    return ,$fixture.windows
}
