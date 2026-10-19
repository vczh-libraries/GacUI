# Point Input, Capture and Predefined Gestures

## Native listener contract

Touch is an overlapping-contact sequence, delivered through `INativeWindowListener`. Implemented acquisition is currently Windows touch and the remote protocol provider. Pen and physical mouse retain their mouse path. Forwarding support does not imply hardware acquisition on another native platform.

The first Down queries `PointSequenceStarted(firstPoint, accept)`. Listener acceptance is ORed and latched through End. An accepted sequence delivers `PointDown`, `PointUpdate`, `PointUp`, `PointEntered` and `PointLeaved`. Rejected sequences receive Started and End but no point actions; `PointSequenceEndInfo::accepted` describes that window's answer. Started is a query, not an extra Down.

`NativePointInputState` in `<GacUI repo>/Source/NativeWindow/GuiNativeWindow.h` owns the native-window UI-thread sequence. A nonzero unsigned 64-bit sequence ID comes from a lifetime counter that survives sequence reset. Source tokens identify physical/injected contacts; every Down receives a new zero-based `pointId` in Down order. A reused source token receives a new slot. Up removes its contact before dispatch. `activePointCount` counts occupied contacts, not allocated slots; `primary` does not define ownership.

`BeginNativePointSequence`, `UpdateNativePointInput`, `EndNativePointSequence` and `ResetCompletedNativePointSequence` support explicitly bounded sources. `ProcessNativePointInput` handles native/automation acquisition, completing after the last Up. Cancellation reports exactly one End without invented Ups, then drains native contacts with `RetireNativePoint`. Reasons are `Succeeded`, `Canceled_Unknown`, `Canceled_PlatformCanceled`, `Canceled_CaptureLost` and `Canceled_WindowDestroyed`.

`NativeWindowPointInfo` uses native client pixels; `WindowPointInfo` uses logical pixels. Samples carry IDs, monotonic microsecond timestamps, active count, position, modifiers, primary hint and optional bounds, pressure, orientation and opaque platform frame ID. Null metadata differs from zero. Frame IDs never identify contacts.

Hosted routing captures one hosted window and rebases position/contact bounds by its rendering offset. The graphics host preserves canonical native screen positions and reprojects both stored baselines and current contacts with the current window conversion, client origin and accepted-composition bounds. DPI/window changes therefore do not accumulate stale logical-coordinate scaling.

Windows WM_POINTER acquisition lives in `<GacUI repo>/Source/PlatformProviders/Windows/WinNativeWindow.cpp`. Ordinary Windows applications let rejected touch use platform mouse compatibility. Remote renderers accept locally to own the complete stream and suppress promotion; Core makes the application-level acceptance decision.

## Remote protocol and renderer acquisition

The wire schema is `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_IO.txt`. Touch senders currently include Windows and GacJS. The records are `IOPointStarted`, `IOPointDown`, `IOPointUpdate`, `IOPointUp`, `IOPointEnter`, `IOPointLeave` and `IOPointEnded`. Started has eventIndex 0; first Down repeats its sample at 1. All later actions and End share one strictly consecutive index stream across contacts. These records are never coalesced through the pending mouse-move slot. Final Up does not replace explicit End.

Sequence IDs, timestamps and optional frame IDs are canonical unsigned decimal strings through 18446744073709551615. Never round-trip them through JavaScript numbers or `vint`. Contact/event indices remain bounded integers. Core ignores actions outside a sequence, foreign identities, nested Started, retired identities and stale/duplicate indices. Forward gaps are errors. Core validates first-Down equality, Down-order IDs and post-transition active counts; it assigns its own public sequence identity from the window counter also used by automation. End acceptance is Core's latched answer.

Rejected Core streams use `ProcessIOPointFallback`: only the original first contact sends mouse Down/Move/Up, with its original modifiers and current position. Its Up releases the mouse even if another contact remains; later contacts never replace it. Cancellation releases outside the client to avoid activation. Disconnect/destruction cancels active input and releases fallback/capture.

GacJS acquires touch PointerEvents on a stable renderer target, with pointer capture, non-passive pointerdown and `touch-action: none` on that target and descendants before input. Coordinates are CSS pixels independent of canvas pixel ratio. Coalesced samples remain ordered. Normal lostpointercapture after Up does not cancel other contacts; unexpected loss cancels once and drains cleanup. Touch-origin compatibility mouse/boundary/activation events are suppressed while independent mouse/pen input retains its path. Connected disposal sends one canceled End; disconnected cleanup is silent because Core owns that boundary.

## Composition events, selection and lifetime

`GuiGraphicsEventReceiver` in `<GacUI repo>/Source/Application/GraphicsCompositions/GuiGraphicsEventReceiver.h` exposes four events:

| Event | Recipient and purpose |
| --- | --- |
| `pointSequenceStarted` | Original hit composition and ancestors, querying one `acceptedComposition`. |
| `pointAction` | Accepted composition only; Down/Update/Up/Enter/Leave and sparse contacts. |
| `pointGesture` | Accepted composition only; offered incremental gesture payload. |
| `pointSequenceEnded` | Accepted composition once, on success or cancellation. |

The first non-null Started selection is sticky within dispatch and across ancestor traversal. Handled queries still visit handlers and ancestors, permitting explicit custom acceptance. Selection must belong to the same graphics host. Started borrows a temporary one-slot list, rebased for each receiver. Only after acceptance does the host initialize its private manager and establish capture, before recognizer Started callbacks. A rejected sequence cannot consult earlier recognition state.

Action, Gesture and End have both source fields set to the accepted composition; they do not bubble or hand off to parents. Each event starts unhandled. Gesture `selected = true` is sticky within dispatch and for the rest of the sequence, independently of `handled`. Consumers should select only an unhandled offer, then mark it handled if consumed. Later handlers cannot undo selection. Selecting or pausing a gesture does not stop raw Action bookkeeping or the single public End.

Action/Started/End `Points` is a borrowed `List<Nullable<Point>>` in receiving-composition coordinates. Slot 0 belongs to the first contact, lifted slots are empty and IDs are never reused. Successful End has empty allocated slots; canceled End retains live contacts and holes. Capture and borrowed storage remain valid through End callbacks and are cleared afterwards. Copy values to retain them beyond a callback. Synchronous deletion of the receiving composition during input has the same unsupported lifetime behavior as mouse callbacks; queue removal with `InvokeInMainThread` and retain data identity, not a row composition or stale index.

`GuiControl::PointEventComposition` observes capture inside the control's bounds subtree, including during End. It becomes null after cleanup/detachment. The host exposes only a narrow capture query; there is no public manager access or recognizer registration API.

### Typed C++ payload and Workflow bridge

`GuiPointGestureEventArgs` is a reflected class deriving from `GuiEventArgs`, with writable `selected` and a read-only object-valued `Gesture` property backed by `GetGesture()`. Its native public `gesture` is exactly `Variant<GuiPointGesture_Scrolling, GuiPointGesture_DraggingAndRotation>` and is not registered as a reflected field. The getter boxes an independent copy of the active value struct and works in generated C++ without reflection metadata. There is no null payload or setter; changing a boxed copy cannot change the native payload.

Workflow consumers inspect the runtime object type:

```text
if (not arguments.handled and arguments.Gesture is GuiPointGesture_Scrolling)
{
  var scrolling = cast GuiPointGesture_Scrolling arguments.Gesture;
  arguments.selected = true;
  arguments.handled = true;
  // Apply scrolling.distance along scrolling.isVertical's axis.
}
```

The same type-check/cast pattern applies to `GuiPointGesture_DraggingAndRotation`. XML event handlers receive the conventional `sender` and `arguments` names through their `GuiEventArgs` ancestry. Borrowed-list argument types have no reflected constructors.

## Four fixed recognizers and click fallback

`GuiPointActionManager` in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost_Point.h` is a private graphics-host helper for contacts, conversion, capture and recognizer dispatch. `IGuiPointGesture` is internal. Its fixed implementations are defined in `predefined_gestures` in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost_Point.cpp`, in this priority order:

1. `RightClickGesture` recognizes a successful single-Down sequence lasting at least 500000 microseconds with no excursion beyond eight logical units.
2. `LeftClickGesture` recognizes the same eligible sequence without the duration minimum. Right wins at the time boundary.
3. `ScrollingGesture` offers locked-axis incremental movement after crossing the radial eight-unit threshold.
4. `DraggingAndRotationGesture` offers identity immediately at Started and continues only if that offer is selected.

This extension boundary is for GacUI maintainers only. There is no end-user recognizer registration, OS gesture integration, renderer-specific recognition or additional wire message.

Started resets candidates until one wins. Action and End visit candidates in order until selected; afterwards only the winner receives recognizer callbacks. Public raw End is independent and delivered once, including when scrolling stopped after another contact. Click candidates do not emit events or select themselves. They require exactly one Down and a successful End; final Up participates in the excursion test. Wide arithmetic compares squared radius with `StaticPointRange * StaticPointRange`. Exactly eight stays eligible. Once crossed, returning to the start cannot restore click eligibility. Cancellation or any extra Down disqualifies clicks.

Eligible accepted sequences now get automatic deferred fallback without an End opt-in. The former `fallbackClick` field is removed; End `handled` cannot veto recognition. Selected scrolling/manipulation never clicks. Before synthesizing MouseMove, Down and Up, the host clears point state, then hit-tests at the original screen start using current conversion/layout and original modifiers. Independent physical mouse capture/button bookkeeping is preserved. This deferred accepted-sequence click differs from the immediate rejected-sequence mouse stream used for text selection.

### Incremental movement

Scrolling tracks point 0. At threshold crossing, the larger absolute displacement fixes the axis; horizontal wins ties. The first distance includes the entire projected displacement from initial Down. Later distances are signed projected increments (right/down positive). Any second Down permanently stops scrolling offers for that sequence, even after the extra point lifts. No contact replaces point 0. A declined locked scrolling offer may be offered again; declined initial manipulation is never offered again.

Manipulation starts with `(rotation=0, scale=1, distance=(0,0))`. One contact translates, two translate/rotate/scale, and more than two pause. Occupied IDs determine membership. Membership changes, coincident pairs and recovery from a pause establish a new baseline without a jump. Selection remains active through pauses.

Apply a payload in definition order, `p' = scale * R(rotation) * p + distance`, in accepted-composition logical coordinates. Positive radians rotate clockwise on screen. For two contacts, translation is `newCentroid - scale * R(rotation) * oldCentroid`, so off-origin rotation/scaling follows the contact centroid. Calculations use doubles. Integral `Point` translation rounds to nearest with ties towards positive infinity; transformed fractional residuals remain in `[-0.5, 0.5)` and carry into later increments. This bounds fractional drift and keeps unchanged samples stationary.

## Built-in controls and sample consumers

`GuiControl::AcceptPointEvents` defaults false. Four protected virtual hooks attach once to its persistent bounds. Only Started performs default acceptance: opted in, no previous owner and unhandled query. Action, Gesture and End defaults are empty.

`GuiScrollView` opts in and overrides only `OnPointGesture`. For an owned, unhandled scrolling gesture, it requires the locked-axis scrollbar to exist, be visible and visually enabled. It selects/handles and subtracts the increment from the current view coordinate through ordinary range-clamped APIs. Reversal therefore responds immediately after clamping. An unavailable axis neither switches axes nor hands off to an ancestor. A nearer accepting child remains the sole owner.

`GuiDocumentCommonInterface::ReplaceMouseArea` owns a Started handler alongside its mouse handlers. Enabled Selectable/Editable content marks the query handled without accepting, so default ancestor scroll views cannot take it. Rejected input supplies immediate mouse dragging and range selection. Explicit custom acceptance remains possible. Template replacement detaches/reattaches the handler; ViewOnly content and scrollbar/chrome starts retain scrolling behavior.

`Misc > Touching` in `<GacUI repo>/Test/Resources/App/FullControlTest/TouchingTabPage.xml` selects manipulation, applies payloads to its center/axis and keeps raw handlers only for acceptance and markers. Raw drawing remains `centerX,centerY,axisX,axisY,count` followed by `pointId,x,y` triples. Native/GacJS painters only render supplied state.

`Misc > TouchingList` in `<GacUI repo>/Test/Resources/App/FullControlTest/TouchingListTabPage.xml` accepts at list bounds. Its raw Started remembers the hit row's data identity; horizontal scrolling reveals a measured red DELETE IT region and vertical movement uses built-in scrolling. Row templates only present data. Reveal is clamped, right-only motion stays closed and reversal retracts. Raw extra Down/cancellation resets. Successful single-contact completion at maximum queues identity-based membership-checked removal. Clear and virtualization cannot redirect removal to a different row.

## Automation and verification boundaries

`<GacUI repo>/Source/Utilities/SharedServices/GuiSharedAutomationService.cpp` parses `!PointDown:CONTACT,X,Y[,MODIFIER...]` and the analogous Update/Up/Enter/Leave commands, plus `!PointCancel:CONTACT,REASON`. CONTACT is a canonical nonnegative source token fitting `vint`, not the public slot. X/Y are signed logical client coordinates. Ctrl/Control, Shift, Alt/Menu and Win/Command/Super modifiers are case-insensitive. Cancellation names are Unknown, PlatformCanceled, CaptureLost and WindowDestroyed.

POST individual UTF-8 command bodies with Content-Type `application/json; charset=utf8`. Core input uses `http://localhost:8888/Automation/RemotingTest_Core/IO`; a native renderer started with `/AsPort:8889` uses `http://localhost:8889/Automation/RemotingTest_Rendering_Native/IO`. Omit window IDs for hosted/remote endpoints. Core exposes Controls; native renderers expose Dom. Queued means scheduled, so observe resulting state before asserting completion.

Core injection checks application dispatch, renderer injection additionally checks forwarding, and OS/browser input checks acquisition. GacJS needs browser input because it has no native renderer automation endpoint. Record synthetic input separately from physical hardware. `<GacUI repo>/Test/UIA_CppTest_Shared.cs` contains focused Touching, TouchingList, list scrolling and text selection scenarios; `<GacUI repo>/Test/Touching_NativeInput.ps1` supports standalone and remote native input. Setup is documented in `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md`.

After rebuilding Debug x64 consumers, run `<GacUI repo>/Test/UIA_CppTest.ps1` and `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1` with `-Scenario Touching -SkipBuild`. The shared wrapper also accepts `-Application GacUI_Host -AsPort 8892` for interpreted Workflow. These checks combine automation-service point input with UIA roles, selection, scrolling, text ranges and template replacement.

`<GacUI repo>/Test/Touching_Remote.ps1 -Transport Http -Backend Direct2D` drives the native renderer's IO endpoint and observes Core state and renderer DOM. Add `-Browser` to launch the focused Chromium CDP touch fixture in `<GacUI repo>/Test/Touching_Browser.mjs`; it requires the sibling GacJS dependencies and built website served on port 8896. The browser fixture observes outgoing protocol events, rendered pixels and application state, checks outer-page scrolling and compatibility mouse suppression, and does not install its own input-suppression handlers. Both modes own their Core lifecycle and restore project debugger arguments.
