# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

1. Execute `GacUI items outside the requested GacJS-only change scope` section in [TODO_RPC_Missing.md](GacJS/TODO_RPC_Missing.md)
2. follow [DebugRemoteProtocolWithNativeRenderer.md](../Jobs/DebugRemoteProtocolWithNativeRenderer.md) and [DebugRemoteProtocolWithGacJS.md](../Jobs/DebugRemoteProtocolWithGacJS.md)but only run `/cli` related part of the test matrix. since this is a very local refactoring, if any test item fails, limit the change in GacUI repo. It has been verified before this refactoring.
3. commit and push all local changes once finishing

# UPDATES

# TEST [CONFIRMED]

The split `RemotingTest_Core /RVMT <renderer-transport> /Cli:<path>` topology is constructed in `Test/GacUISrc/RemotingTest_Core/GuiMain.cpp`. Its renderer side currently uses a plain `RemotingChannelServer<TServerBase>` with renderer admission enabled from construction, while its stdio host side separately owns `RemoteViewModelChannelServer<StdioRedirectionServer>` and the requester phase. This statically confirms that an early renderer bypasses the `RpcServerHelpers::CanAdmitRenderer()` check used by the combined topology.

Add focused coverage around the split renderer-server type. The regression must prove that a renderer is rejected while the supplied RVM requester gate is not running, then prove that the first renderer and a replacement renderer are both accepted after that exact gate reports running. Build the complete GacUISrc solution and run the required UnitTest project. Finally, execute only the `/Cli` rows from `REPO-ROOT/.github/Jobs/DebugRemoteProtocolWithNativeRenderer.md` and `REPO-ROOT/.github/Jobs/DebugRemoteProtocolWithGacJS.md`, including successful view-model interaction, renderer replacement, application-controlled shutdown, and process cleanup as defined by the shared SOP.

Success requires all focused assertions, the GacUI build, the GacUI unit test (including no memory-leak dump), and every exercised `/Cli` executable row to pass. Any failure-induced source change remains confined to GacUI.

# PROPOSALS

- No.1 Share the requester admission gate with the split renderer server [CONFIRMED]

## No.1 Share the requester admission gate with the split renderer server

The combined RVM server already owns the authoritative requester phase and exposes renderer admission only after `RequestService` transitions that phase to `Running`. Preserve that single source of truth. Add a renderer-only RVM channel-server composition that delegates ordinary renderer assignment and replacement to `RemotingChannelServer<TServerBase>` but first queries the associated RVM server's live `CanAdmitRenderer()` result. Use this composition only for the split `/Cli` renderer transport, with the stdio RVM server as its gate owner.

This keeps early admission as an immediate rejection, removes any secondary readiness flag and transition window, and preserves the existing replacement implementation after the requester is running.

### CODE CHANGE

- Expose the authoritative `CanAdmitRenderer()` observation from `RemoteViewModelChannelServer`.
- Add `RemoteViewModelRendererChannelServer`, which checks that observation before delegating renderer admission.
- Use the gated renderer-only server in the split `/Cli` branch of `RemotingTest_Core`.
- Add focused admission and replacement regression coverage.

### CONFIRMED

- The split renderer server now reads the live requester phase from the exact stdio RVM server that owns the `/Cli` host. It rejects renderer admission before `Running`, then delegates first admission and replacement to the existing renderer server. The original renderer-server construction order is preserved and the gate pointer is detached before the RVM server is destroyed.
- The focused regression rejects two early renderers, accepts the first renderer after the gate enters `Running`, and accepts its replacement.
- The owning GacUI guide already has the corrected `/RVMT` host table and the POSIX `/bin/sh -c` filename limitation required by the TODO item.
- The complete GacUISrc Debug/x64 build passed with 0 warnings and 0 errors.
- The final UnitTest run passed 89/89 test files and 1714/1714 test cases with no memory-leak output.
- Native `/Cli` verification passed the standalone requester row and all Core renderer transports (`/Pipe`, `/Http`, and `/MiniHttp`), including RPC-backed input, renderer replacement, application-controlled shutdown, and child cleanup. An intentionally early `/Http` renderer was rejected before requester readiness, and the post-readiness run passed the full interaction/replacement sequence.
- Browser `/Cli` verification passed native and GacJS SEA hosts over both `/Http` and `/MiniHttp` (4/4 rows), including state continuity, post-replacement input, graceful shutdown, and host reaping. The checked-in CLI tests also passed the accepted-host-loss error path and the direct C++ requester/SEA launcher row.
