param([string]$SdkVersion = '10.0.26100.0')

$ErrorActionPreference = 'Stop'
$uiaDirectory = Split-Path $PSScriptRoot -Parent
$sdkHeader = Join-Path ${env:ProgramFiles(x86)} "Windows Kits\10\Include\$SdkVersion\um\UIAutomationClient.h"
$sdk = Get-Content -Raw -LiteralPath $sdkHeader
$catalog = Get-Content -Raw -LiteralPath (Join-Path $uiaDirectory 'UiaList\ViewModel\UiaCatalog.Windows.cpp')
$checks = @(
    @{ Name = 'Property'; Pattern = 'UIA_\w+PropertyId'; Count = 175 },
    @{ Name = 'Pattern'; Pattern = 'UIA_\w+Pattern\d*Id'; Count = 35 },
    @{ Name = 'Attribute'; Pattern = 'UIA_\w+AttributeId'; Count = 44 },
    @{ Name = 'ControlType'; Pattern = 'UIA_\w+ControlTypeId'; Count = 41 },
    @{ Name = 'Metadata'; Pattern = 'UIA_\w+MetadataId'; Count = 1 }
)
foreach ($check in $checks) {
    $section = [regex]::Match($catalog, '(?s)const \w+Descriptor ' + $check.Name + 'Catalog\[\].*?const vint ' + $check.Name + 'CatalogCount').Value
    $declared = @([regex]::Matches($section, '\{\s*(' + $check.Pattern + ')\s*,') | ForEach-Object { $_.Groups[1].Value } | Sort-Object)
    $expected = @([regex]::Matches($sdk, '\b(' + $check.Pattern + ')\s*=') | ForEach-Object { $_.Groups[1].Value } | Sort-Object -Unique)
    $difference = @(Compare-Object $expected $declared)
    if ($difference.Count -or $declared.Count -ne $check.Count -or @($declared | Sort-Object -Unique).Count -ne $declared.Count) {
        $difference | Format-Table | Out-String | Write-Output
        throw "$($check.Name) catalog differs from Windows SDK $SdkVersion."
    }
    Write-Output "$($check.Name): $($declared.Count) unique SDK identifiers match."
}

[xml]$strings = Get-Content -Raw -LiteralPath (Join-Path $uiaDirectory 'UiaList\UI\Strings.xml')
$baseline = @($strings.LocalizedStrings.Strings[0].String | ForEach-Object { $_.Name } | Sort-Object)
foreach ($locale in $strings.LocalizedStrings.Strings) {
    $keys = @($locale.String | ForEach-Object { $_.Name } | Sort-Object)
    if (@(Compare-Object $baseline $keys).Count -or @($keys | Sort-Object -Unique).Count -ne $keys.Count) { throw "Translation keys differ: $($locale.Locales)" }
    Write-Output "$($locale.Locales): $($keys.Count) translation keys match."
}

[xml]$project = Get-Content -Raw -LiteralPath (Join-Path $uiaDirectory 'UiaList\UiaList.vcxproj')
[xml]$filters = Get-Content -Raw -LiteralPath (Join-Path $uiaDirectory 'UiaList\UiaList.vcxproj.filters')
$projectFiles = @($project.Project.ItemGroup.ClCompile.Include) + @($project.Project.ItemGroup.ClInclude.Include)
$filterFiles = @($filters.Project.ItemGroup.ClCompile.Include) + @($filters.Project.ItemGroup.ClInclude.Include)
foreach ($folder in @('Source', 'ViewModel')) {
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $uiaDirectory "UiaList\$folder") -File) {
        if ($file.Extension -notin @('.cpp', '.h')) { continue }
        $relative = "$folder\$($file.Name)"
        if ($relative -notin $projectFiles -or $relative -notin $filterFiles) { throw "Missing explicit project/filter entry: $relative" }
    }
}
Write-Output 'All generated and handwritten C++ files have explicit project and filter entries.'
