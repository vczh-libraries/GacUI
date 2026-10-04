#requires -Version 7.0
param(
    [Parameter(Mandatory)][int]$FixtureProcessId,
    [ValidateSet('x64','Win32')][string]$Platform = 'x64',
    [ValidateSet('Debug','Release')][string]$Configuration = 'Debug',
    [string]$Solution = (Split-Path $PSScriptRoot -Parent)
)
. (Join-Path $PSScriptRoot 'CliCommon.ps1')
$client = New-UiaClient -Solution $Solution -Configuration $Configuration -Platform $Platform
$log = Join-Path $PSScriptRoot "UiaFixture-$FixtureProcessId.synthetic.txt"
function Get-Calls { return (Read-UiaFixtureLog $log).TrimEnd([char[]]"`r`n") -split '\r?\n' }
function Get-NodeId([string]$Name) { return (Get-UiaNode $script:tree $Name).id }
$windows = Get-UiaFixtureWindows $client $FixtureProcessId
$synthetic = $windows | Where-Object { $_.class -eq 'UiaSyntheticFixture' -and $_.title -notlike '*10,000*' } | Select-Object -First 1
$tree = $client.Send('Print-Window', $synthetic.id).result
$capability = 'Synthetic capabilities'
$document = 'Synthetic document'
$current = $client.Send('Query-Properties', (Get-NodeId $capability)).result
$special = $current.properties | Where-Object propertyId -EQ 30158 | Select-Object -First 1
Assert-UiaEqual $special.value.value 'A' 'Windows UIA truncates this property before client delivery'
$patterns = @(@($current.providers | Where-Object patternId -NE 0 | ForEach-Object { $_.patternId }) + @(10014,10024,10032) | Sort-Object -Unique)
Assert-UiaEqual $patterns @(10000..10034) 'All 35 patterns'
foreach ($malformed in @('Help-Command {broken}','Help-Command {"x":"\q"}','Help-Command {"x":01}','Help-Command {"x":1e}','Help-Command {"x":NaN}','Help-Command {} {}')) {
    Assert-UiaEqual ($client.Send($malformed, $null, $null, $false).error.code) 'InvalidJson'
}
$before = @(Get-Calls)
$null = $client.Send('Run-IValueProvider::SetValue', (Get-NodeId $capability), @{value="invalid`0suffix"}, $false)
$null = $client.Send('Run-IRangeValueProvider::SetValue', (Get-NodeId $capability), @{value='12'}, $false)
$null = $client.Send('Run-IScrollProvider::Scroll', (Get-NodeId $capability), @{horizontalAmount=$true;verticalAmount=2}, $false)
Assert-UiaEqual @(Get-Calls) $before 'Rejected input reached the provider'

function Invoke-UiaProvider {
    param([string]$Command, [object]$Arguments = $null, [string]$Method, [string]$TargetName = 'Synthetic capabilities', [bool]$Ok = $true)
    $before = @(Get-Calls)
    $old = Get-NodeId $TargetName
    $response = $client.Send("Run-$Command", $old, $Arguments, $Ok)
    if (!$Ok) { return $response }
    $result = $response.result
    if ($Method) {
        $added = @(Get-Calls | Select-Object -Skip $before.Count | Where-Object { $_.Contains("`t$Method`t") })
        Assert-Uia ($added.Count -eq 1) "$Command / $Method exact call count: $($added.Count)"
        $targetKey = if ($TargetName -eq $document) { '2' } else { '1' }
        Assert-Uia ($added[0].StartsWith("target=$targetKey`t")) "$Command target"
        $fields = @{}
        foreach ($field in ($added[0] -split "`t")) {
            if ($field.Contains('=')) { $pair=$field -split '=',2; $fields[$pair[0]]=$pair[1] }
        }
        $numeric = @{
            'SetValue(R8)'=@('value'); Scroll=@('horizontalAmount','verticalAmount')
            SetCurrentView=@('value'); SetVisualState=@('value'); SetDockPosition=@('value')
            Move=@('screenX','screenY'); Resize=@('width','height'); Rotate=@('degrees')
            Zoom=@('value'); ZoomByUnit=@('zoomUnit'); 'Legacy.Select'=@('flagsSelect')
            StartListening=@('inputType'); GetItem=@('row','column')
            WaitForInputIdle=@('milliseconds'); 'Custom.Navigate'=@('direction')
        }
        $names = $numeric[$Method]
        for ($index = 0; $index -lt $names.Count; $index++) {
            $field = @('a','b')[$index]
            $actual = [double]::Parse($fields[$field], [Globalization.CultureInfo]::InvariantCulture)
            Assert-Uia ($actual -eq $Arguments[$names[$index]]) "$Command / $field typed argument"
        }
        if ($Method -eq 'SetValue(BSTR)') {
            $text = if ($Arguments.ContainsKey('value')) { $Arguments.value } else { $Arguments.szValue }
            $expected = ($text.ToCharArray() | ForEach-Object { '{0:X4}' -f [int]$_ }) -join ' '
            Assert-UiaEqual (($added[0] -split "`t")[-1].Trim()) $expected 'UTF-16 argument units'
            Assert-Uia ([int]$fields.utf16 -eq $text.Length) 'UTF-16 argument length'
        }
        if ($Method -eq 'SetScrollPercent') {
            $horizontal = if ($Arguments.horizontalNoScroll) { -1 } else { $Arguments.horizontalPercent }
            $vertical = if ($Arguments.verticalNoScroll) { -1 } else { $Arguments.verticalPercent }
            Assert-Uia ([double]::Parse($fields.a,[Globalization.CultureInfo]::InvariantCulture) -eq $horizontal) 'Horizontal percentage'
            Assert-Uia ([double]::Parse($fields.b,[Globalization.CultureInfo]::InvariantCulture) -eq $vertical) 'Vertical percentage'
        }
    }
    if ($result.Contains('tree')) {
        $script:tree = $result.tree
        Assert-UiaEqual ($client.Send('Query-Node', $old, $null, $false).error.code) 'ExpiredId'
        $properties = @{}
        foreach ($property in $result.readback.properties) { $properties[[string]$property.propertyId] = $property.value.value }
        $states = @{
            Expand=@('30070','1'); Collapse=@('30070','0'); Select=@('30079',$true)
            AddToSelection=@('30079',$true); RemoveFromSelection=@('30079',$false)
        }
        if ($Arguments) {
            $states['SetValue(BSTR)'] = @('30045', $(if ($Arguments.ContainsKey('value')) { $Arguments.value } else { $Arguments.szValue }))
            $states['SetValue(R8)'] = @('30047',$Arguments.value)
            $states.SetCurrentView = @('30071',[string]$Arguments.value)
            $states.SetVisualState = @('30075',[string]$Arguments.value)
            $states.SetDockPosition = @('30069',[string]$Arguments.value)
            $states.Zoom = @('30145',$Arguments.value)
        }
        if ($Method -and $states.ContainsKey($Method)) {
            $state = $states[$Method]
            Assert-UiaEqual $properties[$state[0]] $state[1] "$Command actual state readback"
        }
    }
    return $result
}

# Explicit independent inputs and provider methods, not generated from the inspector catalog.
$mutations = @(
    @('IInvokeProvider::Invoke',$null,'Invoke'),
    @('IValueProvider::SetValue',@{value="Alpha 日本語`nBeta 中文"},'SetValue(BSTR)'),
    @('IRangeValueProvider::SetValue',@{value=38.5},'SetValue(R8)'),
    @('IScrollProvider::Scroll',@{horizontalAmount=4;verticalAmount=3},'Scroll'),
    @('IScrollProvider::SetScrollPercent',@{horizontalNoScroll=$false;horizontalPercent=17;verticalNoScroll=$false;verticalPercent=29},'SetScrollPercent'),
    @('IExpandCollapseProvider::Expand',$null,'Expand'),
    @('IExpandCollapseProvider::Collapse',$null,'Collapse'),
    @('IMultipleViewProvider::SetCurrentView',@{value=42},'SetCurrentView'),
    @('IWindowProvider::SetWindowVisualState',@{value=0},'SetVisualState'),
    @('ISelectionItemProvider::RemoveFromSelection',$null,'RemoveFromSelection'),
    @('ISelectionItemProvider::AddToSelection',$null,'AddToSelection'),
    @('ISelectionItemProvider::Select',$null,'Select'),
    @('IDockProvider::SetDockPosition',@{value=5},'SetDockPosition'),
    @('IToggleProvider::Toggle',$null,'Toggle'),
    @('IToggleProvider::Toggle',$null,'Toggle'),
    @('IToggleProvider::Toggle',$null,'Toggle'),
    @('ITransformProvider::Move',@{screenX=-15;screenY=33},'Move'),
    @('ITransformProvider::Resize',@{width=310;height=210},'Resize'),
    @('ITransformProvider::Rotate',@{degrees=22.5},'Rotate'),
    @('ITransformProvider2::Zoom',@{value=150},'Zoom'),
    @('ITransformProvider2::ZoomByUnit',@{zoomUnit=4},'ZoomByUnit'),
    @('IScrollItemProvider::ScrollIntoView',$null,'ScrollIntoView'),
    @('ILegacyIAccessibleProvider::DoDefaultAction',$null,'DoDefaultAction'),
    @('ILegacyIAccessibleProvider::Select',@{flagsSelect=3},'Legacy.Select'),
    @('ILegacyIAccessibleProvider::SetValue',@{szValue="Alpha 日本語`nBeta 中文"},'SetValue(BSTR)'),
    @('IVirtualizedItemProvider::Realize',$null,'Realize')
)
foreach ($case in $mutations) { $null = Invoke-UiaProvider $case[0] $case[1] $case[2] }
foreach ($value in 0..4) {
    $null = Invoke-UiaProvider 'IScrollProvider::Scroll' @{horizontalAmount=$value;verticalAmount=$value} 'Scroll'
    $null = Invoke-UiaProvider 'ITransformProvider2::ZoomByUnit' @{zoomUnit=$value} 'ZoomByUnit'
    $null = Invoke-UiaProvider 'ICustomNavigationProvider::Navigate' @{direction=$value} 'Custom.Navigate'
}
foreach ($value in 0..5) { $null = Invoke-UiaProvider 'IDockProvider::SetDockPosition' @{value=$value} 'SetDockPosition' }
foreach ($value in 0..2) { $null = Invoke-UiaProvider 'IWindowProvider::SetWindowVisualState' @{value=$value} 'SetVisualState' }
foreach ($value in @(1,2,4,8,16,3,5,9,17,12,20,13,21)) { $null = Invoke-UiaProvider 'ILegacyIAccessibleProvider::Select' @{flagsSelect=$value} 'Legacy.Select' }
foreach ($value in @(1,2,4,8,16,32,3,12,48)) {
    $null = Invoke-UiaProvider 'ISynchronizedInputProvider::StartListening' @{inputType=$value} 'StartListening'
    $null = Invoke-UiaProvider 'ISynchronizedInputProvider::Cancel' $null 'Cancel'
}
foreach ($horizontal in @($false,$true)) {
    foreach ($vertical in @($false,$true)) {
        $null = Invoke-UiaProvider 'IScrollProvider::SetScrollPercent' @{horizontalNoScroll=$horizontal;horizontalPercent=25;verticalNoScroll=$vertical;verticalPercent=75} 'SetScrollPercent'
    }
}
$before = @(Get-Calls)
foreach ($case in @(
    @('IRangeValueProvider::SetValue',@{value=101}),
    @('ITransformProvider2::Zoom',@{value=401}),
    @('ITransformProvider::Resize',@{width=0;height=50}),
    @('IDockProvider::SetDockPosition',@{value=6}),
    @('IWindowProvider::WaitForInputIdle',@{milliseconds=5001})
)) { $null = Invoke-UiaProvider $case[0] $case[1] -Ok $false }
Assert-UiaEqual @(Get-Calls) $before 'Out-of-bounds arguments reached the provider'
$null = Invoke-UiaProvider 'IUIAutomationElement::SetFocus' -Method 'SetFocus'
$null = Invoke-UiaProvider 'IUIAutomationElement::GetClickablePoint'
$null = Invoke-UiaProvider 'IUIAutomationElement7::GetCurrentMetadataValue' @{propertyId=30005;metadataId=100000}
$null = Invoke-UiaProvider 'IStylesProvider::GetCurrentExtendedPropertiesAsArray'
$null = Invoke-UiaProvider 'ISynchronizedInputProvider::StartListening' @{inputType=3} 'StartListening'
$null = Invoke-UiaProvider 'ISynchronizedInputProvider::Cancel' $null 'Cancel'
Assert-UiaEqual (Invoke-UiaProvider 'IGridProvider::GetItem' @{row=0;column=0} 'GetItem').value.kind 'Element'
Assert-UiaEqual (Invoke-UiaProvider 'IMultipleViewProvider::GetViewName' @{viewId=42}).value.value 'View forty-two'
Assert-UiaEqual (Invoke-UiaProvider 'IWindowProvider::WaitForInputIdle' @{milliseconds=10} 'WaitForInputIdle').value.value $true
foreach ($command in @('ISelectionProvider::GetCurrentSelection','ISelectionProvider2::GetCurrentSelection','ITableProvider::GetCurrentRowHeaders','ITableProvider::GetCurrentColumnHeaders','ITableItemProvider::GetCurrentRowHeaderItems','ITableItemProvider::GetCurrentColumnHeaderItems','ILegacyIAccessibleProvider::GetCurrentSelection','ISpreadsheetItemProvider::GetCurrentAnnotationObjects','ISpreadsheetItemProvider::GetCurrentAnnotationTypes','IDragProvider::GetCurrentGrabbedItems')) {
    Assert-UiaEqual (Invoke-UiaProvider $command).value.kind 'Array' $command
}
foreach ($command in @('ILegacyIAccessibleProvider::GetIAccessible','IObjectModelProvider::GetUnderlyingObjectModel')) {
    Assert-UiaEqual (Invoke-UiaProvider $command).value.kind 'Opaque' $command
}
Assert-UiaEqual (Invoke-UiaProvider 'ISpreadsheetProvider::GetItemByName' @{name='A1'} 'GetItemByName').value.kind 'Element'
Assert-UiaEqual (Invoke-UiaProvider 'ISpreadsheetProvider::GetItemByName' @{name='missing'} 'GetItemByName').value.kind 'Null'
$null = Invoke-UiaProvider 'ICustomNavigationProvider::Navigate' @{direction=0} 'Custom.Navigate'
$external = Invoke-UiaProvider 'IItemContainerProvider::FindItemByProperty' @{startAfter=$null;propertyId=30005;value='External document'} 'FindItemByProperty'
$externalId = $external.value.value
$reference = $external.references | Where-Object id -CEQ $externalId | Select-Object -First 1
Assert-UiaEqual $reference.treeNodeId $null
Assert-UiaEqual ($client.Send('Select-Node', $externalId, $null, $false).error.code) 'OutsideTree'
$null = $client.Send('Query-Node', $externalId)
$externalRange = $client.Send('Run-ITextProvider::DocumentRange', $externalId).result.value.value
$externalText = $client.Send('Run-IUIAutomationTextRange::GetText', $externalRange, @{maxLength=-1}).result.value
# CDB confirmed UIA delivers a one-character BSTR; the direct value tests preserve all seven units.
Assert-UiaEqual $externalText.value 'A'
foreach ($case in @(@(40006,'NaN'),@(40010,'Infinity'),@(40011,'-Infinity'))) {
    $actual = $client.Send('Run-IUIAutomationTextRange::GetAttributeValue', $externalRange, @{attributeId=$case[0]}).result.value
    Assert-UiaEqual $actual.value @{nonfinite=$case[1]}
}
$textRange = (Invoke-UiaProvider 'ITextProvider::DocumentRange' -Method 'Text.DocumentRange' -TargetName $document).value.value
$before = @(Get-Calls)
Assert-UiaEqual ($client.Send('Run-IUIAutomationTextRange::Compare', $textRange, @{range=$externalRange}, $false).error.code) 'WrongDocument'
Assert-UiaEqual ($client.Send('Run-IUIAutomationTextRange::Compare', $textRange, @{range=(Get-NodeId $capability)}, $false).error.code) 'WrongKind'
Assert-UiaEqual @(Get-Calls) $before 'Invalid range operands reached the provider'
foreach ($case in @(
    @('ITextProvider::GetSelection',$null,'Text.GetSelection'),
    @('ITextProvider::GetVisibleRanges',$null,'Text.GetVisibleRanges'),
    @('ITextProvider::RangeFromPoint',@{screenX=900;screenY=160},'Text.RangeFromPoint'),
    @('ITextProvider::RangeFromChild',@{child=(Get-NodeId $capability)},'Text.RangeFromChild'),
    @('ITextProvider2::RangeFromAnnotation',@{annotation=(Get-NodeId $capability)},'Text.RangeFromAnnotation'),
    @('ITextProvider2::GetCaretRange',$null,'Text.GetCaretRange'),
    @('ITextEditProvider::GetActiveComposition',$null,'Text.GetActiveComposition'),
    @('ITextEditProvider::GetConversionTarget',$null,'Text.GetConversionTarget')
)) { $null = Invoke-UiaProvider $case[0] $case[1] $case[2] $document }
$null = Invoke-UiaProvider 'ITextChildProvider::TextRange' -Method 'TextChild.TextRange'
$section = $client.Send('Query-Range', $textRange).result
$values = @($section.readouts | ForEach-Object { $_.value })
Assert-Uia (@($values | Where-Object kind -EQ 'Mixed').Count -gt 0) 'Mixed attribute'
Assert-Uia (@($values | Where-Object kind -EQ 'Unsupported').Count -gt 0) 'Unsupported attribute'
$arrays = @($values | Where-Object { $_.kind -eq 'Array' -and $_.dimensions.Count -eq 1 -and $_.dimensions[0].lower -eq 0 -and $_.dimensions[0].upper -eq 1 -and $_.value.Count -eq 2 -and $_.value[0].value -eq 10 -and $_.value[1].value -eq 20 })
Assert-Uia ($arrays.Count -gt 0) 'Array text attribute with UIA-normalized bounds'
$clone = $client.Send('Run-IUIAutomationTextRange::Clone', $textRange).result.value.value
foreach ($unit in 0..6) {
    $null = $client.Send('Run-IUIAutomationTextRange::ExpandToEnclosingUnit', $textRange, @{unit=$unit})
    $null = $client.Send('Run-IUIAutomationTextRange::Move', $textRange, @{unit=$unit;count=0})
    foreach ($endpoint in @(0,1)) { $null = $client.Send('Run-IUIAutomationTextRange::MoveEndpointByUnit', $textRange, @{endpoint=$endpoint;unit=$unit;count=0}) }
}
$rangeCases = @(
    @('Compare',@{range=$clone}), @('CompareEndpoints',@{srcEndPoint=0;range=$clone;targetEndPoint=1}),
    @('FindText',@{text='Beta';backward=$false;ignoreCase=$true}),
    @('FindAttribute',@{attributeId=40005;value='Segoe UI';backward=$false}),
    @('GetAttributeValue',@{attributeId=40006}), @('GetText',@{maxLength=-1}),
    @('GetBoundingRectangles',$null), @('GetEnclosingElement',$null), @('GetChildren',$null),
    @('ExpandToEnclosingUnit',@{unit=0}), @('Move',@{unit=2;count=1}),
    @('MoveEndpointByUnit',@{endpoint=1;unit=0;count=-1}),
    @('MoveEndpointByRange',@{srcEndPoint=0;range=$clone;targetEndPoint=0})
)
foreach ($case in $rangeCases) {
    $method = $case[0]
    $before = @(Get-Calls)
    $null = $client.Send("Run-IUIAutomationTextRange::$method", $textRange, $case[1])
    if ($method -notin @('GetEnclosingElement','GetChildren')) {
        $added = @(Get-Calls | Select-Object -Skip $before.Count | Where-Object { $_.Contains("`tRange.$method`t") })
        Assert-Uia ($added.Count -eq 1) "$method exact range call count"
    }
}
$attributes = $client.Send('Run-IUIAutomationTextRange::ReadAllAttributes', $textRange).result.value
Assert-Uia ($attributes.kind -eq 'Array' -and $attributes.value.Count -eq 44) 'All text attributes'
foreach ($case in @(@('Select',$null),@('AddToSelection',$null),@('RemoveFromSelection',$null),@('ScrollIntoView',@{alignToTop=$true}))) {
    $textRange = $client.Send('Run-ITextProvider::DocumentRange', (Get-NodeId $document)).result.value.value
    $result = $client.Send("Run-IUIAutomationTextRange::$($case[0])", $textRange, $case[1]).result
    $tree = $result.tree
    Assert-UiaEqual ($client.Send('Query-Range', $textRange, $null, $false).error.code) 'ExpiredId'
}
$textRange = $client.Send('Run-ITextProvider::DocumentRange', (Get-NodeId $document)).result.value.value
$section = $client.Send('Query-Range', $textRange).result
foreach ($action in $section.commands) {
    if ($action.command.Contains('TextRange3::')) {
        $arguments = if ($action.parameters.Count) { @{attributeIds=@(40005,40006)} } else { $null }
        $null = $client.Send($action.command, $textRange, $arguments)
    }
}
Write-Host 'PASS pattern and range operation coverage'

$native = $windows | Where-Object class -EQ 'UiaFixture' | Select-Object -First 1
$tree = $client.Send('Print-Window', $native.id).result
foreach ($title in @('Open large/deep raw tree','Open navigation windows')) {
    $tree = $client.Send('Run-IInvokeProvider::Invoke', (Get-NodeId $title)).result.tree
}
$windows = Get-UiaFixtureWindows $client $FixtureProcessId
$navigation = @($windows | Where-Object class -EQ 'UiaNavigationFixture')
Assert-Uia (@($navigation | Where-Object title -EQ 'Duplicate navigation title').Count -eq 2) 'Duplicate navigation titles'
Assert-Uia (@($navigation.id | Sort-Object -Unique).Count -eq $navigation.Count) 'Distinct navigation identities'
Assert-Uia (@($navigation | Where-Object { $_.title -in @('Hidden navigation window','Cloaked navigation window') }).Count -eq 0) 'Excluded navigation windows'
$large = $windows | Where-Object title -Like '*10,000 siblings*' | Select-Object -First 1
$tree = $client.Send('Print-Window', $large.id).result
Assert-Uia ($tree.nodes.Count -eq 11053) 'Full large tree'
$wide = Get-NodeId '10,000 siblings'
Assert-Uia (@($tree.nodes | Where-Object parentId -CEQ $wide).Count -eq 10000) 'Ten thousand siblings'
$depths = @{}
foreach ($node in $tree.nodes) { $depths[$node.id] = if ($node.parentId) { $depths[$node.parentId] + 1 } else { 0 } }
Assert-Uia (($depths.Values | Measure-Object -Maximum).Maximum -ge 1000) 'Thousand-level ancestry'
$client.Close()
