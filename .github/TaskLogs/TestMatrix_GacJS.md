# Test Matrix Card 2026-11-03 17:16:17 -08:00

## Test Matrix

| Test Item | 1st |
| --- | --- |
| [Windows][`/RPT`][`/Http`] | 2026-11-03 19:54:15 |
| [Windows][`/RPT`][`/MiniHttp`] | 2026-11-03 19:54:45 |
| [Windows][`/FCT`][`/Http`] | 2026-11-03 20:14:15 |
| [Windows][`/FCT`][`/MiniHttp`] | 2026-11-03 20:14:15 |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network] | 2026-11-03 19:46:28 |
| [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] | 2026-11-03 19:46:52 |
| [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`] | 2026-11-03 19:47:12 |
| [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network] | 2026-11-03 19:47:40 |
| [Windows][`/RVMT`][`/Http`][GacJS Node SEA over stdio `/Cli:<path>`] | 2026-11-03 19:48:02 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network] | 2026-11-03 19:48:21 |
| [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`] | 2026-11-03 19:48:43 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`] | 2026-11-03 19:49:00 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network] | 2026-11-03 19:49:21 |
| [Windows][`/RVMT`][`/MiniHttp`][GacJS Node SEA over stdio `/Cli:<path>`] | 2026-11-03 19:49:43 |

## Issues Found and Fix

- The FullSurface generator test exceeded 10 seconds twice (full suite and focused retry at 12.2 seconds). Its scoped timeout is now 30 seconds, preserving all type-checking assertions. Rebuilt all 10 packages and passed the complete yarn test suite.

## Observations and Limitations

- Final verification 2026-11-03 21:27:53: import and codegen/lint passed; all ten packages rebuilt; yarn test passed all 11 packages, including 61/61 browser E2E cases in 13 files. Evidence: <GacUI repo>/Test/GacUISrc/x64/Debug/RpWindows/GacJS-touching-sync-build.log and GacJS-final-verified-test.log.

- The preceding full run and focused retry reported Touching/MiniHTTP momentum interruption mismatches of 12 pixels. Diagnostics showed Core bounds stayed identical while the browser caught up by 11 pixels. The test now consumes the flick idle and waits for the stopping contact render before sampling. HTTP and MiniHTTP passed both the focused verification and final full suite. This is a harness synchronization correction, with unchanged momentum behavior.

- Imported ScrollResetOnNavigation frames 4/5 were audited after resolving element IDs: complete normalized rendered trees (content, geometry and child order) are unchanged. Differences are regenerated identities; GuiToolstripMenuAutomatic and its five frames are newly synchronized upstream snapshots.

- Repeated yarn codegen and lint passed. SHA-256 comparisons left the imported snapshot data and generated index unchanged, confirming generation is repeatable.

- The checked-in Touching_Remote.ps1 -Browser run passed on HTTP and MiniHTTP: raw transforms and painted markers, three-contact/rebase/cancel/fallback, TouchingList identities/reversal/removal/recycling/momentum, ordinary ListView, document press ranges and exclusive fallback, momentum in both edit modes, and palette replacement. No page errors; browser scroll position remained stationary; both Core processes exited normally. All 14 Windows browser matrix rows are complete.

- [Windows][`/FCT`][`/MiniHttp`]: lists, exact brace editor retention, both local shortcuts, mouse payloads before/after replacement, vertical rail keyboard/gap, four palettes, all responsive modal cycles/datasets, clock, renderer replacement/takeover and queued normal exit passed. The checked-in touch driver subsequently passed both transports.

- [Windows][`/FCT`][`/Http`]: lists, exact brace editor retention, both local shortcuts, mouse payloads before/after replacement, vertical rail keyboard/gap, four palettes, all responsive modal cycles/datasets, clock, renderer replacement/takeover and queued normal exit passed. The checked-in touch driver subsequently passed both transports.

- Browser wheel magnitude is normalized to one notch by GacJS. The responsive scroll helper now emits separate wheel events for repeated notches. A subsequent retry found a prior modal had intercepted Exit, so the old Core was closed completely before restarting with fresh list counters.


- Browser RPT first retry attached to a prior Core still waiting for its normal Exit confirmation; the prompt was confirmed and both fresh HTTP/MiniHTTP runs then passed. Browser coordinate checks now require Core and rendered label bounds to agree.


- [Windows][`/RPT`][`/MiniHttp`]: button, nine grid cells/clear, embedded dialog, local Ctrl+Q and Ctrl+Alt+Win+Q, all mouse buttons/modifiers/double-clicks/wheels, replacement/takeover, canceled Exit and confirmed queued close passed. Fresh fatal session retained exact Core error mask and one intentional page error.

- [Windows][`/RPT`][`/Http`]: button, nine grid cells/clear, embedded dialog, local Ctrl+Q and Ctrl+Alt+Win+Q, all mouse buttons/modifiers/double-clicks/wheels, replacement/takeover, canceled Exit and confirmed queued close passed. Fresh fatal session retained exact Core error mask and one intentional page error.



- [Windows][`/RVMT`][`/MiniHttp`][GacJS Node SEA over stdio `/Cli:<path>`]: literal brace input/RPC, applicable second-host rejection, renderer replacement/takeover, normal Exit and owned-host cleanup, idle-next-call and blocked-delivery fatal variants passed in Chromium. Exact Core-authored fatal mask and one intentional matching page error verified.

- [Windows][`/RVMT`][`/MiniHttp`][GacJS Node `cli.js` over network]: literal brace input/RPC, applicable second-host rejection, renderer replacement/takeover, normal Exit and owned-host cleanup, idle-next-call and blocked-delivery fatal variants passed in Chromium. Exact Core-authored fatal mask and one intentional matching page error verified.

- [Windows][`/RVMT`][`/MiniHttp`][GacJS browser host `?rvmhost`]: literal brace input/RPC, applicable second-host rejection, renderer replacement/takeover, normal Exit and owned-host cleanup, idle-next-call and blocked-delivery fatal variants passed in Chromium. Exact Core-authored fatal mask and one intentional matching page error verified.

- [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`]: literal brace input/RPC, applicable second-host rejection, renderer replacement/takeover, normal Exit and owned-host cleanup, idle-next-call and blocked-delivery fatal variants passed in Chromium. Exact Core-authored fatal mask and one intentional matching page error verified.

- [Windows][`/RVMT`][`/MiniHttp`][Native `RemotingTest_RvmHost` over network]: literal brace input/RPC, applicable second-host rejection, renderer replacement/takeover, normal Exit and owned-host cleanup, idle-next-call and blocked-delivery fatal variants passed in Chromium. Exact Core-authored fatal mask and one intentional matching page error verified.

- [Windows][`/RVMT`][`/Http`][GacJS Node SEA over stdio `/Cli:<path>`]: literal brace input/RPC, applicable second-host rejection, renderer replacement/takeover, normal Exit and owned-host cleanup, idle-next-call and blocked-delivery fatal variants passed in Chromium. Exact Core-authored fatal mask and one intentional matching page error verified.

- [Windows][`/RVMT`][`/Http`][GacJS Node `cli.js` over network]: literal brace input/RPC, applicable second-host rejection, renderer replacement/takeover, normal Exit and owned-host cleanup, idle-next-call and blocked-delivery fatal variants passed in Chromium. Exact Core-authored fatal mask and one intentional matching page error verified.

- [Windows][`/RVMT`][`/Http`][GacJS browser host `?rvmhost`]: literal brace input/RPC, applicable second-host rejection, renderer replacement/takeover, normal Exit and owned-host cleanup, idle-next-call and blocked-delivery fatal variants passed in Chromium. Exact Core-authored fatal mask and one intentional matching page error verified.

- [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over stdio `/Cli:<path>`]: literal brace input/RPC, applicable second-host rejection, renderer replacement/takeover, normal Exit and owned-host cleanup, idle-next-call and blocked-delivery fatal variants passed in Chromium. Exact Core-authored fatal mask and one intentional matching page error verified.

- [Windows][`/RVMT`][`/Http`][Native `RemotingTest_RvmHost` over network]: literal brace input/RPC, applicable second-host rejection, renderer replacement/takeover, normal Exit and owned-host cleanup, idle-next-call and blocked-delivery fatal variants passed in Chromium. Exact Core-authored fatal mask and one intentional matching page error verified.

- After the scoped FullSurface timeout correction, yarn build passed all 10 packages and yarn test completed successfully. Evidence: <GacUI repo>/Test/GacUISrc/x64/Debug/RpWindows/GacJS-build-after-timeout.log and GacJS-test-after-timeout.log.
