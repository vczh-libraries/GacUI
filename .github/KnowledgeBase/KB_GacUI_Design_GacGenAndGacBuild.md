# GacGen and GacBuild Resource Generation

`GacGen` compiles one GacUI XML resource for one target CPU architecture. Native `GacBuild` orchestrates it in two modes: `GacGen` compiles both architectures, merges generated C++ and deploys binaries; `GacBuild` discovers a tree of resources and calculates an incremental dependency-aware build, calling the same `RunGacGen` implementation for every candidate. `<Tools repo>/Tools/GacGen.ps1` and `<Tools repo>/Tools/GacBuild.ps1` are thin wrappers preserving their original interfaces. `<Tools repo>/Tools/GacClear.ps1` invalidates the per-resource caches used by that calculation.

The compiler source of truth is [`<GacUI repo>/Tools/GacGen/GacGen/Main.cpp`](https://github.com/vczh-libraries/GacUI/blob/master/Tools/GacGen/GacGen/Main.cpp) and [`<GacUI repo>/Tools/GacGen/GacGen/GacGen.cpp`](https://github.com/vczh-libraries/GacUI/blob/master/Tools/GacGen/GacGen/GacGen.cpp). Orchestration lives in [`<GacUI repo>/Tools/GacBuild/GacBuild/GacBuild.cpp`](https://github.com/vczh-libraries/GacUI/blob/master/Tools/GacBuild/GacBuild/GacBuild.cpp), with native process, copy and timestamp operations in its sibling `Native.Windows.cpp` and `Native.Linux.cpp`. `<Tools repo>/Tools/GacCommon.ps1` remains available to `<Tools repo>/Tools/GacClear.ps1`.

## Choosing an Entry Point

| Entry point | Use it for | Architectures | Dependency mapping |
| --- | --- | --- | --- |
| `<Tools repo>/Tools/GacBuild.ps1 -FileName <driver-xml>` | Normal Windows incremental builds of every resource below one directory | x86 and x64 | Discovered, ordered, and supplied automatically |
| `<Tools repo>/Tools/GacBuild.ps1 -FileName <driver-xml> -Dump` | Inspect discovery, timestamps, names, dependencies, and proposed candidates without compiling | Metadata is dumped with `/D32`; no resource is compiled | Generated but not consumed by a compile |
| `<Tools repo>/Tools/GacGen.ps1 -FileName <resource-xml> [-MappingFileName <mapping>]` | Build one resource through the same two-platform staging and merge path used by `<Tools repo>/Tools/GacBuild.ps1` | x86 and x64 | Optional mapping is forwarded to both invocations |
| `GacGen /C32 <resource-xml>` or `/C64` | Directly publish one architecture, normally for a standalone resource or platform-specific maintenance script | Exactly one | None; a resource declaring dependencies cannot resolve them through this mode |
| `GacGen /P32 <resource-xml> [mapping]` or `/P64` | Produce the architecture staging tree expected by `<Tools repo>/Tools/GacGen.ps1` or another orchestration tool | Exactly one | Optional |
| `GacGen /D32 <resource-xml> <output-xml>` or `/D64` | Export input/output timestamp metadata for a build planner | Exactly one metadata environment | Not used |
| `<Tools repo>/Tools/GacClear.ps1 -FileName <driver-xml>` | Delete all discovered `<application repo>/<resource-xml>.log` caches so the next `<Tools repo>/Tools/GacBuild.ps1` run rebuilds everything | Both caches are deleted | Discovery only |

The wrappers resolve adjacent `GacBuild.exe`, `GacGen.exe` and `CppMerge.exe` through `$PSScriptRoot`, independent of the caller's working directory. The native orchestrator itself makes no location assumptions. `GacClear.ps1` retains its `GacCommon.ps1` and `StartProcess.ps1` helpers.

Direct native invocation requires `-mode:GacBuild` or `-mode:GacGen`, `-pathGacGen:<absolute executable>`, `-pathCppMerge:<absolute executable>`, and `-FileName <xml>`. Both child paths must identify existing executable files. Missing, duplicate, empty, relative or invalid child paths are rejected before logs change. `GacBuild` mode accepts `-Dump`; `GacGen` mode accepts `-MappingFileName <mapping>`. Relative XML and mapping paths use the caller's working directory. Quote each path separately. Windows child creation preserves wide arguments; `CppOutput.txt` still limits production C++ paths to characters representable in the active MBCS code page. Native Linux/macOS builds use POSIX child processes and file operations.

EasyLayout duplicate initial composition/control content is rejected during XML compilation by `GuiEasyInstanceLoader<T>::AssignParameters` in `<GacUI repo>/Source/Compiler/InstanceLoaders/GuiInstanceLoader_EasyLayout.cpp`. The default property uses `SupportArray` with layout, control and composition acceptable types, so the framework passes all direct children in one batch. A local flag detects duplicate payloads without state in `types::ResolvingResult`. The loader returns `Unsupported()` for the named `Composition` and `Layouts` properties, rejecting their XML assignment and binding forms before reflected-property fallback. Their native and Workflow APIs remain unchanged. Generated C++ contains ordinary assignments without runtime duplicate guards. `BuildLayout` still validates the resulting descriptor tree, including composition/sublayout mixing, during initialization or explicit rebuilding.

## GacGen Runtime Inputs

### Reflection metadata beside the executable

`GacGen` is compiled in meta-only reflection mode and must load type metadata matching the requested target architecture. The names in this subsection are basenames beside the selected executable (for example, `<Tools repo>/Tools/GacGen.exe`), unless an override below selects another directory.

- By default it reads `Reflection32.bin` for `/P32`, `/D32`, or `/C32`, and `Reflection64.bin` for the `64` variants, from the executable directory.
- An optional `Metadata.txt` beside the executable, or one directory above it when no beside-executable file exists, overrides this. It contains at least three lines: metadata folder, x86 binary name, and x64 binary name. The folder is resolved relative to `Metadata.txt`.
- The GacUI development copy uses [`<GacUI repo>/Tools/GacGen/Metadata.txt`](https://github.com/vczh-libraries/GacUI/blob/master/Tools/GacGen/Metadata.txt) to select the basenames `ReflectionCore32.bin` and `ReflectionCore64.bin` in `<GacUI repo>/Test/Resources/Metadata`. The released tool normally has `Reflection32.bin` and `Reflection64.bin` beside it and needs no override.
- The target architecture is a compilation input even when the executable itself was built as Win32. In particular, Workflow `vint` and RPC schemas can differ between x86 and x64.

### Resource XML and `GacGenConfig`

The input is a normal GacUI XML resource loadable by `GuiResource::LoadFromXml`. File-backed folders/items and imported XML files are inputs too. The compiler configuration is stored inside the resource:

```xml
<Resource>
  <Folder name="GacGenConfig">
    <Xml name="Metadata">
      <ResourceMetadata Name="MyResource" Version="1.0">
        <Dependencies>
          <Resource Name="BaseResource"/>
        </Dependencies>
      </ResourceMetadata>
    </Xml>
    <Folder name="Cpp">
      <Text name="SourceFolder">Source</Text>
      <Text name="NormalInclude">GacUI.h;MyTypes.h</Text>
      <Text name="ReflectionInclude">GacUIReflection.h</Text>
      <Text name="Name">MyApp</Text>
      <Text name="Resource">../Bin/MyApp.bin</Text>
      <Text name="CppResource">MyAppResource.cpp</Text>
    </Folder>
    <Folder name="ResX86">
      <Text name="Resource">../Bin/MyApp.x86.bin</Text>
      <Text name="Compressed">../Bin/MyApp.x86.compressed.bin</Text>
      <Text name="Assembly">../Bin/MyApp.x86.assembly.bin</Text>
    </Folder>
    <Folder name="ResX64">
      <Text name="Resource">../Bin/MyApp.x64.bin</Text>
      <Text name="Compressed">../Bin/MyApp.x64.compressed.bin</Text>
      <Text name="Assembly">../Bin/MyApp.x64.assembly.bin</Text>
    </Folder>
  </Folder>
</Resource>
```

All configured output paths are resolved from the directory containing the resource XML. For the example resource `<application repo>/UI/Resource.xml`, generated C++ goes to `<application repo>/UI/Source` and configured binaries go to `<application repo>/Bin`. Configuration keys such as `Cpp/SourceFolder` are resource item names, not filesystem references.

| Configuration item | Required when its folder exists | Meaning |
| --- | --- | --- |
| `GacGenConfig/Metadata`: `/ResourceMetadata/@Name` | No | Stable resource name used by dependency mappings and `import-res://<name>/...`. A resource with dependencies must have a nonempty name. Names should be unique within one `GacBuild` tree. |
| `GacGenConfig/Metadata`: `/ResourceMetadata/@Version` | No | Resource format version. Empty is replaced with the current version; a nonempty value must equal the current value, presently `1.0`. |
| `GacGenConfig/Metadata`: `/ResourceMetadata/Dependencies/Resource/@Name` | No | Direct named dependencies. GacGen recursively reads their cached metadata and loads their scripted resources before compiling this resource. |
| `Cpp/SourceFolder` | Yes | Production directory for generated C++ in `/C` mode and merge destination in the two-platform script. |
| `Cpp/NormalInclude` | Yes | Semicolon-separated includes placed in the ordinary generated header. An empty text item is allowed when no include is needed. |
| `Cpp/ReflectionInclude` | No | Semicolon-separated includes guarded for reflection-enabled builds. |
| `Cpp/Name` | No | C++ assembly/base name. It should be a valid C++ identifier and defaults to `GacUIApplication`. |
| `Cpp/Resource` | No | Uncompressed data-only resource binary, without the compiled Workflow assembly. |
| `Cpp/Compressed` | No | The configuration schema describes a compressed data-only resource. The current `CompileResource` implementation in [`<GacUI repo>/Tools/GacGen/GacGen/Main.cpp`](https://github.com/vczh-libraries/GacUI/blob/master/Tools/GacGen/GacGen/Main.cpp) routes this path through the assembly-output list, so it actually receives the serialized `Assembly.bin` payload. Do not use this setting as a compressed-resource output until that implementation is corrected. |
| `Cpp/CppResource` | No | File name, under `SourceFolder`, for a C++ source that embeds the uncompressed data-only resource and registers its loader plugin. |
| `Cpp/CppCompressed` | No | Like `CppResource`, but the embedded byte array is compressed. Configure and compile only one of the two embedded-resource files. |
| `ResX86/Resource`, `ResX64/Resource` | No | Architecture-specific uncompressed resource containing the Workflow instance-class assembly. Load with `GuiResourceUsage::InstanceClass`. |
| `ResX86/Compressed`, `ResX64/Compressed` | No | Compressed form of the corresponding scripted resource. Decompress before loading with `GuiResourceUsage::InstanceClass`. |
| `ResX86/Assembly`, `ResX64/Assembly` | No | Serialized Workflow instance-class assembly only, for pairing with a data-only resource when the two are loaded separately. |

RPC declarations such as `@rpc:Interface` and `@rpc:Ctor` require the `Cpp` folder: GacGen needs `Cpp/Name` and `Cpp/SourceFolder` to name and publish the RPC C++ pair. RPC metadata is produced for the complete validated contract, including dependencies.

### Dependency mapping file

The optional `/P32` or `/P64` mapping is line-oriented text. In these examples, replace `<application repo>` with the application repository's absolute checkout directory before invoking a tool or writing a mapping. In filesystem expressions, `<driver-xml>`, `<resource-xml>` and `<mapped-xml>` stand for XML paths relative to that repository:

```text
BaseResource=><application repo>/BaseResource.xml
MyResource=><application repo>/MyResource.xml
```

Blank lines are ignored. The first `=>` separates the exact resource name from the XML path; duplicate names or malformed nonempty lines abort that compilation path. GacBuild writes absolute paths and includes every named resource it discovered.

GacGen reads mappings with encoding detection so UTF-8/no-BOM driver manifests retain non-ASCII paths; BOM-tagged UTF text and legacy MBCS mappings remain supported. This does not change the separate MBCS-only/no-BOM `CppOutput.txt` contract.

For each declared dependency, GacGen looks up the mapped XML and reads `<application repo>/<mapped-xml>.log/x32/ScriptedResource.bin` or `<application repo>/<mapped-xml>.log/x64/ScriptedResource.bin`. It reads the embedded resource metadata to discover transitive dependencies, then loads each scripted resource with `GuiResourceUsage::InstanceClass`. Therefore dependencies must already have a successful cache for the same architecture. This is why GacBuild supplies a topological order and why direct `/C32` or `/C64`, which accepts no mapping argument, is suitable only for resources without declared dependencies.

## GacBuild Driver XML

`<Tools repo>/Tools/GacBuild.ps1 -FileName` expects a separate discovery file, conventionally `GacUI.xml`, whose directory is the recursive search root:

```xml
<GacUI>
  <Exclude Pattern="/Generated/"/>
  <Exclude Pattern="/Lib/FakeResource.xml"/>
</GacUI>
```

- Every `*.xml` below that directory is parsed. It becomes a candidate resource only when XPath `//Resource/Folder[@name='GacGenConfig']` exists.
- Each exclusion is a case-sensitive substring test against the full path after replacing `\` with `/`. It is not a wildcard or regular expression. Use `/` in the pattern.
- Keep the driver XML distinct from a resource XML. The driver owns `<application repo>/<driver-xml>.log`, while a resource owns `<application repo>/<resource-xml>.log`; using the same file for both makes those roles collide.
- `-Dump` performs all discovery, `/D32` metadata dumps, candidate calculation, and dependency sorting, then stops before resource compilation.

## GacBuild Incremental and Dependency Algorithm

In the algorithm and artifact descriptions below, driver-manifest basenames such as `ResourceFiles.txt` refer to files in `<application repo>/<driver-xml>.log`. Per-resource artifact basenames such as `Errors.txt` refer to files in `<application repo>/<resource-xml>.log/<architecture>`, where `<architecture>` is `x32` or `x64`. The directory trees and tables below distinguish these two groups.

Each run performs these steps:

1. Resolve the driver XML to an absolute path, delete and recreate `<application repo>/<driver-xml>.log`, enumerate resources, and write `ResourceFiles.txt`.
2. Run `<Tools repo>/Tools/GacGen.exe /D32 <resource-xml> <flattened-dump>` for every discovered resource. `/D32` records the resource metadata, all file-backed resource inputs, and the ten standard x86/x64 cache outputs.
3. Mark a resource directly outdated when any recorded cache output is missing, or when any recorded input has a later UTC modification time than any recorded output.
4. Split resources into anonymous resources and resources with metadata names. Anonymous resources are independent and sorted by absolute path. Named resources are topologically sorted from their declared dependencies.
5. Starting with directly outdated named resources, repeatedly add every named transitive dependent. Thus changing a base resource rebuilds all named resources above it even when their own files are unchanged.
6. Write the candidate, order, and mapping manifests. Iterate anonymous resources first and named resources second; call `RunGacGen` directly for each candidate without launching another orchestrator.
7. `RunGacGen` deletes that resource's old log once, runs `/P32` and `/P64`, and rejects failed children, `Errors.txt`, or missing/empty required artifacts. When C++ is configured, both `CppOutput.txt` destinations must agree, both staged filename sets must match, and every input must be nonempty. Create the production directory and invoke CppMerge for every pair, preserving existing destination files and their user content. Verify the published text against CppMerge's owning library algorithms, because the historical CppMerge CLI can silently fail a write.
8. Validate every source and destination parent in both `Deploy.xml` manifests before merging or copying. After successful compilation and merging, copy both inventories using native file operations, x64 first and x32 last to preserve the final x32 choice for shared neutral outputs. `/P` itself only stages binaries; direct `/C` publication is unchanged.

The incremental contract deliberately watches only the ten standard `.bin` cache files listed by `/D32`. It does not timestamp-check `Workflow.txt`, generated C++, RPC metadata, `Deploy.xml`, configured production outputs or tool binaries. Deleting a production C++ or configured binary while leaving a fresh per-resource cache can therefore produce `[SKIPPED]`; run `<Tools repo>/Tools/GacClear.ps1`, delete that resource's `.log` directory, or invoke its build explicitly to regenerate it.

Named dependency metadata must form a closed acyclic graph with unique names. GacBuild rejects duplicate names, missing dependencies, and cycles; GacGen independently rejects an unresolved mapped dependency while compiling. `ResourceNamedFiles.txt` contains all named resources in build order, not only candidates; `BuildCandidates.txt` is the filter.

Native GacBuild stops at the first error and returns nonzero; both PowerShell wrappers throw on that status. A generation, merge or deployment failure invalidates the resource's x32 `Assembly.bin` cache before reporting failure, so an unchanged ten-file freshness check cannot skip it on the next attempt. Other diagnostics remain available. Successful invocation semantics are preserved; the old script's continue-after-error behavior is intentionally removed.

## Generated and Cached File Organization

There are two independent `.log` directory shapes.

### Driver manifest directory

For `<Tools repo>/Tools/GacBuild.ps1 -FileName <application repo>/GacUI.xml`:

```text
<application repo>/GacUI.xml.log\
  ResourceFiles.txt
  BuildCandidates.txt
  ResourceAnonymousFiles.txt
  ResourceNamedFiles.txt
  ResourceNamedMapping.txt
  _sub_folder_Resource.xml
  _another_Resource.xml
  ... one flattened XML metadata dump per discovered resource
```

The following table lists basenames within `<application repo>/<driver-xml>.log`.

| Basename | Format and content | Consumer and purpose |
| --- | --- | --- |
| `ResourceFiles.txt` | One path per discovered resource, relative to the driver directory and normally beginning with `\` on Windows | `DumpResourceFiles`, `<Tools repo>/Tools/GacClear.ps1`, and diagnostic tools use it as the discovery inventory. |
| `<flattened-resource-path>` | UTF-8 BOM XML. Native separators in the relative resource path are replaced with `_`; the original extension is retained. The outer `ResourceMetadata` contains the resource's saved `ResourceMetadata`, sorted absolute `Inputs/Input@Path`, and sorted absolute `Outputs/Output@Path`. | GacBuild parses every dump to calculate timestamps, names, dependencies, order, and mappings; colliding flattened names are rejected. |
| `BuildCandidates.txt` | One absolute resource XML path per direct stale resource plus every named transitive dependent | Records the native build loop's `[BUILD]` selections in anonymous-then-dependency order. |
| `ResourceAnonymousFiles.txt` | Sorted absolute paths of resources whose metadata name is empty | Defines the first part of iteration order. Anonymous resources cannot declare dependencies. |
| `ResourceNamedFiles.txt` | Absolute paths of every named resource in dependency-before-dependent order | Defines the second part of iteration order, including named resources that will be skipped. |
| `ResourceNamedMapping.txt` | Lexically sorted `Name=>absolute-resource-xml-path` lines | Passed to every `/P32` and `/P64` call. GacGen resolves direct and transitive dependency caches through it. |

The five named `.txt` manifests are unquoted UTF-8 without BOM, one record per line. Spaces are part of a path; only `ResourceNamedMapping.txt` assigns separator meaning, to the first `=>`. The flattened dumps are XML rather than line manifests.

The dump's `Outputs` list is always these ten absolute paths, regardless of optional production configuration:

```text
<application repo>/<resource-xml>.log\x32\Assembly.bin
<application repo>/<resource-xml>.log\x32\Compressed.bin
<application repo>/<resource-xml>.log\x32\Resource.bin
<application repo>/<resource-xml>.log\x32\ScriptedCompressed.bin
<application repo>/<resource-xml>.log\x32\ScriptedResource.bin
<application repo>/<resource-xml>.log\x64\Assembly.bin
<application repo>/<resource-xml>.log\x64\Compressed.bin
<application repo>/<resource-xml>.log\x64\Resource.bin
<application repo>/<resource-xml>.log\x64\ScriptedCompressed.bin
<application repo>/<resource-xml>.log\x64\ScriptedResource.bin
```

### Per-resource architecture directory

For `<application repo>/UI/Resource.xml`, `/P32` and `/P64` use:

```text
<application repo>/UI/Resource.xml.log\
  x32\
    Workflow.txt
    Resource.bin
    Compressed.bin
    ScriptedResource.bin
    ScriptedCompressed.bin
    Assembly.bin
    CppOutput.txt                         conditional: Cpp config in /P mode
    Deploy.xml                            successful /P, including empty inventories
    Errors.txt                            error only
    WorkflowRpc.txt                       RPC only
    RpcMetadata.txt                       RPC only
    RpcMetadata.d.ts                      RPC only
    Source\                               C++ staging, when applicable
      <ordinary generated C++ files>
      <configured CppResource file>       embedded resource, when configured
      <configured CppCompressed file>     embedded resource, when configured
      <Name>Rpc.h                         RPC only
      <Name>Rpc.cpp                       RPC only
      <Name>Rpc.gacui.rpc.contract        direct /C RPC staging only
  x64\
    ... the same architecture-local categories ...
```

Each architecture subdirectory is deleted and recreated by a compilation after dependencies have been loaded. The sibling architecture directory is preserved so direct `/C32` and `/C64` RPC runs can rendezvous through their staging contracts. Native GacBuild's `RunGacGen` deletes the entire resource log before starting its `/P32` and `/P64` pair.

Bare names in the following table and cleanup description are basenames within `<application repo>/<resource-xml>.log/<architecture>`; generated C++ and RPC files live in its `Source` subdirectory, shown with qualified paths.

| Per-architecture artifact | Format and exact payload | Consumer and purpose |
| --- | --- | --- |
| `Workflow.txt` | UTF-8 BOM, human-readable generated Workflow code. Modules are separated by `================================(n/count)================================`. | Diagnostic representation of the compiled `Workflow/InstanceClass` program; useful to compiler/tool authors. It is not the runtime assembly. |
| `Resource.bin` | Uncompressed precompiled resource with the `Precompiled/Workflow` folder temporarily removed | Architecture-neutral data-only cache. Load with `GetResourceManager()->LoadResourceOrPending`; generated C++ supplies the instance classes. It is also the source payload for `Cpp/Resource`. |
| `Compressed.bin` | LZW-compressed form of the same data-only resource | Cache/tooling variant. Call `DecompressStream` or use an LZW decoder before loading. |
| `ScriptedResource.bin` | Uncompressed precompiled resource including the architecture-specific Workflow assembly and leading resource metadata | Runtime scripted-resource cache. Load with `GuiResourceUsage::InstanceClass`. GacGen itself consumes this exact file for named dependencies and reads its metadata to expand the transitive closure. |
| `ScriptedCompressed.bin` | LZW-compressed scripted resource | Compressed runtime scripted-resource cache; decompress before loading with `GuiResourceUsage::InstanceClass`. |
| `Assembly.bin` | Serialized Workflow `InstanceClass` assembly only | Used when a program intentionally loads a neutral resource and assembly separately; it is also the source payload for configured `ResX86/Assembly` or `ResX64/Assembly`. |
| `CppOutput.txt` | MBCS/no-BOM text containing one absolute production `Cpp/SourceFolder` path | Written only by `/P` when `Cpp` exists. Native GacBuild decodes both copies with the BOM-aware/MBCS reader and requires equal destinations. On Windows the path must be representable in the active code page. |
| `Deploy.xml` | UTF-8 BOM XML: root `Deploy`, with zero or more `File` children carrying XML-escaped absolute `Source` and `Destination` attributes | Always written on successful `/P`, replacing `Deploy.bat`. GacBuild validates the complete copy inventory, then deploys x64 followed by x32 after merging. `/P` no longer writes configured binary destinations. |
| `Errors.txt` | UTF-8 BOM lines from `GuiResourceError::SortAndLog`: a resource-path/file header, optional `Original:` header, then one-based `(row, column): message` lines | Created only for structured load, precompile, Workflow, C++, RPC, or output errors. Native GacBuild prints it and fails. Absence is necessary but not sufficient: GacBuild also requires both nonempty five-file binary caches, `Workflow.txt`, and `Deploy.xml`. |
| `<application repo>/<resource-xml>.log/<architecture>/Source/` in `/P` | UTF-8 BOM architecture-specific C++ staging files | Native GacBuild validates both filename sets and invokes CppMerge for each pair, publishing merged files while preserving supported user regions. |
| `<application repo>/<resource-xml>.log/<architecture>/Source/<CppResource>` or `<application repo>/<resource-xml>.log/<architecture>/Source/<CppCompressed>` | UTF-8 BOM generated C++ containing the data-only resource byte array and loader plugin | Staged and merged like the ordinary C++ set. Compile exactly one selected embedded-resource implementation into the application. |
| `WorkflowRpc.txt` | UTF-8 BOM Workflow text with two labeled modules: RPC wrapper and JSON RPC wrapper | Reference/debug form of the generated bridge. It describes the glue later translated into `<Name>Rpc.h/.cpp`. |
| `RpcMetadata.txt` | UTF-8 BOM normalized Workflow RPC metadata module for the complete contract and dependencies | Together with `RpcMetadata.d.ts`, authoritative input for generators targeting C++, TypeScript, or another language. It supplies interfaces, operations, inheritance, transfer behavior, and IDs. |
| `RpcMetadata.d.ts` | UTF-8 BOM TypeScript declarations for contract-specific known/unknown JSON value shapes | Complementary to `RpcMetadata.txt`; it does not by itself contain the complete interface/routing contract. x86 and x64 outputs can differ, so a consumer must choose the target ABI directory. |
| `<application repo>/<resource-xml>.log/<architecture>/Source/<Name>Rpc.h/.cpp` | UTF-8 BOM, exactly one non-reflection C++ pair including the ordinary `<Name>.h` entry | Statically linked RPC and JSON-RPC wrappers. The generated global entry is `vl_workflow_global::<Name>Rpc::Instance()`. In `/P`, CppMerge publishes the x86/x64 merged pair. |
| `<application repo>/<resource-xml>.log/<architecture>/Source/<Name>Rpc.gacui.rpc.contract` | UTF-8 BOM line format beginning `GacGen RPC C++ platform contract v1`, followed by input/config values, include counts/lists, and the full generated `WorkflowRpc` text | Direct `/C` mode only. It proves the other architecture's staged RPC pair came from the same input/configuration. When contracts match, the second architecture run merges and publishes the pair; stale or incompatible staging is deleted. |
| `<application repo>/<resource-xml>.log/<architecture>/Source/<Name>Rpc.(h|cpp).gacui.tmp` and the corresponding `.gacui.backup` file | Temporary UTF-8 files and preserved previous files used by the transactional two-file RPC writer | Normally exist only during a write. Cleanup removes them on success; they can remain after an unrecoverable write/rollback failure and are evidence for a future diagnostic tool. |

When RPC is removed or compilation fails after the C++ configuration is known, GacGen removes stale `WorkflowRpc.txt`, `RpcMetadata.txt`, `RpcMetadata.d.ts`, the RPC pair, the platform contract, and legacy `<Name>RpcReflection.h/.cpp` or `<Name>RpcIncludes.h` files. Those legacy files are cleanup targets, not outputs of the current generator.

## Production Outputs and Their Consumers

### Ordinary generated C++

With `Cpp/Name` equal to `MyApp`, GacGen forces Workflow multi-file and reflection generation and uses these basename rules. The production directory is selected by `Cpp/SourceFolder`; for the XML example above it is `<application repo>/UI/Source`:

- `MyApp.h` is the aggregate entry header. It includes `MyAppPartialClasses.h`, all `@cpp:File("X")` headers, and any numbered dependency-split headers.
- `MyAppPartialClasses.h` and `MyAppPartialClasses.cpp` contain declarations and implementations not assigned to a custom file.
- `MyAppPartialClasses1.h`, `MyAppPartialClasses2.h`, and so on are emitted when the Workflow generator splits header declarations to satisfy dependencies. They are included by the aggregate header.
- `X.h` and `X.cpp` are emitted for classes assigned by `@cpp:File("X")`; GacUI code-behind classes commonly use this route.
- `MyAppReflection.h` and `MyAppReflection.cpp` register generated Workflow types when reflection is enabled and retain a no-reflection-compatible shape behind compile guards.
- The configured `CppResource` or `CppCompressed` file embeds the data-only resource bytes, defines a `MyAppResourceReader`, and registers `GacGen_MyAppResourceLoader`. Linking it lets the plugin load the resource before `GuiMain`, eliminating a separate resource file.
- `MyAppRpc.h` and `MyAppRpc.cpp` are added only when validated RPC metadata exists.

Generated files contain explicit `USER_CONTENT_BEGIN(...)` / `USER_CONTENT_END()` regions for `@cpp:UserImpl` methods, custom members, and custom global declarations. Both direct generation and the two-platform merge preserve content only in those regions; all other manual changes may be overwritten. Current marker-delimited regions end at `USER_CONTENT_END()` and do not count braces inside the region. Only the legacy `USERIMPL(...)` form relies on line-oriented brace matching and therefore requires its structural opening and closing braces to begin their lines.

In `/P` mode ordinary files live first under `<application repo>/<resource-xml>.log/x32/Source` and `<application repo>/<resource-xml>.log/x64/Source`, then `CppMerge` writes the combined production set under `Cpp/SourceFolder`. In `/C` mode ordinary files are written directly to `Cpp/SourceFolder` and merged with existing user regions there. Direct-mode RPC files are different: each architecture stages its pair and contract under its log `Source`; the second compatible architecture run publishes the merged pair to `Cpp/SourceFolder`.

### Resource and assembly loading

The cache basenames below are in `<application repo>/<resource-xml>.log/<architecture>`. Configuration item names identify outputs resolved relative to the resource XML, as described above.

- A `Cpp/Resource` file or `Resource.bin` contains data only. Load it with `GetResourceManager()->LoadResourceOrPending`; compile/link the generated C++ classes.
- A `ResX86/Resource`, `ResX64/Resource`, or `ScriptedResource.bin` contains the corresponding Workflow assembly. Load it with `GuiResourceUsage::InstanceClass` and enable the reflection/runtime support required by scripted instances.
- Decompress `Compressed.bin`, `ScriptedCompressed.bin`, or a configured compressed output before passing the resulting stream to the resource manager.
- An embedded `CppResource`/`CppCompressed` plugin performs data-only resource loading automatically; compile exactly the chosen generated source into the application.
- `Assembly.bin` is not a resource container. Consume it through Workflow assembly deserialization when deliberately pairing it with a data-only resource.

## Clearing, Failure Detection, and Tool Integration

`<Tools repo>/Tools/GacClear.ps1` recreates the driver log, writes a fresh `ResourceFiles.txt`, and recursively deletes `<application repo>/<resource-xml>.log` for every discovered resource. It does not delete production C++, configured resource binaries, or the remaining `<application repo>/<driver-xml>.log/ResourceFiles.txt`. Missing standard caches make every resource directly outdated on the next GacBuild run.

For automation built directly on GacGen, use all of these success checks:

- Require the expected architecture log and standard cache files.
- Treat `Errors.txt` as failure and surface its contents.
- Capture console output, because invalid resource versions, missing or malformed mapping files, unresolved dependency binaries, some configuration failures, and log-directory failures can be printed without creating `Errors.txt`.
- Do not use process exit code alone for semantic validation. `SetupGacGenNativeController` returns zero after `GuiMain`, and many handled compiler errors return from `CompileResource` without changing that result.
- For RPC code generation, consume `RpcMetadata.txt` and `RpcMetadata.d.ts` together from the ABI being targeted. If invoking `/C` directly, retain the sibling architecture staging directory until both compatible runs have completed.
- Treat `.log` directories as generated caches and tool interchange, not authored source. Preserve them between incremental GacBuild runs; remove them when a full rebuild or stale-output recovery is required.

## Build and Release Integration

`<Tools repo>/Tools/ProjectGacUI.ps1` preserves the bootstrap sequence: copy metadata, pack the current GacUI sources, build and deploy GacGen and GacBuild, regenerate DarkSkin/TuiSkin through the wrapper, then pack the final sources. Neither generator depends on generated skins. Tool cleanup and deployment include GacBuild alongside GacGen and CppMerge.

`<Tools repo>/Tools/BuildRelease.ps1` copies the owned GacBuild header and implementation files, including both native platform files, into `<Release repo>/Tools/Executables/GacBuild`. Release maintains its own project/filter files and Unix makefile. `<Release repo>/Tools/CopyExecutables.ps1` deploys the Windows executable; `<Release repo>/Tools/BuildExecutables.sh` builds and deploys the Unix executable. Released GacGen loads adjacent full `Reflection32.bin` and `Reflection64.bin`; development `Metadata.txt` selects core-only metadata.

The Bash wrappers `<Release repo>/Tools/GacBuild.sh -FileName <driver-xml> [-Dump]` and `<Release repo>/Tools/GacGen.sh -FileName <resource-xml> [-MappingFileName <mapping>]` forward arguments unchanged to native GacBuild. They calculate absolute sibling executable paths, preserve the caller's working directory, and use `exec` to preserve the native exit status. Release updates replace PowerShell wrappers and executables while retaining the maintained Bash wrappers.
