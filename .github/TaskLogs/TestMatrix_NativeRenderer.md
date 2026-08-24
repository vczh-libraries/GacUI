# Test Matrix Card 2026-08-23 19:29:42

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][CppTest_Rvm][`/Pipe`] | 2026-08-23 19:46:10 (fixed) |
| [Windows][CppTest_Rvm][`/Http`] | 2026-08-23 20:30:21 |
| [Windows][CppTest_Rvm][`/MiniHttp`] | 2026-08-23 20:47:35 |
| [Windows][CppTest_Rvm][`/Cli:<path>`] | 2026-08-23 20:48:07 |
| [Windows][`/RPT`][`/Pipe`] | 2026-08-23 20:48:07 |
| [Windows][`/RPT`][`/Http`] | 2026-08-23 20:55:46 |
| [Windows][`/RPT`][`/MiniHttp`] | 2026-08-23 21:05:05 |
| [Windows][`/FCT`][`/Pipe`] | 2026-08-23 21:09:22 |
| [Windows][`/FCT`][`/Http`] | 2026-08-23 23:18:05 (fixed) |
| [Windows][`/FCT`][`/MiniHttp`] | 2026-08-23 22:09:30 |
| [Windows][`/RVMT`][`/Pipe`] | 2026-08-23 22:17:58 |
| [Windows][`/RVMT`][`/Pipe /Cli:<path>`] | 2026-08-23 22:18:24 |
| [Windows][`/RVMT`][`/Http`] | 2026-08-23 23:17:22 (fixed) |
| [Windows][`/RVMT`][`/Http /Cli:<path>`] | 2026-08-23 23:16:50 (fixed) |
| [Windows][`/RVMT`][`/MiniHttp`] | 2026-08-23 23:19:40 |
| [Windows][`/RVMT`][`/MiniHttp /Cli:<path>`] | 2026-08-23 23:22:03 |

## Issues Found and Fix

### [Windows][CppTest_Rvm][`/Pipe`] — 1st

After the accepted host completed two `Translate` operations, a second host was correctly rejected and the accepted host remained responsive. Closing through automation then hung both requester and accepted host. CDB showed the main thread waiting in `NetworkProtocolChannelServer<..., NamedPipeServer>::Stop` while a named-pipe callback thread was waiting in `NamedPipeConnection::Stop` from `OnReadString`. Fixed the callback-reentrant named-pipe `Stop` deadlock in VlppOS; the rebuilt requester and accepted host now both exit immediately on the same normal shutdown path.

The required idle-next-call host-loss variant then appeared to leave `CppTest_Rvm` alive after the next `Translate`. CDB showed that host loss and `rpc_controller::RpcInjectedException` were already correct, but the Debug CRT had retained `std::terminate` in a modal abort message box. Applied the same `_set_abort_behavior(0, _WRITE_ABORT_MSG)` configuration already used by `RemotingTest_Core`. The idle-next-call and delivery-acknowledgement-loss variants now both terminate promptly with nonzero exit `-2147483645`, with no CRT/WER/debug dialog. The delivery timing was made deterministic by suspending the accepted host under CDB, observing the second `Translate` block, and then terminating only that host.

### [Windows][`/FCT`][`/Http`] — 1st

The renderer repeatedly reordered pointer/keyboard events after already rendering live Core state. A preserved diagnostic session proved the focused search editor contained `DGONIAEDOIAGTW` after exact sequential inputs `DIAGONE` and `DIAGTWO`: every character arrived, but concurrent Windows `HttpClient` `/Response` requests completed out of `SendString` order. The final fix assigns every `/Response` a sequence, sends request bodies through an upload FIFO, and releases completions to the caller strictly in sequence while still allowing earlier HTTP responses to remain outstanding. A 64-message exact-order-and-Stop VlppOS regression, the focused 44/44 inter-process suite, the full 278/278 Debug suite, both official Release architectures, the imported GacUI rebuild, and the post-final-fix complete FCT rerun now pass.

### [Windows][`/RVMT`][`/Http`] — 1st

Core and the manually launched view-model host connected, but the native renderer did not reach the required initial `Hello, !` state even after a three-minute diagnostic bound. The two-connection WinHTTP cap used by the first FCT ordering fix reserved one lane for the `/Request` long poll and left only one `/Response` lane, starving the overlapping response exchanges needed while RVMT constructed its nested remote surface.

Replaced the connection cap with a client-side `/Response` FIFO that advances when WinHTTP reports each request body fully uploaded, while earlier HTTP responses may remain outstanding on other connections. `Stop` drains all pre-stop uploads before closing the API. The strengthened 64-message FIFO-before-Stop regression, focused 44/44 inter-process suite, full 278/278 Debug suite, official Win32/x64 Release tests, imported GacUI rebuild, and this complete RVMT row now pass. The earlier FCT ordering row remains covered for regression.

### [Windows][`/RVMT`][`/Http /Cli:<path>`] — 1st

Core correctly launched `RemotingTest_RvmHost.exe /Cli` as its stdio child, and the native renderer established its two HTTP connections, but the renderer remained on a partial remote surface for over a minute and Core automation never exposed the complete RVMT controls. All three processes stayed alive and responsive. The bounded run was stopped before feature and host-loss checks. The stdio host path was healthy: the remaining race was in the renderer's reverse HTTP direction, where concurrently completed `/Response` requests could still overtake one another after their bodies had been uploaded in order.

Added a monotonically increasing response sequence to the WinHTTP client and buffered `/Response` completions until every earlier sequence was complete, including retries. After the final Release import and clean GacUI rebuild, the complete HTTP CLI row now passes its UI, Workflow RPC, idle-next-call host-loss, delivery-acknowledgement-loss, graceful child reaping, and close checks.
