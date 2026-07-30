# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- New `RemoteViewModelTest` test besides `FullControlTest` and `RemoteProtocolTest`.
  - XML Resource stores in `Test\Resources\App\RemoteViewModelTest` and generated code in `Test\GacUISrc\Generated|RemoteViewModelTest` with a proper vcxitems project created.
  - `RemotingTest_RvmlHost` to host the view model class as a service being used in the main window.
  - `CppTest_Rvm` to start `RemoteViewModelTest` without remote protocol.
  - It requests the view model from `RemotingTest_RvmHost` first before creating the window.
  - The first version of `RemoteViewModelTest` will be a very simple app:
    - An text box to input name, when anything is changed, the label below says "Hello, <NAME>!".
    - The `IViewModel` inteface with `@rpc:Interface` and `@rpc:Ctor` has only one function `func Translate(name:string):string` to print the label.
    - `IViewModel` will be implemented and offered in `RemoteTest_RvmHost`, using generated `RemoteViewModelTestRpc.(h|cpp)` only to run the rpc client.
    - `CppTest_Rvm` will host `RemoteViewModelTest` run the rpc server, wait for `CppTest_Rvm` to connect, and then create the main window which has a parameter of `Ptr<IViewModel>` and start.
    - `CppTest_Rvm` and `RemoteTest_RvmHost` supports `/Http`, `/Pipe` and `/MiniHttp` just like `RemotingTest_(Core|Renderer_Win32)`.
    - `RemotingTest_RvmHost` disconnection will be treated as a fatal error, but such error will be ignored after the window is closed, aka after `GetApplication()->Run`.
- GacUI compiler processes Workflow RPC interfaces.
  - Checkout how `Workflow` knowledge base and learn how to know if RPC is used in any referenced workflow scripts.
  - When RPC is used:
    - In `GacGen`, generated RPC workflow script will be dumped to `WorkflowRpc.txt` next to `Workflow.txt`.
    - In `GacUICompiler` generated RPC workflow script will be dumped to `<NAME>.Rpc.txt` next to `<NAME>.(InstanceClass|TemporaryClass).UI.txt`.
    - In both `GacGen` and `GacUICompiler` The generated RPC workflow script will be generated to C++ stored at `<NAME>Rpc.(h|cpp)` next to `<NAME>(PartialClasses|Reflection).(h|cpp)`.
      - Unlike normal Workflow to C++ codegen, reflection file created from generated RPC workflow script will be ignored.
- `RemotingTest_*` test projects introduces RPC.
  - Add `/RVMT` to `RemotingTest_Core`
    - Only when `/RVMT` is used, it requests the view model from `RemotingTest_RvmHost` first before creating the window, working like `CppTest_Rvm` but this is a remote protocol enabled version.
    - After the window is created, it begins to accept renderers.
    - `RemotingTest_RvmHost` disconnection will be treated as a fatal error, unlike we can accept multiple renderers, but such error will be ignored after the window is closed, aka after `GetApplication()->Run`.
- Create `CppTest_Rvm` and `RemotingTest_RvmHost` linux projects with only vmake file.
- `DebugRemoteProtocol*.md` untouched as they will only be updated after a real `RemoteViewModelTest` is created.
- Update `README.md`, `Project.md` and `Test\GacUISrc\README.md` properly. Not to mention any actual details about `RemoteViewModelTest` but you can still points out `/RMVT` needs `RemotingTest_RvmHost` to connect first.

## DETAILS

### Canonical names and intended topology

- Keep the original bullets above as historical task text, but use the following canonical spellings everywhere in the implementation, generated artifacts, projects, command-line help, logs, and documentation:
  - `Generated_RemoteViewModelTest`, not `Generated|RemoteViewModelTest`.
  - `RemotingTest_RvmHost`, not `RemotingTest_RvmlHost` or `RemoteTest_RvmHost`.
  - `RemotingTest_Rendering_Win32`, matching the existing project name, not `RemotingTest_Renderer_Win32`.
  - `/RVMT`, not `/RMVT`.
- Interpret the contradictory `CppTest_Rvm` bullet as follows:
  - `CppTest_Rvm` runs the local `RemoteViewModelTest` UI and owns the network channel server, the JSON-RPC broker, and a local RPC requester endpoint.
  - `RemotingTest_RvmHost` is a network channel client and the semantic RPC service provider. It implements and declares `rvmt::IViewModel`.
  - `CppTest_Rvm` waits for `RemotingTest_RvmHost` to connect and declare the service, requests the service proxy, validates it, and only then constructs `rvmt::MainWindow`.
  - `RemotingTest_Core /RVMT` has the same RPC server/requester role as `CppTest_Rvm`, while separately remaining the remote-protocol server for a renderer.
  - Generated `RemoteViewModelTestRpc.h` and `RemoteViewModelTestRpc.cpp` are used by the host and both requester processes. The requesters compile the normal UI/resource output plus the RPC pair; the host compiles only `RemoteViewModelTestRpc.cpp` and uses the normal generated entry header transitively for `rvmt::IViewModel`. “RPC client” in the original host bullet refers to its transport/dispatcher endpoint, not to the semantic service consumer.

| Process and mode | Network-channel role | RPC role | Advertised logical channel |
| --- | --- | --- | --- |
| `CppTest_Rvm` | Server with local broker/requester clients | Requests `rvmt::IViewModel`; runs the local UI | Local RPC clients use `ViewModelChannel` |
| `RemotingTest_Core /RVMT` | One server shared by RPC and remote rendering | Requests `rvmt::IViewModel`; runs the remote-protocol core | Local RPC clients use `ViewModelChannel`; local core uses `GacUIRemoteProtocol` |
| `RemotingTest_RvmHost` | Network client | Registers and implements `rvmt::IViewModel` | Exactly `ViewModelChannel` |
| `RemotingTest_Rendering_Win32` | Network client | Remote-protocol renderer, not an RPC participant | Exactly `GacUIRemoteProtocol` |

### Channel identity and transport sharing

- Add one manually owned cross-project header at `Test/GacUISrc/RemotingTest_RvmHost/RemoteViewModelTestShared.h`. In namespace `vl::presentation::remote_view_model_test`, define `inline constexpr const wchar_t* ViewModelChannelName = L"ViewModelChannel"` and `inline constexpr const wchar_t* ViewModelServiceName = L"rvmt::IViewModel"`. Use those symbols in every RVM network/local channel map, lookup, and required-service list. List the header in the host, `CppTest_Rvm`, and Core projects/filters, and put its directory on the corresponding Linux include paths. Do not duplicate either literal or use a spelling variant.
- Keep the following identities distinct:
  - Logical channel name: `ViewModelChannel`.
  - Existing remote-rendering logical channel: `GacUIRemoteProtocolChannelName`, whose value is `GacUIRemoteProtocol`.
  - RPC service key: the fully qualified generated type name `rvmt::IViewModel`.
  - Physical transport address: the named-pipe name or HTTP/MiniHTTP base URL and port.
- `ViewModelChannel` is not a new pipe name, URL, or port. `CppTest_Rvm` and `RemotingTest_Core` use the same physical Pipe/HTTP/MiniHTTP endpoint values already used by the remoting pair. In `/RVMT`, the existing `RemotingTest_Core` channel server multiplexes the view-model host and renderer on the same listener. Do not create a competing RPC listener or a second MiniHTTP socket.
- The channel handshake is the exact-list role discriminator for network clients. Test `channelNames.Count() == 1` and then compare the sole name; a `Contains` check is insufficient:
  - A remote client advertising exactly `{ViewModelChannel}` is the one mandatory RVM host.
  - A remote client advertising exactly `{GacUIRemoteProtocol}` is a renderer.
  - Reject an empty list, an unknown-only list, both known names, either known name plus any unknown name, duplicates, or any other multi-name list.
  - Reject a second `ViewModelChannel` host after any host has been accepted during this process run; the mandatory host is not replaceable, even during fatal-shutdown latency after its current client ID has been cleared.
- Apply that exact-list network gate in phases:
  - Before the RVM host is reserved, accept only exactly `{ViewModelChannel}` and reject every other network connection, including an early `{GacUIRemoteProtocol}` renderer.
  - After the host reservation, `CppTest_Rvm` rejects every further network connection for the rest of the run.
  - After the host reservation, Core continues to reject every network connection until the host is registered, `rvmt::IViewModel` is acquired, and the main window is constructed. It then accepts only exactly `{GacUIRemoteProtocol}` through the existing renderer replacement path; every later list containing `ViewModelChannel` remains rejected.
- Refactor `RemotingChannelServerBase::OnClientConnected` so its admission decision branches on the advertised channel instead of unconditionally requiring `GacUIRemoteProtocol`. Do not send the RPC login or call broker `RegisterClient` from this pre-admission callback; its only RVM side effect may be the locked reservation needed to reject concurrent host candidates. Resolve the post-admission ordering review comment below first.
  - Local + `GacUIRemoteProtocol` is the Core local client; accept it without renderer replacement and require client ID 1.
  - Remote + `GacUIRemoteProtocol` is a renderer only in `RemotingTest_Core`; preserve the existing renderer gate/replacement path. `CppTest_Rvm` has no remote-protocol core and rejects this combination.
  - Local + `ViewModelChannel` + the concrete broadcasting/broker client is accepted without broker registration because it owns `RpcJsonDispatcherServer`.
  - Local + `ViewModelChannel` + the concrete requester client requires a ready broker and may be admitted. After the public `RpcJsonDispatcherClient::ConnectLocalServer(...)` helper returns a positive ID—therefore after its internal `IChannelServer::ConnectLocalClient(...)` call has committed the route—store that ID and call `RpcJsonDispatcherServer::RegisterClient`.
  - Remote + `ViewModelChannel` is the sole RVM host and requires a ready broker. Under the connection-state lock in admission, the first candidate atomically reserves its assigned ID and permanently closes per-run host admission before returning `Accept`; any concurrent candidate sees the reservation and is rejected. At the upstream guaranteed post-admission point, commit that reserved ID as the current host and call `RpcJsonDispatcherServer::RegisterClient`.
  - Reject all other local/remote/channel/type combinations.
  - Track the RVM-host and requester-local client IDs independently from `rendererClientId`.
  - Track a per-run `viewModelHostWasAccepted`/closed-admission reservation independently from the current committed host ID and never reset it. If the reserved connection disappears or admission cannot commit, treat it as loss of the one mandatory host and enter fatal startup rather than reopening admission. Clear only the current committed ID for broker cleanup.
  - Persist each committed role/client ID because the channel server removes its channel map before calling `OnClientDisconnected`.
  - Every endpoint passed to the RPC broker’s `RegisterClient` must be passed to `DisconnectClient` when that exact stored endpoint disconnects. This includes the requester-local client and RVM host, but excludes the broker local client, GacUI core, and renderer. Do not run renderer detach/replacement logic for an RPC client ID.
  - Make upstream broker registration idempotent and cancellation-aware as part of the post-admission fix. `RegisterClient` schedules login only when it newly inserts the ID and captures that ID's registration epoch. The queued task revalidates the same live epoch immediately before sending; `DisconnectClient` removes the ID and invalidates the epoch so a disconnect-before-task cannot deliver a stale login. A later client reusing an ID or a duplicate `RegisterClient` call must not revive or duplicate the old work.
- The channel implementation already restricts direct sends and broadcasts to clients advertising the same channel. Keep this isolation intact rather than adding a handwritten package-level discriminator.

### Workflow RPC detection and code generation

- Use the final `Workflow/InstanceClass` compilation as the authoritative input. Its retained lexical-scope manager contains the shared Workflow modules and generated instance-class modules used by the resource.
- Add one explicit final resource-precompile pass after the existing `Instance_CompileInstanceClass` pass. Name it `Instance_GenerateRpcMetadata`, advance `Instance_Max` and `Everything_Max` to it, document it as pass 8 in `IGuiResourceTypeResolver_Precompile`, and make only the XML/instance resolver return `PerPass` for it. Existing resolvers that do not participate continue to return `NotSupported`. Add `PRINT_PASS(Instance_GenerateRpcMetadata)` to GacUI_Compiler's `DebugCallback::PrintPassName`, add the corresponding case to GacGen's `Callback::PrintPass`, make all progress output report nine passes and identify the new final action as RPC metadata generation, and remove every hard-coded `1/8` through `8/8` denominator.
- Keep every original module in the authoritative manager. All ordinary `Workflow_GenerateAssembly` rebuilds remain in their existing passes but use `Rebuild(true, compilerCallback, false)`, deferring RPC validation until the new last pass. For the final InstanceClass manager this builds ordinary semantics exactly once; disabling the built-in per-module RPC pass must not change module partitioning, code indices, semantic scopes, assembly generation, Workflow dumps, or ordinary C++ generation.
- In `Instance_GenerateRpcMetadata`, retrieve the completed `Workflow/InstanceClass` object after every per-resource generation/compile pass has finished. Its `modules` list now exposes the complete final set, including every user-authored Workflow script plus generated instance-class declarations. Merge those already-analyzed `GuiInstanceCompiledWorkflow::modules` records into one lightweight aggregate `WfModule` solely for direct RPC validation:
  - If the compiled object is absent or `modules.Count() == 0`, leave `rpcMetadata` null, clear any retained virtual-position state, and finish successfully. Never index module 0 for an empty/no-Workflow resource.
  - Copy the first module's `moduleType` and `name` to the aggregate, then append every module's `paths` and `declarations` in the existing deterministic record order. Retain the original AST pointers so the authoritative manager's node scopes, type/reflection maps, virtual-script positions, and resource-error mapping remain applicable.
  - Do not add the aggregate to any manager and do not rebuild it. Call the exported `workflow::analyzer::ValidateModuleRPC(manager, aggregate)` exactly once; it traverses the complete declaration set using the semantic state already built for the original modules and produces one comprehensive `manager.rpcMetadata`.
  - Preserve exact virtual-script positions across the new phase boundary only after a successful `Instance_CompileInstanceClass`: on a pass-7 error, keep the existing translate-and-clear path because pass 8 will not run; on success, defer `Workflow_ClearScriptPosition`. Pass 8 must also handle a shared-only resource whose copied retained modules are nonempty while `Workflow_GetScriptPosition(context)` is null because earlier instance passes cleared it:
    1. Add/use an ensure helper so the position map exists before constructing `WorkflowVirtualScriptPositionVisitor`; make that visitor null-safe instead of dereferencing a missing `sp`.
    2. First call the ordinary module-position recorder for every retained module record, filling missing copied-node mappings from its original `ModuleRecord.position` without replacing exact records retained from instance generation.
    3. Then run virtual-node propagation with an overwrite-capable recording path for expanded expression/statement/declaration subtrees, so the source virtual node's precise record replaces any coarse mapping created during the first traversal.
    4. Validate, translate RPC errors through the original offending node, and call `Workflow_ClearScriptPosition` on every pass-8 exit.
  - Do not reduce a known script/attribute location to the aggregate root or a generic resource error.
  - Follow the merge shape demonstrated by Workflow's `MergeRpcSampleModulesForLog`, but add a regression that executes `Rebuild(..., validateRpc = false)` on the original modules followed by the direct aggregate `ValidateModuleRPC` call. The existing helper currently merges only the module used for a log and is not proof that the merged module was RPC-validated.
- Decide that RPC is present only after the ordinary rebuild and direct aggregate RPC validation both succeed:
  - Report and stop on `manager.errors` after each operation.
  - RPC is present only when both `manager.rpcMetadata` and `manager.rpcMetadata->metadataModule` are non-null.
  - Never scan source text, XML, `Workflow.txt`, comments, or string literals for `@rpc:Interface`.
- Keep the ordinary non-RPC path unchanged. RPC generation is an additional path and must not change normal Workflow assembly, binary resource, reflection, or C++ output.
- When RPC is present, keep the compiled InstanceClass type loader and the original final lexical-scope manager alive and unchanged throughout both normal and RPC C++ generation:
  1. Capture the validated `rpcMetadata` from the original manager and call both `workflow::analyzer::GenerateModuleRpc` and `workflow::analyzer::GenerateModuleRpcJson` with the `RemoteViewModelTestRpc`/`<NAME>Rpc` assembly name. The JSON module is mandatory for this task.
  2. Check errors and null results after each wrapper-generation call.
  3. Print the flat wrapper module followed by the JSON wrapper module, with deterministic section delimiters, into the requested RPC Workflow dump.
  4. Generate the ordinary C++ from the untouched original manager first and retain the returned `WfCppOutput`, especially its actual `entryFileName`. Check both the returned output and manager errors.
  5. Call `CopyAndClearRpcMetadata` without clearing or destructively reusing the original manager. Create a distinct `WfLexicalScopeManager` with the same parser and target architecture; add the copied-and-cleared metadata module first, the flat wrapper second, and the JSON wrapper third; then call `Rebuild(true, compilerCallback, false)` using the existing `IWfCompilerCallback*` terminology. Treat a null copied module or any RPC-manager error after rebuild as a generation failure.
  6. Do not feed all original resource modules to the RPC-only manager because that duplicates their C++ declarations. The ordinary output remains the owner of resource/UI/interface declarations; the RPC output owns only generated RPC support, while the still-active InstanceClass type loader makes types such as `rvmt::IViewModel` available as reflected dependencies.
  7. Create a fresh `WfCppInput` for RPC output with `multiFile = Disabled`, `reflection = Disabled`, and `defaultFileName`/assembly name `<NAME>Rpc`. Add the retained normal output’s `entryFileName + ".h"` as a normal include, rather than assuming the same entry-header name for GacGen and GacUICompiler.
  8. Call `GenerateCppFiles` with the RPC-only manager and explicitly reject a null result or any manager error. Require the generated global entry to be `vl_workflow_global::<NAME>Rpc::Instance()`.
  9. Require the RPC C++ filename set to be exactly `<NAME>Rpc.h` and `<NAME>Rpc.cpp`. Do not generate, include, compile, or register `<NAME>RpcReflection.*` or `<NAME>RpcIncludes.h`. This does not suppress the resource’s normal `<NAME>Reflection.*` output.
- Output mapping:

| Generator | Ordinary dump/output retained | Additional RPC output |
| --- | --- | --- |
| `GacGen` | `.log/x32|x64/Workflow.txt`, configured `<NAME>PartialClasses.*`, `<NAME>Reflection.*`, and resource output | `.log/x32|x64/WorkflowRpc.txt`; `<NAME>Rpc.h/.cpp` in the configured source folder normally, or `.log/x32|x64/Source` in partial mode |
| `GacUI_Compiler` | `<NAME>.Shared.UI.txt`, `<NAME>.TemporaryClass.UI.txt`, `<NAME>.InstanceClass.UI.txt`, normal `<NAME>*` C++, reflection, and binaries | `<NAME>.Rpc.txt` in each `Resource_x86|x64` folder and `<NAME>Rpc.h/.cpp` in each `Source_x86|x64` folder |

- Gate the complete GacGen RPC path, including `WorkflowRpc.txt`, on `GacGenConfig/Cpp`: `<NAME>` and the ordinary entry header are defined only by that configuration. If validated RPC metadata exists without `GacGenConfig/Cpp`, emit a clear configuration diagnostic and produce no ambiguous RPC dump or C++ files.
- Generate and validate new RPC content in memory before replacing a successful pair, so existing generated user regions can still be merged. In normal mode, when the current generator configuration still supplies `<NAME>` and the output folder, a no-RPC input or compilation failure deletes the known RPC dump and configured RPC source pair so no stale successful-looking artifact survives. In GacGen partial mode, clean/replace only `WorkflowRpc.txt` and the pair under `.log/x32|x64/Source`; never mutate or delete the configured production-source pair. Cleanup in the selected output scope must also remove legacy stale `<NAME>RpcReflection.h/.cpp` and `<NAME>RpcIncludes.h` files if an earlier implementation produced them. Do not claim GacGen can locate an old configured-source pair after the entire `GacGenConfig/Cpp` block, including its name/folder, has been removed unless a prior-output manifest is deliberately added and read before log cleanup.
- Keep RPC output deterministic:
  - Flat module before JSON module in the dump.
  - Same filename set on x86 and x64.
  - Keep GacUI application output in its existing separate `Source_x86` and `Source_x64` trees; do not merge the RemoteViewModelTest RPC pair into one tree.
  - At a call site that intentionally emits one shared cross-architecture source tree, use only `MergeCppMultiPlatform` and treat any unsupported architecture difference as a generator failure.
  - Preserve existing generated user regions in each written output through `MergeCppFileContent`.
- Add reusable compiler helpers in `Source/Compiler/GuiCppGen.*` rather than duplicating the detection/link/write algorithm between `Tools/GacGen/GacGen/Main.cpp` and `Test/GacUISrc/GacUI_Compiler/ResourceCompiler.cpp`.
- Extend `GacUI_Compiler` to compile `RemoteViewModelTest` for x86 and x64, write and clean all RPC outputs, and unload the resource/type loader consistently with the existing generated applications.

### `RemoteViewModelTest` resource and generated contract

- Create `Test/Resources/App/RemoteViewModelTest/Resource.xml` with a `GacGenConfig/Cpp` entry named `RemoteViewModelTest`.
- Use Workflow namespace `rvmt` consistently in XML, generated C++, service lookup, host implementation, and both requesters.
- Declare exactly:
  - `IViewModel` with `@rpc:Interface` and `@rpc:Ctor`.
  - `func Translate(name : string) : string`.
- Declare the main-window dependency with `ref.Parameter`, following the existing resource pattern, so generated C++ exposes `rvmt::MainWindow(Ptr<rvmt::IViewModel>)`. Do not use a service locator or global view-model pointer.
- The resource contains one single-line text box and one label. On `TextChanged`, call the injected view model’s synchronous `Translate` method and assign the returned string to the label. Initialize the label through the same translation path for the empty string so the deterministic initial value is `Hello, !`.
- The host implementation returns exactly `L"Hello, " + name + L"!"`. It must not duplicate formatting in the UI process.
- Keep the first version limited to this interface method and UI. Do not add events, state synchronization, reconnection, or unrelated sample features.

### Required `vl::rpc_controller::RpcJson*` stack

- Follow the Workflow repository’s `ChatBotServer`, `ChatBotClient`, and generated `ChatBotJsonDispatcherClient` unit-test pattern. Do not create a separate RPC envelope, serializer, object registry, or request router.
- Use:
  - `vl::rpc_controller::RpcJsonDispatcher`.
  - `vl::rpc_controller::RpcJsonLifecycle`.
  - `vl::rpc_controller::channeling::RpcJsonDispatcherClientForTaskQueue` for endpoint dispatchers.
  - `vl::rpc_controller::channeling::RpcJsonDispatcherServerForTaskQueue` for the server-side broker.
  - The existing JSON network/local channel client and server aliases over `Ptr<glr::json::JsonNode>`.
- Create one generated-app-specific dispatcher subclass. Its `InitializeRpc(clientId)` must:
  1. Construct `RpcJsonDispatcher(clientId, this)` and `RpcJsonLifecycle(clientId, dispatcher)`.
  2. Call `SetRpcObjects`.
  3. Install the generated `rpc_GetIds` map.
  4. Register the generated JSON serializer, object operations, object-event operations, and caller operations.
  5. Register the generated type-ID callback, event attacher, and wrapper factory through `rpcwrapper_GetTypeId`, `rpclistener_Attach`, and `rpcwrapper_Create`.
- Preserve the process-specific local-client order before common requester initialization:
  - In `CppTest_Rvm`, start the channel server, connect the broadcasting/broker local `ViewModelChannel` client, create `RpcJsonDispatcherServerForTaskQueue`, attach it to the role-aware channel server, and then connect the requester local client.
  - In Core `/RVMT`, start the channel server, first connect the existing local GacUI remote-protocol core client and assert client ID 1, then connect the broadcasting/broker local `ViewModelChannel` client, create/attach `RpcJsonDispatcherServerForTaskQueue`, and finally connect the requester local client. Never let an RPC endpoint consume ID 1.
  - The requester endpoint lists `rvmt::IViewModel` as required. On its dispatcher, call `SetServerLocalClientId(brokerDispatcher->GetServerClientId())`; do not mutate the broker’s ID. A remote RVM host learns this ID from the broker login and must not call `SetServerLocalClientId`.
  - Start the owned RPC `TaskQueue` thread, call requester `Initialize()`, wait for the service declaration, request it through `RequestService`, cast to `rvmt::IViewModel`, and fail if null.
- In `RemotingTest_RvmHost`:
  1. Create a network channel client that advertises only `ViewModelChannel`.
  2. Let `OnConnected(clientId)` initialize the generated-app dispatcher objects.
  3. Wait for the server and validate the connected status.
  4. Resolve the generated type ID for `rvmt::IViewModel`.
  5. Register the local implementation before calling dispatcher `Initialize()`, so its service declaration is sent correctly.
  6. Enter one owner-thread try/finally cleanup scope before calling dispatcher `Initialize()`. A disconnect/read/fatal-error callback must first invoke the same thread-safe terminal-failure operation on this host dispatcher, waking `WaitForServerClientId()` if the connection dies before broker login. It queues task-loop exit only when synchronized state says `RunTaskQueue` has begun; merely queuing exit cannot unblock an owner thread that has not reached the loop.
  7. After `Initialize()` succeeds, acquire the same lifecycle lock used by terminal callbacks. If terminal failure is already recorded, release the lock and take cleanup without entering the loop; otherwise set `taskLoopRunning`, release the lock, and immediately call `RunTaskQueue` on the owning thread so incoming `Translate` calls execute serially. A callback under that lock records failure and queues one exit task iff `taskLoopRunning` is set. Thus a callback before the handshake makes the owner skip the loop, while one after the handshake leaves a queued exit that the imminent/running loop consumes.
- A `TaskQueue` is required on both sides because `RpcJsonDispatcher*ForTaskQueue` schedules incoming RPC work there. The GUI process must use a dedicated owned queue thread because the GUI owner thread runs `GetApplication()->Run`. Queue exit, join, and object lifetime must be explicit.

### Startup, renderer admission, and process lifecycle

- Required launch order is:
  - Start `CppTest_Rvm` or `RemotingTest_Core /RVMT` first.
  - In Core, start the channel server and immediately connect the existing local remote-protocol core client, asserting that it receives client ID 1, before announcing that the server is ready for external clients.
  - Connect the RPC broker/requester local clients and print a distinct “waiting for `RemotingTest_RvmHost` on `ViewModelChannel`” readiness message.
  - Start `RemotingTest_RvmHost` with the same transport. It connects and declares `rvmt::IViewModel`.
  - The requester obtains a non-null proxy and the UI process constructs its main window.
  - For `/RVMT`, after construction and under the same connection-state lock, revalidate that the mandatory host remains connected and no fatal state was saved. If valid, atomically change phase from `Starting` to `Running` and open renderer admission; only then print the renderer-ready message. If invalid, keep admission closed, destroy/abandon the unrun window, and take the startup-failure path. Start a fresh renderer only after the message.
- Do not instruct users to prelaunch `RemotingTest_RvmHost`. The existing renderer protocol targets local core client ID 1, while the channel server assigns IDs before its acceptance callback; a racing network client could otherwise consume ID 1. “RvmHost connects first” means before window creation and before a renderer, not before the server process reserves its local clients.
- The Core server must already be running to accept the RVM host, so “begin to accept renderers after the window is created” is a role-specific admission gate, not a delayed `channelServer.Start()`:
  - `/RVMT` starts with renderer admission closed.
  - An early renderer is rejected and must be relaunched after readiness.
  - `/FCT` and `/RPT` preserve their current immediate renderer admission.
- Keep the existing renderer replacement behavior:
  - A new renderer replaces/detaches the old renderer.
  - Renderer loss is nonfatal to Core and does not affect the RVM host or broker.
  - RVM-host loss is fatal and the host is never replaced/reconnected within the same run. This is terminal failure propagation, not a recovery path.
- Protect connection IDs, phase, and one-shot fatal state with the existing locking conventions. Use at least `Starting`, `Running`, and `Stopping` phases:
  - Host loss during `Starting` aborts startup without creating a window or accepting a renderer.
  - Host loss during `Running` records exactly one fatal error, triggers the requester dispatcher's terminal-failure wake-up, and marshals close/termination handling to the GUI owner thread. A channel callback must never manipulate GUI objects directly.
  - Set `Stopping` immediately when `GetApplication()->Run` finishes by return or exception, before any RPC or transport teardown. Host disconnect/read errors after this transition are normal shutdown and must be ignored.
- Put an owner-thread exception/RAII boundary around `GetApplication()->Run` so the process atomically enters `Stopping` whether `Run` returns normally or a terminal RPC exception unwinds it. Preserve that exception as the same one-shot fatal result and keep callback-owned objects alive through reporting and cleanup.
- Preserve a fatal error recorded during `Run` across the window-close transition. After `Run` finishes by normal return or fatal exception, atomically enter `Stopping` and branch before the normal renderer-stop notification:
  - On normal user close, send the existing `ControllerConnectionStopped`, then complete RPC and transport cleanup.
  - On saved RVM-host failure, do not send the normal renderer-stop first. Rethrow/report the saved failure while Core’s `protocolServer`, renderer connection, and channel server are still live so the existing `GuiMain` catch can `BroadcastError`; use a nested scope/RAII cleanup path so RPC and transport cleanup still follows that report. Any later normal-stop attempt during controller finalization may harmlessly observe an already disconnected renderer.
  - Record a nonzero process-result state in Core’s and `CppTest_Rvm`’s GUI catch/entry path and return it from the transport-specific entrypoint after cleanup. Printing from a `void GuiMain` while its setup wrapper still returns zero is not sufficient.
- `RemotingTest_RvmHost` must override disconnect/read/fatal-local-error callbacks. The base callbacks are insufficient for a service process waiting in dispatcher `Initialize()` or `TaskQueue::RunTaskQueue`; a terminal connection must fail its own dispatcher first and one-shot queue task-loop exit only when the locked `taskLoopRunning` state is set.
- Do not throw the Core/requester fatal exception directly out of `OnClientDisconnected`, `OnReadError`, or another transport callback. These callbacks may run on arbitrary threads, and such a throw neither unwinds a UI thread blocked in `Initialize()`/`Translate()` nor signals Workflow's service events or response semaphore; some transports also catch callback exceptions. After the upstream terminal-failure API in the open review comment exists, the callback records the fatal state, invokes that API for the requester, queues owner-thread close handling for the idle case, and returns normally. Blocked and future RPC operations throw the saved exception on their own caller threads.
- Normal shutdown order must keep every callback target alive until its transport has drained:
  1. Mark the GUI process `Stopping`.
  2. On the normal-close branch only, preserve the existing Core-to-renderer `ControllerConnectionStopped` notification.
  3. Gate initiation of new UI `Translate` calls. During normal shutdown, complete accepted calls before finalization. On fatal host loss, immediately fail active and future calls through the upstream terminal-failure API in the open review comment; never silently discard dispatcher `ScheduleTask` callbacks that might need a response.
  4. After all accepted dispatcher work is quiescent, execute `FinalizeRpc()` on the RPC `TaskQueue` owner and wait for that finalization barrier to complete.
  5. Stop the channel server/client so transport callbacks drain while all callback owners still exist.
  6. Queue task-loop exit, join its thread, and only then destroy dispatchers, channels, callbacks, service/proxy objects, and loaded generated context.
- In `RemotingTest_RvmHost`, the owner-thread finally path calls `FinalizeRpc()` exactly once, stops/drains the client transport, and releases the service whether `Initialize()` throws before the task loop, `RunTaskQueue` starts and returns, or normal shutdown occurs. Do not require a nonexistent task-loop return on the pre-login-failure branch, do not finalize concurrently with callbacks, and do not leave a queued exit task or callback capture alive after dispatcher destruction.

### Command line, projects, and documentation

- Add `/RVMT` as Core constructor/application index 2. `/FCT`, `/RPT`, and `/RVMT` are pairwise exclusive; the existing default remains `/FCT`.
- `CppTest_Rvm`, `RemotingTest_RvmHost`, and Core continue to require exactly one of `/Pipe`, `/Http`, or `/MiniHttp`. Reject duplicate, mixed, missing, and unknown switches with a diagnostic and nonzero exit.
- `/Pipe` and `/Http` remain Windows-only. `/MiniHttp` remains the portable transport.
- Preserve the existing physical endpoint values. In MiniHTTP mode, reuse the exact same async socket server for the channel endpoint and automation service; do not bind the same port twice.
- Expose local UI automation as `Automation/CppTest_Rvm`. Preserve Core control-tree automation on port 8888 and renderer DOM automation on port 8889.
- Create/wire the following Windows artifacts:

| Area | Required changes |
| --- | --- |
| Resource | `Test/Resources/App/RemoteViewModelTest/Resource.xml` and any directly referenced resource XML |
| Generator | Add the resource XML to `Test/GacUISrc/GacUI_Compiler/GacUI_Compiler.vcxproj`; create a `Resource Files\RemoteViewModelTest` filter in its `.filters`; add the compile flag/path helpers and x86/x64 invocation |
| Shared constants | `Test/GacUISrc/RemotingTest_RvmHost/RemoteViewModelTestShared.h`, explicitly listed by every Windows consumer |
| Generated shared project | `Test/GacUISrc/Generated_RemoteViewModelTest/Generated_RemoteViewModelTest.vcxitems` and `.vcxitems.filters`, with the actual normal and RPC generated files |
| Direct app | `Test/GacUISrc/CppTest_Rvm/CppTest_Rvm.vcxproj`, `.vcxproj.filters`, owned sources/resources, architecture-selected Generated_DarkSkin and generated-RVM imports/include paths, GacUI library references, and required macros |
| Service host | `Test/GacUISrc/RemotingTest_RvmHost/RemotingTest_RvmHost.vcxproj`, `.vcxproj.filters`, owned RPC/transport sources, architecture-conditioned direct compilation of only `RemoteViewModelTestRpc.cpp`, the matching generated include path, required library references, and required macros |
| Existing Core | Import generated RVM, add RPC sources/glue and `/RVMT`, and retain existing FCT/RPT imports |
| Solution | Add unique GUIDs and exact nesting/shared-items wiring in `Test/GacUISrc/GacUISrc.sln` |

- Generate first, then enumerate the actual `Source_x86` and `Source_x64` file sets in `Generated_RemoteViewModelTest.vcxitems` and `.filters`. Do not use wildcards in Visual Studio projects. Exclude the opposite architecture in Debug/Release × Win32/x64.
- In both new `.vcxproj` files, define `VCZH_DEBUG_NO_REFLECTION` in all four configurations and `VCZH_CHECK_MEMORY_LEAKS` in both Debug configurations. In `CppTest_Rvm`, add the matching architecture-conditioned `Generated_DarkSkin/Source_x86|Source_x64` include paths as well as the RemoteViewModelTest paths.
- Import the full generated shared-items project only into `CppTest_Rvm` and `RemotingTest_Core`, which need the normal window/resource output and RPC pair. Do not import it wholesale into `RemotingTest_RvmHost`; that would compile the full UI/resource/reflection source set into a service-only process.
- In `GacUISrc.sln`, using the actual newly generated unique GUIDs:
  - Give `CppTest_Rvm.vcxproj` and `RemotingTest_RvmHost.vcxproj` all Debug/Release × Win32/x64 `ProjectConfigurationPlatforms` mappings. A `.vcxitems` project does not receive those mappings.
  - Nest `Generated_RemoteViewModelTest` under solution folder `Source Files` (`{1A152782-BECB-4D33-AD39-F54D3B234262}`), matching the existing generated projects.
  - Nest `CppTest_Rvm` under `GacUI_Compiler_DependendTests` (`{F5C3C996-E0C1-41AB-BAB4-22D1AC513EA4}`), preserving that existing spelling.
  - Nest `RemotingTest_RvmHost` under `GacUI_Remoting_Tests` (`{1F207079-CD21-4802-8331-B391E27F6C0A}`).
  - Add `SharedMSBuildProjectFiles` entries for the generated shared project itself with value `9`, `CppTest_Rvm` with value `4`, and existing Core (`{337E0972-C4CE-4863-916E-C1CD395DA085}`) with value `4`.
  - Do not add a generated-RVM shared-items mapping for `RemotingTest_RvmHost`; it directly compiles only the selected architecture’s RPC `.cpp`.
- Do not add Metaonly or Reflection variants of `CppTest_Rvm`; they are outside this task.
- Create only these files for the two new Linux project folders:
  - `Test/Linux/CppTest_Rvm/vmake`.
  - `Test/Linux/RemotingTest_RvmHost/vmake`.
- Make the Linux `CppTest_Rvm` build real rather than reusing `Test/Linux/Main.cpp` unchanged: that generic stub constructs `demo::MainWindow` and cannot construct `rvmt::MainWindow(Ptr<rvmt::IViewModel>)`. Keep portable RPC/UI glue in neutral source files and add separate convention-compliant `Main.Windows.cpp` and `Main.Linux.cpp`, each with its platform guard. List both in `CppTest_Rvm.vcxproj/.filters`, exclude the Linux file from every MSVC configuration, and have `vmake` use `CPP_REMOVES` for the Windows entrypoint and `CPP_ADDS` for the Linux entrypoint. Its `vmake` must include Generated_DarkSkin and Generated_RemoteViewModelTest x64 sources/include paths plus `VCZH_DEBUG_NO_REFLECTION`.
- The host Linux `vmake` imports the host’s own portable project sources, the shared-header include path, and only `Generated_RemoteViewModelTest/Source_x64/RemoteViewModelTestRpc.cpp` plus its include path; it must not pull in the generated UI/resource/reflection sources.
- Extend the existing `Test/Linux/RemotingTest_Core/vmake` with the generated RemoteViewModelTest x64 sources/include path. Generate Linux `makefile`/`vmake.txt` only through `REPO-ROOT/.github/Ubuntu/build.sh`; never hand-edit them. The explicit “only vmake” rule means the two new project folders must remove their generated companions before commit. Core’s `makefile` and `vmake.txt` are already tracked, so regenerate and keep both synchronized with its changed `vmake`.
- Update documentation with only infrastructure-level information and the following exact coverage:
  - `Project.md`: protected generated folders (current lines 22–27), GacUI compiler XML-to-output map (108–118), Windows project/resource descriptions (149–160), automation/remoting topology (164–173), and Linux project list (183–191).
  - Root `README.md` remoting section (current lines 98–110): stop saying there are only two related projects; add `/RVMT`, the RVM host requirement conditional on that selector, and server → host → renderer launch order.
  - `Test/GacUISrc/README.md`: compiler/project list (current lines 14–21) and resource-to-generated-project map (65–72).
  - Include new resource/generated/project names, supported transports, and the `ViewModelChannel` versus `GacUIRemoteProtocol` distinction where connection instructions need it.
  - Do not describe the text box, label, greeting behavior, or other UI details.
- Keep every `DebugRemoteProtocol*.md` file byte-for-byte unchanged.

## VERIFICATION

### Compiler and generated-artifact verification

- Add focused automated compiler coverage for:
  - One valid Workflow module containing `@rpc:Interface`, `@rpc:Ctor`, and `rvmt::IViewModel`.
  - A non-RPC resource.
  - An empty/no-Workflow resource whose final compiled record is absent or has zero modules, proving pass 8 succeeds without indexing module 0 and leaves `rpcMetadata` null.
  - An unannotated interface.
  - Annotation-like text in comments and string literals, proving detection is metadata-based.
  - Invalid RPC annotations/signatures, proving normal diagnostics are emitted and no partial successful-looking output remains.
  - Valid RPC metadata in a GacGen resource without `GacGenConfig/Cpp`, proving a clear configuration error is reported and neither ambiguous dump nor C++ artifacts are written.
  - Two user-authored source modules that each define an RPC interface and are collected through different resource entries. Assert all ordinary passes use `validateRpc = false`, the authoritative InstanceClass manager retains both original modules through pass 7, and only pass 8 `Instance_GenerateRpcMetadata` makes one direct `ValidateModuleRPC` call over the complete aggregate declaration set. Assert both interfaces and deterministic IDs occur in `rpc_GetIds`, the flat wrapper, and the JSON wrapper, and prove an error in either original source module still maps to its original resource/script position.
  - A shared-only RPC resource with no generated instance module and a deliberately invalid RPC declaration. Clear the prior position map before pass 8, then prove copied shared modules recreate it without a null dereference and the diagnostic resolves to the exact original script location rather than the module tag or generic resource.
  - Adapt Workflow's existing merge-helper test shape but explicitly execute and assert the original-module `Rebuild(..., false)` plus direct aggregate `ValidateModuleRPC` sequence; logging a merged AST without validating it is not sufficient coverage. Compare the ordinary multi-module assembly, serialized binary/debug module-code layout, Workflow dump, reflection, diagnostics, and C++ output byte-for-byte with the pre-RPC-validation path.
  - Assert the precompile callback observes exactly nine ordered passes with `Instance_GenerateRpcMetadata` after `Instance_CompileInstanceClass`; GacUI_Compiler and GacGen both print the correct `9/9` final progress entry and contain no stale `/8` progress text. No per-resource collection runs in the last pass, nonparticipating resolvers ignore it, the final InstanceClass type loader remains active, and no RPC validation or metadata generation occurs in an earlier pass. A phase-8 error must abort before output writing and still identify the original resource location.
  - An RPC-to-non-RPC transition that retains the same C++ name/folder configuration, proving stale `.Rpc.txt`, `WorkflowRpc.txt`, and `<NAME>Rpc.h/.cpp` files are removed.
- For the positive case, assert:
  - Both flat and JSON generated Workflow modules appear in deterministic order in the RPC dump.
  - Generated C++ contains the ID map, wrapper factory, JSON serializer/object operations, event-attacher hook, and caller operations needed by the `RpcJson*` setup.
  - The only RPC C++ files are `<NAME>Rpc.h` and `<NAME>Rpc.cpp`.
  - No RPC reflection or includes file is produced or referenced, and deliberately seeded legacy `<NAME>RpcReflection.h/.cpp` and `<NAME>RpcIncludes.h` files are cleaned.
  - The RPC header includes the actual normal codegen entry header.
  - The generated global entry is exactly `vl_workflow_global::<NAME>Rpc::Instance()`.
  - The RPC-only output references but does not re-emit or define `rvmt::IViewModel`; the ordinary manager and ordinary `WfCppOutput` are byte-for-byte unchanged by RPC linking.
  - The compiled InstanceClass type loader remains installed through the RPC-only `Rebuild` and `GenerateCppFiles`, is unloaded afterward, and a sequential second-resource compile proves no leaked loader changes the next resource’s `CopyAndClearRpcMetadata` result.
  - The generated pair compiles and links in the service provider and requester consumers.
- Exercise both GacGen and GacUI_Compiler:
  - GacGen x86/x64 writes `WorkflowRpc.txt` in the matching log folders and the pair in the configured source folder; partial mode writes the pair to `.log/x32|x64/Source`, records the ordinary configured folder through its existing `CppOutput.txt`, and leaves a deliberately seeded configured production pair byte-for-byte unchanged.
  - GacUI_Compiler x86/x64 writes `<NAME>.Rpc.txt` and the pair beside each architecture’s normal output.
  - Existing DarkSkin, FullControlTest, and RemoteProtocolTest remain non-RPC and produce no RPC artifacts.
  - Run each generator twice and require no tracked change on the second run.
  - Require equal x86/x64 RPC filename sets and successful downstream compilation of each architecture. A test-only `MergeCppMultiPlatform` comparison may diagnose unexpected architecture differences, but must not replace the separate GacUI application source trees.
- Run the required metadata sequence before GacUI_Compiler:
  1. Build Debug Win32 and run `Metadata_Generate` Win32.
  2. Build Debug x64 and run `Metadata_Generate` x64.
  3. Run `Metadata_Test` x64.
  4. Run `GacUI_Compiler`, inspect every architecture output, then run it again for idempotence.
- Require no `*.UI.error.txt`/`*.UI.errors.txt`, generator error log, duplicate declaration, unresolved include, or stale RPC output.

### Build and project verification

- Use only `REPO-ROOT/.github/Scripts/copilotBuild.ps1` on Windows. Run it with cwd `Test/GacUISrc` for `GacUISrc.sln` and cwd `Tools/GacGen` for `GacGen.sln`. Inspect and preserve the evidence from `.github/Scripts/Build.log` immediately after every invocation because the next configuration overwrites that log.
- Build both solutions in Debug/Release × Win32/x64. Each result must end with `Build succeeded`, `0 Warning(s)`, and `0 Error(s)`.
- Run the metadata/compiler tools through `copilotExecute.ps1 -Mode CLI` with cwd `Test/GacUISrc`; require a zero process exit, inspect their live output, and inspect generated metadata/compiler diagnostic files because CLI mode does not create `Execute.log`. Run Debug x64 `UnitTest` through `-Mode UnitTest`, then inspect `.github/Scripts/Execute.log` for failed cases and memory-leak output. Ensure `.vcxproj.user` filters do not skip relevant tests.
- Verify the generated shared-items project’s explicit file list exactly matches the real x86/x64 generated source sets and that each consuming project receives only its selected architecture.
- Verify every new GUID is unique; only the two new `.vcxproj` projects have all four solution configuration mappings; the generated `.vcxitems` project has the required unique `ItemsProjectGuid`, nesting, and `SharedMSBuildProjectFiles` entries. Verify every manually owned source/resource/header is present in its `.vcxproj` and `.filters`, and the exact nesting/import mappings above are present.
- On Linux, run `REPO-ROOT/.github/Ubuntu/build.sh -f` while cwd is each of:
  - `Test/Linux/GacUI_Compiler`.
  - `Test/Linux/CppTest_Rvm`.
  - `Test/Linux/RemotingTest_RvmHost`.
  - `Test/Linux/RemotingTest_Core`.
  - `Test/Linux/UnitTest` after shared compiler/runtime changes.
- Inspect generated Linux source lists to ensure Core and `CppTest_Rvm` include Generated_DarkSkin and the required normal/RPC RemoteViewModelTest x64 sources and `VCZH_DEBUG_NO_REFLECTION`, while `RemotingTest_RvmHost` includes its own portable sources and only `Source_x64/RemoteViewModelTestRpc.cpp` plus the matching generated/shared include paths. Prove the direct app uses a valid build-only entrypoint rather than the incompatible generic `demo::MainWindow` stub. Ensure Windows-only transport sources are excluded and a second incremental build is a no-op.
- Keep the regenerated, already-tracked `Test/Linux/RemotingTest_Core/makefile` and `vmake.txt` synchronized with its `vmake`. Do not leave new generated `makefile`/`vmake.txt`, binaries, logs, or temporary fixtures tracked in either of the two new Linux project folders.

### Channel and RPC integration verification

- Add lower-level coverage for all three transport implementations:
  - A `ViewModelChannel` client is accepted as the RVM host and registered with the RPC broker.
  - A `GacUIRemoteProtocol` client follows only the renderer path.
  - Empty, unknown-only, duplicate, both-known, known-plus-unknown, and every other non-singleton channel list are rejected.
  - Before host reservation, the only accepted network list is exactly `{ViewModelChannel}`; an early exact renderer is rejected.
  - After host reservation, `CppTest_Rvm` accepts no more network clients. Core continues rejecting all network clients until service acquisition and window construction, then accepts only exactly `{GacUIRemoteProtocol}` and still rejects every later view-model-host list.
  - A second RVM host is rejected without disturbing the first; a barrier-driven pair of simultaneous candidates proves exactly one reserves admission before either post-admission hook runs.
  - Renderer replacement changes only `rendererClientId`.
  - RVM-host connection/disconnection changes only the stored host ID and broker registration while leaving the per-run host-admission latch closed.
  - Requester-local teardown calls broker `DisconnectClient` for its stored ID; broker-local, core, and renderer teardown never does.
  - Direct/broadcast channel traffic never crosses between the two logical channels.
- Verify the generated-app dispatcher follows the Workflow ChatBot sequence exactly:
  - `InitializeRpc` creates and registers `RpcJsonDispatcher`/`RpcJsonLifecycle` and all generated operations.
  - The broker local client connects before external RPC clients are registered.
  - The broker local client is accepted without registering itself; the requester local client and remote RVM host are registered only after the broker is ready.
  - `RegisterLocalService` happens before host dispatcher `Initialize()`.
  - The requester lists `rvmt::IViewModel` as required, waits for its declaration, requests it, and rejects a null/wrong-type result.
  - Every endpoint calls `FinalizeRpc()` before transport destruction.
- Add Workflow coverage for the terminal-failure operation required by the open disconnect review: failure while waiting for the server ID or required service, one and multiple blocked direct calls, failure before/after send, response-versus-failure races, future calls, repeated failure, and `FinalizeRpc` after failure. Each blocked caller must wake and throw the saved terminal error on its own thread; a throw from the transport callback or an abandoned blocked thread is not a passing result. In GacUI integration, disconnect `RemotingTest_RvmHost` before broker login and prove its own `Initialize()` wakes, reports failure, never enters or hangs in `RunTaskQueue`, finalizes exactly once on the owner thread, drains the transport, and leaves no queued-exit task, callback capture, or lifecycle leak. Add a barrier immediately after successful `Initialize()` but before the lifecycle-lock handshake; disconnect there and prove the owner observes terminal state and skips the loop. Release a second barrier immediately after `taskLoopRunning` is set but before `RunTaskQueue`; disconnect there and prove exactly one queued exit makes the loop return.
- After the post-admission ordering issue in the review comments is resolved upstream, repeatedly connect accepted local requesters and network hosts under an already-running RPC `TaskQueue`. Assert every one receives the broker login/server ID exactly once and only after its route exists, reaches `Initialize()` without a hang, and leaves no queued login targeting a rejected/disconnected ID. Include a server-plus-host case with no renderer or second connection, proving host initialization cannot depend on opening the renderer gate. Add duplicate-`RegisterClient`, disconnect-before-login-task, repeated disconnect, and client-ID-reuse cases; the epoch check must suppress stale/duplicate login while allowing exactly one login for the new registration.
- Verify startup diagnostics expose two observable readiness boundaries in `/RVMT`: waiting for RVM host, then ready for renderer.
- Start Core first in every test and assert its local GacUI core is client ID 1 before starting the host. Do not use a prelaunched-host test: the current channel-server API activates the listener before a local client can be connected and allocates network IDs before admission, so that race has no deterministic result. Treat the documented server-readiness boundary as a required precondition.

### Windows end-to-end matrix

- Run all multi-process applications through the repository execution/debugging workflow; do not invoke built executables directly. Attach a debugger to GacUI processes where required by the running guidelines.
- For each `/Pipe`, `/Http`, and `/MiniHttp`, verify `CppTest_Rvm`:
  1. Start `CppTest_Rvm`; confirm it reports waiting and creates no window.
  2. Start `RemotingTest_RvmHost` with the same transport.
  3. Confirm the service is declared/requested before the window appears.
  4. Through `Automation/CppTest_Rvm`, assert initial `Hello, !`, `Alice` produces `Hello, Alice!`, Unicode input round-trips exactly, and rapid repeated edits leave the last matching greeting.
  5. Close the UI first; confirm server shutdown disconnects the host without a fatal UI error, all processes exit, and the pipe/port is reusable.
  6. Stop the host while the UI is idle and while edits are active; require one fatal outcome, a nonzero UI-process exit code, and no hang or use-after-free.
- For each transport, verify `RemotingTest_Core /RVMT`:
  1. Start Core; confirm the RVM-host readiness message and closed renderer gate.
  2. Attempt an early renderer and prove it is rejected.
  3. Start `RemotingTest_RvmHost`; wait for the renderer-ready message, then start a fresh renderer.
  4. Confirm Core uses `ViewModelChannel` for the host and `GacUIRemoteProtocol` for the renderer on the same listener.
  5. Edit through Core control-tree automation on port 8888 and renderer DOM automation on port 8889. After each direction, both views must converge on the same translated label.
  6. Disconnect and reconnect renderers repeatedly. Core and RVM host must remain alive, and only the old renderer is detached.
  7. Disconnect the RVM host while `Run` is active. Core must broadcast the saved fatal error to the still-connected renderer before teardown, return a nonzero process exit code, and terminate cleanly without first sending a normal `ControllerConnectionStopped`.
  8. Close the window first, then let transport teardown disconnect the host. The post-Run callback must be ignored as normal shutdown.
  9. While fatal shutdown from host loss is pending, attempt another `ViewModelChannel` host and prove it is rejected; stale service declarations/proxies must never be reused.
- Run the command-line negative matrix:
  - Pairwise/multiple `/FCT`, `/RPT`, `/RVMT`.
  - `/RVMT` accepted and `/RMVT` rejected.
  - Missing, duplicate, and mixed transport switches.
  - Unknown switches.
  - `/Pipe` and `/Http` on Linux report their platform restriction and do not fall through to MiniHTTP.
- Regression-smoke `/FCT` and `/RPT` on all transports after `/RVMT` changes. Confirm their immediate renderer admission, renderer replacement, control/DOM synchronization, automation ports, and shutdown behavior are unchanged.
- Repeat normal close, renderer replacement, host loss while idle, and host loss during rapid translation at least five times per transport. Require no stale process, blocked task queue, leaked callback, reused client ID, occupied pipe/port, duplicate fatal report, or memory leak.

### Linux runtime and repository verification

- Linux must at minimum build all affected targets in Debug x64. Exercise the real `/MiniHttp` RVM host/Core path where the Linux harness provides a runnable entry point; do not claim local-UI coverage from the existing Linux CppTest stub.
- In a runnable MiniHTTP test, start Core/server first, then RVM host, then the available remote renderer/GacJS client. Verify service declaration/request, `ViewModelChannel` isolation, translation, host-loss fatal handling, and clean socket reuse.
- Review the final diff and generated state:
  - `DebugRemoteProtocol*.md` has no byte change.
  - The original task text above `## DETAILS` is unchanged.
  - New implementation/docs use only the canonical spellings; the intentionally preserved historical bullets are excluded from this spelling check.
  - Documentation contains only project/transport/startup information and no RemoteViewModelTest UI description.
  - No unrelated generated resources, metadata binaries, build outputs, logs, or temporary test fixtures are tracked.
  - `git diff --check` reports no whitespace errors.

## REVIEW COMMENTS

### RPC metadata from more than one Workflow module

**review comment**: `WfLexicalScopeManager::Rebuild` currently calls `ValidateModuleRPC` once per module, and each module containing RPC interfaces replaces `manager.rpcMetadata`. Therefore a resource with RPC definitions in two referenced Workflow modules can silently generate wrappers for only the later module. The task says to detect RPC in any referenced Workflow scripts, but it does not say whether all such modules must be aggregated or whether one RPC-definition module is an intentional restriction.

**suggested solution**: Prefer fixing the Workflow compiler upstream so RPC phase 1 aggregates candidates across every module, performs reflection validation and ID generation once over the complete set, and produces one deterministic metadata result; add a two-module regression test there and then update GacUI’s Workflow imports through the documented upstream-import process. If that expansion is not intended, expose a structured analyzed-AST/per-module signal or other upstream API with which GacUI can enforce exactly one RPC-definition module and emit a diagnostic. The final `rpcMetadata` cannot reveal an earlier overwritten module, so the restricted fallback must not infer this by scanning source text.

If you take a more careful look of Workflow compiler API you will know it is just possible to merge multiple WfModule into one and then do workflow RPC metadata generation. This is also reflected in one of the Workflow test case.

The resource compiler now adds a final `Instance_GenerateRpcMetadata` pass after `Instance_CompileInstanceClass`. All ordinary passes keep their original modules/output and rebuild with per-module RPC validation disabled; the new pass can see every completed user-authored Workflow script, merges their already-analyzed AST pointers into a transient root, and passes it once to exported `ValidateModuleRPC`. Verification uses two RPC-bearing resource entries and, unlike the existing log helper, exercises this last-pass aggregate-validation path, checks both generated wrapper families and original error positions, and compares all ordinary outputs byte-for-byte.

### Cancelling an in-flight synchronous call when the RVM host disconnects

**review comment**: `RpcJsonDispatcherClient::OnJsonRequest` waits for a direct-call response through its received-message semaphore. If `RemotingTest_RvmHost` disconnects after a `Translate` request is sent but before replying, the requester is still a connected local channel client; the broker’s `DisconnectClient` does not wake that wait or synthesize a response. A synchronous UI-thread `Translate` can therefore block forever and prevent the required fatal-disconnect shutdown. The same family of issue can leave requester initialization waiting forever if the host disconnects before declaring the required service. `RpcJsonDispatcherServer` also retains cached service declarations after a client disconnects, so reconnecting/reusing the old proxy is not a safe fallback.

**suggested solution**: Add an upstream Workflow `RpcJsonDispatcherClient` terminal-failure API that is thread-safe and idempotent, atomically makes future requests fail immediately, wakes required-service, server-ID where applicable, and every outstanding response waiter with one deterministic exception, and resolves response/disconnect/finalize races exactly once. Invoke it for the local requester only when the disconnected client ID equals the stored accepted RVM-host ID. Cover disconnect before declaration, one and multiple in-flight direct calls, response-versus-disconnect races, calls after cancellation, repeated cancellation, and `FinalizeRpc` after cancellation in Workflow tests; add a GacUI integration case proving a blocked UI `Translate` returns or throws and then shuts down. Update GacUI’s Workflow imports through the documented process. Avoid process-forced termination or an abandoned blocked thread because those would not satisfy the required clean shutdown and leak verification.

Any `RemotingTest_RvmHost` disconnection should be immediately generating a fatal error so there is no need to worry about how to recover, just throw an exception from there.

**review comment**: Treating RVM-host loss as terminal removes all recovery and reconnection requirements, but the exception cannot be thrown directly from the channel disconnect callback. That callback may execute on an arbitrary transport thread, while `Initialize()` or a synchronous `Translate()` is blocked on a different thread inside `RpcJsonDispatcherClient`. An exception only unwinds its current thread and does not signal the service events or response semaphore. Some transports explicitly swallow callback exceptions; on others it can bypass callback cleanup. Therefore throwing there can still leave the UI blocked or terminate a transport worker without reaching the `GuiMain` fatal-reporting path. The host process has the symmetric pre-login problem: its owner thread can be blocked in dispatcher `Initialize()` waiting for the broker ID, so queuing only `TaskQueue` exit cannot help before `RunTaskQueue` begins.

**suggested solution**: Keep host loss terminal and nonrecoverable, but add a thread-safe, idempotent upstream `RpcJsonDispatcherClient` failure operation. The disconnect callback records the fatal error and calls this operation without throwing. It wakes every initialization and direct-call wait; each wait checks the terminal state before and after blocking and throws the saved exception on its own caller thread. Future RPC calls fail immediately. Core invokes it on its requester only for the exact accepted RVM-host ID and ignores expected teardown after entering `Stopping`; `RemotingTest_RvmHost` invokes it on its own dispatcher for terminal connection callbacks before queuing task-loop exit. Verify both sides' initialization failure, one and multiple blocked calls, response-versus-failure races, repeated failure/finalization, and host loss during UI `Translate`.

### Sending RPC broker login before an accepted channel route exists

**review comment**: `NetworkProtocolChannelServer` and its local-client path invoke application `OnClientConnected` before installing the accepted client ID and channel routes. The task must register each accepted endpoint with `RpcJsonDispatcherServer`, but the existing ChatBot-style server performs that registration from this callback. `RegisterClient` schedules `SendLoginMessages` on the already-running RPC `TaskQueue`; the task can execute before the callback returns and direct-send to an ID the channel server cannot route yet, so an accepted requester or RVM host can miss its broker/server-ID login and block forever. Re-queuing the registration task again is not a proven ordering barrier.

**suggested solution**: Add an upstream channel-server post-admission hook that runs only after the connection/local-client route and channel names are committed and the client-side acceptance notification is ordered before application data, or buffer direct sends until that state and acknowledge the broker login deterministically. Perform network-client RPC `RegisterClient` from that guaranteed point while retaining the earlier callback only for admission and locked role reservation, never channel/RPC sends; local caller-owned registration may run after `ConnectLocalServer` returns. Add Workflow/VlppOS tests that run the task queue concurrently and stress repeated local and network admissions, proving each accepted client receives exactly one login after its route exists and rejected/disconnected IDs receive none; then update GacUI’s imports through the documented upstream-import process.

`IChannelServer` gives you a callback to reject a connection, so before `RemotingTest_RvmHost` is connected (by knowing the only channel available is `ViewModelChannel`), just reject every comming connection. And later only accept who's only available channel is `GacUIRemoteProtocol`. This makes the first test app easier.

**review comment**: The exact-single-channel gate is useful for role and phase admission, and the task now requires it. It does not order the broker login after the accepted route. For a network client, `NetworkProtocolChannelServer` still calls `OnClientConnected` before adding the client to `connections` and `clientChannels`. `RpcJsonDispatcherServer::RegisterClient` can therefore queue and execute `SendLoginMessages` while the accepted `ViewModelChannel` client is still unroutable. Rejecting other channel roles does not delay that task. A later `GacUIRemoteProtocol` connection cannot be the barrier because the RVM host must receive its login and declare `rvmt::IViewModel` before the window and renderer gate become ready, and `CppTest_Rvm` has no renderer connection. In addition, current `RegisterClient` schedules a login even for an already-registered ID, and `DisconnectClient` does not invalidate a login task that was already queued, so post-admission ordering alone would still permit duplicate or stale sends.

**suggested solution**: Apply the requested exact-singleton admission phases, but retain the upstream post-admission fix: reserve the sole `ViewModelChannel` host in the admission callback, register it only after its route/channel map is committed, and open Core's exact-singleton `GacUIRemoteProtocol` gate only after service acquisition and window construction. Keep local requester registration after the public `RpcJsonDispatcherClient::ConnectLocalServer(...)` helper returns. Make broker registration idempotent and epoch-based: schedule login only for a newly inserted ID, capture its registration epoch, revalidate that live epoch in the queued task, and invalidate it in `DisconnectClient`. Add a server-plus-host test with no renderer or second connection while the RPC task queue runs concurrently, plus duplicate-registration and disconnect-before-task races, proving the host receives exactly one login after route commitment and reaches service declaration without a hang.

### Detecting RVM-host loss on logical HTTP transports

**review comment**: The task requires RVM-host loss to become fatal under `/Pipe`, `/Http`, and `/MiniHttp`, including the end-to-end step that stops the host while a call is active. Named pipes expose OS peer disconnect, but the two HTTP network protocols define only `/Connect`, `/Request`, and `/Response`. A client stopping or dying does not remove its server-side logical token. A failed or replaced long poll is intentionally not a logical disconnect: the legacy HTTP server retains the connection, and MiniHTTP requeues unsent output and waits for a replacement poll. Consequently Core can retain the host client ID forever, never enter `OnClientDisconnected`, and never trigger either the fatal exception or the requester wake-up. Workflow's existing `NotifyServerClientDisconnected()` sends an in-band Logout, but the broker consumes it only as `DisconnectClient`; it does not notify the application, and it cannot cover abrupt process death.

**suggested solution**: Define both graceful and abrupt host-loss semantics upstream. On graceful host shutdown after broker login, require `NotifyServerClientDisconnected()` before transport stop and add a one-shot broker/application logout callback so Core enters the same fatal requester-failure path, deduplicated by the exact host ID and fatal latch against a later channel callback. Because `NotifyServerClientDisconnected()` itself waits for the broker ID, a pre-login shutdown must instead use a new nonblocking/idempotent `TryNotifyServerClientDisconnected()` or skip Logout and rely on terminal dispatcher failure plus the lease; it must never block shutdown waiting for the missing login. For abrupt `/Http` and `/MiniHttp` loss, add an opt-in heartbeat/lease with a documented bound: after the accepted host registers its first poll, the server periodically completes the current `/Request` with the existing empty successful response that both clients filter, the client immediately opens a replacement poll for the same token, each replacement renews the lease, and expiry without renewal stops the logical connection and emits exactly one `OnDisconnected`. Verify pre-login stop does not hang; poll replacement within the grace interval is nonfatal; graceful Logout after login is immediate; client stop/kill without replacement fires once; later lease/channel notifications do not duplicate it; server shutdown after `Stopping` is ignored; and a mid-`Translate` kill wakes the caller, reports the fatal error, returns nonzero, and cleans up. If abrupt loss is intentionally out of scope, narrow every “host disconnection/stop” requirement to controlled graceful Logout instead of claiming all client loss.

# TEST [CONFIRMED]

The fresh baseline has no `RemoteViewModelTest` implementation and no final RPC-metadata resource-precompile phase. The focused root reproducer is `Test/GacUISrc/UnitTest/TestResource_Rpc.cpp`:

- Build `Test/GacUISrc/GacUISrc.sln` in Debug x64 so the new test compiles against the current public resource-compiler API.
- Run only `TestResource_Rpc.cpp`.
- Assert that `IGuiResourceTypeResolver_Precompile::Everything_Max` is `8`, meaning pass indexes `0` through `8` exist and RPC metadata generation is the ninth and final pass.
- The current value is `7`. The test must fail on the untouched baseline and pass only after `Instance_GenerateRpcMetadata` is implemented.

The proposal confirmation must expand this focused reproducer into the complete automated coverage required by the problem description:

- GacUI compiler tests cover ordered pass execution; valid, non-RPC, empty, invalid, shared-only, and two-module aggregate RPC metadata; source-position preservation; deterministic flat/JSON wrapper generation; exact generated filename sets; stale-output cleanup; configuration errors; type-loader lifetime; and unchanged ordinary output.
- Workflow tests cover terminal failure while waiting for broker ID, required services, and one or multiple direct-call responses; failure before/after send; response/failure races; future calls; repeated failure/finalization; graceful logout notification; and idempotent epoch-checked broker login scheduling.
- VlppOS tests cover post-admission ordering for local and network clients, simultaneous/repeated/disconnected/client-ID-reuse admissions, logical-channel isolation, and opt-in HTTP/MiniHTTP heartbeat/lease behavior for poll renewal, graceful stop, abrupt death, deduplicated disconnect, and server shutdown.
- GacUI integration tests cover exact-single-channel role gates and phases, one-shot host reservation, independent broker/requester/host/renderer IDs, renderer replacement, service acquisition, dispatcher setup/finalization, startup barriers, terminal host loss, and no cross-channel traffic.
- Generator, project, command-line, and end-to-end checks cover both GacGen and GacUI_Compiler, Debug/Release × Win32/x64, Linux Debug x64, `/Pipe`, `/Http`, and `/MiniHttp`, the negative option matrix, automation-visible translation, normal close, renderer replacement, host loss while idle/in-flight, nonzero fatal exits, port reuse, repeatability, memory leaks, and unchanged `DebugRemoteProtocol*.md`.

Success requires all focused and regression tests to pass, every required build to end with zero warnings/errors, all generated outputs to be deterministic on a second run, no memory-leak report, no stale process/port/artifact, `git diff --check` success, and byte-for-byte preservation of files explicitly required to remain unchanged.

The baseline reproducer was confirmed on 2026-07-29:

- `copilotBuild.ps1 -Configuration Debug -Platform x64` built `Test/GacUISrc/GacUISrc.sln` successfully with 0 warnings and 0 errors.
- `copilotExecute.ps1 -Mode UnitTest -Executable UnitTest -Configuration Debug -Platform x64`, filtered to `TestResource_Rpc.cpp`, failed only at `IGuiResourceTypeResolver_Precompile::Everything_Max == 8`.
- The observed value is the current public constant `7`, so the repository demonstrably has only passes 0 through 7 and lacks the required final pass 8.
- The focused run produced no memory-leak report and no unrelated test or snapshot changes.

# PROPOSALS

- No.1 Add a final aggregate RPC phase and a terminal role-aware remoting stack

## No.1 Add a final aggregate RPC phase and a terminal role-aware remoting stack

Implement the requested application as one layered change whose ordering constraints are explicit and independently testable.

First, repair the two upstream primitives on which a deadlock-free RVM topology depends:

1. In Workflow, replace the shared direct-response semaphore with request-specific pending records. Add a first-wins, thread-safe `FailRpc` operation that never throws on the callback thread, saves one terminal message, fails and wakes every pending request, wakes broker-ID and required-service initialization waits, makes future requests fail immediately, ignores late responses after terminal failure, and leaves `FinalizeRpc` idempotent after callers have joined. Keep nested direct requests executable on the blocked caller thread. Add nonblocking, one-shot `TryNotifyServerClientDisconnected`, and make the existing blocking notification failure-aware.
2. In Workflow's broker, make registration idempotent and epoch-checked. Schedule login only for a newly inserted client ID, capture its epoch, revalidate the epoch before sending, and invalidate it on disconnect so duplicate registration, disconnect-before-task, and client-ID reuse cannot produce duplicate or stale login. Remove cached service declarations owned by a disconnected client. Treat an in-band Logout as a one-shot disconnect transition and invoke an application callback directly outside the broker lock.
3. In VlppOS, add a no-op `NetworkProtocolChannelServer::OnClientAdmitted` hook. Invoke it only after a local or network route and channel list have been committed, after the local acceptance callback or network assigned-ID acknowledgement has been ordered, and outside internal locks. Coordinate disconnect races so a rejected or uncommitted connection gets no hook and a committed connection gets at most one hook followed by at most one disconnect callback.
4. Add a disabled-by-default, per-connection HTTP polling lease capability to both Windows HTTP and portable MiniHTTP server connections. A valid replacement poll renews the lease; an idle heartbeat completes the current poll with the already-supported empty successful response; expiry idempotently stops the logical connection and reports one disconnect. Enable it only for the accepted remote `ViewModelChannel` host so renderers sharing the listener retain their existing behavior. Use one deadline scheduler or injected clock rather than busy polling or fixed-sleep tests.

Cover these primitives upstream before importing them. Workflow tests will exercise broker-ID, required-service, single-call, multi-call, nested-call, response-versus-failure, late-response, future-call, repeated-failure, finalization, Logout, duplicate registration, disconnect-before-login, and client-ID-reuse cases. VlppOS tests will exercise post-admission visibility and acknowledgement ordering for local/network clients plus enabled, disabled, renewed, expired, stopped, repeated-stop, and mixed leased/unleased HTTP connections. Regenerate Workflow and VlppOS `Release` outputs only through their documented build/release scripts, then refresh GacUI `Import` only through the documented GacUI import/build flow.

Next, add the authoritative GacUI compiler phase:

1. Add `Instance_GenerateRpcMetadata` as pass 8 and the new `Instance_Max`/`Everything_Max`. Only the XML/instance resolver participates per pass. Keep passes 0 through 7 semantically unchanged except that Workflow rebuilds defer built-in per-module RPC validation.
2. Retain the final InstanceClass manager and every original module record. In pass 8, safely restore/complete virtual-script positions, merge every retained module's original AST paths and declarations into one transient module, and call exported `ValidateModuleRPC` exactly once without rebuilding or adding that aggregate to a manager. Always clear the temporary position state on exit and preserve original source locations in diagnostics.
3. When comprehensive RPC metadata exists, generate both flat and JSON Workflow wrappers in memory. Generate ordinary C++ from the untouched final manager first; copy-and-clear RPC metadata into a distinct RPC-only manager; compile only the copied metadata module plus the two wrappers; disable RPC reflection; include the actual ordinary entry header; and require exactly `<NAME>Rpc.h` and `<NAME>Rpc.cpp`.
4. Put reusable generation, validation, and transactional pair replacement in `GuiCppGen`. Update GacGen and GacUI_Compiler to emit deterministic RPC dumps and pairs, validate configuration before mutation, keep partial output inside its staging directory, and remove only known stale RPC artifacts on no-RPC or failed generation. Update all progress output to nine named passes.
5. Expand `TestResource_Rpc.cpp` from the pass-count reproducer into resource/compiler regressions for empty, non-RPC, valid, invalid, shared-only, multi-module, source-position, deterministic-output, exact-file-set, stale-cleanup, and unchanged ordinary-output behavior.

Finally, build the role-aware application and repository surface:

1. Create `RemoteViewModelTest/Resource.xml`, generate and explicitly enumerate architecture-specific `Generated_RemoteViewModelTest` shared items, and add `CppTest_Rvm` and `RemotingTest_RvmHost` Windows projects plus the requested vmake-only Linux projects. The host compiles only the architecture-selected RPC implementation; requesters compile the normal resource output and the RPC pair.
2. Define `ViewModelChannelName` and `ViewModelServiceName` once in `RemoteViewModelTestShared.h`. Reuse the existing Pipe, HTTP, and MiniHTTP physical endpoints; do not add a listener, port, or package discriminator.
3. Refactor Core's channel server into exact-single-channel, phase-aware admission. Before the permanent host reservation, accept only one remote `{ViewModelChannel}` client. Register it only from `OnClientAdmitted`; register the local requester only after `ConnectLocalServer` returns. After service acquisition and window construction, accept only exact `{GacUIRemoteProtocol}` renderers through the existing replacement path. Persist distinct broker, requester, host, core, and renderer IDs and disconnect only the matching broker endpoints.
4. Use a shared dispatcher/lifecycle implementation for `CppTest_Rvm` and Core `/RVMT`. Acquire and validate `rvmt::IViewModel` before constructing the window. Treat host Logout, channel disconnect, or lease expiry as one terminal transition: fail the requester, unwind initialization or `Translate` on its owning thread, marshal fatal UI exit where needed, broadcast one error while transports are alive, drain callbacks, finalize RPC objects in dependency order, and return nonzero. Ignore host-loss callbacks only after the window's normal `Run` has returned and shutdown entered `Stopping`.
5. Implement the RVM host as the sole semantic service provider. It accepts exactly one transport selector, advertises only `ViewModelChannel`, waits for broker login, registers and declares `rvmt::IViewModel`, runs the task queue, attempts graceful nonblocking Logout, fails blocked startup on transport loss, finalizes the dispatcher, drains the transport, and returns nonzero for terminal connection failure.
6. Add `/RVMT` and the strict application/transport negative matrix without changing `/FCT` or `/RPT`. Update the solution, project filters, Linux vmakes, `README.md`, `Project.md`, and `Test/GacUISrc/README.md` using only canonical names and topology/startup facts. Keep every `DebugRemoteProtocol*.md` byte-identical.

Confirmation will require the upstream suites, focused and full GacUI suites, generator determinism checks, Debug/Release and Win32/x64 builds, Linux Debug x64 builds, and real Pipe/HTTP/MiniHTTP lifecycle runs. The runtime matrix must prove service acquisition, translation, exact role isolation, renderer replacement, normal close, idle and in-flight host loss, nonzero fatal exit, repeated port reuse, no deadlock, no duplicate callback, no memory leak, and no unrelated generated diff.

### CODE CHANGE

- Restored both upstream repositories to their pre-task baselines and force-pushed
  `VlppOS/master` at `e46ef1297b6c09c88afae7876740a161b3e86cab` and
  `Workflow/master` at `586df2997a740933567ff89db11bdd112c6e7cbd`.
  The implementation now uses only the APIs already available from GacUI imports.
- Added `Instance_GenerateRpcMetadata` as pass 8. It retains all completed Workflow
  modules, recreates their script positions, validates one merged module, and clears
  temporary positions on every exit path. Ordinary C++ generation remains isolated
  from a separate RPC-only manager which emits the deterministic
  `<NAME>Rpc.h`/`<NAME>Rpc.cpp` pair without RPC reflection.
- Added reusable RPC output validation, stale-output cleanup, and transactional pair
  publication to GacGen and GacUI_Compiler. The compiler stages x86 and x64 outputs,
  verifies their shared contract, and merges architecture-selected outputs only after
  both sides succeed.
- Added the `RemoteViewModelTest` resource, generated shared items, CppTest_Rvm,
  RemotingTest_RvmHost, Core `/RVMT` integration, solution/project wiring, Linux vmake
  definitions, unit tests, and startup documentation. The demo intentionally uses the
  documented ordered startup and permits the applications to block while waiting for
  the next process.
- Kept the semantic service solely in RemotingTest_RvmHost. Core and CppTest_Rvm
  install a local ready-channel receiver before publishing broker readiness; the host
  sends its one-shot ready package only after its physical route exists. This avoids
  sending broker login before the accepted network route is installed.
- Any physical RvmHost disconnect outside deliberate server teardown is terminal.
  Core and CppTest_Rvm print `Fatal error: RemotingTest_RvmHost disconnected.` and
  immediately call `std::_Exit(1)`. A previously latched RPC failure does not suppress
  the host-loss exit.
- No heartbeat, lease, retry, or recovery protocol was added. Renderer replacement
  and `ControllerConnectionStopped` behavior remain unchanged; the internal
  `ViewModelReadyChannel` is only the post-admission startup signal for RvmHost.
- Ran the protected GacUI release/import workflow successfully. Repeated C64+C32
  generation produced the same combined 28-file SHA-256,
  `7D6616200D5B73065A09FE07D6F586749732573A9F261B4E49B142FC3F115F2A`,
  and repeated GacUI_Compiler generation preserved it.
- Built the complete Windows solution in Debug and Release for Win32 and x64 after
  the final ready-channel ordering change. Every build finished with zero warnings and
  zero errors. The complete Debug x64 unit run passed 88/88 files and 1716/1716 cases
  with no memory-leak report.
- Exercised the real named-pipe process topology through the repository wrappers.
  Both Core `/RVMT` and CppTest_Rvm acquired `rvmt::IViewModel`; terminating the host
  produced the required fatal diagnostic and an abnormal nonzero client exit.
- Linux project generation and source/rule inventories were validated. Linux
  compilation could not start on this Windows host because WSL, Docker/Podman, GNU
  `make`, and a Linux C++ toolchain are unavailable; both approved `build.sh` attempts
  stopped at `make: command not found`, before compilation.
- Regenerated Release through the official CodePack helper after the final source
  normalization. `git diff --check` is clean, `DebugRemoteProtocol*.md` is unchanged,
  no generated error file remains, and no test/demo process is left running.
