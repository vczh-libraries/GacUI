param(
    [Parameter(Mandatory = $true)]
    [ValidateSet('http', 'pipe', 'minihttp')]
    [string]$Protocol,

    [ValidateRange(1, 65535)]
    [int]$AsPort = 8889,

    [ValidateNotNullOrEmpty()]
    [string]$CoreHost = 'localhost'
)

$ErrorActionPreference = 'Stop'

if ($CoreHost -match '[\s/\\"?#@]' -or [Uri]::CheckHostName($CoreHost.Trim('[', ']')) -eq [UriHostNameType]::Unknown) {
    throw 'CoreHost must be a hostname or IP address, without a URL, path or port.'
}
if ($Protocol -eq 'pipe' -and $CoreHost.Contains(':')) {
    throw 'Named pipes require a UNC server name or IPv4 address, not a raw IPv6 address.'
}

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
$rendererArguments += "/AsPort:$AsPort"
$rendererArguments += "/corehost:$CoreHost"

Write-Host "Starting: $renderer $($rendererArguments -join ' ')"
$rendererProcess = Start-Process -FilePath $renderer -ArgumentList $rendererArguments -WindowStyle Hidden -PassThru
Write-Output $rendererProcess
