Details are in [TODO_Touch.md](../TODO_Touch.md), but this document has a much bigger scope, so keep the target scope in this request, uncovered parts will be handled in future requests.

There is a known issue when handling mouse events on a composition, the handler deletes the composition, and GacUI will crash. This is expected, and point events will be handled just like how mouse events are handled, it will have the same issue, and this is also expected. When implementing point events, you are going to figure out if any code for mouse events could be reused for propogating and capturing. GacUI will offer a pair of new capture/release functions for point events, they work like how captures are done to mouse events.

- Use point as the term as we dont use legacy touch on both Windows and browser.
- Add all point events to `INativeWindowListener` with Windows implementation.
  - gesture will not be included in this version.
- A point sequence begin and point sequence end event before and after a series of point events, when an event share at least one finger with the next event.
  - Point sequence begin starts with the first finger, it will be used to query id that position supporrs touching, responsing "No" will stop reporting all following events.
  - A sequence has an unique id per native window.
  - Not supported sequence begin also denies gesture.
- Maps events to `GuiGraphicsComposition`'s event receiver:
  - The first point locks the composition receiving point events for the complete sequence.
  - It also handles the point sequence begin event, and the event handler needs to declare if it accepts point events. Such information will be reported back to `INativeWindowListener`, with the same behavior, which is when the composition does not accept point events, all subsequence events will not raise.
- `FullControlTest` has a dedicated page next to `RawClock`, named `Touching`, to demo with single point to drag, with two finger to move, rotate and zoom.
  - It will be a sky blue rectangle to move, rotate and zoom, but since it should be rotated, you are going to draw a technically polygon.
  - All existing points should draw a yellow circle around 32x32.
  - Points are not required to hit the rectangle, but the first point should hit the `<Raw/>`.
  - Detailed behavior:
    - When there is only one point pressing, it moves the rectangle.
      - Relative position will be remembed no matter points increased or decreased to one.
    - When there are two points pressing, they move, rotate and zoom the rectangle as if you are pushing the paper. Points will be relative to the rectangle scaled.
      - Relative position will be remembed no matter points increased or decreased to two.
    - Where there are more than two points, nothing is going to happen.
    - Since point events are taken care of, so dragging by mouse on `<Raw/>` will not respond.
  - Actual rendering will happen in `Test\GacUISrc\Generated_FullControlTest\FullControlTestRawElement.cpp`
- Fix automation service to support touching as IO input:
  - A series of point events are sent, but when the point sequence begin is denied, the first point will be converted to mouse events with left button and other points will be ignored. Automation service doesn't need point sequence start/event, those are generated inside automation service.
- No need to update `DebugRemoteProtocolSop.md` for now.
- No need to update KB pages for now.
- No need to fix remote protocol for now, that means it is fine when the native remote renderer does not respond to touching.

I will verify after you completing everything with a device that has a real touch monitor.

## DETAILS

### Scope and existing design

- The overview is [<GacUI repo>/TODO_Touch.md](TODO_Touch.md); the opening link points to the parent directory by mistake. Use its section 0 declarations and sections 3.2-3.9 for the point contract, subject to this task's narrower scope and explicit automation fallback requirement.
- Implement Windows touchscreen acquisition through `WM_POINTER*`, native listener delivery, hosted-window forwarding, composition routing/reflection, automation IO, and the native `Touching` showcase. Browser/remote transport, other platform acquisition, pen input, gesture recognition, inertia, and built-in list/text-control behavior remain later work.
- The capture/release pair means protected `GuiGraphicsHost` helpers corresponding to `MouseCapture`/`MouseUncapture`. It does not require a new native service or public OS capture API. Preserve the stated limitation on deleting/reparenting a receiving composition during dispatch.

### Sequence and event contract

- A first Down allocates a nonzero ID unique during the native window's lifetime and synchronously queries `PointSequenceStarted` before Down delivery or mouse fallback. Keep the same sequence while contacts overlap, including after its original finger lifts. The last normal Up precedes one successful `PointSequenceEnded`.
- Use the overview's Down/Update/Up/Entered/Leaved callbacks and Started/Ended boundaries. Enter/Leave describe native-surface boundaries for active contacts, not retargeting between compositions. Stationary updates are valid. Keep default listener implementations non-pure and no-op so existing providers/listeners remain source compatible.
- Query native listeners independently with initial `accept == false`, then OR their answers. Broadcast accepted native data through the ordinary listener list. A graphics host whose own composition chain declined still suppresses its own point-data events.
- Composition acceptance starts false and is sticky while the query bubbles. `handled` stops propagation but does not itself accept. Latch the answer until the sequence ends; later fingers, changed positions, and changed handler state do not trigger another query.
- Interpret rejection's suppression of subsequent events as suppression of point-data events. Started/Ended remain paired, including rejection, as specified in the overview. Rejected End reports `accepted == false` and an empty composition points list. No gesture path is enabled by either answer.
- Point IDs are zero-based Down-order indices within a sequence. Preserve holes and never reuse/compact slots until End. Started and first Down share slot 0; Up empties its slot before handlers run. Successful End sees all slots empty; canceled End retains positions of contacts still down. Clear the borrowed list only after End propagation. Copy values that must survive a callback.
- Cancellation ends the whole public sequence once, without synthetic successful Ups. Keep the original accepted/rejected policy while draining remaining native activity; delayed cleanup must not restart or end it again. Use the overview's cancellation reasons, point fields, optional-value semantics, and coordinate conventions.

### Windows, hosted windows, and composition integration

- Native acquisition/bookkeeping belongs to `WindowsForm` in `<GacUI repo>/Source/PlatformProviders/Windows/WinNativeWindow.cpp`. Copy valid native data during message handling and translate screen positions/contact bounds to native client coordinates. Keep Windows code independent of composition hit testing.
- Windows fallback and automation fallback are separate paths. Rejected real Windows input uses OS compatibility mouse processing; rejected automation input explicitly synthesizes the first contact's left-mouse stream. Native verification must establish that accepted contacts do not also produce mouse input, rejected contacts retain mouse fallback, and physical mouse input still works.
- Passing pointer input to `DefWindowProc` can produce gestures or mouse input, and selectively consuming pointer input has undefined behavior. Preserve a consistent processing decision for the sequence. The overview's gesture-suppression policy still needs integration verification; configuring gesture suppression alone does not prove mouse fallback. See [WM_POINTERDOWN](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointerdown) and [SetGestureConfig](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setgestureconfig).
- In `<GacUI repo>/Source/PlatformProviders/Hosted/GuiHostedController.cpp`, select an eligible logical window using the first point's actual position and existing enabled/modal rules. Do not reuse mouse `hoveringWindow`: the mouse cursor may be elsewhere. Retain the chosen window and sequence ID, translating every later sample using its current offset. Secondary fingers over other logical windows do not change the route.
- In `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.cpp`, inspect `MouseCapture`, `RaiseMouseEvent`, and `OnMouseInput` for shared hit testing, source assignment, parent traversal, coordinate conversion, and `handled` stopping. Preserve separate point/mouse state. Mouse release timing cannot be copied unchanged: point capture survives through End handlers.
- Rebase the changed point, optional contact bounds, and every occupied list entry for each receiving composition, including End's list. Use retained native samples and existing DPI conversion instead of repeatedly converting already-rebased values.
- Extend `DisconnectCompositionInternal`, `SetNativeWindow`, and hosted `DestroyNativeWindow` cleanup to clear removed point targets without retargeting or changing fallback. Emit native `Canceled_WindowDestroyed` before `WindowsForm::InvokeDestroying` notifies listeners: `GuiControlHost::Destroying` detaches the graphics host, so later notification would lose the composition route. Deferred removal outside dispatch must not leave stale pointers; synchronous in-handler deletion remains unsupported.

### Automation IO

- Extend `RunIOCommandOnNativeWindow` and its command declarations in `<GacUI repo>/Source/Utilities/SharedServices/GuiSharedAutomationService.h` and `<GacUI repo>/Source/Utilities/SharedServices/GuiSharedAutomationService.cpp`. Both HTTP endpoint implementations already call the shared automation service; no separate point endpoint is needed.
- Define and document commands for contact Down/Update/Up with explicit contact identity and logical client coordinates, plus the stimuli needed to test boundaries/cancellation. Callers do not supply sequence Started/Ended commands or sequence IDs. Generate those internally using the same listener contract; do not inject composition events directly.
- Keep contact identity, active contacts, acceptance, sequence allocation, and fallback ownership scoped to the target native window across IO requests. A new request must not reset an active sequence or its ID counter. Remove state with its owning window so a reused address cannot inherit it. Preserve the existing automation-only input assumption rather than adding simultaneous physical/injected-input merging.
- Parse syntax before queueing, then execute point-state transitions and listener delivery on the UI thread in command order. Preserve synchronous syntax errors and the existing `Queued` success response; it does not mean handlers or rendering have completed. Invalid contact transitions should follow the existing fail-fast state-error convention rather than inventing silent recovery.
- On rejection, the original first contact produces left-button Down, movement, and Up through the existing mouse helpers. Other contacts produce no mouse events, but remain tracked until the sequence ends. If the first contact lifts early, release its left button then; never promote another contact. Accepted input produces no fallback mouse stream. The next sequence queries acceptance anew.

### Touching showcase

- Add the page beside the existing raw-clock page through `<GacUI repo>/Test/Resources/App/FullControlTest/Resource.xml` and its authored page resources. Keep event handling and transform/contact state in the page's XML/Workflow, and native painting in the manually maintained `<GacUI repo>/Test/GacUISrc/Generated_FullControlTest/FullControlTestRawElement.cpp` and its header.
- Follow the raw clock's `Id`/`Data` convention with a distinct element ID and serialized transform/contact state. The existing Controls dump exposes `elementId` and `elementData`, allowing automation to inspect results. Expose the element from `MainWindow` and attach its painter in `CppTest`, `CppTest_Metaonly`, `CppTest_Reflection`, and `GacUI_Host`; the binary-resource host obtains the property through reflection. Implement Direct2D and GDI painting and preserve render-target setup/teardown.
- Accept when the first point hits the raw drawing composition, even outside the rectangle. Keep all active markers centered at their contact positions, approximately 32 by 32 logical units, independent of rectangle scale. Later points need not hit the rectangle or original raw bounds.
- With one contact, translate from the saved contact/rectangle baseline while retaining scale and angle. With two contacts, derive translation from their midpoint, rotation from their directed segment, and uniform scale from their separation. Rebase from the current rectangle whenever active membership changes, including transitions through more than two contacts; adding/removing a contact alone must not jump the rectangle.
- Above two contacts, freeze the rectangle while continuing to update every marker. For coincident two-contact positions, preserve the transform until a nonzero separation permits a fresh baseline; do not divide by zero or collapse the rectangle to an unusable scale. On End/cancellation, clear markers and manipulation baselines while retaining the last rectangle transform. Ordinary mouse dragging does not manipulate the rectangle.

### Reflection and generated resources

- Add type-list entries and value/event registrations in `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionPlugin.h`, `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionBasic.cpp`, `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionEvents.cpp`, and `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionCompositions.cpp`. Initialize every new receiver event with its owner in `<GacUI repo>/Source/Application/GraphicsCompositions/GuiGraphicsEventReceiver.cpp`.
- Follow the overview's read-only `Points` property backed by `GetPoints`. Borrowed-list event arguments are host-constructed; existing `EVENTARGS_CONSTRUCTOR` assumptions and reflected field registration are unsuitable for their reference member. Verify Workflow event handlers and list access in compiled and interpreted consumers.
- Generate architecture-specific showcase source/resources and metadata through the owning projects in `<GacUI repo>/Project.md`. Do not hand-edit generated outputs. The manually maintained raw painter is outside the generated `Source_x86`/`Source_x64` subdirectories. Keep authored changes, generated output, and changed unit-test snapshots in their prescribed separate commits during implementation.

## VERIFICATION

These checks are for implementation of the task. This review only changes the task specification.

### Deterministic sequence and routing coverage

Use ordinary unit tests for event/state/geometry assertions; they do not require remote-protocol changes or empty rendering snapshots. Cover the overview's section 3.9 contract with particular attention to:

| Scenario | Required result |
|---|---|
| Target accepts; only ancestor accepts; nobody accepts; `handled` alone | Correct sticky decision, source/parent propagation, and exactly one Started/Ended pair. Rejection has no point-data callbacks. |
| Multiple native listeners disagree | Aggregate acceptance uses OR; a locally rejecting graphics host still receives no composition point data. |
| A Down, B Down, A Up, C Down, B Up, C Up | One sequence/route; indices 0/1/2 with holes; correct active counts; final Up before End; next sequence gets a new ID and starts at index 0. |
| Secondary points land on other compositions/logical windows | Original composition/window remains selected; the first finger lifting does not release the sequence. |
| Nested receivers, hosted-window movement, DPI conversion | Changed-point fields and every occupied borrowed-list entry are correct in each receiver's coordinates. |
| Capture loss, canceled Up, native destruction, repeated terminal cleanup | One canceled End with remaining occupied slots; no successful synthetic Ups, duplicate End, or restarted sequence from trailing packets. |
| Deferred composition detach or hosted-window removal | No stale delivery or retargeting; a later fresh sequence can select a new valid target. |
| First touch away from mouse cursor; disabled/modal hosted targets | Selection follows touch position and existing window eligibility, without altering mouse capture/hover state. |

Run existing mouse-routing/capture regressions in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_GraphicsHost_Mouse.cpp` after any shared traversal refactor. Preserve the explicit exclusion of safe synchronous deletion inside input handlers.

### Automation and demonstration coverage

1. Exercise the documented point grammar through `/IO` and `/IO/<WINDOW-ID>` where supported. Verify malformed syntax fails synchronously without queueing work, valid commands return `Queued`, and subsequent Controls readback confirms execution. Cover independent native-window state and sequential requests that continue one sequence.
2. Exercise accepted -> rejected -> accepted sequences. On an ordinary control, rejected automation produces exactly the original first contact's left-mouse interaction. Lift that contact while another remains, add another contact, then finish: no replacement mouse Down, no stuck left button, and no early new sequence. Accepted raw input produces no duplicate mouse interaction.
3. Verify one-contact translation and two-contact translation/rotation/scaling with known coordinates. Exercise 1 -> 2 -> 3 -> 2 -> 1 -> 0 contacts, changing which IDs remain, and coincident positions. Check no jumps on membership changes, frozen rectangle with more than two contacts, current markers for every active point, and marker cleanup on completion/cancellation.
4. Start inside the raw region but outside the rectangle, then move/add points outside the raw region. The accepted route continues. Ordinary mouse dragging on the raw region leaves the transform unchanged.
5. Launch each of `CppTest`, `CppTest_Metaonly`, `CppTest_Reflection`, and `GacUI_Host` with both default Direct2D and `/Gdi`, using the repository execution workflow. Check page placement, nonempty raw state, identical manipulation behavior, sky-blue polygon and yellow markers, resizing, and render-target recreation. Use state/geometry readback for assertions and rendered output for painter-specific checks.
6. Extend the shared showcase UIA assertions in `<GacUI repo>/Test/UIA_CppTest_Shared.cs` for the new tab's name, selection, and bounds; run both `<GacUI repo>/Test/UIA_CppTest.ps1` and `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1` as required by `<GacUI repo>/Project.md`.

### Generation, builds, and real Windows input

- Follow `<GacUI repo>/Project.md` and the repository build/run wrappers for fresh Win32/x64 metadata, `Metadata_Test`, `GacUI_Compiler`, rebuilding consumers, and required UnitTest runs. New reflection must be available to resource compilation, and metadata must be regenerated after resource compilation as prescribed. Inspect resource errors, relevant generated changes, test summaries, and memory-leak output. Keep relevant existing test filters and ensure the new tests run.
- Automation directly dispatches native-listener callbacks; it does not establish that `WM_POINTER*` acquisition, gesture suppression, promoted-mouse filtering, or OS fallback works. Separately exercise native multi-contact injection, for example through [InjectTouchInput](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-injecttouchinput), covering accepted/rejected transitions, capture loss/cancellation, leaving window bounds, physical mouse independence, and native/hosted coordinates. Record unavailable interactive/DPI coverage explicitly rather than treating synthetic listener tests as equivalent.
- Leave the real touch-monitor acceptance pass to the user as requested. Report implementation checks that actually ran and remaining hardware checks separately. No remote-renderer touch support, remote SOP update, or knowledge-base update is required for this step.

## REVIEW COMMENTS

No unresolved review comments. The details above apply the existing point design to this step's scope and current implementation structure.
