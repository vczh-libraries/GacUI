param(
    [ValidateRange(1,65535)][int]$AsPort = 8888,
    [ValidateSet('All','Layout','Responsive','ResponsiveWindow','RawClock','Touching','Persistence','DocumentStyles','DocumentContextMenu','DocumentEditorSop','DocumentEditorCommands','PointRegression','TouchingList')][string]$Scenario = 'All',
    [switch]$Gdi,
    [ValidateRange(0,15)][int]$EditorSopStartStep = 0,
    [switch]$EditorSopResumeAtImages,
    [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'UIA_CppTest_Shared.ps1') -Application CppTest -AsPort $AsPort -Scenario $Scenario -EditorSopStartStep $EditorSopStartStep -EditorSopResumeAtImages:$EditorSopResumeAtImages -Gdi:$Gdi -SkipBuild:$SkipBuild
exit $LASTEXITCODE
