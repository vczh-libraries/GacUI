param(
    [ValidateRange(1,65535)][int]$AsPort = 8890,
    [ValidateSet('All','Layout','ShowcaseSkins','Responsive','ResponsiveWindow','RawClock','Touching','Persistence','DocumentExchange','DocumentStyles','DocumentContextMenu')][string]$Scenario = 'All',
    [switch]$Gdi,
    [string]$ExchangeDocument = '',
    [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'UIA_CppTest_Shared.ps1') -Application CppTest_Metaonly -AsPort $AsPort -Scenario $Scenario -Gdi:$Gdi -SkipBuild:$SkipBuild -ExchangeDocument $ExchangeDocument
exit $LASTEXITCODE
