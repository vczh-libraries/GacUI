# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

# Extract Shared Remoting Test Helpers

This document records the third round of review comments for the remote
view-model and remote-protocol test applications. It is an implementation plan
for extracting reusable, test-only code into `Source/RemotingHelpers`.

The intended result is that a future remoting test application can provide its
generated XML/RPC artifacts, a few application-specific callbacks, and a small
`Main`/`GuiMain` composition layer without copying transport, channel,
automation, renderer-terminal, or lifetime boilerplate.

This is a planning document. No implementation or verification described below
has been performed while preparing it.

# Recorded Review Requirements

The following block is preserved as the review input for this refinement:

```text
- Multiple files moved to `Source/RemotingHelpers`
  - Sub folders: `AutomationService`, `RemotingServer`, `RemotingClient`.
  - "Move here" below means move to `Source/RemotingHelpers`
  - Fix `CodegenConfig.xml`
- Move `(Start|Stop)WindowsAutomationService` here, including:
  - `vl::presentation::windows::WindowsAutomationService*`
  - `vl::presentation::windows::HttpAutomationService`
  - `(Start|Stop)WindowsHttpAutomationService`
- Move `(Start|Stop)MiniHttpAutomationService` here, including:
  - `MiniHttpAutomationService`
  - `(Start|Stop)MiniHttpAutomationService`
- `Test/GacUISrc/(CppTest_Rvm|RemotingTest_Core).GuiMain.cpp`:
  - Merge `*Remote(ViewModelUi|Channel)Server` and move here.
  - Simplify error handling code with RAII.
  - `CppTest_Rvm` is a GUI app, remove all `Console` reference.
  - Refactor `RemotingChannelServerBase` to make it generalized and move here. No need to refactor if it doesn't depend on any test apps generated code.
  - Refactor `RemoteViewModelUiServer` to make it generalized and move here. No need to refactor if it doesn't depend on any test apps generated code.
  - The above two might be able to merge into one, just configuring the need of accepting renderers.
- `Test/GacUISrc/RemotingTest_Renderer_Win32/Main.cpp`:
  - Refactor `RemotingTestChannelClient` to make it generalized and move herer. No need to refactor if it doesn't depend on any test apps generated code.
- `Test/GacUISrc/(CppTest_Rvm|RemotingTest_(Core|Renderer_Win32))/(Gui)?Main.cpp`:
  - `Start*(Server|Client)()` with an new enum to pass to the `Start(Server|Client)` callback so that the host knows which automation service to start, instead of using variables.
- Helper functions/classes to make remoting easier based on `IChannel(Server|Client)`.
- Fix `Test/Linux/(Cpp|Remoting)Test*` to make sure they work on both Linux and macOS
- After moving, perform another round to review `Source/RemotingHelpers`:
  - We are going to let any test apps and files in `Source/RemotingHelpers` to crash when exceptions, errors or unexpected things happen.
  - Necessary error recovery or shotdown acknowledgements should just be removed.
  - Limited error recovery is only allowed when `DebugRemoteProtocolSop.md` explicitly requires.
- The point is to let bugs immediately crash apps so that they are known without covering any root cause.
```

The editing scope should remain in `Source/RemotingHelpers` and `Test` as much
as possible. The helpers are test support; they are not part of the GacUI
library API.

The required CodePack outputs are exactly:

- `GacUI.RemotingHelpers.h`
- `GacUI.RemotingHelpers.cpp`
- `GacUI.RemotingHelpers.Windows.h`
- `GacUI.RemotingHelpers.Windows.cpp`

The owner's implementation intent is also part of the requirement:

- Prefer changes in `Source/RemotingHelpers` and `Test`.
- Extract shareable code so future test apps need only a few lines of C++ plus
  their XML resources.
- Keep the helpers test-purpose even though CodePack distributes them.
- Treat the supplied bullets as review comments on the three earlier task
  documents.
- Extend the scope only where a better extraction requires a small donor,
  project, CodePack, or documentation change.

# Execution Environment and Deferred Platform Work

This task will be implemented and verified on Windows. It must not claim that
the resulting changes build or run on Linux or macOS because neither platform
is available in this execution environment.

The recorded review requirement to fix `Test/Linux/(Cpp|Remoting)Test*` for
Linux and macOS is deferred to a separate task. That follow-up owns
`Test/Linux/**`, `vmake`, Linux builds, and builds on an actual macOS
environment. Those deferred results are not completion criteria for this
Windows task.

Portable layering remains a design goal for the neutral
`GacUI.RemotingHelpers` pair. Windows-only validation can enforce dependency
boundaries, but it is not evidence of Linux or macOS compatibility.

# Continuity with the Previous Work

This work follows `TODO_Task.md`, `TODO_RvmtRefine.md`, and
`TODO_RvmtRefine2.md`. Their implementation history matters because this
refinement must extract and simplify the current design, not restore machinery
that the previous refinement intentionally deleted.

## Stable baseline from `TODO_Task.md`

Commit `96d3f3a08cc0eef8d401d582ccd70a35843f0c7c` introduced the generated
Workflow RPC resource, `/RVMT`, `CppTest_Rvm`, `RemotingTest_RvmHost`, and the
shared physical transport with distinct logical channels.

Keep these baseline decisions:

- `CppTest_Rvm` and `RemotingTest_Core /RVMT` are requesters.
- `RemotingTest_RvmHost` provides `rvmt::IViewModel`.
- The broker and requester are separate local RPC endpoints.
- RVM and renderer traffic share one physical transport while using different
  logical channels.
- Generated resource/compiler behavior is already established and is not part
  of this refinement.

## Stable contract from `TODO_RvmtRefine.md`

Commit `4c28cc12b` implemented the clarified lifetime and renderer behavior.
Keep the contract:

- Renderer loss and renderer replacement are nonfatal to Core.
- Core loss is an ordinary renderer disconnection, not a renderer-generated
  fatal error.
- RVM-host loss is fatal to `CppTest_Rvm` and Core.
- When an active renderer exists, Core sends one Core-authored `!Error` for
  RVM-host loss before Core terminates with an error.
- There is no reverse HTTP `/Disconnect`, renderer-to-Core shutdown handshake,
  or renderer heartbeat.

## Fail-fast simplification from `TODO_RvmtRefine2.md`

Commit `f69211457` removed the recovery-heavy implementation and must remain
the starting point. It already removed:

- the third local RVM client;
- heartbeat and stopping acknowledgements;
- the reverse requester lease;
- heartbeat sequence state;
- the general role/terminal state machine;
- the retrying task queue;
- recovery callbacks and cleanup-error aggregation;
- renderer state from the RVM runtime; and
- `RemoteViewModelRoleState.h`.

Do not reintroduce these features while extracting helpers.

The following retained mechanisms are required behavior rather than optional
recovery:

- separate broker and requester local RPC endpoints;
- a post-route `Ready` proof before broker registration can be used safely;
- one-way host heartbeat plus a requester-side timeout for silent `/Http` and
  `/MiniHttp` host loss;
- one-way `RequesterStopping`;
- requester `stopping` classification before transport teardown;
- Core's ordered `!Error` delivery before fatal exit; and
- renderer startup and terminal-event ordering.

# Normalized Interpretation

- The canonical project name is `RemotingTest_Rendering_Win32`, not
  `RemotingTest_Renderer_Win32`.
- `RemotingTestChannelClient` is currently in
  `Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp`, not `Main.cpp`.
- The two server classes are currently in
  `Test/GacUISrc/CppTest_Rvm/GuiMain.cpp` and
  `Test/GacUISrc/RemotingTest_Core/GuiMain.cpp`.
- "herer" means "here", "with an new" means "with a new", and "shotdown"
  means "shutdown".
- "Necessary error recovery ... should be removed" is interpreted as
  "unnecessary error recovery ... should be removed", consistent with
  `Project.md` and the surrounding fail-fast requirements.
- There are no `StartWindowsAutomationService` or
  `StopWindowsAutomationService` functions on current `master`. They are an
  intentional new test-helper lifecycle facade, not existing symbols that can
  merely be moved.
- `Source/RemotingHelpers` may use GacUI and VlppOS interfaces, but it must not
  include anything under `Test`, any `Generated_*` folder, or a particular
  test application's generated header.
- CodePack combines these sources for distribution. This does not make the
  helpers part of the ordinary `GacUI` or `GacUI.Windows` library code pairs.
- All implementation and verification in this task is Windows-only. Linux and
  macOS project fixes and platform builds are separate follow-up work.

# Scope

## Primary implementation scope

- `Source/RemotingHelpers/**`
- `Test/GacUISrc/**`

## Narrow required exceptions

A true move requires small edits outside the preferred scope:

- Remove the moved declarations and implementations from
  `Source/PlatformProviders/Windows/WinNativeWindow.h` and
  `Source/PlatformProviders/Windows/WinNativeWindow.cpp`.
- Remove automatic construction/substitution of the test automation service
  from `Source/PlatformProviders/Windows/GDI/WinGDIApplication.cpp` and
  `Source/PlatformProviders/Windows/Direct2D/WinDirect2DApplication.cpp`.
- Retain or add only the minimal low-level Windows native-window bridge needed
  by the moved test helper.
- Update `Release/CodegenConfig.xml` to produce the four explicitly requested
  CodePack files. This requested configuration and its generated CodePack
  outputs are the only intended `Release` changes; generated files must never
  be hand-edited.
- Update repository documentation only where moving the automation service
  makes an existing statement false.

No changes should be made to `Import`, generated GacUI resource folders, or
the Workflow/GacUI compiler for this refinement.

## Non-goals

- Do not move the helpers into the normal GacUI library code pairs or umbrella
  headers.
- Do not turn the test transport stack into a production networking API.
- Do not change remote-protocol wire semantics.
- Do not add graceful RPC cancellation, reconnection, retries, leases in the
  reverse direction, or shutdown acknowledgements.
- Do not reopen the completed RPC resource/code-generation work from
  `TODO_Task.md`.
- Do not modify `Test/Linux/**`, regenerate its build files, or perform Linux
  or macOS compatibility work in this task.
- Do not report a successful Windows build as proof that the helpers build or
  run on Linux or macOS.

# Current-State Findings

| Concern | Current location/state | Consequence |
| --- | --- | --- |
| Helper folder | `Source/RemotingHelpers` does not exist. | All shared test code is still embedded in library/provider or application files. |
| Windows automation | `WindowsAutomationServiceBase`, `WindowsAutomationService`, `WindowsAutomationServiceHosted`, `WindowsAutomationServiceRenderer`, private `HttpAutomationService`, and the Windows HTTP start/stop functions are in `WinNativeWindow.{h,cpp}`. | Test-only automation is compiled into `GacUI.Windows`. |
| Windows setup | GDI and Direct2D setup functions construct and substitute the automation service automatically. | Moving the classes without removing this dependency would make `GacUI.Windows` depend on test helpers. |
| MiniHTTP automation | `MiniHttpAutomationService` and its start/stop functions are in `RemotingTest_Core/Shared.cpp`, which is compiled by three projects. | Reuse is expressed through cross-project source inclusion and `extern` declarations. |
| Local RVM server | `RemoteViewModelUiServer<TServerBase>` plus three transport subclasses are in `CppTest_Rvm/GuiMain.cpp`. | It depends directly on `RemoteViewModelRequesterSession`. |
| Core server | `RemotingChannelServerBase<TServerBase>` plus three transport subclasses are in `RemotingTest_Core/GuiMain.cpp`. | It repeats transport/server composition and mixes renderer policy with the RVM session. |
| Server base types | Workflow's `JsonNetworkChannelServer<TServerBase>` and GacUI's `GuiRemoteProtocolNetworkChannelServer<TServerBase>` resolve to the same JSON `NetworkProtocolChannelServer` specialization. | The two server families can be one class with optional renderer support. |
| Renderer client | `RemotingTestChannelClient` is in the renderer `GuiMain.cpp` and has no generated-application dependency. | It can move mostly unchanged after hard-coded names/policy are configured. |
| RVM runtime | `RemoteViewModelTestRuntime.h` includes generated `RemoteViewModelTestRpc.h` and exposes `rvmt::IViewModel`. | It cannot move wholesale into `Source/RemotingHelpers`. |
| Startup selection | Cpp/Core/renderer use process globals such as `useWindowsHttpAutomationService` and a pointer to a stack-owned MiniHTTP socket pointer. | Transport wrappers communicate with `GuiMain` through fragile ambient state. |
| Error handling | Start/stop paths contain nested catches, cleanup-result handling, and duplicate stop calls. HTTP automation converts unexpected exceptions into 404 responses. | Bugs can be hidden and the same ownership policy is repeated. |
| `CppTest_Rvm` UI | `GuiMain.cpp`, `Main.Windows.cpp`, and `Main.Linux.cpp` use `vl::console::Console`. | A GUI application depends on a console for diagnostics and control flow. |
| CodePack | The broad `gacui` category currently captures all new files under `Source`. | The helper folder needs separate neutral and Windows categories. |

# Target Source and CodePack Layout

Use the three required top-level areas. Windows-only implementation files
should live in nested `Windows` folders and follow the `*.Windows.h` /
`*.Windows.cpp` naming convention so CodePack categorization is unambiguous.

```text
Source/RemotingHelpers/
  AutomationService/
    MiniHttpAutomationService.h
    MiniHttpAutomationService.cpp
    AutomationServiceHost.h
    AutomationServiceHost.cpp
    Windows/
      WindowsAutomationService.Windows.h
      WindowsAutomationService.Windows.cpp
  RemotingServer/
    RemotingChannelServer.h
    RemotingServer.h
    RemotingServer.cpp
    Windows/
      RemotingServer.Windows.h
      RemotingServer.Windows.cpp
  RemotingClient/
    RemotingChannelClient.h
    RemotingChannelClient.cpp
    Windows/
      RemotingClient.Windows.h
      RemotingClient.Windows.cpp
```

The final filenames may be reduced when a `.cpp` or Windows-specific transport
file is unnecessary, but the ownership boundaries must remain:

- `AutomationService`: service implementations, endpoint hosts, and
  start/stop/RAII ownership.
- `RemotingServer`: channel-server admission, optional renderer policy, server
  composition, and reusable RPC-side server helpers.
- `RemotingClient`: renderer channel client, client composition, and reusable
  RPC-side client helpers.

Do not create a root umbrella that unconditionally pulls Windows-only
dependencies into the neutral pair. Each Windows header should include its
neutral prerequisite explicitly.

## `Release/CodegenConfig.xml`

Update the configuration in this shape:

1. Exclude `\Source\RemotingHelpers\` from the existing `gacui` category.
2. Add a neutral helper category covering `Source/RemotingHelpers` while
   excluding the nested Windows folders.
3. Add a Windows helper category covering the nested Windows folders.
4. Add code pairs named exactly:
   - `GacUI.RemotingHelpers`
   - `GacUI.RemotingHelpers.Windows`
5. Keep the dependency one-way:
   - `GacUI.RemotingHelpers` may depend on `GacUI` and portable VlppOS.
   - `GacUI.RemotingHelpers.Windows` may depend on
     `GacUI.RemotingHelpers`, `GacUI.Windows`, and `VlppOS.Windows`.
   - `GacUI` and `GacUI.Windows` must not depend on either helper pair.

CodePack's category matching is substring-based rather than a general boolean
expression. Using explicit nested Windows paths avoids a broad `.Windows.`
pattern accidentally collecting unrelated files.

## Test solution wiring

- Enumerate every new source/header explicitly in affected `*.vcxproj` and
  `*.vcxproj.filters` files; do not use wildcards.
- A shared-items project is optional. Do not make every test target compile the
  entire helper group merely because CodePack combines it for distribution.
  `RemotingTest_RvmHost`, in particular, should compile only the neutral RPC
  and channel helper files it needs.
- Add new shared-items projects to `GacUISrc.sln` only if they materially reduce
  project wiring.
- Limit project wiring in this task to the Windows solution and MSBuild
  projects. The separate platform follow-up owns `vmake` changes and generated
  Unix build files.

# Target Design

## 1. Automation service ownership

Create a small lifecycle facade in `AutomationService`:

- `StartWindowsAutomationService` / `StopWindowsAutomationService` own
  substitution of the appropriate Windows automation service.
- `StartWindowsHttpAutomationService` /
  `StopWindowsHttpAutomationService` own the Windows HTTP endpoint.
- `StartMiniHttpAutomationService` /
  `StopMiniHttpAutomationService` own the portable MiniHTTP endpoint.
- A non-copyable RAII owner composes the appropriate start/stop calls for one
  GUI run.

The lifecycle facade needs explicit configuration for:

- ordinary native windows;
- hosted windows;
- a remote renderer and its `GuiRemoteRendererSingle`;
- application name;
- HTTP port; and
- the shared `IAsyncSocketServer` required by MiniHTTP.

RAII here is ownership, not recovery. Constructors start resources,
destructors stop resources, and neither catches nor suppresses failures. A
stop failure on normal exit is a test failure. A failure while unwinding is
allowed to terminate the process.

### Minimal Windows provider seam

`WindowsAutomationServiceBase::RunIOCommandInternal` currently accesses the
private `WindowsForm::listeners`, and window lookup accesses private
`WindowsController` state. Moving that code verbatim is therefore impossible.

Use a narrow provider bridge:

- Keep or add an exported low-level function in `WinNativeWindow.{h,cpp}` that
  runs one IO command for a validated `INativeWindow` while the provider owns
  access to its listener list.
- Resolve opaque window IDs by enumerating the public
  `GetAllCreatedWindows` result and using safe RTTI cross-casts, with
  `IWindowsForm::GetWindowHandle` / `GetWindowsFormFromHandle` available where
  needed. Preserve the existing externally observable window-ID encoding.
- Remove the friendship and private-state knowledge that existed only for
  `WindowsAutomationService*`.
- Do not leave HTTP hosting, automation-service policy, or test-app lifecycle
  in `GacUI.Windows`.

Remove automatic automation substitution from the GDI/Direct2D setup
functions. Each test application starts its helper-owned service from
`GuiMain`, after the native controller exists and before the automation service
is first requested.

### HTTP request behavior

Returning 404 for an unsupported verb, route, malformed IO suffix, invalid
content type, or disabled operation is normal endpoint behavior. It is not
error recovery.

Unexpected `Error`, `Exception`, parser failures, automation implementation
failures, or callback invariant failures must not be translated into 404.
They must reach a process-terminating boundary. If an OS, thread, or async
transport callback would swallow a thrown exception, terminate explicitly at
that boundary instead of attempting to unwind across it.

Remove the MiniHTTP destructor/explicit-stop duplication. One owner performs
one stop. Likewise, `INativeAutomationService::Stop` should disable the
service; the endpoint owner should stop its HTTP listener instead of making
the service and the caller both stop the same process-global endpoint.

## 2. One generated-neutral server

Merge `RemoteViewModelUiServer<TServerBase>` and
`RemotingChannelServerBase<TServerBase>` into one generated-neutral
`RemotingChannelServer<TServerBase>`.

The merged type should:

- derive from the common JSON network channel-server specialization;
- accept local-client admission through an injected callback/policy;
- accept non-renderer remote channel roles through an injected
  callback/policy;
- notify an injected owner of disconnection;
- optionally enable `GacUIRemoteProtocol` renderer admission;
- keep the accepted renderer client ID when renderer support is enabled;
- perform the SOP-required renderer replacement/detach behavior; and
- expose only the core JSON/protocol binding needed for renderer delivery.

Configuration should express whether renderers are accepted. The local
`CppTest_Rvm` server disables renderer support; Core enables it after its own
RVM readiness callback permits admission.

The helper must not know:

- `rvmt::IViewModel`;
- `RemoteViewModelRequesterSession`;
- `ViewModelChannelName`;
- a generated header;
- which main window class Core constructs; or
- Core's fatal-state/UI globals.

`CppTest_Rvm` and Core adapt `RemoteViewModelRequesterSession` to the generic
admission/disconnection callbacks in a few lines.

Transport-specific constructors should remain thin. Use a helper only when it
removes repeated policy or lifetime code; do not hide a one-line
`NamedPipeServer`, `HttpServer`, or `SocketHttpServer` construction behind
several trivial wrappers.

## 3. Generalized renderer channel client

Move and rename `RemotingTestChannelClient` under `RemotingClient`. It already
has no generated-application dependency.

Parameterize only application-specific inputs:

- fatal dialog title;
- renderer/automation-service binding;
- main-thread task scheduling where platform setup differs; and
- the callback or action used for the final renderer transition.

Preserve the SOP-required distinction:

- Core-authored `!Error` enters fatal UI and may retain the renderer.
- Fatal local channel error after connection enters the ordinary disconnected
  state without a fatal prompt.
- `OnDisconnected` is an idempotent fallback.
- Pending terminal state is retained until the renderer, automation service,
  and main-thread invoker are ready.
- Renderer replacement settles the old renderer without killing Core.

These rules are the limited terminal handling explicitly required by
`DebugRemoteProtocolSop.md`; they must not be deleted by the fail-fast audit.

## 4. Helpers based on `IChannelServer` and `IChannelClient`

Build reusable composition at the channel layer, not around a particular
transport or generated application:

- helper to advertise and resolve named channels without repeating dummy
  channel maps;
- helper/local client for the RPC broker plus a separate requester endpoint;
- helper for explicit connect, assigned-client-ID validation, channel-reader
  initialization, and dispatcher initialization;
- generated-neutral JSON RPC lifecycle factory/template;
- optional one-way host liveness/control channel for the single mandatory
  service host; and
- small server/client context objects that keep callback targets alive until
  transport stop has completed.

The generated-neutral RPC helper should be parameterized by the generated
module object or factory. It may call the common generated method shape
(`rpc_GetIds`, JSON serializer/object operations, and wrapper factory) through
a template, but no helper source may name
`vl_workflow_global::RemoteViewModelTestRpc` or `rvmt::IViewModel`.

Application-specific code remains responsible for:

- selecting its generated RPC module;
- defining logical channel and service names;
- registering its concrete service implementation;
- requesting and casting its typed service proxy; and
- constructing its generated main window.

After the split, `RemoteViewModelTestRuntime.*` should contain only this thin
RVM adapter and policy constants. If a type remains generated-neutral after
the split, move it; do not retain it under `RemotingTest_RvmHost` merely
because that is its current location.

## 5. Explicit automation backend selection

Add an enum, for example:

```cpp
enum class RemotingAutomationService
{
    WindowsHttp,
    MiniHttp,
};
```

Pass it from every `StartNamedPipeServer`, `StartHttpServer`,
`StartMiniHttpServer`, `StartNamedPipeClient`, `StartHttpClient`, and
`StartMiniHttpClient` path into the generic `StartServer` / `StartClient`
composition callback.

Replace:

- `useWindowsHttpAutomationService`;
- `miniHttpAutomationSocketServer` as a pointer to a stack-owned `Ptr`; and
- equivalent ambient selector state.

Use one stack-owned host context containing the enum and, for MiniHTTP, a
strong `Ptr<IAsyncSocketServer>`. Bind that context for the exact duration of
the native-controller/`GuiMain` callback. The enum is the selection; the
socket pointer is an owned dependency, not another selection flag.

On non-Windows platforms, only `MiniHttp` is valid. Reject or compile out
`WindowsHttp` at the transport selection boundary rather than branching
throughout `GuiMain`.

## 6. Straight-line application lifetime

Replace nested `try`/`catch` cleanup ladders in Cpp/Core/renderer with scoped
owners for:

- channel server/client start and stop;
- requester session start and stop;
- current `GuiMain` context binding;
- native-service substitution;
- automation endpoint lifetime; and
- renderer/channel binding.

The normal shape should be:

1. Construct dependencies.
2. Start the transport/channel.
3. Establish RPC/channel roles.
4. Bind the stack-owned GUI context.
5. Run the native controller/application.
6. Leave the scope in reverse ownership order.

Do not catch `Exception` or `Error` merely to print, select an exit code,
attempt more cleanup, or translate one failure into another. Let synchronous
failures escape to the process boundary. At callback/thread boundaries where
escape cannot work, terminate immediately.

Core's RVM-host fatal path is the one important ordering exception: it must
attempt the Core-authored `!Error` while the renderer channel is alive and
then terminate nonzero even if delivery itself fails. Keep that operation
minimal and one-shot; it is protocol delivery, not recovery.

## 7. `CppTest_Rvm` is a GUI application

- Remove `using namespace vl::console`.
- Remove every `Console::WriteLine` from `GuiMain.cpp` and all entry-point
  files.
- Do not catch errors merely to write them to a console.
- Return a nonzero code for invalid command-line arguments without requiring
  a console.
- Do not otherwise consolidate or redesign the platform entry points in this
  task. The separate Linux/macOS follow-up owns any shared Unix entry-point
  work.

# Fail-Fast Audit

After all code is in its target folder, review every catch, retry, state
transition, event, acknowledgement, and stop call in
`Source/RemotingHelpers`.

## Allowed because the SOP requires the behavior

- Core's one-shot fatal `!Error` delivery before exit on RVM-host loss.
- The post-route `Ready` proof until the channel API offers a real
  post-admission callback.
- One-way host heartbeat and requester-side lease for silent HTTP host loss.
- Setting requester stopping state before sending one-way
  `RequesterStopping`.
- Renderer replacement and stale-renderer detachment.
- Ordered renderer handling of Core-authored fatal packages, local fatal
  channel errors, and disconnection.
- Retaining renderer fatal state until UI/automation initialization is ready.
- Protocol-level 404 responses for unsupported or malformed automation HTTP
  requests.
- A non-returning exception firewall at an OS/thread callback boundary where
  exception propagation is impossible or swallowed.

## Remove

- Any heartbeat acknowledgement or stopping acknowledgement.
- Reverse liveness/lease state.
- Cleanup-error aggregation or "first error" tracking.
- Catch/log/continue and catch/return-success behavior.
- Converting unexpected automation exceptions to 404.
- Retrying task queues or reconnect policies.
- Defensive partial-start booleans made unnecessary by RAII ownership.
- Duplicate `Stop` in destructors and explicit cleanup.
- Shutdown waits whose only purpose is graceful acknowledgement.
- Recovery state that is not named by `DebugRemoteProtocolSop.md`.

# Deferred Linux and macOS Follow-up

Do not perform the items in this section as part of this task. Do not modify
`Test/Linux/**`, run its `build.sh` flows, or claim Linux/macOS compatibility
from Windows-only results.

The affected inventory for the separate follow-up is:

- `Test/Linux/CppTest`
- `Test/Linux/CppTest_Metaonly`
- `Test/Linux/CppTest_Reflection`
- `Test/Linux/CppTest_Rvm`
- `Test/Linux/RemotingTest_Core`
- `Test/Linux/RemotingTest_RvmHost`

Handoff observations for that task:

- Correct `CPP_TARGET` for `CppTest`, `CppTest_Metaonly`, and
  `CppTest_Reflection`; they currently produce `./Bin/UnitTest`.
- Use shared `*.Linux.cpp` source for Linux and macOS under `VCZH_GCC`,
  following the repository naming convention.
- Merge the `CppTest_Rvm` entry points and make the Unix path include macOS.
- Update `CPP_ADDS`, `CPP_REMOVES`, and include paths after moving runtime and
  helper files.
- Keep Windows-only helper files and `VlppOS.Windows` excluded.
- Ensure the neutral helper pair has no Windows headers, symbols, or link
  dependencies.
- Keep `CppTest_Rvm` build-only where the local platform UI is unavailable;
  use Core `/RVMT` plus `RemotingTest_RvmHost` `/MiniHttp` for the runnable
  portable demo.
- Do not edit `vmake.txt` or `makefile`; regenerate them through the documented
  build flow.
- Treat a Linux build as Linux evidence only. Run an actual macOS build/CI job
  before claiming macOS verification.

These observations are neither action items nor blockers for the current
Windows task.

# Phased Actionable TODO

## P0: Freeze boundaries and required behavior

- [ ] Record the current successful `/Pipe`, `/Http`, and `/MiniHttp`
  startup/normal-stop behavior before moving files.
- [ ] Add or identify focused coverage for post-route `Ready`, one-way
  heartbeat timeout, normal `RequesterStopping`, Core `!Error` ordering, and
  renderer replacement.
- [ ] Confirm there is no dependency from the future helper folder to `Test`
  or generated code.
- [ ] Confirm no upstream Workflow/VlppOS change is required for this
  test-helper extraction.

## P1: Create helper and CodePack structure

- [ ] Create the three required helper areas and nested Windows locations.
- [ ] Add neutral and Windows CodePack categories.
- [ ] Exclude the helper folder from the ordinary `gacui` category.
- [ ] Generate `GacUI.RemotingHelpers.(h|cpp)` and
  `GacUI.RemotingHelpers.Windows.(h|cpp)`.
- [ ] Verify ordinary `GacUI` and `GacUI.Windows` no longer contain moved
  test-helper declarations or implementations.
- [ ] Add explicit Windows project/filter wiring without wildcards.

## P2: Extract automation services

- [ ] Add the minimal native Windows IO-command bridge.
- [ ] Move `WindowsAutomationServiceBase` and all concrete Windows automation
  service types.
- [ ] Move `HttpAutomationService` and Windows HTTP start/stop.
- [ ] Add the new Windows automation substitution start/stop facade.
- [ ] Remove automatic test-service substitution from GDI and Direct2D setup.
- [ ] Move `MiniHttpAutomationService` and its start/stop functions.
- [ ] Replace cross-project `Shared.cpp` inclusion and `extern`
  declarations with helper headers.
- [ ] Add scoped owners and remove duplicate stop/destructor cleanup.
- [ ] Remove exception-to-404 translation for unexpected failures.
- [ ] Migrate every test app currently using
  `StartWindowsHttpAutomationService`, including `CppTest*`, `GacUI_Host`,
  `Playground`, Core, and the native renderer.

## P3: Extract server and client composition

- [ ] Implement one generated-neutral configurable remoting channel server.
- [ ] Inject local/remote admission and disconnect policy.
- [ ] Make renderer admission optional and preserve replacement behavior.
- [ ] Replace Cpp's `RemoteViewModelUiServer` family.
- [ ] Replace Core's `RemotingChannelServerBase` family.
- [ ] Move/generalize `RemotingTestChannelClient`.
- [ ] Preserve renderer fatal/disconnected/startup ordering with focused
  coverage.
- [ ] Remove the old classes and transport subclasses from application
  `GuiMain.cpp` files.

## P4: Extract reusable RPC/channel helpers

- [ ] Split generated-neutral channel maps, local broker/requester clients,
  task scheduling, control messages, and liveness from
  `RemoteViewModelTestRuntime.*`.
- [ ] Add a generated-module template/factory for JSON RPC lifecycle setup.
- [ ] Keep separate broker and requester local endpoints.
- [ ] Keep one-way heartbeat, post-route `Ready`, and one-way stopping.
- [ ] Keep only generated `RemoteViewModelTestRpc` and typed
  `rvmt::IViewModel` adaptation under `Test`.
- [ ] Reduce `RemoteViewModelTestRuntime.*` to the application-specific
  adapter, or remove the pair if the remaining code is clearer inline.
- [ ] Do not move RVM-specific channel/service names into a supposedly
  application-neutral API unless they become explicit configuration.

## P5: Simplify test hosts

- [ ] Add and pass the automation backend enum through all server/client start
  paths.
- [ ] Replace global selector booleans and pointer-to-stack-pointer state with
  one owned host context.
- [ ] Replace cleanup ladders with RAII scopes.
- [ ] Remove all `Console` references from `CppTest_Rvm`.
- [ ] Keep Core-specific window choice, fatal delivery, and renderer state in
  Core.
- [ ] Keep app-specific service registration/request and generated window
  creation in the app.
- [ ] Review the resulting app files against the goal that new apps need only
  a few lines of C++ plus XML resources.

## P6: Complete the fail-fast review

- [ ] Classify every nontrivial catch against the allowed SOP list.
- [ ] Delete every unclassified catch/retry/acknowledgement/recovery state.
- [ ] Verify async exception boundaries terminate rather than swallow.
- [ ] Verify normal stop still releases listeners/ports without a peer
  acknowledgement.
- [ ] Verify an unexpected helper invariant immediately terminates the test
  process.

## P7: Finish Windows project cleanup

- [ ] Update moved source and include paths in affected MSBuild projects.
- [ ] Confirm no Windows helper source enters the neutral CodePack pair.

# Verification

## Static and CodePack checks

- Run `git diff --check`.
- Search `Source/RemotingHelpers` for `Test/`, `Generated_`,
  `RemoteViewModelTestRpc`, and `rvmt::`; require no application-generated
  dependency.
- Search `CppTest_Rvm` for `Console`; require no match.
- Search the old files for every moved class/function; require no duplicate
  definition.
- Search helper code for `catch`; justify each match against the explicit SOP
  allowlist.
- Run CodePack and inspect the four requested outputs.
- On Windows, compile the neutral pair without `VlppOS.Windows`, `<Windows.h>`,
  or any Windows GacUI provider. This validates layering only; it does not
  establish Linux or macOS compatibility.
- Confirm helper symbols do not appear in ordinary `GacUI` or
  `GacUI.Windows`.
- Confirm generated folders and `Import` have no direct edits.

## Windows builds and unit tests

Follow the repository build/run guidelines and use the supplied PowerShell
wrappers.

- Build affected Debug/Release and Win32/x64 configurations.
- Build at least:
  - `CppTest`
  - `CppTest_Metaonly`
  - `CppTest_Reflection`
  - `CppTest_Rvm`
  - `GacUI_Host`
  - `Playground`
  - `RemotingTest_Core`
  - `RemotingTest_Rendering_Win32`
  - `RemotingTest_RvmHost`
  - `UnitTest`
- Run `UnitTest` after all C++ changes.
- Verify each migrated automation endpoint starts on its documented port and
  returns the same Controls/DOM/IO behavior.

## Deferred cross-platform verification

Linux and macOS builds are not performed or required by this task. The
separate platform follow-up must run the affected `Test/Linux` build flows on
Linux and on an actual macOS environment. The outcome of this Windows task
must be reported as cross-platform compatibility unverified.

## Multi-process acceptance

On Windows, for `/Pipe`, `/Http`, and `/MiniHttp`:

1. Start requester/Core first, then the RVM host.
2. Prove service acquisition and a successful `Translate`.
3. Stop normally and require no fatal classification.
4. Kill the host before `Ready`, while idle, and during `Translate`.
5. Require `CppTest_Rvm` to terminate nonzero.
6. With Core and a renderer active, require exactly one Core-authored `!Error`
   before Core exits nonzero and require no earlier
   `ControllerConnectionStopped`.
7. Kill/replace the renderer and require Core and the RVM host to remain live.
8. Kill Core and require the renderer's ordinary disconnected state, not a
   fabricated fatal prompt.
9. Repeat enough times to expose stale callbacks, occupied ports, duplicate
   stops, or use-after-scope state.

For GacJS:

- Run Core `/RVMT` plus the host plus GacJS for `/Http` and `/MiniHttp`.
- Kill the RVM host and require the exact Core-authored fatal error in GacJS.
- Require normal Core loss or browser disappearance to retain the existing
  nonfatal contract.

# Completion Criteria

The refinement is complete when:

- all reusable test-only automation/remoting code is owned by
  `Source/RemotingHelpers`;
- the four exact CodePack files are generated;
- ordinary GacUI library code pairs do not contain or depend on the helpers;
- current test apps contain only application/transport selection and generated
  resource composition;
- no helper includes generated application code;
- `CppTest_Rvm` has no console dependency;
- the affected Windows configurations build;
- Linux and macOS compatibility is explicitly reported as unverified and
  deferred;
- all non-SOP recovery and shutdown acknowledgements are gone; and
- the complete Windows fatal, normal-stop, renderer-replacement, and
  automation acceptance matrix passes.

# UPDATES

# TEST [CONFIRMED]

The problem is a test-support ownership and composition problem. Confirm the
current behavior before moving code, then use the same checks as the regression
contract:

- Static donor inventory:
  - `WindowsAutomationService*`, the Windows HTTP endpoint, and setup-time
    substitution currently live in the Windows platform provider.
  - MiniHTTP automation currently lives in
    `RemotingTest_Core/Shared.cpp` and is shared by compiling that source into
    multiple projects.
  - the local RVM server, Core server, renderer channel client, transport
    selection globals, and cleanup ladders currently live in application
    `GuiMain.cpp` files;
  - `Source/RemotingHelpers` and the requested CodePack pairs do not exist.
- Existing focused unit coverage in `TestRemoteViewModelRuntime.cpp` must pass
  before and after extraction. It proves that only the exact RPC/control
  channel pair is classified as an RVM host. Add focused helper tests if a
  newly extracted policy can be tested without a concrete transport.
- Build the Debug x64 solution and run the complete Debug x64 `UnitTest`
  suite. Success requires all test files and cases to pass and no memory-leak
  dump after the summary.
- For `/Pipe`, `/Http`, and `/MiniHttp`, start `CppTest_Rvm` first and then
  `RemotingTest_RvmHost`. Require the requester to acquire
  `rvmt::IViewModel`, expose a healthy automation endpoint, and return
  `Hello, !` from the generated `Translate` binding. Send exact `!Exit` and
  require normal requester and host exit without fatal classification.
- Repeat the three transports after extraction and additionally terminate the
  host while the requester is idle. Require the requester to terminate
  nonzero. Exercise Core/native-renderer admission, renderer replacement,
  Core-authored RVM-host fatal delivery, and normal Core loss according to
  `DebugRemoteProtocolSop.md`.
- Exercise every migrated Windows and MiniHTTP automation endpoint through its
  documented `Controls`, `Dom`, and representative synchronous-error/queued-IO
  behavior. A protocol-level unsupported or malformed request may return 404;
  an unexpected implementation failure must remain fail-fast.
- Run CodePack from `Release/CodegenConfig.xml` and inspect exactly
  `GacUI.RemotingHelpers.{h,cpp}` and
  `GacUI.RemotingHelpers.Windows.{h,cpp}`. The neutral pair must contain no
  Windows provider or `VlppOS.Windows` dependency, and neither helper pair may
  leak into ordinary `GacUI`/`GacUI.Windows`.
- Run the requested affected Debug/Release Win32/x64 builds and static audits:
  no helper dependency on `Test`, `Generated_*`,
  `RemoteViewModelTestRpc`, or `rvmt::`; no `Console` in `CppTest_Rvm`; no
  duplicate moved definitions; every remaining helper `catch` justified by
  the SOP; no direct generated/Import edits; and `git diff --check` clean.

Windows-only results are the success criterion for this investigation.
Linux/macOS compatibility remains explicitly unverified and deferred.

The pre-extraction baseline is confirmed:

- Debug x64 `GacUISrc.sln` built successfully with 0 warnings and 0 errors.
- The complete Debug x64 `UnitTest` run passed 89/89 test files and
  1,714/1,714 test cases, with no memory-leak dump after the summary.
- `/Pipe`, `/Http`, and `/MiniHttp` each exposed the `CppTest_Rvm` control
  tree with the generated value `Hello, !`, proving successful service
  acquisition and `Translate`.
- Exact `!Exit` returned `Queued` for all three transports. Both requester and
  host processes exited after each run, with no fatal UI or surviving process.
- The MiniHTTP listener binds IPv4 loopback. Its automation check therefore
  uses `127.0.0.1`; the first `localhost` probe resolved to IPv6 and did not
  reach the healthy IPv4 listener.
- Static inventory confirms the architectural problem: all donor symbols and
  ambient startup state remain in the locations listed above,
  `Source/RemotingHelpers` is absent, and the two requested CodePack pairs do
  not exist.

# PROPOSALS

- No.1 [CONFIRMED] Extract generated-neutral remoting composition with scoped automation ownership

## No.1 [CONFIRMED] Extract generated-neutral remoting composition with scoped automation ownership

Create `Source/RemotingHelpers` as a test-support layer with the requested
`AutomationService`, `RemotingServer`, and `RemotingClient` ownership
boundaries. Keep neutral files free of Windows and generated-application
dependencies, and put Windows provider/HTTP code only in nested `Windows`
folders.

Move Windows automation implementations and the Windows HTTP endpoint out of
`WinNativeWindow`. Leave one low-level Windows provider bridge that executes a
parsed IO command against a validated native window while the provider still
owns access to its listener list. Resolve window IDs through the public
created-window enumeration, preserve the existing pointer encoding, and remove
automation-specific friendship/private controller access. Remove automatic
automation substitution from GDI and Direct2D startup.

Move MiniHTTP automation into the neutral helper layer. Both HTTP endpoints
will classify unsupported verbs/routes, malformed IO suffixes, invalid content
types, and disabled operations as 404, but will terminate at async/OS callback
boundaries for unexpected parser, automation, or invariant failures. The
MiniHTTP service destructor will no longer duplicate explicit `Stop`.

Add scoped substitution and endpoint owners plus
`RemotingAutomationService::{WindowsHttp,MiniHttp}`. Every GUI app will start
its selected service after the native controller exists. Cpp/Core/renderer
transport entry points will pass the enum and a strong MiniHTTP socket through
one stack-owned context into `GuiMain`, replacing selector booleans and
pointers to stack-owned `Ptr` objects.

Merge the two server families into
`RemotingChannelServer<TServerBase>`. Inject non-renderer local/remote
admission, renderer-readiness, disconnection, and status callbacks. Keep
optional renderer admission, the current renderer ID, stale-renderer detach,
replacement notification/fallback disconnect, and only the core JSON/protocol
binding in the helper. Keep transport construction directly in each app.

Move the renderer channel client under `RemotingClient` and configure its fatal
title, main-thread scheduling, renderer/automation bindings, and retain/exit
policy. Preserve first-error-wins Core-authored fatal handling, prompt-free
fatal local disconnects, idempotent `OnDisconnected`, pending startup state,
and explicit transport stop before stack callback targets disappear.

Split the current RVM runtime into a generated-neutral RPC/channel runtime and
a thin test adapter. Configure channel/service/control names, one-way
`Ready`/heartbeat/stopping values, lease timings, and the generated module
factory. The generic helper will own channel maps, broker/requester local
clients, task scheduling, RPC lifecycle initialization, requester lease, and
host control client without naming `RemoteViewModelTestRpc` or
`rvmt::IViewModel`. The remaining Test runtime will select the generated module
and cast the requested service.

Replace application catch/cleanup ladders with scoped owners and straight-line
lifetime ordering. Keep only SOP-required catches: renderer replacement
delivery fallback, the Core one-shot fatal delivery attempt, and non-returning
async callback/thread firewalls. Remove all `Console` use from `CppTest_Rvm`.

Update every affected Windows project/filter explicitly, split CodePack into
the two exact neutral/Windows helper pairs, regenerate Release through
CodePack, and update automation documentation whose old provider-ownership
description becomes false. Do not modify `Test/Linux`, generated resource
sources, Import, or upstream Workflow/VlppOS.

### CODE CHANGE

Implemented the extraction and confirmed the proposal.

- Added the neutral `Source/RemotingHelpers` ownership areas:
  - `AutomationService` now owns the automation host configuration, the
    portable MiniHTTP endpoint, and scoped service lifetimes.
  - `RemotingServer` now owns the generated-neutral configurable
    `RemotingChannelServer<TServerBase>` plus the reusable RPC requester/host
    runtime.
  - `RemotingClient` now owns the configurable renderer channel client and its
    terminal-state ordering.
- Added the nested Windows automation implementation. It owns native, hosted,
  and renderer automation substitutions plus the Windows HTTP endpoint.
  `WinNativeWindow` retains only the narrow exported IO-command provider seam.
  GDI and Direct2D no longer create a test automation service automatically.
- Moved MiniHTTP automation out of the deleted
  `Test/GacUISrc/RemotingTest_Core/Shared.cpp`. Both HTTP implementations
  reject malformed `/IO/<id>` routes with 404 before parsing while unexpected
  callback failures reach a non-returning process boundary. MiniHTTP returns
  the exact `application/json; charset=utf8` content type.
- Merged the requester-only and Core server families into the single generic
  server. Renderer admission remains optional; renderer replacement,
  notification fallback, stale-renderer detach, and Core bindings preserve the
  SOP contract.
- Split the RVM runtime into a generated-neutral helper and the thin
  `RemoteViewModelTestRuntime` adapter. The helper retains distinct broker and
  requester local endpoints, post-route `Ready`, one-way heartbeat lease,
  one-way `RequesterStopping`, and pre-teardown stopping classification. It
  does not name `RemoteViewModelTestRpc`, `rvmt::`, or `IViewModel`.
- Migrated `CppTest`, `CppTest_Metaonly`, `CppTest_Reflection`,
  `CppTest_Rvm`, `GacUI_Host`, `Playground`, `RemotingTest_Core`,
  `RemotingTest_Rendering_Win32`, and `RemotingTest_RvmHost` to explicit
  scoped setup. `CppTest_Rvm` has no console dependency.
- Added every helper source/header explicitly to the affected MSBuild
  projects and filters. All 24 project filter files parse as XML and no project
  references the removed `Shared.cpp`.
- Updated `Release/CodegenConfig.xml` and regenerated CodePack. The exact new
  root and IncludeOnly outputs are:
  `GacUI.RemotingHelpers.{h,cpp}` and
  `GacUI.RemotingHelpers.Windows.{h,cpp}`. The neutral pair contains no
  Windows provider or `VlppOS.Windows` dependency; the ordinary
  `GacUI`/`GacUI.Windows` pairs contain no extracted helper symbols.
- Updated the automation-service knowledge-base page to describe explicit
  test-helper ownership and lifetime.

Verification passed:

- Final Debug x64, Debug Win32, Release x64, and Release Win32 solution builds
  completed with 0 warnings and 0 errors. The cold Release x64 wrapper reached
  its one-hour timeout while its child build processes continued to successful
  links; a subsequent wrapper run completed cleanly with 0 warnings and
  0 errors.
- The complete Debug x64 `UnitTest` run passed 89/89 test files and
  1,714/1,714 test cases, with no memory-leak sidecar or dump.
- The post-extraction `/Pipe`, `/Http`, and `/MiniHttp` requester/host matrix
  acquired `rvmt::IViewModel`, exposed `Hello, !`, returned exact `Queued` for
  `!Exit`, and exited normally.
- Idle RVM-host loss terminated requesters for all three transports, including
  the heartbeat-timeout path for `/Http` and `/MiniHttp`.
- Native renderer checks for all three transports confirmed: renderer loss is
  nonfatal to Core, a replacement renderer disconnects the previous renderer,
  normal Core loss is an ordinary renderer disconnection, and RVM-host loss
  makes Core terminate after retaining the Core-authored fatal UI in the
  renderer.
- The Windows HTTP and MiniHTTP endpoints returned 404 for malformed
  `/IO/bad/id`, remained healthy, then accepted a valid `!Exit`.
- Real GacJS browser checks for `/Http` and `/MiniHttp` confirmed the exact
  distinction:
  - RVM-host loss shows the visible fatal mask with
    `RemotingTest_RvmHost disconnected.` and no JavaScript dialog.
  - ordinary Core loss shows only the success mask with
    `HTTP remote protocol disconnected.` and no JavaScript dialog.
- Migrated Cpp/Metaonly/Reflection/GacUI_Host automation endpoints exposed
  healthy control trees and normal queued exit; Playground completed normally.
- Source whitespace, generated-neutral dependency searches,
  ordinary-CodePack leakage searches, stale-donor searches, project XML
  validation, and the fail-fast catch audit are clean. Plain
  `git diff --check` reports only CodePack's established extra-blank-line-at-EOF
  convention on the four newly added root CodePack outputs; those generated
  files are retained byte-for-byte as emitted instead of being hand-edited.
  Remaining catches are limited to the SOP-required Core/renderer delivery
  fallback and non-returning callback or thread firewalls.
- `Import` and generated GacUI resource sources were not edited.

All implementation and verification above is Windows-only. `Test/Linux/**`,
Linux builds, and actual macOS builds remain explicitly deferred and
unverified, as required by the task boundary.
