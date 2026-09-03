# Test Matrix Card 2026-09-02 16:43:50

This round covers the mouse/shortcut regressions in FullControlTest and RemoteProtocolTest. RVM rows are outside this task and remain blank.

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][`/RPT`][`/Http`] | 2026-09-02 17:26:10 (fixed) |
| [Windows][`/RPT`][`/MiniHttp`] | 2026-09-02 17:31:58 |
| [Windows][`/FCT`][`/Http`] | 2026-09-02 17:35:13 |
| [Windows][`/FCT`][`/MiniHttp`] | 2026-09-02 17:38:03 |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network] |  |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] |  |
| [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`] |  |
| [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network] |  |
| [Windows][`/RVMT`][`/Http`][GacJS Node SEA over stdio `/Cli:<path>`] |  |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network] |  |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] |  |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`] |  |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network] |  |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node SEA over stdio `/Cli:<path>`] |  |

## Issues Found and Fix

### RPT / Http, 1st

The harness first needed renderer-idle synchronization before clicking dialog buttons. After that correction, all shortcut dialogs and Left/Middle/Right passed, but Mouse4 caused Chromium to navigate Back to `about:blank` after sending down/up. Passive DOM logging confirmed button 3 was delivered before navigation. GacJS now prevents browser defaults for consumed Mouse4/Mouse5 down/up events. The fresh HTTP rerun passed labels, theme font/color, all shortcuts and buttons, renderer replacement/takeover, and application-controlled shutdown.

MiniHTTP also passed those checks. One pending browser request reported `ERR_CONNECTION_REFUSED` after intentional Core shutdown; the success mask was visible, the error mask stayed hidden, and no page error or reconnect occurred. The harness treats that exact post-shutdown diagnostic separately from errors during live operation.
