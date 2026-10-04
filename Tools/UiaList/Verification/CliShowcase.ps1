#requires -Version 7.0
param([Parameter(Mandatory)][int]$ProcessId, [switch]$Hosted)
# Independent FullControlTest expectations; all operations use CLI JSON, not HTTP.
. (Join-Path $PSScriptRoot 'CliCommon.ps1')
$client = New-UiaClient
$windows = Get-UiaFixtureWindows $client $ProcessId
$window = $windows | Where-Object title -CEQ 'Complete Control Showcase' | Select-Object -First 1
Assert-Uia ($null -ne $window) 'Complete Control Showcase window'
$tree = $client.Send('Print-Window', $window.id).result
$checks = 0

function Confirm-Showcase($Condition, [string]$Detail) {
    Assert-Uia $Condition $Detail
    $script:checks++
}
function Confirm-ShowcaseEqual($Actual, $Expected, [string]$Detail) {
    Assert-UiaEqual $Actual $Expected $Detail
    $script:checks++
}
function Update-ShowcaseTree { $script:tree = $client.Send('Refresh-Window', $window.id).result }
function Get-CurrentNode($Node) {
    $current = $script:tree.nodes | Where-Object runtimeId -CEQ $Node.runtimeId | Select-Object -First 1
    Assert-Uia ($null -ne $current) "Retired showcase node: $($Node.name)"
    return $current
}
function Get-ShowcaseNodes {
    param($Role = $null, $Name = $null, $Parent = $null, [switch]$Direct)
    $lookup = @{}
    foreach ($node in $script:tree.nodes) { $lookup[$node.id] = $node }
    $parentId = if ($null -ne $Parent) { (Get-CurrentNode $Parent).id } else { $null }
    foreach ($node in $script:tree.nodes) {
        if ($null -ne $Role -and $node.controlType -ne $Role) { continue }
        if ($null -ne $Name -and $node.name -cne $Name) { continue }
        $ancestor = $node.parentId
        if ($parentId) {
            while ($ancestor -and $ancestor -cne $parentId -and !$Direct) { $ancestor = $lookup[$ancestor].parentId }
            if ($ancestor -cne $parentId) { continue }
        }
        $node
    }
}
function Get-ShowcaseNode($Role = $null, $Name = $null, $Parent = $null) {
    $found = @(Get-ShowcaseNodes $Role $Name $Parent)
    Assert-Uia ($found.Count -gt 0) "Missing showcase node: role=$Role, name=$Name"
    return $found[0]
}
function Get-ShowcaseProperty($Node, [int]$PropertyId) {
    $inspection = $client.Send('Query-Node', (Get-CurrentNode $Node).id).result
    $property = $inspection.properties | Where-Object propertyId -EQ $PropertyId | Select-Object -First 1
    Assert-Uia ($null -ne $property) "Missing property $PropertyId on $($Node.name)"
    return $property.value.value
}
function Invoke-ShowcaseOperation([string]$Operation, $Node, $Arguments = $null) {
    $result = $client.Send("Run-$Operation", (Get-CurrentNode $Node).id, $Arguments).result
    if ($result.Contains('tree')) { $script:tree = $result.tree }
    return $result
}
function Select-ShowcasePage([string[]]$Names) {
    $parent = $null
    foreach ($name in $Names) {
        $page = Get-ShowcaseNode 50019 $name $parent
        $result = Invoke-ShowcaseOperation 'ISelectionItemProvider::Select' $page
        Confirm-Showcase (@($result.readback.properties | Where-Object { $_.propertyId -eq 30079 -and $_.value.value -ceq $true }).Count -eq 1) "Selected tab $name"
        $parent = Get-CurrentNode $page
    }
    return $parent
}
function Invoke-ShowcaseButton([string]$Name, $Parent = $null) {
    $null = Invoke-ShowcaseOperation 'IInvokeProvider::Invoke' (Get-ShowcaseNode 50000 $Name $Parent)
}
function Set-ShowcaseValue($Node, [string]$Text) {
    $result = $client.Send('Set-Property', (Get-CurrentNode $Node).id, @{propertyId=30045;value=$Text}).result
    $script:tree = $result.tree
    Confirm-Showcase (@($result.readback.properties | Where-Object { $_.propertyId -eq 30045 -and $_.value.value -ceq $Text }).Count -eq 1) 'Actual text readback'
}
function Get-ShowcaseRange($Node) { return (Invoke-ShowcaseOperation 'ITextProvider::DocumentRange' $Node).value.value }
function Get-ShowcaseText([string]$TextRange) { return $client.Send('Run-IUIAutomationTextRange::GetText', $TextRange, @{maxLength=-1}).result.value.value }

function Test-TextLists {
    $page = Select-ShowcasePage @('List','TextList')
    Invoke-ShowcaseButton 'Clear' $page
    Invoke-ShowcaseButton 'Add 10 items' $page
    $lists = @(Get-ShowcaseNodes -Role 50008 -Parent $page)
    Confirm-Showcase ($lists.Count -eq 2) 'Two semantic text lists'
    foreach ($container in $lists) {
        Confirm-ShowcaseEqual @((Get-ShowcaseNodes -Role 50007 -Parent $container -Direct).name) @(0..9 | ForEach-Object { [string]$_ }) 'Ten named list items'
    }
    $item = @(Get-ShowcaseNodes -Role 50007 -Parent $lists[0])[1]
    $null = Invoke-ShowcaseOperation 'ISelectionItemProvider::Select' $item
    Confirm-ShowcaseEqual (Get-ShowcaseProperty $item 30079) $true 'List selection'
    $combo = Get-ShowcaseNode -Role 50003 -Parent $page
    foreach ($mode in @('Check','Radio')) {
        $null = Invoke-ShowcaseOperation 'IExpandCollapseProvider::Expand' $combo
        $choice = Get-ShowcaseNode 50007 $mode
        $null = Invoke-ShowcaseOperation 'ISelectionItemProvider::Select' $choice
        if ((Get-ShowcaseProperty $combo 30070) -eq '1') { $null = Invoke-ShowcaseOperation 'IExpandCollapseProvider::Collapse' $combo }
        $old = Get-ShowcaseProperty $item 30086
        $null = Invoke-ShowcaseOperation 'IToggleProvider::Toggle' $item
        Confirm-Showcase ((Get-ShowcaseProperty $item 30086) -cne $old) "$mode toggle"
    }
    Invoke-ShowcaseButton 'Remove odd items' $page
    Confirm-ShowcaseEqual @((Get-ShowcaseNodes -Role 50007 -Parent $lists[0] -Direct).name) @('1','3','5','7','9') 'List deletion'
    $checkbox = Get-ShowcaseNode -Role 50002 -Parent $page
    $old = Get-ShowcaseProperty $checkbox 30086
    $null = Invoke-ShowcaseOperation 'IToggleProvider::Toggle' $checkbox
    Confirm-Showcase ((Get-ShowcaseProperty $checkbox 30086) -cne $old) 'Checkbox state'
    $radio = Get-ShowcaseNode -Role 50013 -Parent $page
    $null = Invoke-ShowcaseOperation 'ISelectionItemProvider::Select' $radio
    Confirm-ShowcaseEqual (Get-ShowcaseProperty $radio 30079) $true 'Radio selection'
    Write-Host 'PASS showcase text lists and buttons'
}

function Test-Grids {
    $page = Select-ShowcasePage @('List','ListView')
    $grids = @(Get-ShowcaseNodes -Role 50028 -Parent $page)
    Confirm-Showcase ($grids.Count -eq 2) 'Two detail list views'
    foreach ($grid in $grids) {
        Confirm-ShowcaseEqual (Get-ShowcaseProperty $grid 30063) '4' 'Four list-view columns'
        $headers = (Invoke-ShowcaseOperation 'ITableProvider::GetCurrentColumnHeaders' $grid).value.value
        $names = @(
            foreach ($reference in $headers) {
                $properties = $client.Send('Query-Node', $reference.value).result.properties
                ($properties | Where-Object propertyId -EQ 30005 | Select-Object -First 1).value.value
            }
        )
        Confirm-ShowcaseEqual $names @('Id','Category','Size','File') 'Header names'
        $cell = (Invoke-ShowcaseOperation 'IGridProvider::GetItem' $grid @{row=0;column=2}).value.value
        $properties = $client.Send('Query-Node', $cell).result.properties
        Confirm-Showcase (@($properties | Where-Object { $_.propertyId -eq 30065 -and $_.value.value -ceq '2' }).Count -eq 1) 'Returned cell column'
        $rows = @(Get-ShowcaseNodes -Role 50029 -Parent $grid -Direct)
        $null = Invoke-ShowcaseOperation 'ISelectionItemProvider::Select' $rows[0]
        $null = Invoke-ShowcaseOperation 'IScrollItemProvider::ScrollIntoView' $rows[-1]
        foreach ($view in 0..5) {
            $null = Invoke-ShowcaseOperation 'IMultipleViewProvider::SetCurrentView' $grid @{value=$view}
            Confirm-ShowcaseEqual (Get-ShowcaseProperty $grid 30071) ([string]$view) "View $view"
        }
    }
    $page = Select-ShowcasePage @('List','TreeView')
    foreach ($container in @(Get-ShowcaseNodes -Role 50023 -Parent $page)) {
        $blue = Get-ShowcaseNode 50024 'Blue+' $container
        $null = Invoke-ShowcaseOperation 'IExpandCollapseProvider::Expand' $blue
        $children = @(Get-ShowcaseNodes -Role 50024 -Parent $blue -Direct)
        Confirm-Showcase ($children.Count -gt 0) 'Expanded tree children'
        $null = Invoke-ShowcaseOperation 'ISelectionItemProvider::Select' $children[0]
        Confirm-ShowcaseEqual (Get-ShowcaseProperty $children[0] 30079) $true 'Nested tree selection'
        $null = Invoke-ShowcaseOperation 'IExpandCollapseProvider::Collapse' $blue
        Confirm-Showcase (@(Get-ShowcaseNodes -Role 50024 -Parent $blue -Direct).Count -eq 0) 'Collapsed tree excludes descendants'
    }
    $page = Select-ShowcasePage @('List','BindableDataGrid')
    $grid = Get-ShowcaseNode -Role 50028 -Parent $page
    Confirm-Showcase ((Get-ShowcaseProperty $grid 30062) -eq '5' -and (Get-ShowcaseProperty $grid 30063) -eq '5') 'Five by five grid'
    $cell = (Invoke-ShowcaseOperation 'IGridProvider::GetItem' $grid @{row=0;column=0}).value.value
    $script:tree = $client.Send('Run-IInvokeProvider::Invoke', $cell).result.tree
    $editor = Get-ShowcaseNode -Role 50004 -Parent $page
    Set-ShowcaseValue $editor 'CLI edited name'
    Confirm-Showcase (@(Get-ShowcaseNodes -Name 'CLI edited name' -Parent $grid).Count -gt 0) 'Grid edit saved immediately'
    Write-Host 'PASS showcase lists, six views, tree and editable grid'
}

function Test-Texts {
    foreach ($name in @('TextBox','TextBox (No Tab)','Document','Document (No Tab)')) {
        $page = Select-ShowcasePage @('Control','TextBox',$name)
        $inputs = @(Get-ShowcaseNodes -Parent $page | Where-Object { $_.providers.Contains('IValueProvider') -and $_.providers.Contains('ITextProvider') })
        foreach ($edit in $inputs) {
            if (Get-ShowcaseProperty $edit 30046) { continue }
            Set-ShowcaseValue $edit 'CLI text 日本語 中文'
            $textRange = Get-ShowcaseRange $edit
            Confirm-ShowcaseEqual (Get-ShowcaseText $textRange) 'CLI text 日本語 中文' 'Text range matches Value'
            $found = $client.Send('Run-IUIAutomationTextRange::FindText', $textRange, @{text='text';backward=$false;ignoreCase=$false}).result.value.value
            Confirm-ShowcaseEqual (Get-ShowcaseText $found) 'text' 'FindText exact result'
            $clone = $client.Send('Run-IUIAutomationTextRange::Clone', $found).result.value.value
            Confirm-Showcase ($client.Send('Run-IUIAutomationTextRange::Compare', $found, @{range=$clone}).result.value.value) 'Clone identity'
            $script:tree = $client.Send('Run-IUIAutomationTextRange::Select', $found).result.tree
            $selected = (Invoke-ShowcaseOperation 'ITextProvider::GetSelection' $edit).value.value
            Confirm-Showcase ($selected.Count -eq 1 -and (Get-ShowcaseText $selected[0].value) -ceq 'text') 'Selected text readback'
            Set-ShowcaseValue $edit ''
            Confirm-ShowcaseEqual (Get-ShowcaseText (Get-ShowcaseRange $edit)) '' 'Empty document'
        }
        foreach ($document in @(Get-ShowcaseNodes -Role 50030 -Parent $page)) {
            if (!$document.providers.Contains('ITextProvider')) { continue }
            $section = $client.Send('Query-Range', (Get-ShowcaseRange $document)).result
            Confirm-Showcase ($section.readouts.Count -gt 0) 'Document attributes and geometry'
        }
    }
    $page = Select-ShowcasePage @('Control','Embedded Controls')
    $document = Get-ShowcaseNode -Role 50030 -Parent $page
    $references = $client.Send('Run-IUIAutomationTextRange::GetChildren', (Get-ShowcaseRange $document)).result.value.value
    Confirm-Showcase ($references.Count -eq 17) 'Seventeen embedded children'
    Write-Host 'PASS showcase editable text, document ranges and embedded controls'
}

function Test-CalendarLayout {
    $page = Select-ShowcasePage @('Misc','DatePicker')
    $calendars = @(Get-ShowcaseNodes -Role 50001 -Parent $page)
    Confirm-Showcase ($calendars.Count -eq 2) 'Two calendars'
    foreach ($calendar in $calendars) {
        Confirm-Showcase ((Get-ShowcaseProperty $calendar 30062) -eq '6' -and (Get-ShowcaseProperty $calendar 30063) -eq '7') 'Calendar grid dimensions'
        $day = (Invoke-ShowcaseOperation 'IGridProvider::GetItem' $calendar @{row=2;column=2}).value.value
        $result = $client.Send('Run-ISelectionItemProvider::Select', $day).result
        $script:tree = $result.tree
        Confirm-Showcase (@($result.readback.properties | Where-Object { $_.propertyId -eq 30079 -and $_.value.value -ceq $true }).Count -eq 1) 'Calendar day selected'
    }
    foreach ($name in @('Easy Layout','Eazy Layout (Table)')) {
        $page = Select-ShowcasePage @('Layout',$name)
        $labels = if ($name -eq 'Easy Layout') { @('Left one','Left two','Left three','Right check') } else { @('Row A','Row B','Shared 120','Column 1x','Column 2x') }
        foreach ($label in $labels) {
            $node = Get-ShowcaseNode -Name $label -Parent $page
            $bounds = $node.bounds
            Confirm-Showcase (!$node.offscreen -and $bounds.right -gt $bounds.left -and $bounds.bottom -gt $bounds.top) "Layout bounds $label"
        }
        $edit = Get-ShowcaseNode -Role 50004 -Parent $page
        Set-ShowcaseValue $edit 'CLI layout binding'
        Confirm-Showcase (@(Get-ShowcaseNodes 50020 'CLI layout binding' $page).Count -gt 0) 'Layout binding readback'
        $button = if ($name -eq 'Easy Layout') { 'Rebuild' } else { 'Rebuild tables' }
        Invoke-ShowcaseButton $button $page
        $null = Select-ShowcasePage @('Layout',$name)
        Confirm-Showcase (@(Get-ShowcaseNodes -Role 50004).Count -gt 0) 'Layout rebuilt providers reacquired'
    }
    Write-Host 'PASS showcase calendars and both Easy Layout pages'
}

function Test-TabInventory($Parent = $null, [int]$Depth = 0) {
    Assert-Uia ($Depth -lt 8) 'Tab inventory recursion'
    $tabs = @(Get-ShowcaseNodes -Role 50018 -Parent $Parent)
    # Only the outermost tab under this page; nested tabs are visited recursively.
    if (!$tabs.Count) { return }
    $pages = @(Get-ShowcaseNodes -Role 50019 -Parent $tabs[0] -Direct)
    foreach ($old in $pages) {
        if ($old.name -eq 'Exit') { continue }
        $null = Invoke-ShowcaseOperation 'ISelectionItemProvider::Select' (Get-CurrentNode $old)
        $current = Get-CurrentNode $old
        foreach ($node in @(Get-ShowcaseNodes -Parent $current)) {
            Confirm-Showcase ($node.controlType -ge 50000 -and $node.client -and $node.runtimeId) "Role and identity $($node.name)"
        }
        Test-TabInventory $current ($Depth + 1)
    }
}

function Test-WindowOperations {
    $root = $script:tree.nodes[0]
    $bounds = $root.bounds
    $null = Invoke-ShowcaseOperation 'ITransformProvider::Move' $root @{screenX=$bounds.left+7;screenY=$bounds.top+9}
    $moved = (Get-CurrentNode $root).bounds
    Confirm-Showcase ($moved.left -eq $bounds.left+7 -and $moved.top -eq $bounds.top+9) 'Physical window move'
    $null = Invoke-ShowcaseOperation 'ITransformProvider::Move' $root @{screenX=$bounds.left;screenY=$bounds.top}
    $null = Invoke-ShowcaseOperation 'ITransformProvider::Resize' $root @{width=$bounds.right-$bounds.left+13;height=$bounds.bottom-$bounds.top+17}
    $resized = (Get-CurrentNode $root).bounds
    Confirm-Showcase ($resized.right-$resized.left -eq $bounds.right-$bounds.left+13) 'Window resize'
    $null = Invoke-ShowcaseOperation 'ITransformProvider::Resize' $root @{width=$bounds.right-$bounds.left;height=$bounds.bottom-$bounds.top}
    $page = Select-ShowcasePage @('Control','Document Editor (Toolstrip)')
    Confirm-Showcase (@(Get-ShowcaseNodes -Role 50021 -Parent $page).Count -gt 0) 'Toolbar role'
    $file = Get-ShowcaseNode 50011 'File' $page
    $null = Invoke-ShowcaseOperation 'IExpandCollapseProvider::Expand' $file
    Confirm-ShowcaseEqual (Get-ShowcaseProperty $file 30070) '1' 'File menu expanded'
    $null = Invoke-ShowcaseOperation 'IExpandCollapseProvider::Collapse' $file
    Confirm-ShowcaseEqual (Get-ShowcaseProperty $file 30070) '0' 'File menu collapsed'
    Update-ShowcaseTree
    $page = Get-ShowcaseNode 50019 'Document Editor (Toolstrip)'
    $document = Get-ShowcaseNode -Role 50030 -Parent $page
    Set-ShowcaseValue $document 'CLI toolbar command test'
    $script:tree = $client.Send('Run-IUIAutomationTextRange::Select', (Get-ShowcaseRange $document)).result.tree
    $beforeWeight = $client.Send('Run-IUIAutomationTextRange::GetAttributeValue', (Get-ShowcaseRange $document), @{attributeId=40007}).result.value.value
    Invoke-ShowcaseButton 'Bold' $page
    $afterWeight = $client.Send('Run-IUIAutomationTextRange::GetAttributeValue', (Get-ShowcaseRange $document), @{attributeId=40007}).result.value.value
    Confirm-Showcase ($beforeWeight -cne $afterWeight) 'Toolbar changes document formatting'
    $page = Select-ShowcasePage @('Misc','Dialogs','MessageDialog')
    Invoke-ShowcaseButton 'Show Dialog' $page
    if ($Hosted) {
        $modal = Get-ShowcaseNode 50032 'The Title'
    } else {
        $modalWindow = (Get-UiaFixtureWindows $client $ProcessId) | Where-Object title -CEQ 'The Title' | Select-Object -First 1
        $script:tree = $client.Send('Print-Window', $modalWindow.id).result
        $modal = $script:tree.nodes[0]
    }
    Confirm-ShowcaseEqual (Get-ShowcaseProperty $modal 30077) $true 'Modal window state'
    Confirm-Showcase (@(Get-ShowcaseNodes -Role 50000 -Parent $modal).Count -gt 0) 'Modal dismissal buttons'
    $null = Invoke-ShowcaseOperation 'IWindowProvider::Close' $modal
    $script:tree = $client.Send('Print-Window', $window.id).result
    Confirm-Showcase (@(Get-ShowcaseNodes 50032 'The Title').Count -eq 0) 'Modal closed'
    $page = Select-ShowcasePage @('Window Manager')
    Invoke-ShowcaseButton 'Open New Window' $page
    if ($Hosted) {
        $children = @(Get-ShowcaseNodes -Role 50032 | Where-Object { $_.parentId })
        Confirm-Showcase ($children.Count -gt 0) 'Hosted logical window in same HWND tree'
        $null = Invoke-ShowcaseOperation 'IWindowProvider::Close' $children[0]
    } else {
        $children = @((Get-UiaFixtureWindows $client $ProcessId) | Where-Object id -CNE $window.id)
        Confirm-Showcase ($children.Count -gt 0) 'Ordinary subwindow with own HWND'
        $other = $children[-1]
        $script:tree = $client.Send('Print-Window', $other.id).result
        $null = Invoke-ShowcaseOperation 'IWindowProvider::Close' $script:tree.nodes[0]
        $script:tree = $client.Send('Print-Window', $window.id).result
    }
    $page = Select-ShowcasePage @('Window Manager')
    $palette = Get-ShowcaseNode 50013 'Aurora' $page
    $null = Invoke-ShowcaseOperation 'ISelectionItemProvider::Select' $palette
    Update-ShowcaseTree
    $palette = Get-ShowcaseNode 50013 'Aurora'
    Confirm-ShowcaseEqual (Get-ShowcaseProperty $palette 30079) $true 'Palette replacement retains selection'
    Write-Host 'PASS showcase menus, toolbar, window lifecycle, transform and palette refresh'
}

Test-TextLists
Test-Grids
Test-Texts
Test-CalendarLayout
Test-TabInventory
Write-Host 'PASS showcase recursive tab inventory'
Test-WindowOperations
$null = Invoke-ShowcaseOperation 'IWindowProvider::Close' $tree.nodes[0]
$client.Close()
Write-Host "PASS showcase $checks assertions; hosted=$Hosted"
