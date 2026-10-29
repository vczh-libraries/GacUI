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
