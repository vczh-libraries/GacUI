# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

This is a continuation of the previous work:
- TODO_Injection.md

1. Previous work also fixes a bug when saved private document format doesn not contain styles, but there is one thing to improve:
The full style copy is done in `DocumentModel::CopyDocument()`, but I think we should move the work into `DocumentModel::CopyDocument(TextPos, TextPos, deepCopy)`.
Currently `DocumentModel::CopyDocument(TextPos, TextPos, deepCopy)` only copies used styles, but I would like when `deepCopy` is true it copies all styles just like how `DocumentModel::CopyDocument()` is doing.
So that the simple `CopyDocument` could just delegate the work to the complex one.

### DETAILS

- Implement the style-copy policy in `<GacUI repo>/Source/Resources/GuiDocument_Edit.cpp`: a deep range copy owns a complete style catalog, including unused definitions, parent names, authored properties and resolved properties. Clone each definition once. Keep paragraph slicing and invalid-range behavior unchanged, including the existing empty-document case.
- Keep `deepCopy=false` limited to styles referenced by the copied content and their transitive parents, sharing the original style objects. The parameterless overload should only determine the full range and delegate to the deep range overload. Update the range-overload contract in `<GacUI repo>/Source/Resources/GuiDocument.h` and obsolete comments about selection copies excluding unused styles.
- This intentionally changes the deep-range expectation in `<GacUI repo>/Test/GacUISrc/UnitTest/TestDocumentConfig.cpp`, which currently requires the `Unused` style to be absent. Deep range copies also feed selection copying and undo/redo in `<GacUI repo>/Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`; account for style merging and renaming when these copies are inserted again.

### VERIFICATION

1. Extend the ordinary document tests for full, partial, zero-length and zero-paragraph deep copies. Check unused styles, parent chains and baseline overrides, and mutate both authored and resolved properties in a copy to prove independence. Separately verify shallow-copy filtering and pointer sharing, and retain invalid-range assertions.
2. Exercise selection copy/paste and repeated undo/redo. Verify text, effective formatting and style references remain correct without unnecessary accumulation of renamed style definitions. Save and reload a private document containing unused styles and verify the complete catalog survives.
3. Build and run the affected GacUI tests using the repository wrappers and required configurations. Use ordinary tests for model assertions; any rendering snapshot must show useful document output. These checks belong to implementation, not this task review.

2. Using `CppTest` and switch to the ribbon document tab, after selecting a piece of text you can change its style, e.g., "Header 1".
In the style dropdown, if you select any item in the ribbon directly, it works fine.
But when you open the dropdown, or when the window does not have enough space and you can only use the dropdown, select a style item in the dropdown does not respond.
Figure it out and fix the issue.

### DETAILS

- `<GacUI repo>/Test/Resources/App/FullControlTest/DocumentEditorRibbon.xml` already applies styles from `ItemApplied`. Trace the actual popup buttons, selection synchronization and event dispatch in `<GacUI repo>/Source/Controls/ToolstripPackage/GuiRibbonGalleryList.cpp` before choosing the fix. Preserve the distinction between selection-only `SelectItem` and effect-applying `ApplyItem`; a user application must work even when that gallery item is already selected.
- The existing dropdown case in `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_Ribbon_Gallery.cpp` uses `GetListControlInDropdown()`, whose implementation returns the inline list. The popup is built from separate repeated group/item compositions. Existing `DocumentStyles` UIA assertions also exercise inline items, so neither proves that popup selection works.

### VERIFICATION

1. In `CppTest`, select recognizable document text and click actual popup items in both a wide ribbon and a narrow layout where only the dropdown is available. Cover both style groups, multiple styles, an already-selected gallery item, reopening and resizing. Check the resulting document formatting and preserved text selection; an updated gallery highlight alone is insufficient.
2. Add focused gallery coverage proving each popup application raises `ItemApplied` exactly once, while programmatic selection synchronization and popup dismissal do not apply anything. Retain inline selection as a regression case and repeat after theme refresh.
3. Extend the existing gallery tests and `<GacUI repo>/Test/UIA_CppTest_Shared.cs` assertions to target actual popup controls. Run the affected unit tests and both `<GacUI repo>/Test/UIA_CppTest.ps1` and `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1` as applicable. Use automation service first for investigation, then UIA; record unavailable desktop-only coverage explicitly.
4. If authored XML changes, run the required GacUI compiler and metadata generation/test sequence from `<GacUI repo>/Project.md`, rebuild consumers and run both showcase UIA wrappers. Do not edit generated C++ to repair the behavior.

3. Support `RemotingTest_Win32_Renderer /corehost:localhost`, so that it could connects to other computer.
Fix `test.sh` in `wGac` and `iGac` to support it, but skip verification as you are on Windows.

First you need to fix VlppOS. There are `AsyncSocketClient` in `AsyncSocket.(Windows|macOS|Linux).h` with the same layout.
The current constructor connects a socket server in localhost.
You are going to add another constructor with an extra `WString host`.
The host could be IPv4 address, IPv6 address or a name that needs to look up via DNS (e.g. `getaddrinfo` but you must choose the best way per OS).
You may or may not need to parse the IPv4/6 address by yourself, use OS provided ways when possible.
Therefore `SocketHttpClientApi` could actually connects to other computer.
When such API change is ready, you can then fix all test apps when using `/corehost` with `/MiniHttp`.

You still need to make it work for `/Http` and `/Pipe`.
`windows_http::HttpClient` should already support this feature.
`named_pipe::NamedPipeClient` needs to add another constructor, with an extra `WString host`. And then you can pass "." or "host" to `ClientCreatePipe to construct the actual pipe url.

Both added `WString host` should be the first argument.

`/corehost` should be optional, when it is not used, it means `/corehost:localhost`.

During verification, you may need to pay attention if `/Http` and `/MiniHttp` accepts connections other than "localhost". You should verify if it accept "127.0.0.1" and the machine name, by not performing any constraint on the host name.
And then you can start core and native renderer, see if using `/corehost:127.0.0.1` and `/corehost:machine name` work or not.

### DETAILS

- The actual Windows project name is `RemotingTest_Rendering_Win32`. Its entry points are `<GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32/Main.cpp` and `<GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp`; use that name in commands and documentation.
- Define `/corehost:<host>` as one optional, case-sensitive destination option, defaulting to `localhost`. Reject empty and duplicate options before starting transports. Consume only the fixed prefix so IPv6 colons remain part of the value. Accept a hostname or address, not a URL, path or separate port; the TCP protocol port remains 8888 and `/AsPort` still controls local automation. Use an unbracketed IPv6 address at the socket boundary and bracket it when constructing an HTTP authority.
- Apply the option to native renderers and the network modes of `RemotingTest_RvmHost`. Its parser in `<GacUI repo>/Test/GacUISrc/RemotingTest_RvmHost/Main.cpp` currently permits only one argument. Reject an explicitly supplied `/corehost` with its local `/Cli` mode. `RemotingTest_Core` and `CppTest_Rvm` host the network servers and do not need a destination-host option; their locally launched stdio host remains local.
- In `<VlppOS repo>/Source/InterProcess/AsyncSocket/AsyncSocket.Windows.h`, `<VlppOS repo>/Source/InterProcess/AsyncSocket/AsyncSocket.Linux.h` and `<VlppOS repo>/Source/InterProcess/AsyncSocket/AsyncSocket.macOS.h`, add the host-first constructors and retain the existing constructors as local defaults. Add a matching host-first `CreateDefaultAsyncSocketClient` overload in `<VlppOS repo>/Source/InterProcess/AsyncSocket/AsyncSocket.h` and its platform implementations. `CreateSameEndpointClient()` must preserve the host, port and backend, including when called after the original client stops: the MiniHTTP protocol obtains its additional physical connections through that method.
- Use native address resolution/connection facilities for IPv4, IPv6 and DNS, trying usable address candidates while preserving the existing bounded retry, cancellation, callback and terminal-state contracts. The Windows implementation also binds its local client address to IPv4 loopback; update its source binding and address-family handling so a remote destination is actually reachable.
- Remove the loopback-only hostname restrictions consistently from `<VlppOS repo>/Source/InterProcess/AsyncSocket/AsyncSocket_HttpClient.cpp`, `<VlppOS repo>/Source/InterProcess/AsyncSocket/AsyncSocket_HttpClientApi.cpp` and `<VlppOS repo>/Source/InterProcess/AsyncSocket/AsyncSocket_HttpServerApi.cpp`. Pass the requested host to both the socket and HTTP layers. Preserve HTTP/1.1 structural validation, including a single valid `Host`, correct port and bracketed IPv6 authority; accepting arbitrary valid hostnames does not mean accepting malformed requests.
- The current native socket servers bind IPv4 loopback only. Make the protocol listeners reachable through the machine's network interfaces and supported address families. Windows HTTP also registers only a localhost URL in `<VlppOS repo>/Source/InterProcess/Windows/HttpServer.Windows.cpp`; update protocol registration so requests addressed to the machine name or IP reach the server. Keep protocol endpoint selection distinct from automation-port selection.
- The HTTP assumption above needs correction: `windows_http::HttpClientApi` accepts a host, but `windows_http::HttpClient` constructs it with `localhost` in `<VlppOS repo>/Source/InterProcess/Windows/HttpClient.Windows.cpp`. Add host forwarding to the protocol client while retaining its existing local overload.
- In `<VlppOS repo>/Source/InterProcess/Windows/NamedPipe.Windows.cpp`, add the host-first client overload and preserve the old local pipe behavior. Treat the default `localhost` as the local `.` endpoint and retain explicit remote machine names when constructing the UNC pipe path. The server currently specifies `PIPE_REJECT_REMOTE_CLIENTS`; adjust that restriction as part of making remote `/Pipe` work. IPv6 socket support does not imply that raw IPv6 text is a valid Windows UNC server name.
- Add `--corehost:<host>` to `<wGac repo>/test.sh` and `<iGac repo>/test.sh` for `--app:renderer`, forwarding one quoted `/corehost:<host>` argument in both foreground and `--unblock` branches. Their `--port` remains the renderer automation port. Also update `<wGac repo>/RemotingTest_Rendering_Wayland/Main.cpp` and `<iGac repo>/RemotingTest_Rendering_macOS/Main.cpp`, which currently reject unknown arguments.
- Keep shared host parsing and renderer composition canonical in `<GacUI repo>/Test/GacUISrc/SharedArguments.h` and `<GacUI repo>/Test/GacUISrc/RemotingTest_Rendering_Win32/GuiMain.cpp`. Both `<wGac repo>/syncProj.sh` and `<iGac repo>/syncProj.sh` copy those files, but do not copy the platform entry points listed above; synchronize the shared files and maintain the platform parsers separately. The local Core/requester launchers do not need a remote destination option. Expose optional `-CoreHost` through `<GacUI repo>/Test/StartRendrerer.ps1` without changing its existing filename or defaults.
- Implement upstream changes in VlppOS source and publish them through the existing release/import synchronization workflow before rebuilding consumers. Do not hand-edit `<GacUI repo>/Import` or the platform repositories' generated imports. Keep the existing shutdown/disconnection behavior; no new recovery protocol is needed.

### VERIFICATION

1. Extend `<VlppOS repo>/Test/Source/TestInterProcess_AsyncSocket.cpp` for old/default construction, explicit localhost, IPv4, IPv6, DNS, endpoint-preserving clones, address-candidate fallback, failed resolution and cancellation during connect/retry. Verify failures finish through the established status/callback paths without hanging. Use controlled local endpoints instead of requiring public Internet access.
2. Update `<VlppOS repo>/Test/Source/TestInterProcess_AsyncSocket_MiniHttpApi.cpp`, including the case that currently requires `example.com` rejection. Test valid DNS/IPv6 Host serialization and dispatch, plus malformed, duplicate and wrong-port Host rejection. Extend `<VlppOS repo>/Test/Source/TestInterProcess.cpp` to prove every MiniHTTP connection uses the requested endpoint and to cover the HTTP/pipe host overloads and old local overloads.
3. Verify argument ordering with the transport, `/AsPort` and injection flags, plus empty/duplicate host rejection. Cover the renderer and network RVM host; preserve `/Cli` behavior and reject its incompatible host option before starting it.
4. On Windows, run Core `/FCT` and the native renderer using each of `/Http`, `/MiniHttp` and `/Pipe`. Cover omitted host, explicit `localhost`, `127.0.0.1` and the actual machine name. For TCP also exercise the machine's non-loopback address and IPv6 where available. Use Core `/AsPort:8888` and renderer `/AsPort:8889`, and run transports sequentially because their protocol endpoints are fixed.
5. Check rendered UI and bidirectional traffic through Core `Controls`, renderer `Dom` and IO operations on both sides. Exercise an editor operation, renderer replacement and Core shutdown to verify normal lifecycle behavior. Also connect a network `RemotingTest_RvmHost` to Core `/RVMT` and to `CppTest_Rvm` with an explicit destination, and verify a real view-model call.
6. A machine-name connection on the same computer does not prove remote-machine access. When another Windows computer is available, repeat a TCP and named-pipe smoke check across machines; otherwise record that coverage as unavailable. Distinguish actual environment failures from code failures instead of treating a local connection as remote verification.
7. Use each repository's prescribed build/test wrappers. Inspect all three native backend changes, but skip wGac/iGac build and runtime verification as requested on Windows. Record Windows results separately from unexecuted Linux/macOS coverage.

4. Introduce this new parameter in `Project.md`, and `AGENTS.md` in `wGac` and `iGac`, as well as their `README.md` if `test.sh` is already explained there.

### DETAILS

- Update `<GacUI repo>/Project.md`, `<wGac repo>/AGENTS.md` and `<iGac repo>/AGENTS.md` with the destination option, local default, supported client modes, platform-specific transport availability and examples separating protocol port from automation port.
- Both platform READMEs already describe their launchers. Update `<wGac repo>/README.md`, its required Chinese counterpart `<wGac repo>/README_CN.md`, and `<iGac repo>/readme.md` using that file's actual casing. Include foreground and `--unblock` examples and the Windows `-CoreHost` launcher option in the GacUI guidance.
- Refresh the affected canonical knowledge-base guidance under `<Tools repo>/Copilot/KnowledgeBase` and synchronize it through the established workflow: the MiniHTTP API and channel guidance currently describe loopback-only transports, while the renderer/persistence guidance describes all range copies as retaining only used styles. Update stale transport descriptions in `<GacUI repo>/Test/RemotingHelpers/README.md` as well.

### VERIFICATION

1. Cross-check documentation examples against the actual parsers and launchers, including optional defaults, IPv6 syntax, accepted modes and distinct automation ports. Check both shell execution branches and platform entry points by inspection without claiming native platform verification.
2. Review generated release/import changes separately from authored files, and keep snapshot changes in their dedicated commit when implementation produces any. Do not include temporary host overrides or machine-specific paths in committed guidance.

5. `GacJS` it out of scope.

## REVIEW COMMENTS

No unresolved review comments. The implementation details and verification requirements above resolve the identified gaps using the existing APIs and repository conventions.

# TEST [CONFIRMED]

Reproduction confirmed for item 1: the unmodified implementation fails `selection->styles.Keys().Contains(L"Unused")` in the Debug x64 wrapper run. The baseline solution build completed with zero warnings/errors. For item 2, the hosted CppTest automation service reproduces the failure: type `Popup style sample`, select all 18 characters, open the collapsed Style gallery, and click the actual popup `Header 1` caption. The popup remains open and the document still contains an unstyled `nop` run with selection `(0,0)-(0,18)`. The popup button ignores mouse events originating from its child custom control. Its separate selection-based application path also omits an already-selected item. Item 3 is reproduced in the VlppOS Debug x64 wrapper run: the updated constructor case compiles with zero warnings/errors and fails with `SocketHttpClientApi requires an explicit loopback server.` No external connection is attempted by this case. Native socket clients and servers also hardcode loopback addresses, and native client constructors have no host argument.

1. Extend ordinary document-copy tests for full, partial, empty-range and empty-document copies, style independence, shallow sharing, invalid ranges, insertion, serialization and repeated undo/redo.
2. Exercise actual ribbon popup controls, event counts, selection preservation and formatting in gallery tests and both showcase UIA consumers. The extended gallery Dropdown case clicks a child label inside the popup, verifies one application per click including repeats, checks selection-only and explicit application APIs, verifies dismissal has no effect, retains inline-click coverage, and repeats after theme refresh and resizing.
3. Verify host-aware transports upstream, synchronize generated imports, then exercise Windows renderer and RVM destinations and lifecycle; inspect Unix code and scripts without executing native platform tests. First replace the MiniHTTP constructor test that requires rejection of `example.com` with successful construction and stopping, and add an unbracketed IPv6 constructor case. These cases do not connect to an external endpoint. They expose the existing loopback-only restriction before introducing the new native constructors. Final checks must include actual connections and endpoint-preserving physical lanes, in addition to constructor acceptance.
4. Cross-check documentation against implemented parsers and launchers and synchronize canonical knowledge.

# PROPOSALS

- No.1 Copy complete style catalogs in deep ranges and preserve style identity during reinsertion [CONFIRMED]
- No.2 Route popup child clicks through explicit gallery application [CONFIRMED]
- No.3 Preserve destination hosts through native transports, HTTP authorities and application launchers [CONFIRMED]

## No.1 Copy complete style catalogs in deep ranges and preserve style identity during reinsertion

Move complete-catalog cloning into the deep branch of `DocumentModel::CopyDocument(TextPos, TextPos, bool)` in `<GacUI repo>/Source/Resources/GuiDocument_Edit.cpp`; the parameterless overload determines its endpoint and delegates. Retain paragraph slicing, invalid-range returns and the existing zero-paragraph equal-position case. Shallow copies retain their existing referenced-style/parent traversal.

Insertion must compare the complete named-style ancestry before reusing an existing definition. A child with the same authored properties but a differently defined parent is a conflict too. Reuse an equivalent previously renamed definition when pasting the same source again, while keeping source definitions and parent references intact.

### CODE CHANGE

Implemented complete-catalog enumeration for deep copies, removed the redundant full-copy cloning pass, and documented the range contract. Insertion now compares named ancestry and reuses equivalent numbered aliases; destination baseline behavior is unchanged. Regression cases in `<GacUI repo>/Test/GacUISrc/UnitTest/TestDocumentConfig.cpp` cover complete and partial copies, empty ranges/documents, independent authored/resolved properties, baseline overrides, private serialization, shallow pointer sharing, invalid ranges and repeated insertion with conflicting ancestry. Ordinary editor integration coverage in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_Startup.cpp` checks selection copy/paste and five undo/redo cycles with stable style references, effective formatting and catalog size.

### CONFIRMED

The Debug x64 solution build through `copilotBuild.ps1` completed with zero warnings and errors. The subsequent `copilotExecute.ps1 -Mode UnitTest -Executable UnitTest` run passed all 219 cases in the four selected files: `TestDocumentConfig.cpp`, `TestControls_Editor_Styles.cpp`, `TestControls_Editor_RichText.cpp` and `TestRemote_Startup.cpp`. The completed execution log ends with the passing summary and contains no appended memory-leak report. The formerly failing unused-style assertion passes, as do full/partial/zero-length/zero-paragraph copies, private serialization, shallow sharing, invalid ranges and conflicting-parent reinsertion. Fifty-seven generated compiler text logs differed only in line ordering; their sorted contents were verified identical and those incidental changes were restored. No rendering snapshots changed. This task changes no reflection declarations or authored resource XML, so the additional metadata-generation configurations are not triggered.

## No.2 Route popup child clicks through explicit gallery application

The popup consists of independent selectable buttons containing the supplied item template. Unlike inline list selection, their click handling currently ignores child-control mouse events. Enable those events on the popup background button and call `ApplyItem` from its application handler. Make `ApplyItem` perform selection-only synchronization first, then emit one application event for a valid item, even if already selected. Keep `SelectItem` free of application effects and preserve its previous suppression state during reentrant synchronization. Inline pointer/keyboard selection continues through its existing selection-change handler.

The new popup regression builds successfully but fails at its first popup click: `applications->Count() == initialCount + 1 && applications->Get(initialCount) == 1`. This matches the CppTest automation-service reproduction. The original dropdown test clicked the compatibility accessor's inline list, bypassing the popup entirely.

### CODE CHANGE

Extend `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_Ribbon_Gallery.cpp` to use actual popup child controls and verify application counts before and after theme refresh. Implement the popup/application changes in `<GacUI repo>/Source/Controls/ToolstripPackage/GuiRibbonGalleryList.cpp`. Extend the existing showcase UIA style checks to exercise actual popup items in wide and collapsed layouts and inspect document formatting and preserved selection.

The focused showcase driver reacquires providers after responsive template replacement and waits for the owning UI thread to become idle after popup opening/closure. Initial normal runs exited during palette replacement, while a debugger run completed all style assertions and returned zero at normal shutdown; investigate this timing-dependent difference before claiming showcase verification.

CDB with the debugger heap disabled reproduces the access violation reported by Windows at `GuiControl::GetBoundsComposition`, called by `GuiPopup::CalculatePopupPosition` during hosted rendering. The popup is a `GuiTooltip`, and its type-3 position stores a freed template-owned ribbon control. `GuiControl` destruction removes and deletes its tooltip content without closing the shared tooltip or clearing that positioning reference. Before deleting tooltip content, detach it through its owning `GuiTooltip` and close that tooltip. Removing temporary content must invalidate the control-relative positioning immediately, since native window closure can be asynchronous. Extend the tooltip fixture to destroy an owner while its tooltip is visible, including pointer hover inside the tooltip, then reopen the surviving owner's tooltip. This is an ownership fix; idle waits cannot repair the freed pointer.

### CONFIRMED

The Debug x64 wrapper build completed with zero warnings and errors. The completed unit-test wrapper log reports 234/234 cases in seven files, with no appended memory-leak report: the four document/editor files from No.1, plus `TestControls_Ribbon_Gallery.cpp`, `TestApplication_Tooltip.cpp` and `TestApplication_Theme.cpp`. The gallery checks actual popup child clicks, exactly one application including repeated clicks, selection-only synchronization, dismissal, inline behavior, explicit `ApplyItem`, resizing and theme refresh. The tooltip regression verifies immediate detachment after owner deletion and successful reuse by another owner. Its four frames show the original tooltip, owner removal and the surviving tooltip. Nine incidental compiler logs differed only in ordering and were restored after sorted-content comparison.

Both prescribed showcase wrappers passed with `-Scenario DocumentStyles -AsPort 8888 -SkipBuild` against this build: `UIA_CppTest.ps1` passed 1,125 assertions and `UIA_CppTest_Metaonly.ps1` passed 1,115. Each covered all eight styles in both groups, inline application, actual popup application at widths 1400, 650 and 1250, repeated application of the selected item, both Default and Aurora palettes, exact formatting and retained heading selection, complete saved catalogs, private reload, normal `Window.Close` and endpoint release. The formerly failing normal CppTest palette transition now passes. The temporary unavailable-provider retry was removed after fixing the actual dangling tooltip pointer; popup open/close idle barriers remain.

The driver uses UIA to locate and scroll actual popup controls and automation IO to click their rendered captions. These popup backgrounds currently expose unnamed UIA Toggle controls without Invoke, so a Toggle-only application path is not claimed. No physical desktop mouse verification is claimed. No authored resource XML or reflection declarations changed, so metadata regeneration is not triggered. Authored changes and generated rendering snapshots are committed separately.

## No.3 Preserve destination hosts through native transports, HTTP authorities and application launchers

Retain immutable host and port in each native client and copy both into independent clients, including after stopping. Keep the existing port-only overloads as `localhost` defaults. Resolve IPv4, IPv6 and DNS through native facilities, rotate usable candidates across the bounded connection attempts, and cancel outstanding resolution/connection work during Stop. On Windows use cancellable asynchronous `GetAddrInfoExW` and family-specific wildcard source binding before `ConnectEx`; give pending connection attempts a deadline. On Linux retain resolution-request ownership through native asynchronous resolution completion and integrate connection deadlines with the existing io_uring cancellation/drain machinery. On macOS use Network.framework hostname endpoints and its connection timeout. Listen on supported network families and wildcard interfaces rather than loopback.

Share host validation and HTTP authority construction across MiniHTTP request creation and dispatch. Preserve single-Host, port, bracketed IPv6 and request framing checks while accepting valid DNS destinations. Forward the host through Windows HTTP protocol clients; register the protocol server wildcard URL. Preserve named-pipe local defaults, retain explicit machine names in UNC paths, and allow remote clients at the server.

Keep `/corehost:<host>` parsing canonical in the shared GacUI arguments. Reject empty/duplicate/malformed destinations before transport startup and reject the option in local RVM `/Cli` mode. Forward it through native renderers, network RVM hosts and the Windows/platform launchers. Synchronize upstream release/imports and canonical renderer files before consumer builds. Verify upstream behavior, then Windows applications and lifecycle; inspect Linux/macOS code and scripts without claiming runtime coverage.

### CODE CHANGE

Implemented host-first constructors, immutable endpoint cloning, native resolution and deadlines, wildcard listeners, structural HTTP authority validation, WinHTTP forwarding and remote-capable named pipes in VlppOS. Windows uses GetAddrInfoExW/ConnectEx with cancellation; Linux uses owned getaddrinfo_a requests and io_uring deadline cancellation (linking libanl); macOS uses Network.framework endpoints and a dispatch deadline that is drained during Stop. Added socket, HTTP-authority, physical-lane and host-overload regression coverage. The GacUI shared parser, Windows renderer, network RVM host and StartRendrerer launcher now forward the destination. The separately maintained wGac/iGac entry points and both shell launch branches use the same shared parser/composition. Generated VlppOS release/imports are synchronized before the consumer build. Windows verification and source review are complete with the environment limits below. The consumer build exposed WS2tcpip header-order dependence in renderer composition; keep that implementation-only include in the native source and its direct native test instead of exporting it from the public header.

The initial Windows transport build succeeds with zero warnings/errors. The existing HTTP protocol regression then fails registering its new wildcard URL with Windows error 5 (`ERROR_ACCESS_DENIED`); the current token is not elevated and the machine has no URL reservation for that test prefix. Retain the required wildcard behavior and report this as unavailable HTTP.sys server coverage. Make server tests probe the exact URL registration and report only access-denied cases as unavailable, while preserving all other failures. Continue MiniHTTP, socket, pipe and WinHTTP-client-to-MiniHTTP tests. Registration failures should include the native error and release partially constructed HTTP resources. Do not silently substitute a localhost listener or change machine-wide access control.

The host-overload regression also exposes a WinHTTP boundary requirement: direct native probes show `WinHttpConnect` rejects `::1` with error 12005 but accepts `[::1]`. Bracket raw IPv6 destinations inside `HttpClientApi` when opening the native connection, preserving its existing bracketed-host input. Keep socket destinations unbracketed. Include the native error and close the session if connection-handle creation fails.

A full-suite stress run passes 17 consecutive times, then fails the existing MiniHTTP channel chat timeout. An amplified diagnostic run under CDB stops exactly at TimeoutThread's deadline: threadCounter is 2, both Hello exchanges completed, client1Stopped is true and client2Stopped is false, both client worker threads have returned, and the remaining server worker is waiting for eventServer. The fixture signals each client to destroy itself immediately after BatchWrite queues its final Stop message; asynchronous teardown can cancel that send before server receipt. Move the fixture's client-completion event signals to ServerChannelClient::OnRead after recording the respective Stop. This is shared test synchronization, not a new transport acknowledgement or a longer timeout. Keep the production cancellation contract. Temporarily amplify the existing channel repetition for diagnosis, then restore the normal repeat count after verification.

The Windows MiniHTTP application matrix passes explicit localhost, IPv4 loopback, the computer name, non-loopback IPv4, IPv6 loopback and non-loopback IPv6, followed by the omitted-host default. Each replacement preserves the previous editor content; renderer IO and Core IO reach both the Core document and renderer DOM. An initial wrapper reported renderer exit 1 after FCT Close. The follow-up CDB run with first-chance exception breaks enabled exits 0 without an exception. A separate normal run repeats three renderer destinations and editor operations before FCT Close, and its retained process handle reports renderer exit 0; Core also exits 0. Pipe verification subsequently passes as recorded below.

### CONFIRMED

The final VlppOS Debug x64 wrapper build completes with zero warnings/errors. The full wrapper suite passes 319/319 registered cases in 16 files with no appended leak report. Seven HTTP.sys server cases explicitly report unavailable wildcard registration with Windows error 5; their server bodies are not claimed as executed. Native socket coverage includes localhost/IPv4/IPv6 exchange, active/stopped endpoint cloning, address-family fallback, failed DNS without loopback substitution, cancellation during startup and retry, callback ordering, no callbacks after Stop, MiniHTTP authority validation and actual physical-lane destinations. WinHTTP clients reach the MiniHTTP server through IPv4, IPv6 and machine DNS, and named-pipe local aliases preserve compatibility. After fixing the fixture's premature client destruction, an amplified 2,000-iteration MiniHTTP channel run passes as part of the full suite with no leaks; the normal 20 iterations are restored and the full suite passes again. No production teardown acknowledgement or increased test timeout is introduced.

The GacUI Debug x64 consumer wrapper build completes with zero warnings/errors; the focused wrapper suite passes 235/235 cases in seven files with no appended leak report. The native-only destination parser composes with automation/injection options and is excluded from Wasm builds. Nine compiler text snapshots differ only in ordering, verified by sorted content comparison and restored. No new rendering snapshots arise from destination selection.

| Windows application check | Result |
| --- | --- |
| Core FCT + MiniHTTP renderer: omitted host, localhost, IPv4 loopback, machine name, non-loopback IPv4, IPv6 loopback and non-loopback IPv6 | Passed editor operations from renderer and Core IO; Core Controls and renderer Dom agree; no retained fatal error during operation. Replacement retains previous editor content. |
| Core FCT + Pipe renderer: omitted host, localhost, IPv4 loopback and machine name | Passed the same bidirectional editor/DOM checks, replacement and normal Close; Core and renderer return 0. |
| MiniHTTP normal shutdown after replacements | Passed both the follow-up CDB run and a separate normal three-destination run; Core and renderer return 0. |
| Renderer option ordering with destination before transport, automation and all three injection flags | Passed the actual MiniHTTP application/editor run. Raw and bracketed IPv6 destinations are exercised. |
| Core RVMT + network RvmHost with explicit non-loopback destination and IPv6 renderer | Passed initial service acquisition and Translate calls from renderer and Core text input. |
| CppTest_Rvm + network RvmHost with explicit machine name | Passed Translate and requester exit. Host exits 1 on disconnect by its existing ExitViewModelHostProcess contract. |
| CppTest_Rvm local Cli regression | Passed Translate and normal requester exit 0. |
| Empty/duplicate destinations; malformed bracketed DNS; explicit host with RvmHost Cli in either argument order | Rejected by actual executable wrapper runs before transport startup. |
| Core Http FCT | CDB confirms HttpServerApi registration of the wildcard protocol URL fails with result 5. Full HTTP.sys application address/replacement matrix unavailable; no ACL/elevation or localhost fallback is applied. |
| Cross-machine TCP and named pipe | Unavailable: no second Windows machine is provided. Same-machine DNS and non-loopback addresses do not prove cross-machine access. |
| Linux/macOS runtime and native builds | Not executed, as requested on this Windows host. Backend resolution, candidate selection, deadlines, cancellation ownership, listeners and cloning are inspected. |

Documentation now describes deep versus shallow style copies, actual popup application, destination/default/validation rules, IPv6 authority syntax, Windows transport availability, automation versus protocol ports, and local Cli incompatibility. Updated Project, RemotingHelpers README, platform AGENTS and English/Chinese/macOS READMEs match the implemented entry points and both shell branches. The Linux libanl dependency is present in canonical Tools build guidance, synchronized Ubuntu tooling and wGac linkage.

Canonical knowledge is updated in Tools and propagated to GacUI and VlppOS with copilotInit.ps1; this also brings VlppOS's previously stale injection guidance up to the canonical version. VlppOS release files are regenerated with Release-Project, GacUI imports with Import-Project, and GacUI release files with Release-Project. Both platform import.sh workflows complete successfully. Their shared arguments and renderer GuiMain match canonical GacUI byte-for-byte; platform entry points are maintained separately. Only the shared-copy portion of syncProj is applied on Windows because its full workflow builds native generators; no Unix build/runtime result is claimed. Imported snapshots match canonical content after line-ending normalization. Authored, synchronized release/import/shared copies and rendering snapshots are kept in separate commits. Temporary application debugger arguments are restored; all owned test processes have exited. GacJS is unchanged.
