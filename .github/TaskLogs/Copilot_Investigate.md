# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

About "**## Remote View Model Test (`/RVMT`): Force-Terminate `RemotingTest\_RvmHost`" in** [DebugRemoteProtocolSop.md](DebugRemoteProtocolSop.md) , I would like to slightly update the document to make it specific, about the timing of when the UI should know rvmhost disconnects. For pipe the disconnection should be sensed from server directly, but for http or minihttp there is no way to do that. So I think it is better to say, CppTest_Rvm or RemotingTest_Core should at least know the rvmhost is gone, right at or before user typing anything in the text box (because it calls IViewModel). Use your own word.

And I have manually verified it, test apps doesn't behave like this, in fact after RvmHost is closed, UI hangs.
Fix the document, figure out the root cause, fix it, verify, commit and push all local changes.

At the end, you are going to run through both [DebugRemoteProtocolWithNativeRenderer.md](DebugRemoteProtocolWithNativeRenderer.md) and [DebugRemoteProtocolWithGacJS.md](DebugRemoteProtocolWithGacJS.md) to verify the complete matrix, you can't skip any single combination written in these documents.

# UPDATES

## UPDATE

My opinion, I think adding timeout to workflow does not fix the root cause. Since the issue is clearly /Http or /MiniHttp specific, that's because of the design, causing server sending client messages relying on a live /request (or /response I can't remember clearly) pending request. So maybe a better idea is to implement the timeout to both http.sys and socket http `INetworkProtocolServer` implementation. What do you think?

# TEST [CONFIRMED]

Reproduce accepted-host loss after a successful `IViewModel::Translate` for both requester shapes and every supported Windows transport:

- `CppTest_Rvm` with `RemotingTest_RvmHost` over `/Pipe`, `/Http`, and `/MiniHttp`.
- `RemotingTest_Core /RVMT` with `RemotingTest_RvmHost` and the matching native renderer over `/Pipe`, `/Http`, and `/MiniHttp`.

For `/Pipe`, force-terminate the accepted host while the requester is idle and require the server-side disconnect callback to record host loss without user input. For `/Http` and `/MiniHttp`, where no heartbeat or reverse disconnect route exists, trigger the first post-loss `Translate` by typing in the text box. In every transport, the requester must surface the persistent host-loss exception no later than that first RPC operation; the UI must not remain blocked.

Also cover host termination while a `Translate` response is pending. The waiting requester call must be released within a bounded wait. `CppTest_Rvm` must terminate nonzero from the unhandled `RpcInjectedException`. `RemotingTest_Core` must send exactly one Core-authored `!Error` containing exactly `RemotingTest_RvmHost disconnected.` before terminating nonzero, and the native or GacJS renderer must expose that exact fatal error.

After the focused regression passes, run the complete combination matrix required by `DebugRemoteProtocolWithNativeRenderer.md` and `DebugRemoteProtocolWithGacJS.md`, following the shared SOP for each listed application, transport, renderer, replacement, normal-shutdown, and fatal scenario. Every run must be bounded and leave no requester, Core, host, renderer, browser session, listener, prompt, or crash dialog behind.

The unmodified `Debug|x64` executables reproduce the defect after a successful initial greeting. `CppTest_Rvm /Pipe` receives the server-side named-pipe disconnect and the first post-loss keystroke reaches the injected exception instead of hanging. Both `CppTest_Rvm /Http` and `CppTest_Rvm /MiniHttp` accept the focus click and post-loss `!Type:X` automation command, then stop answering the `Controls` endpoint because the UI thread remains blocked in the synchronous `Translate` response wait. The accepted host process is gone and the requester remains alive, confirming that failed HTTP poll delivery is not being promoted into the channel-server disconnection callback.

# PROPOSALS

- No.1 Promote failed accepted-client HTTP poll delivery to channel disconnection [DENIED]
- No.2 Bound configured channel RPC response waits with persistent injection [DENIED]
- No.3 Require a replacement HTTP poll after server message delivery

## No.1 Promote failed accepted-client HTTP poll delivery to channel disconnection

The raw Windows HTTP and portable Socket HTTP protocols intentionally allow a failed `/Request` response to be retried: the outbound logical message is retained so a live client can submit a replacement long poll. That policy is appropriate below the channel layer, but the channel server currently discards the corresponding nonfatal local-error callback. After a client has completed the channel handshake, retrying an ambiguous delivery cannot preserve reliable channel semantics and can leave a synchronous RPC waiting forever when the client process is gone.

Both HTTP server implementations should report failed `/Request` response delivery through `INetworkProtocolCallback::OnLocalError` as a nonfatal raw transport error. If the callback declines promotion, preserve the existing raw retry/requeue behavior. `NetworkProtocolChannelServer` should promote every such local error after the remote client has been accepted, matching the connected channel-client policy. Promotion makes the raw HTTP connection stop and deliver `OnDisconnected`, allowing `RpcServerHelpers::OnClientDisconnected` to inject `RemotingTest_RvmHost disconnected.` and release the pending or next RPC call. No heartbeat, reverse disconnect endpoint, or requester recovery is introduced.

Update the RVM host-loss SOP so the timing is explicit: named pipe should observe host loss directly, while HTTP and MiniHTTP may remain unaware during idle time; regardless of transport, the requester must surface host loss no later than the first post-loss `IViewModel` call and must not leave the UI blocked.

### CODE CHANGE

- In the upstream VlppOS sources, make `NetworkProtocolChannelServer` promote server-side local transport errors only after channel admission.
- Make Windows `HttpServerConnection` and portable `SocketHttpServerConnection` report failed `/Request` response delivery and stop only when their installed callback promotes the error; retain raw retry/requeue behavior otherwise.
- Add deterministic VlppOS regressions for pre-admission versus post-admission channel promotion and for promoted Socket HTTP failed-poll delivery, while preserving the existing raw requeue regression.
- Regenerate the VlppOS release and import it into GacUI through the supported release pipeline.
- Update `DebugRemoteProtocolSop.md` and synchronized inter-process knowledge/manual text to state the timing and promotion contract.
- Run the focused/unit builds and the complete native-renderer and GacJS verification matrices required by the task.

### DENIED

The transport change correctly preserves retry before channel admission and promotes a reported failed delivery after admission, and all 274 VlppOS Debug x64 tests plus all 275 Release Win32 tests passed. It is nevertheless insufficient as the complete host-loss fix. In the focused `CppTest_Rvm /Http` run, the requester completed `Translate("AliveHttp")`, the accepted host was force-terminated, and the first post-loss `Translate("AfterLoss")` still blocked the UI. The requester process and window remained alive while its automation `Controls` endpoint timed out. Windows HTTP accepted the server's pending-poll response after the client process was gone, so there was no failed-send callback for the channel layer to promote. A successful local HTTP send therefore cannot establish peer receipt, and a missing RPC response still needs an application-selected deadline.

## No.2 Bound configured channel RPC response waits with persistent injection

`RpcJsonDispatcherClient::OnJsonRequest` is the transport-independent synchronous boundary that owns response matching. Add an optional response timeout to this dispatcher, disabled by default so existing Workflow clients retain their current policy. A caller can configure a positive timeout and terminal exception text before requests begin. Each request gets one absolute deadline; nested incoming requests and unrelated buffered responses do not extend it. If the matching response has not committed by the deadline, the dispatcher stores the configured text as its persistent injected exception under the same lock used for response selection, wakes all waits, and throws `RpcInjectedException` on the original caller thread. This preserves the existing fail-fast, last-write-wins injection behavior and introduces no heartbeat or recovery path.

Configure only the RVM requester's local dispatcher with `RemotingTest_RvmHost disconnected.`. Named pipe will normally inject immediately through its direct disconnect callback. HTTP and MiniHTTP may remain unaware while idle, but the first post-loss `IViewModel` call will either observe an earlier disconnect or time out instead of freezing the UI. A call already pending when the host is terminated is released by the same bound.

### CODE CHANGE

- Add a public, pre-request `SetResponseTimeout(milliseconds, exceptionMessage)` configuration to the upstream Workflow `RpcJsonDispatcherClient`; reject nonpositive timeouts and reconfiguration after initialization.
- Make the response wait use a per-request absolute deadline and atomically promote expiry to the existing persistent injected-exception state.
- Add Workflow `LibraryTest` coverage that a missing response times out on the caller thread, poisons later calls with the exact configured text, and does not time out when a response commits first.
- Document the opt-in response-timeout contract in the Workflow JSON request-routing knowledge base and manual.
- Regenerate Workflow release files and import them into GacUI through the supported release pipeline.
- Configure the RVM requester helper with the exact host-disconnect text, then repeat both requester shapes and all three transports for idle-next-call and in-flight host termination.

### DENIED BY USER

The focused Workflow unit test proved that a generic response deadline could release the call, but the proposal put HTTP peer-liveness policy into the transport-independent RPC dispatcher. The defect is specific to the `/Request` long-poll design: both HTTP servers may report successful local submission of a nonempty poll response even after the client process is gone. A Workflow timeout would mask that missing transport state transition and affect every channel-backed RPC transport. The Workflow code, tests, and documentation from this proposal were removed before release or downstream import.

## No.3 Require a replacement HTTP poll after server message delivery

Both HTTP clients continuously keep one `/Request` long poll alive. After receiving a `/Request` response, each client already submits the replacement poll before invoking the protocol callback. Therefore the next `/Request` is an implicit acknowledgement that the previous server-to-client message was actually received, independent of whether HTTP.sys or the socket stack merely accepted the response bytes locally.

Keep idle long polls unbounded. When either HTTP server successfully submits a nonempty server message through `/Request`, arm a bounded replacement-poll deadline. The next `/Request` cancels the deadline before it becomes eligible to carry another message. If no replacement arrives, report a nonfatal local delivery error through `INetworkProtocolCallback::OnLocalError`. Raw protocol callbacks may decline promotion and keep the logical connection; an admitted `NetworkProtocolChannelServer` promotes the error, stops the connection, delivers `OnDisconnected`, and causes the RVM helper to inject the exact persistent host-loss exception. This applies the deadline only to ambiguous server delivery and introduces no heartbeat or generic RPC policy.

The synchronous `/Response` route may continue returning a generated reply in its own HTTP response. The RVM failure is the long-poll path: the broker sends the first post-loss request through the host's outstanding `/Request`, local submission succeeds, and the absent replacement poll is what proves the host did not receive it.

### CODE CHANGE

- Move the existing reusable HTTP deadline controller from the async-socket request implementation into the shared HTTP protocol layer so both server implementations can use the same monotonic, cancellable deadline primitive.
- Add a shared replacement-poll timeout constant and arm it only after a nonempty `/Request` response is locally submitted successfully.
- In both Windows HTTP.sys and portable Socket HTTP servers, cancel the pending acknowledgement when the next `/Request` is registered; on expiry, report the same recoverable local delivery error and stop only when the installed callback promotes it.
- Preserve failed-send requeue behavior and the existing `/Response` piggyback behavior.
- Add focused raw-server regressions for both implementations that complete one nonempty poll response, omit the replacement poll, and require one recoverable local error within the bound. Retain the channel admission promotion regression and verify the RVM applications surface the exact fatal error.
- Regenerate and import VlppOS through the supported pipelines, update the HTTP and RVM SOP documentation, then run every required native-renderer and GacJS matrix combination.
