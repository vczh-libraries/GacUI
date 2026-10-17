# Point Input, Capture and Remote Forwarding

## Ownership and native acquisition

Point input is an overlapping-contact sequence. The first Down asks listeners through `INativeWindowListener::PointSequenceStarted(firstPoint, accept)`; acceptance is ORed and latched until End. Accepted sequences deliver `PointDown`, `PointUpdate`, `PointUp`, `PointEntered` and `PointLeaved`. Rejected sequences still receive Started and End, but no point actions. `PointSequenceEndInfo::accepted` describes that local answer.

`NativePointInputState` belongs to the native window, on its UI thread. Its unsigned 64-bit lifetime counter allocates nonzero sequence IDs; resetting a sequence never resets that counter. Source tokens identify live physical/injected contacts. Each Down allocates a zero-based `pointId` in Down order. Up removes the contact before its callback. A reused source token after Up receives a new slot. `activePointCount` is the number of live contacts after the transition, not the sparse list length. The platform `primary` flag does not define sequence lifetime or fallback ownership.

`BeginNativePointSequence`, `UpdateNativePointInput`, `EndNativePointSequence` and `ResetCompletedNativePointSequence` implement explicitly bounded sources. `ProcessNativePointInput` wraps them for native/automation input and ends successfully after the last Up. Cancellation emits one End without fabricated Ups, then retains native contacts until cleanup retires them through `RetireNativePoint`. Reasons are `Succeeded`, `Canceled_Unknown`, `Canceled_PlatformCanceled`, `Canceled_CaptureLost` and `Canceled_WindowDestroyed`.

Windows acquires touch through WM_POINTER in `<GacUI repo>/Source/PlatformProviders/Windows/WinNativeWindow.cpp`. Ordinary local Windows applications let rejected touch use platform mouse compatibility. A remote renderer accepts locally to own the complete stream and suppress platform promotion; the Core makes the application-level decision. Pen and physical mouse keep their existing mouse path. Other native providers must explicitly implement acquisition before claiming device support.

## Payload and coordinate spaces

`NativeWindowPointInfo` stores native client coordinates; `WindowPointInfo` stores logical coordinates. Both carry sequence/contact IDs, monotonic microsecond timestamp, active count, position, primary hint, Ctrl/Shift/Alt/Super, optional contact bounds, pressure, orientation and opaque platform frame ID. Missing metadata stays null; zero is a present value. Platform frame IDs are never contact IDs.

Hosted routing captures one hosted window for the sequence and adjusts position and contact bounds for that window's rendering offset. The graphics host converts native coordinates using its window's DPI conversion. Event bubbling then rebases both the sample and sparse points for each receiving composition. Pressure, timestamps, frame identity, modifiers and primary are copied unchanged. Browser coordinates are CSS pixels relative to the stable renderer target, independent of canvas device-pixel ratio.

## Composition contract and lifetime

Compositions expose `pointSequenceStarted`, unified `pointAction`, and `pointSequenceEnded` through `GuiGraphicsEventReceiver`. `GuiPointEventsType` distinguishes Started, Down, Update, Up, Enter and Leave. Started is the query, not an extra Down action. Acceptance is sticky as the query bubbles; `handled` stops propagation but does not imply acceptance. The first hit composition remains the sequence target even when contacts move outside it or other contacts land elsewhere.

`GuiPointEventArgs::GetPoints()` returns a borrowed host-owned `List<Nullable<Point>>`. Started and the first Down use slot 0. Lifted slots become empty and are not reused. At successful End all allocated slots are empty. At canceled End occupied contacts and earlier holes remain visible through End handlers. Rejected End exposes an empty list. After End, the list is cleared; copy values when retaining them beyond a callback. Detach handlers and respect composition deletion/capture lifetime instead of retaining event-argument pointers.

## Wire boundaries and Core dispatch

The source is `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_IO.txt`; regenerate C++ schema and metadata with Metadata_UpdateProtocol, then GacJS import/codegen. The seven ordered records are `IOPointStarted`, `IOPointDown`, `IOPointUpdate`, `IOPointUp`, `IOPointEnter`, `IOPointLeave`, and `IOPointEnded`. Started carries the first sample at eventIndex 0. First Down repeats that sample at index 1. All later actions and End share one strictly consecutive index stream across contacts. These events have no repeat-dropping annotation and never use the pending mouse-move slot.

Sequence IDs, timestamps and optional platform frame IDs are canonical unsigned decimal strings, including values above 2^53 through 18446744073709551615. Null differs from `"0"`. Decode checks digits, canonical spelling and overflow without passing through `vint` or a JavaScript number. Contact and event indices remain bounded integers.

Core ignores actions/End outside a sequence, foreign sequence IDs, nested Started, retired sequence identities and stale/duplicate indices. A forward index gap is a protocol error. It verifies first Down matches Started, contacts follow Down order, and active counts match bookkeeping. Final Up does not synthesize End. Core assigns its own public sequence ID from the same window counter used by Core automation, so renderer-local identities cannot collide with completed automation sequences. The End acceptance bit is replaced with Core's latched answer.

Rejected streams use `ProcessIOPointFallback`, shared with typed automation dispatch: only the original first contact generates mouse Down/Move/Up. Its Up releases fallback even while other contacts remain, and no contact is promoted afterwards. Modifiers and coordinates come directly from the sample. Cancellation releases a held fallback button outside the client to avoid activation. Disconnect/window destruction cancel active Core input and release fallback/capture; disconnected renderers stop forwarding. Channel lifetime isolation remains the transport layer's responsibility.

## Browser acquisition and drawing

GacJS handles touch PointerEvents on its stable renderer target, captures each pointer there, and installs `touch-action: none` for the target and all descendants before contacts begin. This covers nested overflow-hidden labels, paragraphs and raw canvases after DOM replacement. It processes coalesced samples in order. Normal Up followed by lostpointercapture does not cancel other contacts. Unexpected lost capture cancels once, retains the contact until physical Up/Cancel, and drains canceled cleanup without new point actions. A reused pointer ID receives a fresh Down-order slot.

Cancelable touch pointerdown is handled by a non-passive listener. Touch-origin mouse compatibility, boundary events and activation defaults are suppressed synchronously using pointer/source-capability correlation. Independent mouse/pen input and non-touch activation retain their path. Connected disposal sends one `Canceled_WindowDestroyed` End before removing event listeners, captures and surface styling. After transport loss, `stop(true)` performs silent cleanup; Core owns the disconnect cancellation boundary.

`Misc > Touching` owns gesture logic in `<GacUI repo>/Test/Resources/App/FullControlTest/TouchingTabPage.xml`. Raw state is `centerX,centerY,axisX,axisY,count`, followed by `count` triples `pointId,x,y`. The native and browser painters draw the supplied transformed rectangle and contact markers. They do not interpret gestures. Changing raw IDs replaces the native painter attachment; browser subscriptions own canvas/redraw cleanup. The browser painter and input contract are shared with the WASM entry, but WASM runtime verification is a separate task.

## Automation commands and endpoints

The parser is in `<GacUI repo>/Source/Utilities/SharedServices/GuiSharedAutomationService.cpp`. Commands are case-sensitive:

`!PointDown:CONTACT,X,Y[,MODIFIER...]` (also Update, Up, Enter, Leave) and `!PointCancel:CONTACT,REASON`. CONTACT is a nonnegative canonical integer source token fitting the platform's `vint`; it is not the public Down-order slot. X/Y are signed logical client coordinates. Modifiers accept Ctrl/Control, Shift, Alt/Menu, Win/Command/Super, case-insensitively. Held automation modifiers are combined with per-sample modifiers. Cancellation reasons are exactly Unknown, PlatformCanceled, CaptureLost, or WindowDestroyed.

For example, post each line as a separate UTF-8 body with Content-Type `application/json; charset=utf8`:

```text
!PointDown:10,80,100,Ctrl
!PointDown:20,180,100,Ctrl
!PointUpdate:20,200,120,Ctrl
!PointUp:10,80,100,Ctrl
!PointUp:20,200,120,Ctrl
!PointDown:10,80,100
!PointCancel:10,PlatformCanceled
```

Use `http://localhost:8888/Automation/RemotingTest_Core/IO` for Core injection and `http://localhost:8889/Automation/RemotingTest_Rendering_Native/IO` for a native renderer started with `/AsPort:8889`. Omit window IDs for hosted/remote endpoints. Core exposes Controls; a native renderer exposes Dom. Ordinary Windows automation exposes its application's Controls and IO endpoints. GacJS input must be driven in the browser; it has no separate native renderer automation endpoint. A Queued response means dispatch was scheduled: observe state before asserting completion.

Core injection verifies application dispatch, native renderer injection additionally verifies forwarding, and OS/browser injection verifies acquisition. Keep those results separate from real touch hardware. `<GacUI repo>/Test/Touching_NativeInput.ps1` supports standalone and remote renderers with distinct renderer/Core ports and verifies transported Dom state. The shared operations and expected outcomes are in `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md`.
