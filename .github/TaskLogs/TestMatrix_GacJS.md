# Test Matrix Card 2026-08-30 09:24:36

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][`/RPT`][`/Http`] |  |
| [Windows][`/RPT`][`/MiniHttp`] |  |
| [Windows][`/FCT`][`/Http`] |  |
| [Windows][`/FCT`][`/MiniHttp`] |  |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network] | 2026-08-30 09:28:38 |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] | 2026-08-30 09:31:23 |
| [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`] | 2026-08-30 09:34:05 (fixed) |
| [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network] | 2026-08-30 09:42:25 |
| [Windows][`/RVMT`][`/Http`][GacJS Node SEA over stdio `/Cli:<path>`] | 2026-08-30 09:45:13 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network] | 2026-08-30 09:47:51 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] | 2026-08-30 09:50:20 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`] | 2026-08-30 09:57:48 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network] | 2026-08-30 09:53:04 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node SEA over stdio `/Cli:<path>`] | 2026-08-30 09:55:45 |

## Issues Found and Fix

### [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`] — 1st

The normal flow passed, including second-host rejection, accepted-host continuity, renderer replacement, a subsequent `Translate`, visible Exit, and Core exit code `0`. In the first fatal harness, opening the ordinary replacement as another in-app top-level tab caused the host tab's `pagehide` cleanup to remove `window.__gacui_rvmhost_session`; the required explicit host-only stop could therefore no longer be invoked. The fatal run was aborted and closed cleanly rather than credited. This was an in-app tab lifecycle mismatch in the verifier, not product behavior.

The repository Playwright harness was then used for the host-only fatal boundary because it retains both top-level pages as required by the SOP. The already-clean full suite supplied the idle-next-call case for both Http and MiniHTTP: it replaced only the renderer, invoked exact `window.__gacui_rvmhost_session.host.stop()`, produced one exact `RemotingTest_RvmHost disconnected.` mask/page error, and terminated Core nonzero. A fresh response-bound test armed after renderer replacement and invoked the same exact host stop when the delivered host `/Request` response arrived, before a replacement poll could acknowledge delivery. Both Http and MiniHTTP delivery-loss cases passed with the exact mask and nonzero Core exit. The temporary test was removed after execution; no GacJS product or website change was made.
