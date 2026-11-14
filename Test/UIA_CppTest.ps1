param(
    [ValidateRange(1,65535)][int]$AsPort = 8888,
    [ValidateSet('All','Layout','ShowcaseSkins','Responsive','ResponsiveWindow','RawClock','Touching','Persistence','DocumentExchange','DocumentStyles','DocumentContextMenu','DocumentEditorSop','DocumentEditorCommands','PointRegression','TouchingList','FileDialogMutations')][string]$Scenario = 'All',
    [switch]$Gdi,
    [ValidateRange(0,15)][int]$EditorSopStartStep = 0,
    [switch]$EditorSopResumeAtImages,
    [string]$ExchangeDocument = '',
    [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'UIA_CppTest_Shared.ps1') -Application CppTest -AsPort $AsPort -Scenario $Scenario -EditorSopStartStep $EditorSopStartStep -EditorSopResumeAtImages:$EditorSopResumeAtImages -Gdi:$Gdi -SkipBuild:$SkipBuild -ExchangeDocument $ExchangeDocument
exit $LASTEXITCODE
