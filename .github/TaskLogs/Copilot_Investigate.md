# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

# Ensure GacUI Working Properly on Windows

## Goal

- Makesure test apps build and work as expected.
- `DebugRemoteProtocolWithNativeRenderer.md` and `DebugRemoteProtocolWithGacJS.md` have details of verification.
- You must fix any issue you see during verification.

## Maintain a Test Matrix Card

You need to maintain test card in:
- `<repo-root>/.github/TaskLogs/TestMatrix_NativeRenderer.md` for `DebugRemoteProtocolWithNativeRenderer.md`
- `<repo-root>/.github/TaskLogs/TestMatrix_GacJS.md` for `DebugRemoteProtocolWithGacJS.md`

If these files already exist, delete them, and recreate them following the `### Test Matrix Card` section in each document.
During verification, test matrix cards should be updated in time:
- When a test item starts, succeeds, fails, fixed, the file need to be updated immediately and saved, but no need to commit immediately, a commit is only required after all test items are done.
- I will read the file to check the progress of your verification.
Without explicitly instructed, you are required to complete every available test item in the matrix:
- If some test items are excluded explicitly, you still need to keep complete test items in the test matrix card, but leave blanks for results of excluded items.

## Verification

Try to limit changes in `GacUI` only, unless there are fundemental issues to fix in upstream repos.
Follow `DebugRemoteProtocolWithNativeRenderer.md` and `DebugRemoteProtocolWithGacJS.md` to make sure all test apps are behaving expectedly.
Without explicitly instructed, the complete matrix of all combinations of test apps, test UI loaded with remote protocol, and available network protocol choices, should be verified.
Make sure to commit and push local changes across all affected repos.

# UPDATES

# TEST [CONFIRMED]

Build the complete `Test/GacUISrc/GacUISrc.sln` Debug x64 solution through `copilotBuild.ps1`. Build and test GacJS through its required import, code-generation, build, test, and repeat-code-generation phases.

Execute every Windows row in `.github/TaskLogs/TestMatrix_NativeRenderer.md` and `.github/TaskLogs/TestMatrix_GacJS.md` with fresh processes. For each row, perform the applicable normal feature operations and fatal regression variants from `.github/Jobs/DebugRemoteProtocolSop.md`: exact initial UI, renderer-side input and visible state changes, renderer replacement/state continuity where applicable, second-host rejection for non-CLI RVM modes, application-controlled shutdown, Core-authored fatal error, both accepted-host-loss timings, exact terminal state, bounded cleanup, and absence of stale processes, listeners, native prompts, retry loops, page errors beyond the one deliberate fatal rethrow, and Debug runtime dialogs.

Success requires every matrix row to be completed, every required observable result to match exactly, the GacUI and GacJS builds/tests to pass, repeated GacJS code generation to leave no additional diff, and all detected issues to be fixed at their owning source repository and reverified. Record every row transition immediately in its matrix card.

# PROPOSALS

- No.1 Separate generator compilation from runtime-backed generator tests [CONFIRMED]

## No.1 Separate generator compilation from runtime-backed generator tests

`@gaclib/codegen-workflow-rpc` is an import-phase code generator, but its import script currently lints and compiles `test/**/*.ts`. Those tests import `@gaclib/workflow-rpc`, which is intentionally a later build-phase package. The generator package also omits that test-only workspace dependency, so a fresh workspace cannot resolve it. This makes the documented `yarn run import` → `yarn codegen` → `yarn build` → `yarn test` sequence circular.

Keep the import phase limited to generator source compilation. Move generator-test linting/type checking into the test phase, which already runs after `@gaclib/workflow-rpc` has been built, and declare the runtime as a development dependency of the generator tests. Preserve the generator/runtime ownership boundary and the documented root phase order.

### CODE CHANGE

- Add a source-only TypeScript build configuration for the Workflow RPC generator import phase.
- Limit import-phase ESLint to generator source.
- Run test ESLint and TypeScript checking from the generator test phase before Vitest.
- Declare `@gaclib/workflow-rpc` as the generator package's test-only workspace dependency.
- Give type-aware ESLint an explicit default-project allowlist for the two authored runtime-backed test files; generated fixtures remain outside lint mutation.
- Update the package-phase documentation to describe the ordering dependency.

### CONFIRMED

A fresh `yarn run import` now compiles the generator before the later Workflow RPC runtime exists. After the runtime build, the generator test phase type-checks and lints both authored runtime-backed tests and passes all 15 focused tests. The complete required GacJS import, codegen, build, and test sequence passed, and repeated code generation produced the identical diff hash.

- No.2 Normalize the generated golden's checkout line endings before comparison [CONFIRMED]

## No.2 Normalize the generated golden's checkout line endings before comparison

The deterministic generator test correctly proves that LF and BOM/CRLF metadata inputs produce identical LF output, but it compares that output to a checked-in golden read verbatim. On Windows with `core.autocrlf=true`, Git checks the golden out with CRLF, so the byte-for-byte comparison fails even though every generated line is identical. Normalize only the golden's CRLF checkout representation to the generator's canonical LF form before comparison; keep the generator output and tracked golden content unchanged.

### CODE CHANGE

- Normalize CRLF to LF when reading `RvmMetadata.generated.ts` for the deterministic comparison.

### CONFIRMED

The Windows checkout now compares the tracked golden in the generator's canonical LF representation without changing generated output. Both metadata line-ending variants still produce byte-identical LF output, the focused 15-test generator suite passes, and the tracked golden itself remains unchanged.

- No.3 Make Windows named-pipe connection shutdown callback-reentrant [CONFIRMED]

## No.3 Make Windows named-pipe connection shutdown callback-reentrant

Rejecting a second Workflow RPC host calls `INetworkProtocolConnection::Stop` from the named-pipe read callback that received the admission handshake. `NamedPipeConnection::Stop` currently waits for all pending read callbacks, including the callback that invoked it, so the rejected connection deadlocks. A later requester shutdown then waits forever for the channel server. CDB confirmed the cycle: the main thread waited in `NetworkProtocolChannelServer<..., NamedPipeServer>::Stop`, while the read callback waited in `NamedPipeConnection::Stop`.

Make the Windows named-pipe connection publish terminal state and cancel I/O immediately when `Stop` is called from its own registered read callback, but defer callback draining and handle closure until a later non-reentrant `Stop`/destruction boundary. This matches the callback-reentrant stop contract already used by the channel layer without weakening the normal external `Stop` guarantee.

### CODE CHANGE

- Track the active registered read-callback thread on each named-pipe connection.
- Let a same-connection callback-reentrant `Stop` return after setting `stopped` and canceling pending I/O, without waiting on itself or closing callback-owned handles.
- Add a Windows named-pipe regression test that calls connection `Stop` from `OnReadString`, requires the call and callback to return within the existing bound, and then proves external server shutdown still drains cleanly.

### CONFIRMED

The focused regression proves callback-reentrant `Stop` returns within the bound and a later external server/client shutdown drains the callback and handles. The previously deadlocked second-host rejection and normal close paths now complete over named pipes. The focused 44/44 inter-process suite, full 278/278 Debug suite, and both official Release architectures pass.

- No.4 Release an HTTP timeout callback before leaving its callback context [CONFIRMED]

## No.4 Release an HTTP timeout callback before leaving its callback context

The existing missing-poll-acknowledgement test passes its behavioral assertions but leaves one `SocketHttpServerConnectionLifecycle` and its timeout worker allocated in the Debug leak dump. The deadline worker resets `HttpRequestTimeoutController::currentCallbackState` before its local `Func<void()> callback` is destroyed. That callback owns the last connection-lifecycle reference; destroying it then destroys the timeout controller outside the marked callback context, so `CancelAndWait` waits for the worker that is currently destroying the callback and deadlocks. The retained connection token, timeout state, synchronization objects, and callback functor match the 15 leaked allocations reported after the otherwise-passing 277-test suite.

Destroy the local timeout callback while the thread still advertises the matching timeout callback state. The controller destructor then recognizes callback reentrancy and does not wait on its own worker; after the local callback is gone, restore the previous thread callback marker and let the worker finish normally.

### CODE CHANGE

- Clear the deadline worker's local callback functor immediately after invocation/catch and before restoring `currentCallbackState`.
- Re-run the existing successful-delivery-without-replacement-poll regression and the full Debug suite; require 277/277 with no leak dump.

### CONFIRMED

Destroying the local callback before restoring the callback marker removes the self-waiting lifetime cycle. The missing-replacement-poll regression completes, and the expanded full Debug suite passes 278/278 with no memory-leak dump; both official Win32 and x64 Release test runs also pass.

- No.5 Suppress the CppTest_Rvm Debug abort dialog on an expected unhandled host-loss exception [CONFIRMED]
- No.6 Preserve HTTP renderer message order without serializing response round trips [DENIED]
- No.7 Gate HTTP response-request submission by upload completion [CONFIRMED]
- No.8 Deliver piggybacked HTTP response messages in request order [CONFIRMED]

## No.5 Suppress the CppTest_Rvm Debug abort dialog on an expected unhandled host-loss exception

The named-pipe idle host-loss path correctly injects `rpc_controller::RpcInjectedException` into the next `Translate`, and `CppTest_Rvm` intentionally leaves it unhandled. In a Debug build, however, the default CRT `abort` behavior opens a modal assertion-style message box while `std::terminate` is processing that exception. The requester therefore remains as an apparently live process until the hidden dialog is dismissed, violating the bounded direct-termination requirement. CDB confirms the main thread is in `USER32!MessageBoxWorker` above `ucrtbased!abort` and `terminate`, with the original `RpcInjectedException` stack beneath it.

Apply the same Windows CRT configuration already used by `RemotingTest_Core`: disable only `_WRITE_ABORT_MSG` at process startup. This preserves the intentional unhandled-exception/nonzero-abort semantics while preventing an interactive Debug runtime dialog from retaining the process.

### CODE CHANGE

- Call `_set_abort_behavior(0, _WRITE_ABORT_MSG)` at the start of `CppTest_Rvm`'s Windows `WinMain`.
- Rebuild and rerun both accepted-host-loss timings, requiring prompt nonzero requester termination and no CRT, WER, or crash dialog.

### CONFIRMED

Both idle-next-call and delivery-acknowledgement host-loss variants now terminate the requester promptly with the intended unhandled `rpc_controller::RpcInjectedException` and nonzero exit, without retaining a CRT, WER, assertion, or crash dialog. The full GacUI Debug x64 rebuild passes with zero warnings and errors.

## No.6 Preserve HTTP renderer message order without serializing response round trips

The native `/Http` renderer can receive and display Core state, yet renderer-side text is reordered. A preserved CDB session held the renderer alive while two exact automation inputs, `DIAGONE` and `DIAGTWO`, crossed the renderer HTTP channel. Core Controls exposed the focused search editor containing `DGONIAEDOIAGTW`: all fourteen characters arrived, but each seven-character input was permuted. The Windows `HttpClient::SendString` implementation starts one concurrent `/Response` request per channel package, so HTTP completion order—not call order—controls the order in which Core receives text-input events. Subsequent channel traffic can then trip the established-channel local-error policy and settle the renderer.

Limit each Windows `HttpClientApi` WinHTTP session to two connections per server. The network protocol keeps one connection occupied by its independent `/Request` long poll, leaving one connection on which WinHTTP queues concurrent `/Response` submissions in `SendString` call order. This preserves channel ordering without forcing every message to wait for a full HTTP response round trip or weakening the existing five-second missing-replacement fatal-loss contract.

### CODE CHANGE

- Add a focused Windows HTTP network-protocol regression that bursts many sequential client messages and requires exact FIFO receipt.
- Configure the Windows WinHTTP session with a two-connection-per-server limit, reserving one long-poll lane and one ordered response lane.
- Rebuild and rerun the focused regression, the full VlppOS/GacUI test suites as appropriate, and the complete native `/FCT /Http` row.

### DENIED

The two-connection cap made the focused FIFO test and native `/FCT /Http` row pass, but it starved the overlapping `/Response` exchanges required to construct the RVMT remote surface. `/RVMT /Http` remained partial for three bounded minutes. The cap was removed; No.7 preserves request-body order without restricting concurrent response lifetimes.

## No.7 Gate HTTP response-request submission by upload completion

The two-connection WinHTTP cap from No.6 makes burst input FIFO, but the `/RVMT /Http` renderer never completes its initial surface even after three minutes. Core, renderer, and view-model host remain responsive, Core has the complete hosted UI, and the renderer has only a partially populated remote surface. One connection is held by the `/Request` long poll and the single remaining `/Response` connection cannot support the overlapping request/response exchanges needed while RVMT builds its nested remote state. Raising the connection count alone would restore progress but also restore the observed event reordering.

Preserve ordering at the request-upload boundary instead of at the full HTTP-connection boundary. Queue `/Response` bodies in `HttpClient::SendString`; start the first request immediately, and start each successor only when WinHTTP reports `WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE` for its predecessor. At that callback, the predecessor's entire request body has been uploaded in call order, while its response may remain outstanding on another connection. This retains the overlapping HTTP round trips needed by RVMT without allowing later message bodies to overtake earlier ones.

### CODE CHANGE

- Let `HttpClientApi::HttpQuery` optionally report request-upload completion exactly once.
- Queue Windows `HttpClient` `/Response` submissions and advance the FIFO from upload completion, while retaining existing completion/error processing and retry behavior.
- Remove the two-connection session cap.
- Extend the focused Windows regression to burst many messages and require exact FIFO receipt before immediate `Stop`; existing repeated protocol interop also requires queued pre-stop messages to drain.
- Rebuild and rerun the focused/full VlppOS suites plus native `/FCT /Http` and `/RVMT /Http` rows.

### CONFIRMED

Upload-completion gating preserves client-to-server `/Response` body order while allowing earlier response round trips to overlap. The strengthened burst-before-Stop regression, native `/FCT /Http`, and native manual `/RVMT /Http` all pass. This change is necessary but not sufficient for the faster CLI topology; No.8 adds the complementary reverse-direction completion ordering and is retained with this proposal.

## No.8 Deliver piggybacked HTTP response messages in request order

Upload gating fixes client-to-server body order and restores manual `/RVMT /Http`, but `/RVMT /Http /Cli:<path>` still stalls on a partial remote surface. The rebuilt Pipe CLI path passes, repeated HTTP/manual paths pass, Core correctly launches the stdio host, and the native renderer has both HTTP connections. The CLI host makes Core-to-renderer initialization faster and exposes the remaining reverse-direction race: each outstanding client `/Response` request can carry a piggybacked Core message in its HTTP response body, and WinHTTP completion callbacks may deliver those bodies out of request order.

Assign a monotonically increasing sequence to each client `/Response` request at FIFO submission. Buffer completed HTTP results by that sequence and process each result only when every predecessor has reached a final success or terminal failure. A retry retains its original sequence and therefore keeps later successful completions buffered until the retry resolves. This preserves server-to-client piggyback order while retaining the overlapping request lifetimes required by RVMT.

### CODE CHANGE

- Add a response-request sequence to the existing upload state.
- Buffer Windows `/Response` completions and drain them in sequence order; keep the sequence reserved across retries.
- Extend the focused regression to send an exact reply per burst message and require both server receipt and client receipt in FIFO order before `Stop`.
- Rebuild and rerun focused/full VlppOS suites, official releases, imported GacUI, and all affected native HTTP rows.

### CONFIRMED

The bidirectional 64-message regression now requires exact FIFO receipt at both server and client before immediate `Stop`, and passes with retries retaining their original sequence. The focused 44/44 suite, full 278/278 Debug suite, official Win32/x64 Release tests, imported GacUI rebuild, native `/FCT /Http`, manual `/RVMT /Http`, and `/RVMT /Http /Cli:<path>` rows all pass. Both complete Windows matrix cards are populated; GacJS live Chromium verification additionally passes normal input, renderer replacement, second-host rejection where applicable, exact Core-authored fatal states, host loss, graceful child reaping, and bounded cleanup across all 14 browser rows.
