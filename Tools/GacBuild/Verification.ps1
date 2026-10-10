param(
    [string]$GacBuildPath = "$PSScriptRoot/Bin/GacBuildD.exe",
    [string]$GacGenPath = "$PSScriptRoot/../GacGen/Bin/GacGenD.exe",
    [string]$CppMergePath = "$PSScriptRoot/../../../Workflow/Tools/CppMerge/Bin/CppMerge.exe",
    [switch]$Baseline
)

$ErrorActionPreference = 'Stop'
$repo = [System.IO.Path]::GetFullPath("$PSScriptRoot/../..")
$toolsRepo = [System.IO.Path]::GetFullPath("$repo/../Tools")
if (!(Test-Path -LiteralPath $CppMergePath)) { $CppMergePath = "$toolsRepo/Tools/CppMerge.exe" }
$GacBuildPath = (Resolve-Path -LiteralPath $GacBuildPath).Path
$GacGenPath = (Resolve-Path -LiteralPath $GacGenPath).Path
$CppMergePath = (Resolve-Path -LiteralPath $CppMergePath).Path
$fixture = Join-Path $PSScriptRoot ('Bin/Verification-' + [Guid]::NewGuid().ToString('N'))
[void][System.IO.Directory]::CreateDirectory($fixture)
$utf8 = [System.Text.UTF8Encoding]::new($true)
$utf8Plain = [System.Text.UTF8Encoding]::new($false)
$userFile = "$PSScriptRoot/GacBuild/GacBuild.vcxproj.user"
$userBackup = if (Test-Path -LiteralPath $userFile) { [System.IO.File]::ReadAllBytes($userFile) } else { $null }
$script:invocations = 0

function Assert-That([bool]$Condition, [string]$Message) {
    if (!$Condition) { throw $Message }
}

function Write-Text([string]$Path, [string]$Content) {
    [void][System.IO.Directory]::CreateDirectory([System.IO.Path]::GetDirectoryName($Path))
    [System.IO.File]::WriteAllText($Path, $Content, $utf8)
}

function Invoke-Native([string[]]$InvocationArguments, [bool]$Success = $true) {
    $script:invocations++
    $command = ($InvocationArguments | ForEach-Object { '"' + $_.Replace('"', '\"') + '"' }) -join ' '
    $escaped = [System.Security.SecurityElement]::Escape($command)
    $userXml = '<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003"><PropertyGroup Condition="''$(Configuration)|$(Platform)''==''Debug|x64''"><LocalDebuggerCommandArguments>' + $escaped + '</LocalDebuggerCommandArguments></PropertyGroup></Project>'
    [System.IO.File]::WriteAllText($userFile, $userXml, $utf8Plain)
    Push-Location $PSScriptRoot
    try {
        $output = & "$repo/.github/Scripts/copilotExecute.ps1" -Mode CLI -Executable GacBuild -Configuration Debug -Platform x64 2>&1
        $exitCode = $LASTEXITCODE
    } finally { Pop-Location }
    $text = $output -join "`r`n"
    Write-Text "$fixture/Invocation-$script:invocations.log" $text
    if (($exitCode -eq 0) -ne $Success) { throw "Unexpected exit $exitCode in invocation $script:invocations`n$text" }
    return $text
}

function Build-Arguments([string]$Mode, [string]$InputFile, [string[]]$Extra = @()) {
    return @(("-mode:$Mode"), ("-pathGacGen:$GacGenPath"), ("-pathCppMerge:$CppMergePath"), '-FileName', $InputFile) + $Extra
}

function Get-PlanPaths([string]$Output, [string]$Status = 'BUILD|SKIPPED') {
    [regex]::Matches($Output, "(?m)^\[(?:$Status)\] (.+)$") | ForEach-Object { $_.Groups[1].Value.TrimEnd("`r") }
}

function New-Resource([string]$Folder, [string]$Name, [string]$Dependency = '', [switch]$BinaryOnly, [switch]$Rpc, [switch]$Anonymous) {
    $dependencyXml = if ($Dependency) { '<Dependencies><Resource Name="' + $Dependency + '"/></Dependencies>' } else { '' }
    $metadataName = if ($Anonymous) { '' } else { $Name }
    $cpp = if ($BinaryOnly) { '' } else {
        '<Folder name="Cpp"><Text name="SourceFolder">Source</Text><Text name="NormalInclude">GacUI.h</Text><Text name="Name">' + $Name + '</Text><Text name="Resource">Published/Neutral.bin</Text><Text name="CppResource">Embedded.cpp</Text></Folder>'
    }
    $rpcCode = if ($Rpc) { '@rpc:Interface @rpc:Ctor interface IService { func Translate(value : string) : string; }' } else { '' }
    $editable = if ($BinaryOnly) { '' } else { '@cpp:File("Editable") class Editable { @cpp:UserImpl func Read() : int { return 1; } }' }
    $code = 'module fixture_' + $Name + '; namespace fixture_' + $Name + ' { func Identity(value : int) : int { return value; } func Unsigned(value : uint) : uint { return value; } ' + $editable + ' ' + $rpcCode + ' }'
    $xml = @"
<Resource>
  <Folder name="GacGenConfig">
    <Xml name="Metadata"><ResourceMetadata Name="$metadataName" Version="1.0">$dependencyXml</ResourceMetadata></Xml>
    $cpp
    <Folder name="ResX86"><Text name="Resource">Published/x32.bin</Text><Text name="Compressed">Published/x32.compressed.bin</Text><Text name="Assembly">Published/x32.assembly.bin</Text></Folder>
    <Folder name="ResX64"><Text name="Resource">Published/x64.bin</Text><Text name="Compressed">Published/x64.compressed.bin</Text><Text name="Assembly">Published/x64.assembly.bin</Text></Folder>
  </Folder>
  <Script name="Logic"><Workflow><![CDATA[$code]]></Workflow></Script>
</Resource>
"@
    [void][System.IO.Directory]::CreateDirectory("$Folder/Published")
    Write-Text "$Folder/Resource.xml" $xml
    return "$Folder/Resource.xml"
}

function Assert-Outputs([string]$Resource, [bool]$Cpp = $true) {
    $folder = Split-Path -Parent $Resource
    foreach ($architecture in @('x32', 'x64')) {
        Assert-That (!(Test-Path -LiteralPath "$Resource.log/$architecture/Errors.txt")) 'Unexpected compiler errors.'
        foreach ($name in @('Resource.bin','Compressed.bin','ScriptedResource.bin','ScriptedCompressed.bin','Assembly.bin','Workflow.txt','Deploy.xml')) {
            Assert-That ((Get-Item -LiteralPath "$Resource.log/$architecture/$name").Length -gt 0) "Missing $architecture/$name"
        }
        foreach ($pair in @(@('ScriptedResource.bin', "$architecture.bin"), @('ScriptedCompressed.bin', "$architecture.compressed.bin"), @('Assembly.bin', "$architecture.assembly.bin"))) {
            Assert-That ((Get-FileHash -LiteralPath "$Resource.log/$architecture/$($pair[0])").Hash -eq (Get-FileHash -LiteralPath "$folder/Published/$($pair[1])").Hash) 'Deployment differs from cache.'
            $cached = Get-Item -LiteralPath "$Resource.log/$architecture/$($pair[0])"
            $published = Get-Item -LiteralPath "$folder/Published/$($pair[1])"
            Assert-That ($cached.CreationTimeUtc -eq $published.CreationTimeUtc -and $cached.LastWriteTimeUtc -eq $published.LastWriteTimeUtc) 'Copy did not preserve creation/modification times.'
        }
        Assert-That (!(Test-Path -LiteralPath "$Resource.log/$architecture/Deploy.bat")) 'Legacy deployment batch remains.'
    }
    if ($Cpp) {
        $source32 = @(Get-ChildItem -LiteralPath "$Resource.log/x32/Source" -File | Sort-Object Name)
        $source64 = @(Get-ChildItem -LiteralPath "$Resource.log/x64/Source" -File | Sort-Object Name)
        Assert-That (($source32.Name -join '|') -eq ($source64.Name -join '|')) 'Architecture filename sets differ.'
        foreach ($file in $source32) { Assert-That (Test-Path -LiteralPath "$folder/Source/$($file.Name)") 'Missing merged output.' }
        Assert-That ((Get-FileHash -LiteralPath "$Resource.log/x32/Resource.bin").Hash -eq (Get-FileHash -LiteralPath "$folder/Published/Neutral.bin").Hash) 'Neutral deployment must select x32.'
    }
}

try {
    # The execution wrapper uses standard configuration folders; keep ignored aliases for the tools' Bin layout.
    [void][System.IO.Directory]::CreateDirectory("$PSScriptRoot/x64/Debug")
    Copy-Item -LiteralPath $GacBuildPath -Destination "$PSScriptRoot/x64/Debug/GacBuild.exe" -Force
    $driver = "$fixture/GacUI.xml"
    Write-Text $driver '<GacUI><Exclude Pattern="/Excluded/"/></GacUI>'
    $base = New-Resource "$fixture/Base" Base
    $dependent = New-Resource "$fixture/Dependent" Dependent Base
    $leaf = New-Resource "$fixture/Leaf" Leaf Dependent
    $independent = New-Resource "$fixture/Independent" Independent -Anonymous
    [void](New-Resource "$fixture/Excluded" Excluded)
    $buildArgs = Build-Arguments GacBuild $driver

    Write-Text $driver '<GacUI><Exclude Pattern=""/></GacUI>'
    $plan = Invoke-Native ($buildArgs + '-Dump')
    Assert-That (@(Get-PlanPaths $plan).Count -eq 0) 'An empty exclusion pattern must exclude every path.'
    Write-Text $driver '<GacUI><Exclude/><Exclude Pattern="/Excluded/"/></GacUI>'
    $plan = Invoke-Native ($buildArgs + '-Dump')
    Assert-That (@(Get-PlanPaths $plan).Count -eq 4) 'An absent Pattern attribute must be ignored.'
    $ordinalFolder = "$fixture/Excluded/Patterns"
    $softHyphen = [string][char]0xAD
    [void](New-Resource "$ordinalFolder/Soft${softHyphen}Hyphen" Ordinal)
    [void](New-Resource "$ordinalFolder/UpperCase" CaseSensitive)
    Write-Text "$ordinalFolder/GacUI.xml" '<GacUI><Exclude Pattern="/SoftHyphen/"/><Exclude Pattern="/uppercase/"/></GacUI>'
    $plan = Invoke-Native (Build-Arguments GacBuild "$ordinalFolder/GacUI.xml" @('-Dump'))
    Assert-That (@(Get-PlanPaths $plan).Count -eq 2) 'Exclusions must use ordinal case-sensitive substring matching.'
    Write-Text $driver '<GacUI><Exclude Pattern="/Excluded/"/></GacUI>'

    if ($Baseline) {
        . "$toolsRepo/Tools/GacCommon.ps1"
        [void][System.IO.Directory]::CreateDirectory("$driver.log")
        EnumerateResourceFiles $driver
        $oldInventory = @(Get-Content -LiteralPath "$driver.log/ResourceFiles.txt" | Sort-Object)
        $plan = Invoke-Native ($buildArgs + '-Dump')
        $newInventory = @(Get-PlanPaths $plan | ForEach-Object { $_.Substring(([System.IO.Path]::GetFullPath($fixture)).Length) } | Sort-Object)
        Assert-That (($oldInventory -join '|') -eq ($newInventory -join '|')) 'Native discovery differs from the old script.'
        $dumps = @{}
        Get-ChildItem -LiteralPath "$driver.log" -Filter '*.xml' | ForEach-Object {
            [xml]$dump = Get-Content -LiteralPath $_.FullName -Raw
            $path = @($dump.ResourceMetadata.Inputs.Input.Path | Where-Object { $_ -like '*Resource.xml' })[0]
            $dumps[$path] = $dump
        }
        EnumerateBuildCandidates $dumps "$fixture/OldCandidates.txt"
        EnumerateNamedResources $dumps "$fixture/OldNamed.txt" "$fixture/OldMapping.txt"
        $oldCandidates = @(Get-Content -LiteralPath "$fixture/OldCandidates.txt" | Sort-Object)
        $newCandidates = @(Get-PlanPaths $plan 'BUILD' | Sort-Object)
        Assert-That (($oldCandidates -join '|') -eq ($newCandidates -join '|')) 'Build candidates differ from the legacy planner.'
        $oldMapping = @(Get-Content -LiteralPath "$fixture/OldMapping.txt" | Sort-Object)
        $newMapping = @(Get-Content -LiteralPath "$driver.log/ResourceNamedMapping.txt" | Sort-Object)
        Assert-That (($oldMapping -join '|') -eq ($newMapping -join '|')) 'Dependency mapping differs from the legacy planner.'
    }
    $plan = Invoke-Native ($buildArgs + '-Dump')
    Assert-That (!(Test-Path -LiteralPath "$base.log")) '-Dump compiled resources.'
    Assert-That (@(Get-PlanPaths $plan 'BUILD').Count -eq 4) 'Cold build should select four resources.'
    foreach ($obsolete in @('ResourceFiles.txt','BuildCandidates.txt','ResourceAnonymousFiles.txt','ResourceNamedFiles.txt')) {
        Assert-That (!(Test-Path -LiteralPath "$driver.log/$obsolete")) "Obsolete planning manifest remains: $obsolete"
    }
    $plan = Invoke-Native $buildArgs
    foreach ($resource in @($base,$dependent,$leaf,$independent)) { Assert-Outputs $resource }
    $expectedOrder = @($independent,$base,$dependent,$leaf).ForEach({ [System.IO.Path]::GetFullPath($_) }) -join '|'
    Assert-That ((@(Get-PlanPaths $plan) -join '|') -eq $expectedOrder) 'Anonymous resources must precede dependency-ordered named resources.'
    $merged = Get-Content -LiteralPath "$fixture/Base/Source/BasePartialClasses.h" -Raw
    Assert-That ($merged.Contains('::vl::vint') -and $merged.Contains('::vl::vuint') -and !$merged.Contains('::vl::vint32_t') -and !$merged.Contains('::vl::vint64_t')) 'Native-width types were not merged.'
    $output = Invoke-Native $buildArgs
    Assert-That (($output | Select-String -Pattern '\[BUILD\]') -eq $null) 'Unchanged build did not skip.'
    Assert-That (@(Get-PlanPaths $output 'SKIPPED').Count -eq 4) 'Unchanged plan did not retain all resources.'
    Remove-Item -LiteralPath "$fixture/Base/Published/x32.bin"
    [void](Invoke-Native $buildArgs)
    Assert-That (!(Test-Path -LiteralPath "$fixture/Base/Published/x32.bin")) 'Production output unexpectedly affected freshness.'
    [System.IO.File]::SetLastWriteTimeUtc($base, [DateTime]::UtcNow.AddSeconds(1))
    $plan = Invoke-Native ($buildArgs + '-Dump')
    Assert-That (@(Get-PlanPaths $plan 'BUILD').Count -eq 3) 'Transitive dependents were not selected.'
    [System.IO.File]::SetLastWriteTimeUtc($base, [DateTime]::UtcNow.AddSeconds(-10))
    Remove-Item -LiteralPath "$base.log/x32/Assembly.bin"
    [void](Invoke-Native $buildArgs)
    Assert-Outputs $base

    $cppFile = Get-Item -LiteralPath "$fixture/Base/Source/Editable.cpp"
    $cppText = [System.IO.File]::ReadAllText($cppFile.FullName)
    Assert-That ($cppText.Contains('USER_CONTENT_BEGIN')) 'Missing user-content fixture marker.'
    $cppText = $cppText.Replace('/* USER_CONTENT_BEGIN(custom global declarations) */', "/* USER_CONTENT_BEGIN(custom global declarations) */`r`n// GacBuild verification retained content")
    Write-Text $cppFile.FullName $cppText
    [void](Invoke-Native (Build-Arguments GacGen $base))
    Assert-That ([System.IO.File]::ReadAllText($cppFile.FullName).Contains('// GacBuild verification retained content')) 'User content was lost.'
    $stamp = [System.IO.File]::GetLastWriteTimeUtc($cppFile.FullName)
    [void](Invoke-Native (Build-Arguments GacGen $base))
    Assert-That ([System.IO.File]::GetLastWriteTimeUtc($cppFile.FullName) -eq $stamp) 'Identical C++ output was rewritten.'

    $sharedFolder = New-Resource "$fixture/Shared output folder" SharedOutput
    Write-Text $sharedFolder ((Get-Content -LiteralPath $sharedFolder -Raw).Replace('Published/Neutral.bin','Source/Neutral.bin'))
    [void](Invoke-Native (Build-Arguments GacGen $sharedFolder))
    Assert-That ((Get-FileHash -LiteralPath "$sharedFolder.log/x32/Resource.bin").Hash -eq (Get-FileHash -LiteralPath "$fixture/Shared output folder/Source/Neutral.bin").Hash) 'Cannot deploy into the newly created C++ directory.'

    $binary = New-Resource "$fixture/Binary only" BinaryOnly -BinaryOnly
    [void](Invoke-Native (Build-Arguments GacGen $binary))
    Assert-Outputs $binary $false
    $rpc = New-Resource "$fixture/Rpc with spaces" RpcFixture -Rpc
    [void](Invoke-Native (Build-Arguments GacGen $rpc))
    Assert-Outputs $rpc
    foreach ($architecture in @('x32','x64')) {
        Assert-That ((Get-Item -LiteralPath "$rpc.log/$architecture/RpcMetadata.txt").Length -gt 0) 'Missing RPC metadata.'
        Assert-That ((Get-Item -LiteralPath "$rpc.log/$architecture/RpcMetadata.d.ts").Length -gt 0) 'Missing RPC declarations.'
    }

    [void](Invoke-Native @('-mode:GacGen','-pathGacGen:relative.exe',('-pathCppMerge:' + $CppMergePath),'-FileName',$base) $false)
    [void](Invoke-Native ((Build-Arguments GacGen $base) + '-mode:GacGen') $false)
    $badMapping = "$fixture/BadMapping.txt"
    Write-Text $badMapping 'malformed mapping'
    [void](Invoke-Native (Build-Arguments GacGen $base @('-MappingFileName',$badMapping)) $false)
    Assert-That (!(Test-Path -LiteralPath "$base.log/x32/Assembly.bin")) 'Failed compilation retained a fresh cache.'
    [void](Invoke-Native (Build-Arguments GacGen $base))

    $missing = New-Resource "$fixture/Missing" Missing Absent
    [void](Invoke-Native ($buildArgs + '-Dump') $false)
    Write-Text $driver '<GacUI><Exclude Pattern="/Excluded/"/><Exclude Pattern="/Missing/"/></GacUI>'
    $cycleA = New-Resource "$fixture/CycleA" CycleA CycleB
    $cycleB = New-Resource "$fixture/CycleB" CycleB CycleA
    [void](Invoke-Native ($buildArgs + '-Dump') $false)
    Write-Text $driver '<GacUI><Exclude Pattern="/Excluded/"/><Exclude Pattern="/Missing/"/><Exclude Pattern="/Cycle"/></GacUI>'

    # A singleton strongly connected component is still a cycle when it depends on itself.
    $selfCycle = New-Resource "$fixture/Excluded/SelfCycle" SelfCycle SelfCycle
    $selfDriver = "$fixture/Excluded/SelfCycle/GacUI.xml"
    Write-Text $selfDriver '<GacUI/>'
    [void](Invoke-Native (Build-Arguments GacBuild $selfDriver @('-Dump')) $false)

    # A locked destination makes CppMerge's old unchecked write return success; the orchestrator must detect it.
    $original = [System.IO.File]::ReadAllText($cppFile.FullName)
    Write-Text $cppFile.FullName ($original.Replace('namespace', 'namespace /* deliberately stale */'))
    $locked = [System.IO.File]::Open($cppFile.FullName, 'Open', 'Read', 'Read')
    try { [void](Invoke-Native (Build-Arguments GacGen $base) $false) } finally { $locked.Dispose() }
    Assert-That (!(Test-Path -LiteralPath "$base.log/x32/Assembly.bin")) 'Failed merge retained a fresh cache.'
    [void](Invoke-Native $buildArgs)
    Assert-Outputs $base

    $published = "$fixture/Base/Published/x64.bin"
    $locked = [System.IO.File]::Open($published, 'Open', 'Read', 'Read')
    try { [void](Invoke-Native (Build-Arguments GacGen $base) $false) } finally { $locked.Dispose() }
    Assert-That (!(Test-Path -LiteralPath "$base.log/x32/Assembly.bin")) 'Failed deployment retained a fresh cache.'
    [void](Invoke-Native $buildArgs)
    Assert-Outputs $base

    # Exercise missing artifacts and mismatched architecture sets after otherwise successful real compiles.
    $proxySource = @'
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
public class CompilerProxy {
    public static int Main(string[] args) {
        string[] config = File.ReadAllLines(System.Reflection.Assembly.GetExecutingAssembly().Location + ".txt");
        var child = Process.Start(new ProcessStartInfo(config[0], String.Join(" ", args.Select(a => "\"" + a + "\""))) { UseShellExecute = false, CreateNoWindow = true });
        child.WaitForExit();
        if (child.ExitCode != 0) return child.ExitCode;
        if (args[0] == "/P64") {
            string log = args[1] + ".log/x64";
            if (config[1] == "missing") File.Delete(log + "/Assembly.bin");
            if (config[1] == "mismatch") File.Copy(Directory.GetFiles(log + "/Source")[0], log + "/Source/Extra.cpp");
        }
        return 0;
    }
}
'@
    Write-Text "$fixture/CompilerProxy.cs" $proxySource
    Write-Text "$fixture/CompileProxy.ps1" 'Add-Type -Path "$PSScriptRoot/CompilerProxy.cs" -OutputAssembly "$PSScriptRoot/CompilerProxy.exe" -OutputType ConsoleApplication -ReferencedAssemblies System.Core'
    & "$env:SystemRoot/System32/WindowsPowerShell/v1.0/powershell.exe" -NoProfile -File "$fixture/CompileProxy.ps1"
    Assert-That ($LASTEXITCODE -eq 0) 'Cannot build compiler fault fixture.'
    $realGacGen = $GacGenPath
    try {
        $GacGenPath = [System.IO.Path]::GetFullPath("$fixture/CompilerProxy.exe")
        foreach ($fault in @('missing','mismatch')) {
            Write-Text "$GacGenPath.txt" "$realGacGen`r`n$fault"
            $publishedStamp = [System.IO.File]::GetLastWriteTimeUtc($published)
            [void](Invoke-Native (Build-Arguments GacGen $base) $false)
            Assert-That ([System.IO.File]::GetLastWriteTimeUtc($published) -eq $publishedStamp) 'Deployment followed a failed prerequisite.'
            Assert-That (!(Test-Path -LiteralPath "$base.log/x32/Assembly.bin")) 'Failed prerequisite retained a fresh cache.'
        }
    } finally { $GacGenPath = $realGacGen }
    [void](Invoke-Native $buildArgs)

    $invalid = New-Resource "$fixture/Excluded/Invalid" Invalid
    $validXml = Get-Content -LiteralPath $invalid -Raw
    Write-Text $invalid ($validXml.Replace('return value;', 'return unknown_symbol;'))
    [void](Invoke-Native (Build-Arguments GacGen $invalid) $false)
    Assert-That (Test-Path -LiteralPath "$invalid.log/x32/Errors.txt") 'Expected structured compiler diagnostics.'
    Write-Text $invalid ($validXml.Replace('Version="1.0"', 'Version="9.0"'))
    [void](Invoke-Native (Build-Arguments GacGen $invalid) $false)
    Assert-That (!(Test-Path -LiteralPath "$invalid.log/x32/Errors.txt")) 'Version fixture should exercise console-only diagnostics.'
    Write-Text $invalid '<Resource>'
    [void](Invoke-Native (Build-Arguments GacGen $invalid) $false)
    Write-Text $invalid $validXml
    [void](Invoke-Native (Build-Arguments GacGen $invalid))
    $duplicate = New-Resource "$fixture/Duplicate" Base
    [void](Invoke-Native ($buildArgs + '-Dump') $false)
    Write-Text $driver '<GacUI><Exclude Pattern="/Excluded/"/><Exclude Pattern="/Missing/"/><Exclude Pattern="/Cycle"/><Exclude Pattern="/Duplicate/"/></GacUI>'
    foreach ($bad in @(@('-mode:GacGen','-FileName',$base),@('-mode:GacGen','-pathGacGen:',("-pathCppMerge:$CppMergePath"),'-FileName',$base),@('-mode:GacGen',("-pathGacGen:$fixture/missing.exe"),("-pathCppMerge:$CppMergePath"),'-FileName',$base))) {
        $stamp = [System.IO.File]::GetLastWriteTimeUtc("$base.log/x32/Assembly.bin")
        [void](Invoke-Native $bad $false)
        Assert-That ([System.IO.File]::GetLastWriteTimeUtc("$base.log/x32/Assembly.bin") -eq $stamp) 'Invalid options changed the resource cache.'
    }

    # Swap the ignored execution alias to exercise direct /C publication through the same CLI runner.
    $direct = New-Resource "$fixture/Excluded/Direct" Direct
    # Direct single-ABI generation has no platform merge for pointer-sized user method signatures.
    Write-Text $direct ((Get-Content -LiteralPath $direct -Raw).Replace('@cpp:UserImpl ',''))
    [void][System.IO.Directory]::CreateDirectory("$fixture/Excluded/Direct/Source")
    Copy-Item -LiteralPath $GacGenPath -Destination "$PSScriptRoot/x64/Debug/GacBuild.exe" -Force
    $metadataRelative = [System.IO.Path]::GetRelativePath("$PSScriptRoot/x64/Debug", "$repo/Test/Resources/Metadata")
    Write-Text "$PSScriptRoot/x64/Debug/Metadata.txt" "$metadataRelative`r`nReflectionCore32.bin`r`nReflectionCore64.bin"
    try {
        foreach ($arch in @('32','64')) {
            [void](Invoke-Native @("/C$arch",$direct))
            Assert-That ((Get-FileHash -LiteralPath "$direct.log/x$arch/ScriptedResource.bin").Hash -eq (Get-FileHash -LiteralPath "$fixture/Excluded/Direct/Published/x$arch.bin").Hash) 'Direct /C did not publish.'
        }
        if ($Baseline) {
            $hashes = @{}
            Get-ChildItem -LiteralPath "$fixture/Excluded/Direct/Published","$fixture/Excluded/Direct/Source" -File | ForEach-Object { $hashes[$_.FullName] = (Get-FileHash -LiteralPath $_.FullName).Hash }
            Copy-Item -LiteralPath "$toolsRepo/Tools/GacGen.exe" -Destination "$PSScriptRoot/x64/Debug/GacBuild.exe" -Force
            [void](Invoke-Native @('/C64',$direct))
            foreach ($file in $hashes.Keys) { Assert-That ((Get-FileHash -LiteralPath $file).Hash -eq $hashes[$file]) 'Direct compiler output differs from the baseline.' }
            Copy-Item -LiteralPath $GacGenPath -Destination "$PSScriptRoot/x64/Debug/GacBuild.exe" -Force
        }
        $staged = New-Resource "$fixture/Excluded/Staging only" StagingOnly
        [void](Invoke-Native @('/P32',$staged))
        Assert-That (!(Test-Path -LiteralPath "$fixture/Excluded/Staging only/Published/x32.bin")) '/P published before orchestration.'
        Assert-That (Test-Path -LiteralPath "$staged.log/x32/Deploy.xml") 'Missing standalone /P deployment manifest.'
    } finally { Copy-Item -LiteralPath $GacBuildPath -Destination "$PSScriptRoot/x64/Debug/GacBuild.exe" -Force }

    # Wrappers must find sibling tools while resolving user resource/mapping paths from another directory.
    $wrapperFolder = "$fixture/Excluded/Tools with spaces & markup"
    [void][System.IO.Directory]::CreateDirectory($wrapperFolder)
    Copy-Item -LiteralPath $GacBuildPath -Destination "$wrapperFolder/GacBuild.exe"
    Copy-Item -LiteralPath $GacGenPath -Destination "$wrapperFolder/GacGen.exe"
    Copy-Item -LiteralPath $CppMergePath -Destination "$wrapperFolder/CppMerge.exe"
    Copy-Item -Path "$toolsRepo/Tools/Gac*.ps1","$toolsRepo/Tools/StartProcess.ps1" -Destination $wrapperFolder
    $metadataRelative = [System.IO.Path]::GetRelativePath($wrapperFolder, "$repo/Test/Resources/Metadata")
    Write-Text "$wrapperFolder/Metadata.txt" "$metadataRelative`r`nReflectionCore32.bin`r`nReflectionCore64.bin"
    $unicode = [string][char]0x00E9
    $wrapperResource = New-Resource "$fixture/Excluded/Wrapper $unicode & markup" Wrapper
    $wrapperRelative = [System.IO.Path]::GetRelativePath($fixture,$wrapperResource)
    Push-Location $fixture
    try {
        & "$wrapperFolder/GacBuild.ps1" -FileName 'GacUI.xml' -Dump *> "$fixture/WrapperDump.log"
        & "$wrapperFolder/GacGen.ps1" -FileName $wrapperRelative -MappingFileName 'GacUI.xml.log/ResourceNamedMapping.txt' *> "$fixture/WrapperGen.log"
        Assert-Outputs $wrapperResource
        $failed = $false
        try { & "$wrapperFolder/GacGen.ps1" -FileName $wrapperRelative -MappingFileName 'BadMapping.txt' *> "$fixture/WrapperFailure.log" } catch { $failed = $true }
        Assert-That $failed 'PowerShell wrapper hid native failure.'
        & "$wrapperFolder/GacClear.ps1" -FileName 'GacUI.xml' *> "$fixture/WrapperClear.log"
        Assert-That (!(Test-Path -LiteralPath "$base.log")) 'GacClear did not invalidate caches.'
        & "$wrapperFolder/GacBuild.ps1" -FileName 'GacUI.xml' *> "$fixture/WrapperBuild.log"
        Assert-Outputs $base
        Write-Text "$fixture/Excluded/Encoding/GacUI.xml" '<GacUI/>'
        $encodedBase = New-Resource "$fixture/Excluded/Encoding/Base $unicode" EncodingBase
        $encodedDependent = New-Resource "$fixture/Excluded/Encoding/Dependent" EncodingDependent EncodingBase
        & "$wrapperFolder/GacBuild.ps1" -FileName 'Excluded/Encoding/GacUI.xml' *> "$fixture/WrapperEncoding.log"
        Assert-Outputs $encodedDependent
    } finally { Pop-Location }
    Write-Host "PASS: $script:invocations native invocations; fixtures and logs: $fixture"
} finally {
    if ($null -ne $userBackup) { [System.IO.File]::WriteAllBytes($userFile, $userBackup) }
    elseif (Test-Path -LiteralPath $userFile) { Remove-Item -LiteralPath $userFile }
}
