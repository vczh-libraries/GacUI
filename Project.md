# Project Specific Instruction

## Maintaining Tools

- Native resource orchestration lives in `<GacUI repo>/Tools/GacBuild`; the single-architecture compiler remains in `<GacUI repo>/Tools/GacGen`. Update their canonical guidance in `<Tools repo>/Copilot/KnowledgeBase/KB_GacUI_Design_GacGenAndGacBuild.md` and propagate it through instruction synchronization.
- If the behavior of any tool changes, update:
  - `<GacUI repo>/Tools/GacBuild`, `<GacUI repo>/Tools/GacGen`, and the thin wrappers `<Tools repo>/Tools/GacBuild.ps1` and `<Tools repo>/Tools/GacGen.ps1`: the canonical GacGen/GacBuild guidance above.
  - `REPO-ROOT/Tools/UiaList`:
    - Update `REPO-ROOT/.github/KnowledgeBase/KB_GacUI_Design_UiaList.md` for UiaListApp usage.
    - This tool is Windows only, check out `REPO-ROOT/Tools/UiaList/AGENTS.md` if and only if you are working on it.
  - `REPO-ROOT/Tools/GitView`:
    - Update `REPO-ROOT/.github/KnowledgeBase/KB_GacUI_Design_GitTui.md` for GitTui usage.
    - This tool is cross platform, check out `REPO-ROOT/Tools/GitView/AGENTS.md` if and only if you are working on it.
- When only changing tools without changing GacUI itself or built-in skins, GacUI test process could be completely skipped.
- When only changing test apps without changing GacUI itself or built-in skins, GacUI unit test could be completely skipped.

### Native Resource Tools

- Build `<GacUI repo>/Tools/GacBuild/GacBuild.sln` and `<GacUI repo>/Tools/GacGen/GacGen.sln` with the repository build wrapper. Both provide Debug/Release and Win32/x64 configurations. GacBuild produces `<GacUI repo>/Tools/GacBuild/Bin/GacBuildD.exe` or `GacBuild.exe`; GacGen produces the corresponding executables under `<GacUI repo>/Tools/GacGen/Bin`. Native Unix builds use `<GacUI repo>/Tools/GacBuild/vmake` and `<GacUI repo>/Tools/GacGen/vmake`.
- GacBuild requires `-mode:GacBuild` or `-mode:GacGen`, explicit normalized absolute `-pathGacGen` and `-pathCppMerge`, and `-FileName`. The former accepts `-Dump`; the latter accepts `-MappingFileName`. Named dependency ordering uses `PartialOrderingProcessor`; driver logs retain the mapping and metadata XML, with build/skip decisions printed to the console. VlppOS owns path validation, file metadata and metadata-preserving copies.
- GacGen `/P32` and `/P64` stage outputs and write UTF-8 `Deploy.xml` manifests. GacBuild validates both architectures, merges C++, then copies the manifests with x32 last for shared neutral outputs. Direct `/C32` and `/C64` still publish configured outputs.
- Run `<GacUI repo>/Tools/GacBuild/Verification.ps1 -Baseline` for focused Windows integration coverage. It uses the repository CLI wrapper, keeps fixtures in the ignored tool `Bin` directory, and restores debugger arguments after execution. Optional tool-path parameters select other built configurations.
- Tools-only changes use this focused verification; native Linux/macOS execution must be recorded separately from Windows tests and static inventory checks.

## Solution to Work On

You are working on the solution `REPO-ROOT/Test/GacUISrc/GacUISrc.sln`,
therefore `SOLUTION-ROOT` is `REPO-ROOT/Test/GacUISrc`.

## Files not Allowed to Modify

Files in these folders (recursively) are not allowed to modify.
You can only change them using what is described in the `Code Generation Projects` section.
If you encounter any error that prevent these files from being generated,
always fix the root cause.
- `REPO-ROOT/Test/Resources/Metadata`
- `REPO-ROOT/Test/GacUISrc/Generated_DarkSkin`
- `REPO-ROOT/Test/GacUISrc/Generated_TuiSkin/Source_x86` and `Source_x64`
- `REPO-ROOT/Test/GacUISrc/Generated_TuiSkin/Resource_x86` and `Resource_x64`
- `REPO-ROOT/Test/GacUISrc/Generated_TuiDialogs`
- `REPO-ROOT/Test/GacUISrc/Generated_TuiControlTest/Source_x86` and `Source_x64`
- `REPO-ROOT/Test/GacUISrc/Generated_TuiControlTest/Resource_x86` and `Resource_x64`
- `REPO-ROOT/Test/GacUISrc/Generated_Dialogs`
- `REPO-ROOT/Test/GacUISrc/Generated_FullControlTest/Source_x86` and `Source_x64`
- `REPO-ROOT/Test/GacUISrc/Generated_FullControlTest/Resource_x86` and `Resource_x64`
- `REPO-ROOT/Test/GacUISrc/Generated_RemoteProtocolTest`
- `REPO-ROOT/Test/GacUISrc/Generated_RemoteViewModelTest`
- `REPO-ROOT/Test/GacUISrc/Generated_UnitTestViewer`
- `REPO-ROOT/Source/Utilities/FakeServices/Dialogs/Source`
- `REPO-ROOT/Source/Utilities/FakeServices/TuiDialogs/Source`
- `REPO-ROOT/Source/UnitTestUtilities/SnapshotViewer/Source`
- `REPO-ROOT/Source/Compiler/InstanceQuery/Generated`
- `REPO-ROOT/Source/Compiler/RemoteProtocol/Generated`
- `<GacUI repo>/Tools/GitView/GitView/Source`
- `<GacUI repo>/Tools/UiaList/UiaList/Source`

Files in `REPO-ROOT/Import` and `REPO-ROOT/Release` (recursively) are also not allowed to modify.
These files are prepared for foreign dependencies.

When files here are modified, they should be included in a dedicated commit, aka manually udpated files and generated files should be included in two different commits. If there is any unit test snapshot changes, a dedicated third commit should be created for these.

### Source/Utilities/AutomationService

The reusable MiniHTTP and Windows HTTP automation endpoints live in this folder and are compiled through `Test/GacUISrc/Source_GacUI_Core/Source_GacUI_Core.vcxitems`.
- `MiniHttpAutomationService.*` is CodePacked into `Release/GacUI.h` and `Release/GacUI.cpp`.
- `Windows/WindowsAutomationService.Windows.*` is CodePacked into `Release/GacUI.Windows.h` and `Release/GacUI.Windows.cpp`.
- Standalone test applications receive these implementations through their GacUI library dependency; they do not import `Source_RemotingHelpers` for automation.

### Test/RemotingHelpers

Files in this folder are for test apps only:
- Only test apps could use these source files.
- No need to create unit test for them.
- Source files in `Source` cannot use anything in `Test/RemotingHelpers`.
- They are excluded from ordinary public GacUI amalgamations and the aggregate `Release/Import` production snapshot. GacUI CodePack emits the dedicated neutral `Test.RemotingHelpers` pair in `Release` and `Release/IncludeOnly`; `UpdateRelease` copies the root pair and its README to the aggregate `Release/Import-Test` snapshot. The test-only stdio transport and its platform implementations are supplied by imported VlppOS release files.
- No production quality required, these files are only for building test apps quickly.

### Test/RemotingHelpers/Rvmt

Files in this folder are generic client and requester helpers used only by `CppTest_Rvm`, `RemotingTest_Core`, and `RemotingTest_RvmHost`.
- They are enumerated and compiled through the single `Test/GacUISrc/Source_RemotingHelpers/Source_RemotingHelpers.vcxitems` inventory shared by all helper consumers.
- `ViewModelShared.h` owns generic aliases, fixed RVM channel/control constants, and inline Ready-message helpers; `ViewModelHostClient.*` owns the generic network-side host client; and `ViewModelHostServer.*` owns the specialized channel server's generic RPC helpers and server-side local-client behavior.
- Generated RemoteViewModelTest RPC composition belongs to `Test/GacUISrc/Generated_RemoteViewModelTest/RemoteViewModelTestInitialize.*`, which each affected application invokes after its generic connection is assigned a client ID.
- `Release/CodegenConfig.xml` scans this helper tree only into the dedicated test pairs. Ordinary `GacUI*` pairs and the aggregate `Release/Import` production snapshot remain independent of these helpers.

## Reflectable Types

- You must be really careful when changing any interface, especially structs, classes, unions and a few functions.
- Check if the class and the method is registered in reflection.
- Reflection allow registering normal functions into a class, becoming its static functions. It is not easy to determine by the function definition itself.
- You must read the knowledge base about reflection and try to find the pattern in any *.cpp file.
- If the reflection registration is affected, you should always fix the reflection and run necessary code generation projects.

## Projects for Verification

You are required to follow the guideline to run any project in this solution,
do not run the compiled binary directly.

The `REPO-ROOT/Test/GacUISrc/UnitTest/UnitTest.vcxproj` is the unit test project.
When any *.h or *.cpp file is changed, unit test is required to run.
Except for the `GuiRemoteRendererSingle` class which is not covered in the unit test.

When any test case fails, you must fix the issue immediately, even those errors are unrelated to the issue you are working on.

For any GacUI specific unit test that running with the GacUI unit test framework,
when it calls `GacUIUnitTest_StartFast_WithResourceAsText` with path, for example, `Application/Windows/Order`,
running it ends up creating log files in `REPO-ROOT/Test/Resources/UnitTestSnapshots/Application/Windows`:
- Order.json: an entry of snapshots for this test case
- Order[*].*: log files about remote protocol recordings and compiler output
- Order/Frame_*.json: snapshot of the UI DOM tree for each frame.

Each Frame_*.json is captured at each `OnNextIdleFrame` call, recording what the UI look like before running the code in this frame.
This is the reason why the name of the frame should say what the previous frame was done,
so that frame names in snapshot files make sense.

Remote-protocol snapshot tests (`GacUIUnitTest_Start`, `GacUIUnitTest_StartFast_WithResourceAsText`, and related entry points) are for recording and reviewing rendering results. Every such case must capture at least one useful rendering frame; an empty frame list is not allowed. Merely recording a frame does not justify using this framework when the rendered output adds no useful coverage.

Use ordinary unit tests when the coverage consists only of compiler rejection, descriptor properties, geometry, bindings, event handlers, or other non-rendering behavior. Construct the objects directly, or use native service and resource setup when needed, without starting the remote snapshot framework. Keep TUI buffer assertions in ordinary TUI provider tests. When converting or removing a snapshot case, remove its index, frame files, protocol logs and compiler artifacts from `Test/Resources/UnitTestSnapshots` as well.

### Code Generation Tools

#### REPO-ROOT/../Tools/Tools/GlrParserGen.exe

This executable needs to run if any file in the following folders are changed:
- `REPO-ROOT/Source/Compiler/InstanceQuery/Syntax`
- `REPO-ROOT/Source/Compiler/RemoteProtocol/Syntax`

There is a `Parser.xml` file in these folder.
You need to offer the absolute path of `Parser.xml` to the tool as a command-line argument.
Only run necessary `Parser.xml` in folders that are changed.

### Code Generation Projects

Code generation projects are CLI projects.
They are required to run when a certain set of files are changed,
in order to generate code paring with them.
Here are a list of projects to run and files that should trigger them:

#### Metadata_Generate and Metadata_Test

These two projects need to run if any reflection code is touched:
- `GuiReflection*.cpp` is updated.
- `GacUI_Compiler` project is executed.

To execute these projects, you should:
- Build the solution with Debug|Win32.
- Run `Metadata_Generate` with Debug|Win32.
- Build the solution with Debug|x64.
- Run `Metadata_Generate` with Debug|x64.
- Run `Metadata_Test` with Debug|x64.

It generates binary metadata files containing type informations from reflection code.
This step cannot be skipped after changing any reflection code,
because GacUI_Compiler and some other test applications consume these binary metadata files.

#### Metadata_UpdateProtocol

This project need to run if `REPO-ROOT/Source/PlatformProviders/Remote/Protocol/*.txt` is updated.

It generates `REPO-ROOT/Source/PlatformProviders/Remote/Generated/*`.

#### GacUI_Compiler

This project need to run if any of the following XML file is updated:
- `REPO-ROOT/Source/Utilities/FakeServices/Dialog/*.xml` -> generates `REPO-ROOT/Source/Utilities/FakeServices/Dialog/Source/*`.
- `REPO-ROOT/Source/UnitTestUtilities/SnapshotViewer/*.xml` -> generates `REPO-ROOT/Source/UnitTestUtilities/SnapshotViewer/Source/*`.
- `REPO-ROOT/Test/Resources/App/DarkSkin/*.xml` -> generates `REPO-ROOT/Test/GacUISrc/Generated_DarkSkin/Source_(x86|x64)/*`.
  - **IMPORTANT**: `REPO-ROOT/Source/Skins/DarkSkin` has another copy, this is used by the CI and is not involved in this solution, ignore it.
- `REPO-ROOT/Test/Resources/App/FullControlTest/*.xml` -> generates `REPO-ROOT/Test/GacUISrc/Generated_FullControlTest/Source_(x86|x64)/*`.
- `REPO-ROOT/Test/Resources/App/RemoteProtocolTest/*.xml` -> generates `REPO-ROOT/Test/GacUISrc/Generated_RemoteProtocolTest/Source_(x86|x64)/*`.
- `REPO-ROOT/Test/Resources/App/RemoteViewModelTest/*.xml` -> generates ordinary and RPC C++ in `REPO-ROOT/Test/GacUISrc/Generated_RemoteViewModelTest/Source_(x86|x64)/*`.

After running `GacUI_Compiler`, you should always `git status` to find if there is any untracked `*.UI.errors.txt`.
- Such file means there are compile errors in some xml files, read it to find the detail.
- You don't need to delete the file, if `GacUI_Compiler` succeeds the next time, they will be gone.
- `GacUI_Compiler` may also fail by printing one line of error message or return non-zero exit code. If the `*.UI.errors.txt` file does not exist, you are recommended to debug the project to find out what happened.
- Whenever `GacUI_Compiler` reports any error, you must fix the issue immediately, even those errors are unrelated to the issue you are working on.

## Maintaining `darkskin::Theme`

This is a default skin that not only releases, but also used by all projects in this solution.
To make a change:
- Update `REPO-ROOT/Test/Resources/App/DarkSkin`.
- Run `GacUI_Compiler` and make sure it updated generated C++ code expectely.
  - Sometimes reordering could happen in generated C++ code even when correlated resource is not changed.
- Rebuild before running any test project.

DarkSkin defines `darkskin::ColorPackage`, the shared neutral initializer and the complete default palette in its authored Workflow resource. `Source/Skins/DarkSkin/Config/DarkSkinConfig.h/.cpp` owns the Aurora/Ember/Moonstone/Lagoon/Rosewood accent values in C++. Each named factory calls Workflow's `CreateColorPackageInternal`; the default factory and setter forward to their Workflow implementations. The installed palette initializes once from the Workflow default before templates read it. Creating another theme or window does not reset a selected palette.

TuiSkin follows the same ownership pattern. `Test/Resources/App/TuiSkin/TuiSkin.xml` defines its two-color initializer and complete opaque SkyBlue default, and initializes `tuiColors` once. `TuiTheme::SetColorPackage` assigns that variable directly; the native default factory and setter forward to Workflow. The five named native factories supply only their accent/highlight pairs. Applications need no startup default installation. The SkyBlue showcase selector uses `CreateDefaultColorPackage`. Internal palette-based baseline document factories remain in both skins; the TUI grid editor uses `tuiskin::TuiDocumentLabelTemplate`.

Both skin inventories and the direct configuration consumers (`UnitTest`, `CppTest_Tui`, the FullControlTest inventory and `GacUI_Host`) define `GACUI_SKIN_DEVELOPMENT` to select the matching `Generated_<Skin>/Source_x86` or `Source_x64` header. Linux development `vmake` inputs define the same macro. Without it, canonical config headers include the merged release `Source` header. CodePack ignores development branches and includes each `Config` folder in its own skin pair. The release workflow first packs current sources and builds/deploys GacGen and GacBuild, which do not depend on skins. It then regenerates both skins through native orchestration and packs the final release, preserving the config files throughout.

`SetColorPackage` affects subsequent template construction. FullControlTest raises `PaletteSelected(int)` from its palette radios, following TuiControlTest. Its manually maintained `Generated_FullControlTest/FullControlTestEventHandlers.h/.cpp` owns `demo::OnPaletteSelected`, which queues preset creation, installation and `GuiApplication::RefreshThemes()` together after input dispatch. Every showcase entry point attaches that shared handler. All presets are always available; DarkSkin has no selector callback, plugin or availability state. `GacUI_Host` loads the showcase from Workflow binary and uses native DarkSkin from `Lib_GacUI_App_Reflection`, attaching the same handler through reflection. DarkSkin document templates construct palette-based baseline overrides, retaining dark text defaults and document state across refresh.

## Debugging Remote Protocol Issues

### Opt-in feature injection and document persistence

`SetupRemoteNativeController` accepts `GuiRemoteControllerConfig`; its DateTime,
Locale and filesystem flags default to false. `GuiRemoteRendererSingle::GlobalConfig`
defaults to true for library consumers. The demo entry points explicitly start
with all flags false; see the `/remoteDT`, `/remoteLocale` and `/remoteFS` sections
under [Maintaining Test Apps](#maintaining-test-apps). Renderer connection capabilities
are reevaluated on replacement. DateTime/Locale fall back to the predecessor;
an enabled but unavailable remote filesystem exposes an empty `/` root.

The canonical renderer entry point is
`<GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp`; wGac/iGac
copy it and `<GacUI repo>/Test/GacUISrc/SharedArguments.h` through their synchronization
scripts. See [Linux/macOS Specific](#linuxmacos-specific) for their replacement projects and launchers.

`<GacUI repo>/Test/GacUISrc/Generated_FullControlTest/FullControlTestEventHandlers.h/.cpp`
is authored code. It attaches the palette handler and four persistence events to
each editor's own viewer. Private `.bin` save, RTF and HTML export finish encoding
and stream close synchronously. Host attaches the same handlers through reflection
without linking generated FullControlTest classes. Private files embed their images;
remote image encoding is available independently of filesystem injection.
Private saves retain the complete style catalog, including unused definitions.

Use `REPO-ROOT/.github/Jobs/DebugRemoteProtocolSop.md` for the shared end-to-end UI operations and
observable results.

Remote protocol is involved in three ways:
- Core with native renderer: `REPO-ROOT/.github/Jobs/DebugRemoteProtocolWithNativeRenderer.md`.
- Core with `GacJS`: `REPO-ROOT/.github/Jobs/DebugRemoteProtocolWithGacJS.md`.
- `UnitTest`, some e2e test cases are running on top of a unit test only remote protocol renderer, which is designed to save snapshots of UI between frames at `REPO-ROOT/Test/Resources/UnitTestSnapshots`.

Running core always uses network protocols.
Three ways are calling three different renderer implementations, but with the same core implementation.
By careful tell if a bug repro in some or all three ways, you can easily narrow down the scope of the possible cause.

### Remote Protocol HTTP Disconnection Contract

- The HTTP remote protocol consists of `/Connect`, `/Request`, and `/Response`. Do not add a reverse `/Disconnect` endpoint or require a renderer-to-core shutdown handshake.
- A renderer can close independently while the core remains available, and another renderer can connect later. If a new renderer connects while an old renderer is still active, accepting the new renderer drops the old connection and token.
- HTTP 404 on an old renderer request after replacement means that renderer's connection is no longer active. After a VlppOS channel has connected, its `IChannelClient` implementation promotes this and every other local protocol error to a fatal local channel error because delivery is no longer reliable.
- If the core has exited, an error from an outstanding or subsequent renderer request, including 404 or another transport failure, is handled by the same channel-level fatal transition. The renderer acts on that callback directly, without requiring `OnDisconnected`, stops emitting requests, and presents its ordinary disconnected state rather than a fatal transport prompt.
- Core shutdown does not wait for a renderer acknowledgement. Requests that reach the core while it is still serving should receive their normal protocol response when possible; errors after the core has stopped are expected.

## Maintaining Test Apps

| Test app | `/AsPort` | `/Gdi` | `/FCT` | `/RPT` | `/RVMT` | `/Pipe` | `/Http` | `/MiniHttp` | `/Cli` | `/corehost` | `/remoteDT` | `/remoteLocale` | `/remoteFS` |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `CppTest` | ✓ | ✓ | | | | | | | | | | | |
| `CppTest_Metaonly` | ✓ | ✓ | | | | | | | | | | | |
| `CppTest_Reflection` | ✓ | ✓ | | | | | | | | | | | |
| `CppTest_Tui` | | | | | | | | | | | | | |
| `GacUI_Host` | ✓ | ✓ | | | | | | | | | | | |
| `CppTest_Rvm` | ✓ | | | | | ✓ | ✓ | ✓ | ✓ | | | | |
| `RemotingTest_Core` | ✓ | | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | | ✓ | ✓ | ✓ |
| `RemotingTest_Rendering_Win32` | ✓ | ✓ | | | | ✓ | ✓ | ✓ | | ✓ | ✓ | ✓ | ✓ |
| `RemotingTest_RvmHost` | | | | | | ✓ | ✓ | ✓ | ✓ | ✓ | | | |

✓ means the argument has a supported effect in that application; blank cells do not imply that every parser rejects the argument. Argument spelling is case-sensitive. The table describes the projects in this solution; platform restrictions and the wGac/iGac replacements are listed under [Linux/macOS Specific](#linuxmacos-specific).

### /AsPort

Use `/AsPort:<decimal-port>` once to select the local automation HTTP port, in `1..65535`; the default is `8888`. Empty, invalid, duplicate and out-of-range values fail before listeners start. Assign distinct ports to independent automation endpoints, such as Core on `8888` and its renderer on `8889`. This option does not change the remote-protocol TCP port (`8888`), named-pipe endpoint or stdio transport; multiple protocol servers still cannot own the same fixed transport endpoint.

Automation URLs use `http://localhost:<AsPort>/Automation/<PROJECT-NAME>/...`, except that `RemotingTest_Rendering_Win32` uses the name `RemotingTest_Rendering_Native`. Core exposes a window-control tree; the renderer exposes a DOM tree. `CppTest_Rvm` uses its own project name and has no separate renderer. `Playground` and `UiaListApp` also accept this option; UiaList's endpoint is Debug-only. `CppTest_Tui` and `RemotingTest_RvmHost` have no automation endpoint.

On Windows, standalone GUI apps use `StartWindowsHttpAutomationService`, as do the remoting apps in `/Pipe` and `/Http` modes and `CppTest_Rvm` in `/Cli` mode. In `/MiniHttp` mode, Core and `CppTest_Rvm` share their protocol socket when automation uses `8888`; another automation port creates a separately owned MiniHTTP socket, stopped with the automation endpoint while preserving protocol shutdown order. The renderer always owns a separate automation socket and still connects to protocol port `8888`.

See `<GacUI repo>/.github/Guidelines/Running-GacUI.md` for the automation contract and execution procedure.

### /Gdi

Use the exact `/Gdi` switch once to select GDI instead of the default Direct2D renderer on Windows. It can be combined with `/AsPort`. For remote FullControlTest, run Core with `/FCT /Pipe /AsPort:8888` and the renderer with `/Pipe /Gdi /AsPort:8889`. The renderer attaches the clock painter when it receives an element whose ID is `FullControlTest.Clock`. Core has no native drawing backend, and `CppTest_Rvm` always uses hosted Direct2D, so `/Gdi` has no effect in either application.

### /FCT

Select FullControlTest in `RemotingTest_Core`. This is also the default if no application selector is supplied. `/FCT`, `/RPT` and `/RVMT` are mutually exclusive and cannot be repeated. Core requires one renderer transport: `/Pipe`, `/Http` or `/MiniHttp`. For example, `/FCT /MiniHttp /AsPort:8888` starts the showcase Core for a MiniHTTP renderer.

### /RPT

Select RemoteProtocolTest in `RemotingTest_Core`, together with one renderer transport. It is mutually exclusive with `/FCT` and `/RVMT`.

### /RVMT

Select RemoteViewModelTest in `RemotingTest_Core`, together with one renderer transport. It is mutually exclusive with `/FCT` and `/RPT`. The view-model host connection is independent of the renderer connection: use the same network transport for both, or add `/Cli:<path>` for an automatically launched stdio host.

Without `/Cli`, start Core first, then `RemotingTest_RvmHost` with the same transport selector, then the renderer after Core automation contains `Remote View Model Test`. RVM RPC uses the exact logical channels `ViewModelChannel` and `ViewModelReadyChannel`; renderers use `GacUIRemoteProtocol`. A requester terminates with an error if its view-model host disconnects while the application is running.

### /Pipe

Select the Windows named-pipe transport. Core and the renderer each require exactly one of `/Pipe`, `/Http` or `/MiniHttp`. `CppTest_Rvm` and `RemotingTest_RvmHost` also accept a network selector, with `/Cli` as an alternative described below. A network host must use the same transport as its requester. Named-pipe access is subject to Windows permissions and SMB availability; destinations may be UNC server names or IPv4 addresses, but not raw IPv6.

### /Http

Select the Windows HTTP transport on protocol port `8888`, exclusively with respect to the other transport selectors. The server registers `http://+:8888/GacUIRemoteProtocolHttp/` and requires the corresponding HTTP.sys URL-registration permission. Use `/MiniHttp` for a portable HTTP transport.

### /MiniHttp

Select the portable socket-based HTTP transport on protocol port `8888`, exclusively with respect to the other network transport selectors. It works on Windows, Linux and macOS, including the wGac/iGac renderer replacements. Native TCP listeners use wildcard interfaces and supported address families; same-machine hostname/IP tests do not establish access from another computer. Automation socket sharing is described under `/AsPort`.

### /Cli

Use `/Cli:<nonempty-host-path>` in a requester to launch `RemotingTest_RvmHost` automatically with the exact `/Cli` switch. Do not start that host manually.

- `RemotingTest_Core` requires explicit `/RVMT` and one renderer transport in addition to `/Cli:<path>`. It starts a renderer-only network server and a host-only stdio server, then stops the host server before the renderer server.
- `CppTest_Rvm` accepts exactly one of `/Pipe`, `/Http`, `/MiniHttp` or `/Cli:<path>`. Its network modes wait for a manually started host. Here `/Cli` is itself the exclusive RVM transport; this application never uses a separate renderer.
- `RemotingTest_RvmHost` accepts bare `/Cli` instead of a network selector. Stdin/stdout are reserved for framed protocol traffic and the ordinary startup banner is suppressed. Explicit `/corehost` is invalid in this local mode.

A process launcher may pass `/Cli:<path>` or the single literal argument `/Cli:"<path>"`. Requesters remove exactly one balanced quote pair and reject empty, duplicate or unmatched quoted values. The path names one executable, never a command plus arguments. Linux/macOS stdio support comes from the imported `<GacUI repo>/Import/VlppOS.Linux.cpp`.

### /corehost

Use `/corehost:<host>` once in a native renderer or network `RemotingTest_RvmHost` to select the destination, defaulting to `localhost`. Place it before or after the transport selector; renderer arguments also compose with automation and injection options. Core and `CppTest_Rvm` host the servers and do not accept this option.

Supply an ASCII DNS hostname, IPv4 address or IPv6 address, without a URL, path or separate port. Empty, duplicate and malformed values fail before transports start. IPv6 colons remain part of the host; one surrounding bracket pair is accepted and removed before socket construction, then restored for HTTP authorities. `/Pipe` does not support IPv6.

The destination does not change protocol port `8888` or the local automation port. For example, run Core with `/FCT /MiniHttp /AsPort:8888`, then a Windows renderer with `/MiniHttp /corehost:::1 /AsPort:8889`. A network view-model host can use `/MiniHttp /corehost:core.example.com` to connect to Core or `CppTest_Rvm`. The wGac/iGac renderers support `/corehost` too; see their launcher and port spellings under [Linux/macOS Specific](#linuxmacos-specific).

### /remoteDT

Enable renderer-provided DateTime services by passing `/remoteDT` once to both Core and the renderer. The test apps default to disabled and reject duplicate switches. If the connected renderer does not provide the capability, DateTime uses the predecessor service; capabilities are reevaluated when the renderer is replaced.

### /remoteLocale

Enable renderer-provided Locale services by passing `/remoteLocale` once to both Core and the renderer. It is independently disabled by default and rejects duplicates. If the renderer cannot provide the capability, Locale uses the predecessor service; capabilities are reevaluated on renderer replacement.

### /remoteFS

Enable renderer-provided filesystem services by passing `/remoteFS` once to both Core and the renderer. It is independently disabled by default and rejects duplicates. An enabled but unavailable remote filesystem exposes an empty `/` root. Remote image encoding is available independently of filesystem injection. All three injection arguments also work with the wGac/iGac renderers.

### Maintenance expectations

Test apps include `CppTest*`, `GacUI_Host` and `RemotingTest*`; they are demos and do not require production-level quality. Let unexpected exceptions crash the app so failures in `<GacUI repo>/Source` remain visible. Keep test apps simple without unnecessary recovery.

Remote-view-model requesters finish normal shutdown inside `GuiMain`: destroy the window, release service proxies and stop the RPC server while the host transport is available. `GuiApplicationInitialize` subsequently finalizes global storage, including the shared MiniHTTP listener. Keep unexpected host loss fatal as specified by the debugging SOP.

`<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md` defines their expected connection/disconnection behavior. Test apps running FullControlTest should follow its "Complete Control Showcase (`/FCT`)" section; apps running TuiControlTest should follow `<GacUI repo>/.github/Jobs/DebugTuiControlTestSop.md`.

## Windows Specific

### UI Automation verification and maintenance

Whenever `Test/Resources/App/FullControlTest` changes, update and run `Test/UIA_CppTest.ps1` and `Test/UIA_CppTest_Metaonly.ps1` so the showcase and its UIA expectations stay synchronized. The entry wrappers share launch/cleanup logic in `Test/UIA_CppTest_Shared.ps1` and assertions in `Test/UIA_CppTest_Shared.cs`. Add targeted role, name, pattern, state and bounds assertions for new behavior; generic tab traversal alone is insufficient. The Easy Layout checks cover its five mixed docking controls, selection/toggling, equal outer heights, resizing, rebuilding and palette refresh.

Both entry wrappers accept `-Scenario ResponsiveWindow` for focused ribbon-document resizing, responsive view/stack/group levels, and Easy Layout responsive modal checks. These include repeated row-height contraction, resizing while scrolled near the end, selection and reopening. This scenario omits palette cycles and unrelated page navigation. Use `-SkipBuild` after a successful build of the current source.


Both showcase UIA wrappers also accept `-Scenario DocumentStyles`. This focused scenario checks actual style-popup controls in wide and collapsed ribbons, both groups and all eight styles, repeated application of the selected item, reopening/resizing, Default/Aurora theme refresh, preserved selection and effective document formatting, and private save/reload with the complete style catalog. Use `-SkipBuild` only after building the current source. The gallery compatibility accessor returns the inline list and is not evidence of popup behavior.

Use Windows with the UIAutomationClient/UIAutomationTypes assemblies, an out-of-process MTA client and rebuilt Debug x64 binaries. The wrappers build and launch fresh processes, preserve project user arguments and shut down their owned processes. `Test/UIA_CppTest.ps1 -AsPort 8888` exercises hosted logical windows sharing one HWND; `Test/UIA_CppTest_Metaonly.ps1 -AsPort 8890` exercises ordinary native windows. Inspect owned HWNDs with EnumWindows when MainWindowHandle is unavailable, and check for native runtime-error dialogs after timeouts. Desktop focus and multi-monitor DPI checks require a suitable interactive session; record unavailable coverage explicitly.

`Tools/UiaList` is the native UIA inspector. Read `Tools/UiaList/AGENTS.md` and `Tools/UiaList/Verification/README.md`, build its separate solution with the repository wrapper, and use `Tools/UiaList/Verification/Inspector.ps1` for geometry-based inspection. Its automation endpoint is Debug-only: run with `/AsPort:8891` and use `http://localhost:8891/Automation/UiaListApp/Controls` and `/IO` beside CppTest on 8888 and CppTest_Metaonly on 8890. The verification fixture and `Smoke.ps1 -GetterRegression` check explicit refresh, cached presentation, edits/readback and asynchronous generations. Use UiaList's preview and Nodes view to review showcase changes; preview clicks select cached nodes without sending target input.

### Test applications and automation

Windows test applications include:
- `CppTest`                       : Run FullControlTest in hosted mode, built without reflection (`VCZH_DEBUG_NO_REFLECTION`).
- `CppTest_Tui`                   : Run TuiControlTest in Windows Terminal using TuiSkin and the hosted TUI provider, built without reflection. It has no automation endpoint. Follow [DebugTuiControlTestSop.md](.github/Jobs/DebugTuiControlTestSop.md).
- `CppTest_Rvm`                   : Run RemoteViewModelTest in hosted mode after acquiring its service from `RemotingTest_RvmHost`.
- `CppTest_Metaonly`              : Run FullControlTest, built with metaonly reflection (`VCZH_DEBUG_METAONLY_REFLECTION`).
- `CppTest_Reflection`            : Run FullControlTest, built with full reflection.
- `GacUI_Host`                    : Run FullControlTest, by loading Workflow binary assembly instead of generated C++ code.
- `Playground`                    : Run `REPO-ROOT/Test/GacUISrc/Playground/Resources/Resource*.xml`, resource file to load specified, main window specified in `OpenMainWindow` function.
- `RemotingTest_Core`             : Run FullControlTest, RemoteProtocolTest, or RemoteViewModelTest with a separate renderer.
- `RemotingTest_Rendering_Win32`  : Renderer of `RemotingTest_Core`.
- `RemotingTest_RvmHost`          : Provide the remote view-model service used by `CppTest_Rvm` and Core's RemoteViewModelTest.

After building Debug x64, use the launchers under `<GacUI repo>/Test`:

- `<GacUI repo>/Test/StartCore.ps1 -App cpptest_rvm|fct|rpt|rvmt -Protocol http|pipe|minihttp [-Cli] [-AsPort <1-65535>]` starts `CppTest_Rvm` or `RemotingTest_Core`. `-Cli` is valid only for `cpptest_rvm` and `rvmt`; without it, those two app modes start their required `RemotingTest_RvmHost` manually one second after the requester/Core. The automation port defaults to 8888.
- `<GacUI repo>/Test/StartRendrerer.ps1 -Protocol http|pipe|minihttp [-CoreHost <host>] [-AsPort <1-65535>]` starts `RemotingTest_Rendering_Win32` with automation port 8889 by default, allowing it to run beside Core. The file name intentionally preserves the requested `Rendrerer` spelling.
- Both launchers use `<GacUI repo>/Test/GacUISrc/x64/Debug` and return their `System.Diagnostics.Process` objects for inspection or cleanup. The renderer launcher starts without opening a console window.

FullControlTest means `Generated_FullControlTest.vcxitems`, generated from `REPO-ROOT/Test/Resources/App/FullControlTest/Resource.xml`.
TuiControlTest means `Generated_TuiControlTest.vcxitems`, generated from `REPO-ROOT/Test/Resources/App/TuiControlTest/Resource.xml`. Its skin is `Generated_TuiSkin.vcxitems`, which compiles the canonical, manually maintained `Source/Skins/TuiSkin/Config/TuiSkinConfig.h/.cpp` exactly once for each consumer. GacUI_Compiler also generates `Source/Utilities/FakeServices/TuiDialogs/Source`; run the same metadata generation steps after these resource changes. The release pipeline mirrors TuiSkin XML into `Source/Skins/TuiSkin`, generates its `Source` folder, preserves its canonical `Config` folder, and publishes `TuiSkin`/`TuiSkinReflection`. Configuration must not be recreated under `Generated_TuiSkin` or the release-generated `Source` folder.
RemoteProtocolTest means `Generated_RemoteProtocolTest.vcxitems`, generated from `REPO-ROOT/Test/Resources/App/RemoteProtocolTest/Resource.xml`.
RemoteViewModelTest means `Generated_RemoteViewModelTest.vcxitems`, generated from `REPO-ROOT/Test/Resources/App/RemoteViewModelTest/Resource.xml`.
When `FakeDialogService` is used, all system dialogs are replaced by `REPO-ROOT/Source/Utilities/FakeServices/Dialogs/Resource.xml`.

See [Maintaining Test Apps](#maintaining-test-apps) for argument usage, automation URLs and transport selection.

Each application owns its automation stack directly. It constructs the concrete service matching its setup (`WindowsAutomationService`, `WindowsAutomationServiceHosted`, `WindowsAutomationServiceRenderer`, `RemoteProtocolAutomationService`, or a platform renderer service), substitutes it, starts the selected Windows HTTP or MiniHTTP endpoint, runs the application, then stops the endpoint and service before unsubstituting it. Both endpoint implementations expose the same `Controls`, `Dom`, and `IO` contract.

Both `RemotingTest_Core` and `RemotingTest_Rendering_Win32` support automation IO operations:

- When performing IO via the renderer, remote protocol events pass the IO operations to the core.
- When performing IO via the core, the renderer only receives UI updates and redraws.
- Core and renderer should synchronize to the same UI state afterwards.
- Performing IO through either the renderer or the core should result in the same UI state.

`Playground` is for adhoc testing:
- Playground is for trying UI from XML. All UI, including controls, layout, document content and interaction behavior, must be implemented in XML resources and their Workflow scripts, not constructed in C++. C++ may provide native interop hooks that the scripts call.
- The UI in resource file, including `GuiMain` and `OpenMainWindow`, could be modified freely without any concern, it is not part of the release. DO NOT revert `Playground` change as I can also use it for manual verification.
- Actual resource files to load is specified in `GuiMain`. Loading one candidate at a time by editing this function is sufficient; no command-line fixture selector is required.
- Actual theme type and main window type is specified in `OpenMainWindow`.
- All candidate resource files to load are supposed to put in the same folder, and add to the same solution explorer folder in `Playground` project.

## Linux/macOS Specific

`<GacUI repo>/Test/Linux` stores Linux configurations for:
- `Metadata_Generate`: `<GacUI repo>/Test/Linux/Metadata_Generate/vmake`.
- `Metadata_Test`: `<GacUI repo>/Test/Linux/Metadata_Test/vmake`.
- `GacUI_Compiler`: `<GacUI repo>/Test/Linux/GacUI_Compiler/vmake`.
- `RemotingTest_Core`: `<GacUI repo>/Test/Linux/RemotingTest_Core/vmake`.
- `RemotingTest_RvmHost`: `<GacUI repo>/Test/Linux/RemotingTest_RvmHost/vmake`.
- `UnitTest`: `<GacUI repo>/Test/Linux/UnitTest/vmake`.
- `WasmFCT`: `<GacUI repo>/Test/Linux/WasmFCT/vmake`.
- `WasmRPT`: `<GacUI repo>/Test/Linux/WasmRPT/vmake`.
- `WasmRVMT`: `<GacUI repo>/Test/Linux/WasmRVMT/vmake`.

`<GacUI repo>/Tools` stores Linux configurations for:
- `GacBuild`: `<GacUI repo>/Tools/GacBuild/vmake`.
- `GacGen`: `<GacUI repo>/Tools/GacGen/vmake`.
- `GitTui`: `<GacUI repo>/Tools/GitView/Linux/GitTui/vmake`.
- `GitViewTests`: `<GacUI repo>/Tools/GitView/Linux/GitViewTests/vmake`.

`Metadata_UpdateProtocol` is not included. If it is needed, create it and remove this line.

You need to build, test and debug in that specific folder, otherwise the unit test will not function properly.
On Linux, only configuration "debug x64" is available, no need to build or run projects with other configurations.
Unlike Windows, building have to be done in each folder separately.

The Unix unit-test entry point uses the environment locale for wide console output, including Unicode test names. Run it with an installed UTF-8 locale; on macOS, use `LC_ALL=en_US.UTF-8 ./Bin/UnitTest /C` if the inherited locale is unavailable.

The portable `RemotingTest_Core` entry point initializes `LC_CTYPE` from the environment before using filesystem services. Use an installed UTF-8 locale for Core-local Unicode filenames, including when remote filesystem injection is disabled. This does not change the native en-US date/number formatting implementation.

It is expected if `makefile` and `vmake.txt` is changed due to build any project, they should be included in the commit for manually updated files, do not revert them.

### Replacement test applications in wGac and iGac

The native GUI projects in this solution use Windows providers. wGac supplies Wayland replacements and iGac supplies macOS replacements; their supported launcher arguments are documented in `<wGac repo>/test.sh` and `<iGac repo>/test.sh`.

| Application or role in this solution | wGac replacement | iGac replacement | Arguments to the repository's test.sh |
| --- | --- | --- | --- |
| Hosted FullControlTest (`CppTest`) | `Test_FullControlTest` | `Test_FullControlTest` | `--app:fct --hosted` |
| FullControlTest with native windows | `Test_FullControlTest` | `Test_FullControlTest` | `--app:fct` |
| `CppTest_Tui` | `Test_TuiControlTest` | `Test_TuiControlTest` | `--app:tui` |
| `CppTest_Rvm` | `Test_CppTest_Rvm` | `Test_CppTest_Rvm` | `--app:rvmt` |
| `RemotingTest_Rendering_Win32` | `RemotingTest_Rendering_Wayland` | `RemotingTest_Rendering_macOS` | `--app:renderer` |

These replacement test apps are built without reflection; they do not provide separate metaonly, full-reflection or Workflow-binary variants corresponding to `CppTest_Metaonly`, `CppTest_Reflection` and `GacUI_Host`. `RemotingTest_Core` and `RemotingTest_RvmHost` use the portable projects under `<GacUI repo>/Test/Linux`.

For a renderer, both test launchers accept `--corehost:<host>`, `--port:<1-65535>`, `-remoteDT`, `-remoteLocale` and `-remoteFS`. They forward `/MiniHttp`, `/corehost:<host>`, `/port:<port>` and the corresponding slash injection switches in foreground and `--unblock` modes. The renderer's local automation option is `/port`, default `8889`; Core continues to use `/AsPort`. `/corehost` is supported by these renderers and the portable network view-model host, so it is not Windows-specific. Shared argument semantics are described under [Maintaining Test Apps](#maintaining-test-apps).

`<wGac repo>/test_core.sh` and `<iGac repo>/test_core.sh` launch the portable Core with `--app:fct`, `--app:rpt` or `--app:rvmt` and `--protocol:minihttp`. Their `-remoteDT`, `-remoteLocale` and `-remoteFS` options forward the matching slash switches to Core. For a stdio view-model host, use `--app:rvmt --protocol:minihttp --cli`.

The `Test_CppTest_Rvm` replacements accept only `/MiniHttp`; their test launcher does not automatically start a host. Use the corresponding test-core launcher with `--app:cpptest_rvm --protocol:minihttp` to launch the requester and its network host together. That mode does not accept `--cli`. The portable Core's RemoteViewModelTest remains available for stdio-host testing.

### Windows-only arguments

The following arguments from the test-app table do not work on Linux/macOS:

- `/Gdi`: selects the Windows GDI drawing backend.
- `/Pipe`: selects the Windows named-pipe transport.
- `/Http`: selects the Windows HTTP transport, including HTTP.sys on the server.

Use `/MiniHttp` for network transport on Linux/macOS. The application-specific differences in the replacement projects are listed above; `/AsPort`, `/Cli`, `/corehost` and the remote-injection switches are not inherently Windows-only.

## WebAssembly

The Linux-only browser demos `Test/Linux/WasmFCT`, `WasmRPT` and `WasmRVMT` run FullControlTest, RemoteProtocolTest and RemoteViewModelTest. Build each from its directory with `../../../.github/Ubuntu/build.sh -bw -o`. They use `VCZH_DEBUG_NO_REFLECTION`, the merged DarkSkin sources in `Source/Skins/DarkSkin/Source`, the demos' generated x86 sources and `-O3`. Shared composition lives in `Test/Linux/Wasm`; the test-only transport lives in `Test/RemotingHelpers/RemotingServer/Wasm` and is not added to MSBuild projects.

Each demo produces `Bin/app.mjs`, `Bin/app.wasm` and `Bin/app.worker.js`. These are required by GacJS but remain ignored build artifacts; build them when missing. Keep the minimal `vbuild` Wasm opt-in. Packaging preserves Emscripten's generated module unchanged; its default factory loads the adjacent pthread worker, which imports the adjacent module. GacJS's `wasm-worker.js` hosts the application and is distinct from this generated `app.worker.js`. After building GacJS, run its `copy-wasm.sh` explicitly and start the entry website. `/wasm-fct/`, `/wasm-rpt/` and `/wasm-rvmt/` own independent worker/module lifetimes; RVMT hosts its TypeScript view model in the page. Follow [.github/Jobs/job.rpWasm.prompt.md](.github/Jobs/job.rpWasm.prompt.md) for the build and browser verification procedure.

`StartApplication` installs the fixed transport connections during startup; `SendDataToWasmCore` carries their channel traffic. These are the only application Embind exports. After normal Core shutdown, Reload becomes enabled and reloads the page, creating a fresh Core, renderer and view-model host with no retained application state. Reload stays disabled during startup, operation, canceled Exit and fatal errors.

The same three projects also support `../../../.github/Ubuntu/build.sh -f -o` as native CLI compile/link checks. Their `VCZH_GCC` entry point returns 0 without starting GacUI. The full source inventory is linked, using x64 demo sources for native builds and x86 for Wasm, while both use the merged DarkSkin sources. Native full builds clean `Bin`; rebuild with `-bw -o` before copying the Wasm outputs to GacJS.

The following unit test projects could be built to web assembly and run with a browser:
- `REPO-ROOT/Test/Linux/UnitTest`

The browser build selects the generated 32-bit skins and compiler baselines and uses the shared Ubuntu tools' `-O3` setting for compilation and linking. Its `vbuild` preloads test inputs into OPFS and creates an empty `Resources/UnitTestSnapshots` output folder; existing host snapshots are not input fixtures. Debug information is stored in `app.debug.wasm`, with a relative reference in `app.wasm` for automatic discovery. Exceptions and assertions remain enabled. Native builds default to `-O0`; add `-o` or `--optimize` to select `-O2` for native unit tests.

Keep XML fixtures linked from `<GacUI repo>/Test/Resources/App` in `<GacUI repo>/Test/Linux/UnitTest/vbuild`, including the two FullControlTest touching fixtures used by the point-scrolling tests.

The full browser suite writes hundreds of megabytes of snapshots. For automated Firefox runs, use a fresh regular profile (`firefox.launchPersistentContext`) with sufficient OPFS quota; isolated contexts can exhaust their smaller storage allowance before the suite finishes.

The terminal provider and test cases in TestTuiProvider.cpp and TestControls_EasyLayout.cpp are native-only and excluded from WebAssembly. The shared XML fixture helper remains available to resource tests.
