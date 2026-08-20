# Project Specific Instruction

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
- `REPO-ROOT/Test/GacUISrc/Generated_Dialogs`
- `REPO-ROOT/Test/GacUISrc/Generated_FullControlTest`
- `REPO-ROOT/Test/GacUISrc/Generated_RemoteProtocolTest`
- `REPO-ROOT/Test/GacUISrc/Generated_RemoteViewModelTest`
- `REPO-ROOT/Test/GacUISrc/Generated_UnitTestViewer`
- `REPO-ROOT/Source/Utilities/FakeServices/Dialogs/Source`
- `REPO-ROOT/Source/UnitTestUtilities/SnapshotViewer/Source`
- `REPO-ROOT/Source/Compiler/InstanceQuery/Generated`
- `REPO-ROOT/Source/Compiler/RemoteProtocol/Generated`

Files in `REPO-ROOT/Import` and `REPO-ROOT/Release` (recursively) are also not allowed to modify.
These files are prepared for foreign dependencies.

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
- They are excluded from ordinary public GacUI amalgamations and the aggregate `Release` repository. GacUI CodePack emits dedicated neutral, Windows, and Linux pairs (`Test.RemotingHelpers*`) in `Release` and `Release/IncludeOnly` only for platform repositories' `Import-Test` snapshots.
- No production quality required, these files are only for building test apps quickly.

### Test/RemotingHelpers/Rvmt

Files in this folder are generic client and requester helpers used only by `CppTest_Rvm`, `RemotingTest_Core`, and `RemotingTest_RvmHost`.
- They are enumerated and compiled through the single `Test/GacUISrc/Source_RemotingHelpers/Source_RemotingHelpers.vcxitems` inventory shared by all helper consumers.
- `ViewModelShared.h` owns generic aliases, fixed RVM channel/control constants, and inline Ready-message helpers; `ViewModelHostClient.*` owns the generic network-side host client; and `ViewModelHostServer.*` owns the specialized channel server's generic RPC helpers and server-side local-client behavior.
- Generated RemoteViewModelTest RPC composition belongs to `Test/GacUISrc/Generated_RemoteViewModelTest/RemoteViewModelTestInitialize.*`, which each affected application invokes after its generic connection is assigned a client ID.
- `Release/CodegenConfig.xml` scans this helper tree only into the dedicated test pairs. Ordinary `GacUI*` pairs and the aggregate `Release` repository remain independent of these helpers.

### Test/RemotingHelpers/StdioRedirection

This folder implements the test-only stdio transport behind the platform-neutral `StdioRedirectionServer`, `StdioRedirectionClient`, and `StdioRedirectionConnection` names.
- Shared code owns callback-safe connection lifecycle and one-line UTF-8/Base64 framing. Exact `!Exit` is the only raw control line; ordinary serialized channel packages remain opaque `WString` messages.
- `StdioRedirection.Windows.cpp` owns Windows process and anonymous-pipe work. `StdioRedirection.Linux.cpp` owns the shared Linux/macOS `fork`/pipe implementation.
- The server launches no process in `Start`. Each `ConnectNewClient` call launches and owns one independent child; `Stop` sends `!Exit`, drains callbacks and readers, and reaps every child.
- The unconditional `Source_RemotingHelpers.vcxitems` inventory lists every platform translation unit. Portable `vmake` inputs remove only the Windows implementation.

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
- Update `REPO-ROOT/Test/Resources/DarkSkin`.
- Run `GacUI_Compiler` and make sure it updated generated C++ code expectely.
  - Sometimes reordering could happen in generated C++ code even when correlated resource is not changed.
- Rebuild before running any test project.

## Debugging Remote Protocol Issues

Use `REPO-ROOT/DebugRemoteProtocolSop.md` for the shared end-to-end UI operations and
observable results.

Remote protocol is involved in three ways:
- Core with native renderer: `REPO-ROOT/DebugRemoteProtocolWithNativeRenderer.md`.
- Core with `GacJS`: `REPO-ROOT/DebugRemoteProtocolWithGacJS.md`.
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

Test apps means `CppTest*` and `RemotingTest*`, they are demos and do not require production level quality.
No need to gracefully handle any exception, actually we need them to just crash when anything unexpected thing happens, that's how we know anything in `REPO-ROOT/Source` is going wrong.
`REPO-ROOT/DebugRemoteProtocolSop.md` defines the expected behavior of these test apps apon connection/disconnection.
Keep test apps simple without introducing unnecessary "gracefully recovering".

## Windows Specific

Automation HTTP service for GUI applications are available for Windows:
- `CppTest`                       : Run FullControlTest in hosted mode, built without reflection (`VCZH_DEBUG_NO_REFLECTION`).
- `CppTest_Rvm`                   : Run RemoteViewModelTest in hosted mode after acquiring its service from `RemotingTest_RvmHost`; `/Cli:<path>` auto-launches that host with `/Cli`.
- `CppTest_Metaonly`              : Run FullControlTest, built with metaonly reflection (`VCZH_DEBUG_METAONLY_REFLECTION`).
- `CppTest_Reflection`            : Run FullControlTest, built with full reflection.
- `GacUI_Host`                    : Run FullControlTest, by loading Workflow binary assembly instead of generated C++ code.
- `Playground`                    : Run `REPO-ROOT/Test/GacUISrc/Playground/Resources/Resource*.xml`, resource file to load specified, main window specified in `OpenMainWindow` function.
- `RemotingTest_Core`             : Run FullControlTest, RemoteProtocolTest, or RemoteViewModelTest with renderer traffic hosted by HTTP, MiniHTTP, or NamedPipe; `/RVMT` may independently use `/Cli:<path>` for its host.
- `RemotingTest_Rendering_Win32`  : Renderer of `RemotingTest_Core`.
- `RemotingTest_RvmHost`          : Provide the remote view-model service used by `CppTest_Rvm` and `RemotingTest_Core /RVMT`, including the stdio `/Cli` mode.

FullControlTest means `Generated_FullControlTest.vcxitems`, generated from `REPO-ROOT/Test/Resources/App/FullControlTest/Resource.xml`.
RemoteProtocolTest means `Generated_RemoteProtocolTest.vcxitems`, generated from `REPO-ROOT/Test/Resources/App/RemoteProtocolTest/Resource.xml`.
RemoteViewModelTest means `Generated_RemoteViewModelTest.vcxitems`, generated from `REPO-ROOT/Test/Resources/App/RemoteViewModelTest/Resource.xml`.
When `FakeDialogService` is used, all system dialogs are replaced by `REPO-ROOT/Source/Utilities/FakeServices/Dialogs/Resource.xml`.
For the non-remoting projects above, the automation endpoint is `http://localhost:8888/Automation/<PROJECT-NAME>/...` and is hosted by `StartWindowsHttpAutomationService`.
- Checkout `REPO-ROOT/.github/Guidelines/Running-GacUI.md` for details.

Each application owns its automation stack directly. It constructs the concrete service matching its setup (`WindowsAutomationService`, `WindowsAutomationServiceHosted`, `WindowsAutomationServiceRenderer`, `RemoteProtocolAutomationService`, or a platform renderer service), substitutes it, starts the selected Windows HTTP or MiniHTTP endpoint, runs the application, then stops the endpoint and service before unsubstituting it. Both endpoint implementations expose the same `Controls`, `Dom`, and `IO` contract.

Both `RemotingTest_Core` and `RemotingTest_Rendering_Win32` expose automation in `/Http`, `/Pipe`, and `/MiniHttp` modes:
- `RemotingTest_Core` exposes the UI as a window-control tree at `http://localhost:8888/Automation/RemotingTest_Core/...`.
- `RemotingTest_Rendering_Win32` exposes the UI as a DOM tree at `http://localhost:<renderer-port>/Automation/RemotingTest_Rendering_Native/...`. Pass `/port:<renderer-port>` to select the automation port; omitting it keeps the default port `8889`.
- `/Http` and `/Pipe` use `StartWindowsHttpAutomationService`.
- In `/MiniHttp` mode, the core registers its automation prefix with the exact same `IAsyncSocketServer` that hosts the remote protocol on port `8888`. The renderer is a separate process, so it hosts its automation prefix with a separate MiniHTTP socket server on the selected renderer automation port (default `8889`).
- Both support IO operations:
  - When performing IO via the renderer, remote protocol events pass the IO operations to the core.
  - When performing IO via the core, the renderer only receives UI updates and redraws.
  - Core and renderer should synchronize to the same UI state afterwards.
  - Performing IO through either the renderer or the core should result in the same UI state.

RVM RPC uses the exact logical channels `ViewModelChannel` and `ViewModelReadyChannel`; renderers use `GacUIRemoteProtocol`. Core-to-renderer transport and Core-to-host mode are separate dimensions:

- Without `/Cli`, `RemotingTest_Core /RVMT` and its manually started `RemotingTest_RvmHost` share the selected `/Pipe`, `/Http`, or `/MiniHttp` server. Start Core, then the host with the same selector, then start the renderer after Core automation contains `Remote View Model Test`.
- With `/Cli:<nonempty-host-path>`, Core still requires `/RVMT` and one renderer transport. It starts a renderer-only server plus a host-only stdio server, quotes the path, and auto-launches `RemotingTest_RvmHost /Cli`; do not start that host manually. Stop order is host server first, renderer server second.
- A process launcher may pass `/Cli:<path>` directly or pass the single literal argument `/Cli:"<path>"`. `RemotingTest_Core` and `CppTest_Rvm` remove exactly one balanced quote pair around the path and reject empty or unmatched quoted values; the path remains one executable, never a command plus arguments.
- `CppTest_Rvm` accepts exactly one of `/Pipe`, `/Http`, `/MiniHttp`, or `/Cli:<nonempty-host-path>`. The first three wait for a manually started host using the same selector. `/Cli` auto-launches the host and is itself the exclusive RVM transport. This variant never uses a renderer.
- `RemotingTest_RvmHost` accepts exact `/Cli` in addition to its network selectors. In stdio mode, stdin/stdout are reserved for framed protocol traffic and the ordinary startup banner is suppressed.
- A requester terminates with an error if `RemotingTest_RvmHost` disconnects while the application is running.
- `CppTest_Rvm` exposes automation at `http://localhost:8888/Automation/CppTest_Rvm/...`. `/Pipe`, `/Http`, and `/Cli` use the Windows HTTP endpoint; `/MiniHttp` registers MiniHTTP automation on the same port-8888 socket server that carries its RVM traffic.

`Playground` is for adhoc testing:
- The UI in resource file, including `GuiMain` and `OpenMainWindow`, could be modified freely without any concern, it is not part of the release. DO NOT revert `Playground` change as I can also use it for manual verification.
- Actual resource files to load is specified in `GuiMain`.
- Actual theme type and main window type is specified in `OpenMainWindow`.
- All candidate resource files to load are supposed to put in the same folder, and add to the same solution explorer folder in `Playground` project.

## Linux/macOS Specific

`REPO-ROOT/Test/Linux` stores linux configurations for:
- `Metadata_Generate`: `Metadata_Generate.vcxproj`.
- `Metadata_Test`: `Metadata_Test.vcxproj`.
- `CppTest`: `CppTest.vcxproj`.
- `CppTest_Metaonly`: `CppTest_Metaonly.vcxproj`.
- `CppTest_Reflection`: `CppTest_Reflection.vcxproj`.
- `GacUI_Compiler`: `GacUI_Compiler.vcxproj`.
- `RemotingTest_Core`: `RemotingTest_Core.vcxproj`.
- `RemotingTest_RvmHost`: `RemotingTest_RvmHost/vmake`.
- `UnitTest`: `UnitTest.vcxproj`.

`Metadata_UpdateProtocol` is not included. If it is needed, create it and remove this line.

You need to build, test and debug in that specific folder, otherwise the unit test will not function properly.
On Linux, only configuration "debug x64" is available, no need to build or run projects with other configurations.
Unlike Windows, building have to be done in each folder separately.

`CppTest_Rvm` is Windows-only in this solution. The portable RVM demo is `RemotingTest_Core /RVMT` with `/MiniHttp` for renderers; its host is either manually started with `/MiniHttp` or auto-launched with `/Cli:<path>`. Linux/macOS stdio code is shared by `StdioRedirection.Linux.cpp`.
