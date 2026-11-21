param(
    [ValidateRange(1,65535)][int]$AsPort = 8890,
    [ValidateSet('All','Layout','ShowcaseSkins','LocalizedLabels','Responsive','ResponsiveWindow','RawClock','RawClockHttp','ThemeProperty','RawClockPassive','Touching','Persistence','DocumentExchange','DocumentStyles','GalleryLabels','DocumentContextMenu')][string]$Scenario = 'All',
    [switch]$Gdi,
    [string]$ExchangeDocument = '',
    [ValidateRange(0,5)][int]$ClockSwitch = 0,
    [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'UIA_CppTest_Shared.ps1') -Application CppTest_Metaonly -AsPort $AsPort -Scenario $Scenario -Gdi:$Gdi -SkipBuild:$SkipBuild -ClockSwitch $ClockSwitch -ExchangeDocument $ExchangeDocument
exit $LASTEXITCODE
