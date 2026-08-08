# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

About "**## Remote View Model Test (`/RVMT`): Force-Terminate `RemotingTest\_RvmHost`" in** [DebugRemoteProtocolSop.md](DebugRemoteProtocolSop.md) , I would like to slightly update the document to make it specific, about the timing of when the UI should know rvmhost disconnects. For pipe the disconnection should be sensed from server directly, but for http or minihttp there is no way to do that. So I think it is better to say, CppTest_Rvm or RemotingTest_Core should at least know the rvmhost is gone, right at or before user typing anything in the text box (because it calls IViewModel). Use your own word.

And I have manually verified it, test apps doesn't behave like this, in fact after RvmHost is closed, UI hangs.
Fix the document, figure out the root cause, fix it, verify, commit and push all local changes.

At the end, you are going to run through both [DebugRemoteProtocolWithNativeRenderer.md](DebugRemoteProtocolWithNativeRenderer.md) and [DebugRemoteProtocolWithGacJS.md](DebugRemoteProtocolWithGacJS.md) to verify the complete matrix, you can't skip any single combination written in these documents.

# UPDATES

# TEST [CONFIRMED]

Reproduce accepted-host loss after a successful `IViewModel::Translate` for both requester shapes and every supported Windows transport:

- `CppTest_Rvm` with `RemotingTest_RvmHost` over `/Pipe`, `/Http`, and `/MiniHttp`.
- `RemotingTest_Core /RVMT` with `RemotingTest_RvmHost` and the matching native renderer over `/Pipe`, `/Http`, and `/MiniHttp`.

For `/Pipe`, force-terminate the accepted host while the requester is idle and require the server-side disconnect callback to record host loss without user input. For `/Http` and `/MiniHttp`, where no heartbeat or reverse disconnect route exists, trigger the first post-loss `Translate` by typing in the text box. In every transport, the requester must surface the persistent host-loss exception no later than that first RPC operation; the UI must not remain blocked.

Also cover host termination while a `Translate` response is pending. The waiting requester call must be released within a bounded wait. `CppTest_Rvm` must terminate nonzero from the unhandled `RpcInjectedException`. `RemotingTest_Core` must send exactly one Core-authored `!Error` containing exactly `RemotingTest_RvmHost disconnected.` before terminating nonzero, and the native or GacJS renderer must expose that exact fatal error.

After the focused regression passes, run the complete combination matrix required by `DebugRemoteProtocolWithNativeRenderer.md` and `DebugRemoteProtocolWithGacJS.md`, following the shared SOP for each listed application, transport, renderer, replacement, normal-shutdown, and fatal scenario. Every run must be bounded and leave no requester, Core, host, renderer, browser session, listener, prompt, or crash dialog behind.

The unmodified `Debug|x64` executables reproduce the defect after a successful initial greeting. `CppTest_Rvm /Pipe` receives the server-side named-pipe disconnect and the first post-loss keystroke reaches the injected exception instead of hanging. Both `CppTest_Rvm /Http` and `CppTest_Rvm /MiniHttp` accept the focus click and post-loss `!Type:X` automation command, then stop answering the `Controls` endpoint because the UI thread remains blocked in the synchronous `Translate` response wait. The accepted host process is gone and the requester remains alive, confirming that failed HTTP poll delivery is not being promoted into the channel-server disconnection callback.

# PROPOSALS

- No.1 Promote failed accepted-client HTTP poll delivery to channel disconnection

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
