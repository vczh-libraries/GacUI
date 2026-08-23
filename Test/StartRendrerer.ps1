param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('http', 'pipe', 'minihttp')]
    [string]$Protocol,

    [ValidateRange(1, 65535)]
    [int]$Port
)

$ErrorActionPreference = 'Stop'

$renderer = Join-Path $PSScriptRoot 'GacUISrc\x64\Debug\RemotingTest_Rendering_Win32.exe'
if (-not (Test-Path -LiteralPath $renderer -PathType Leaf)) {
    throw "RemotingTest_Rendering_Win32 is not built in Debug x64: $renderer"
}

$rendererArguments = @(
    switch ($Protocol) {
        'http' { '/Http' }
        'pipe' { '/Pipe' }
        'minihttp' { '/MiniHttp' }
    }
)
if ($PSBoundParameters.ContainsKey('Port')) {
    $rendererArguments += "/port:$Port"
}

Write-Host "Starting: $renderer $($rendererArguments -join ' ')"
$rendererProcess = Start-Process -FilePath $renderer -ArgumentList $rendererArguments -PassThru
Write-Output $rendererProcess
