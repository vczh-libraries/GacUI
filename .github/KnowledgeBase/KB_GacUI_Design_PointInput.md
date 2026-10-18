# Point Input, Capture and Remote Forwarding

## Ownership and native acquisition

Point input is an overlapping-contact sequence. The first Down asks listeners through `INativeWindowListener::PointSequenceStarted(firstPoint, accept)`; acceptance is ORed and latched until End. Accepted sequences deliver `PointDown`, `PointUpdate`, `PointUp`, `PointEntered` and `PointLeaved`. Rejected sequences still receive Started and End, but no point actions. `PointSequenceEndInfo::accepted` describes that local answer.

`NativePointInputState` belongs to the native window, on its UI thread. Its unsigned 64-bit lifetime counter allocates nonzero sequence IDs; resetting a sequence never resets that counter. Source tokens identify live physical/injected contacts. Each Down allocates a zero-based `pointId` in Down order. Up removes the contact before its callback. A reused source token after Up receives a new slot. `activePointCount` is the number of live contacts after the transition, not the sparse list length. The platform `primary` flag does not define sequence lifetime or fallback ownership.

`BeginNativePointSequence`, `UpdateNativePointInput`, `EndNativePointSequence` and `ResetCompletedNativePointSequence` implement explicitly bounded sources. `ProcessNativePointInput` wraps them for native/automation input and ends successfully after the last Up. Cancellation emits one End without fabricated Ups, then retains native contacts until cleanup retires them through `RetireNativePoint`. Reasons are `Succeeded`, `Canceled_Unknown`, `Canceled_PlatformCanceled`, `Canceled_CaptureLost` and `Canceled_WindowDestroyed`.

Windows acquires touch through WM_POINTER in `<GacUI repo>/Source/PlatformProviders/Windows/WinNativeWindow.cpp`. Ordinary local Windows applications let rejected touch use platform mouse compatibility. A remote renderer accepts locally to own the complete stream and suppress platform promotion; the Core makes the application-level decision. Pen and physical mouse keep their existing mouse path. Other native providers must explicitly implement acquisition before claiming device support.

## Payload and coordinate spaces

`NativeWindowPointInfo` stores native client coordinates; `WindowPointInfo` stores logical coordinates. Both carry sequence/contact IDs, monotonic microsecond timestamp, active count, position, primary hint, Ctrl/Shift/Alt/Super, optional contact bounds, pressure, orientation and opaque platform frame ID. Missing metadata stays null; zero is a present value. Platform frame IDs are never contact IDs.

Hosted routing captures one hosted window for the sequence and adjusts position and contact bounds for that window's rendering offset. `GuiPointActionManager` in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost_Point.h/.cpp` stores canonical native screen positions. Each callback rebuilds logical samples, contact bounds and occupied sparse slots with the current window conversion, client origin and receiving composition bounds. Stationary contacts and the gesture baseline therefore follow DPI changes and window movement without accumulated rescaling. Pressure, timestamps, frame identity, modifiers and primary are copied unchanged. Browser coordinates are CSS pixels relative to the stable renderer target, independent of canvas device-pixel ratio.

## Composition contract and lifetime

Compositions expose `pointSequenceStarted`, unified `pointAction`, and `pointSequenceEnded` through `GuiGraphicsEventReceiver`. `GuiPointEventsType` distinguishes Started, Down, Update, Up, Enter and Leave. Started is the query, not an extra Down action. Its independent `GuiPointSequenceStartedEventArgs` has `acceptedComposition` and source/sample fields, with neither `accept` nor inherited `handled`. The first handler returning a non-null selection wins; later handlers cannot clear or replace it. The query still visits every ancestor of the original hit composition. Selection must belong to the same graphics host.

`Workflow_GenerateEventHandler` in `<GacUI repo>/Source/Compiler/WorkflowCodegen/GuiInstanceLoader_WorkflowInstallBindings.cpp` recognizes both `GuiEventArgs` descendants and the independent Started query when supplying the conventional XML handler names `sender` and `arguments`. Borrowed-list event arguments have no reflected constructors.

Action and End go directly to the selected composition, with both sources set to that composition and `handled` initially false. Their coordinates are relative to it, even outside its bounds. Rejected sequences have no composition Action or End recipient; the native listener contract still reports End. `GuiPointEventArgs::GetPoints()` and the query/completion equivalents return a borrowed host-owned `List<Nullable<Point>>`. Started and the first Down use slot 0. Lifted slots become empty and are not reused. At successful End all allocated slots are empty. At canceled End occupied contacts and earlier holes remain visible through End handlers. After End, the list is cleared; copy values when retaining them beyond a callback. Detach handlers and respect composition deletion/capture lifetime instead of retaining event-argument pointers.

## Control acceptance, scrolling and deferred clicks

`GuiControl::AcceptPointEvents` defaults to false. The three protected virtual point hooks are attached once to the persistent bounds composition. The default Started hook selects those bounds only while unclaimed. The read-only `PointEventComposition` property returns the active manager target when it is inside the control's bounds subtree, including during End, and null after cleanup or detachment. It is observed state, not another routing decision.

`GuiScrollView` defaults to accepting points. A nearer accepting child owns its sequence. When the scroll view's own bounds own the sequence, unhandled actions from point 0 scroll along one locked axis. Recognition crosses a radial distance strictly greater than eight logical units; the larger absolute displacement chooses the axis, with horizontal winning ties. Final Up also participates. View position is the initial view position minus displacement from the original point. The locked scrollbar must exist, be visible and visually enabled; ordinary scrollbar APIs clamp its range. Reversal keeps the original baseline, and unavailable axes neither hand off to a parent nor switch axes. Other contacts never replace point 0, and any additional Down permanently disqualifies a click.

`GuiPointSequenceEndedEventArgs::fallbackClick` defaults to false. The scroll view's unhandled End hook requests it; custom surfaces such as Touching leave it false. The manager grants a button only after successful End, exactly one Down and no threshold crossing: less than 500000 microseconds yields left, otherwise right. The host clears point capture and samples before synthesizing ordinary MouseMove, Down and Up at the original start screen position using current conversion/layout. This establishes normal hover and button behavior while preserving independent physical mouse capture and button bookkeeping. Handled suppresses the scroll view's default request, but an explicit custom fallback request remains authoritative. Native rejection fallback remains a separate existing path.

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
