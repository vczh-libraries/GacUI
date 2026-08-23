param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('cpptest_rvm', 'fct', 'rpt', 'rvmt')]
    [string]$App,

    [Parameter(Mandatory = $true)]
    [ValidateSet('http', 'pipe', 'minihttp')]
    [string]$Protocol,

    [switch]$Cli
)

$ErrorActionPreference = 'Stop'

$bin = Join-Path $PSScriptRoot 'GacUISrc\x64\Debug'
$hostExe = Join-Path $bin 'RemotingTest_RvmHost.exe'
$protocolArgument = switch ($Protocol) {
    'http' { '/Http' }
    'pipe' { '/Pipe' }
    'minihttp' { '/MiniHttp' }
}

if ($Cli -and $App -ne 'cpptest_rvm' -and $App -ne 'rvmt') {
    throw '-Cli is only supported by -App cpptest_rvm or -App rvmt.'
}

if ($App -eq 'cpptest_rvm') {
    $application = Join-Path $bin 'CppTest_Rvm.exe'
    if ($Cli) {
        $applicationArguments = @('/Cli:"{0}"' -f $hostExe)
    }
    else {
        $applicationArguments = @($protocolArgument)
    }
}
else {
    $application = Join-Path $bin 'RemotingTest_Core.exe'
    $appArgument = switch ($App) {
        'fct' { '/FCT' }
        'rpt' { '/RPT' }
        'rvmt' { '/RVMT' }
    }
    $applicationArguments = @($protocolArgument, $appArgument)
    if ($Cli) {
        $applicationArguments += '/Cli:"{0}"' -f $hostExe
    }
}

foreach ($executable in @($application)) {
    if (-not (Test-Path -LiteralPath $executable -PathType Leaf)) {
        throw "Test application is not built in Debug x64: $executable"
    }
}

if (($App -eq 'cpptest_rvm' -or $App -eq 'rvmt') -and -not (Test-Path -LiteralPath $hostExe -PathType Leaf)) {
    throw "Test application is not built in Debug x64: $hostExe"
}

Write-Host "Starting: $application $($applicationArguments -join ' ')"
$applicationProcess = Start-Process -FilePath $application -ArgumentList $applicationArguments -PassThru
Write-Output $applicationProcess

if (-not $Cli -and ($App -eq 'cpptest_rvm' -or $App -eq 'rvmt')) {
    Start-Sleep -Seconds 1
    Write-Host "Starting: $hostExe $protocolArgument"
    $hostProcess = Start-Process -FilePath $hostExe -ArgumentList $protocolArgument -PassThru
    Write-Output $hostProcess
}
