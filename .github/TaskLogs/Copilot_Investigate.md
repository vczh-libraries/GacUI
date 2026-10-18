# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

Details are in [TODO_Touch.md](./TODO_Touch.md), but this document has a much bigger scope, so keep the target scope in this request, uncovered parts will be handled in future requests.

`TODO_Touch_Step[2]?.md` are completed works.

There is a known issue when handling mouse events on a composition, the handler deletes the composition, and GacUI will crash. This is expected, and point events will be handled just like how mouse events are handled, it will have the same issue, and this is also expected. When implementing point events, you are going to figure out if any code for mouse events could be reused for propogating and capturing. GacUI will offer a pair of new capture/release functions for point events, they work like how captures are done to mouse events.

- Make sure `handled` (default false) is available on `point(Action|Sequence|Ended)` events.
  - This is a hint for events in compositions, in predefined controls, if `handled` is true, it means there is a sub composition to override the behavior, any event handler in predefined controls should skip this event.
  - `accept` in `pointSequenceStarted` should be updated to `acceptedComposition`, and all future point events should be sent to this composition. If it is `nullptr`, it means not accepted. This is only for compositio's event, it does not affect `INativeWindowListener` design. `pointSequenceStarted` does not have `handled`, but assigning `acceptedComposition` has the same hint.
    - Unlike `pointSequenceStarted` which will sent to the hittest deepest composition and its all parent compositions, `pointAction` and `pointSequenceEnded` will only sent to `acceptedComposition`.
- Add `bool fallbackClick` to `pointSequenceEnded`, false by default. 
  - `GuiGraphicsHost::pointsScale` should not exist, it should call `Convert` whenever scaling is needed. We can't assume scale does not change.
  - `GuiGraphicsHost` will also remember the position of the point in `pointSequenceStarted`, and when it is accepted the state machine will run, and `pointSequenceEnded` as a true `fallbackClick`, it will first hittest the position, and do regular mouse up and mouse down with the specified mouse button decided by the state machine. If the state machine decides to none, this won't happen. This should have a lot of shared code to trigger the mouse click.
- Add a `GuiControl::(Get|Set)AcceptPointEvents` property, default to false:
  - No matter the property value, `pointSequenceStarted` of its bounds composition will be listened, and when it is true, `acceptedComposition` will be assigned with the bounds composition when `handled` is false.
  - When `assignedComposition` is assigned, the reasonly property `GetPointEventComposition` will return that composition.
  - In all conditions, `pointSequenceEnded` will clear `GetPointEventComposition`.
  - All three point events will be handled by its virtual protected function `OnPoint(Action|Sequence(Started|Ended))`.
- As an exception, `GuiScrollView` will call `SetAcceptPointEvents(true)`:
  - In `OnPointSequenceStarted`, it will remember the position of the first point.
- Implement a state machine in `GuiGraphicsHost`:
  - When the first point drags more than 8 pixels, it will find the nearest direction of up-down or left/right, by checking the angle:
    - Lock that position. You can use 0/1/2 to represent it.
    - If the corresponding scroll bar is visibled and enabled, dragging means scrolling that much pixels comparing to the start position. Otherwise no scrolling happens.
  - Only the first point will be used for scrolling.
   - On `PointSequenceEnded`, only when there was one point activated and the 8 pixels never exceeds (you can check the state for example if it is 0), it fallbacks to left button, or right button when >= 0.5 second has passed.
- Just like `altActionManager` and `tabActionManager`, a `pointActionManager` should be created to store all point related code, leaving `GuiGraphicsHost` to do:
  - Trigger `pointSequenceStarted` by itself so that the `acceptedComposition` will be stored.
  - The rest of the events will be redirected to `pointActionManager`.
  - When `pointSequenceEnded` is called, the `Nullable<NativeMouseButton> pointActionManager->GetFallbackClick()` will tell `GuiGraphicsHost` if a fallback needs to happen.
  - This refactor is to clean up as many code as possible from `GuiGraphicsHost` so all those complex logic will not mix up.
  - `GuiGraphicsHost_Point.(h|cpp)` will be the file to store the new `pointActionManager`.
- To verify, you can find any scrollable list or text box and try to scroldl them:
  - Make new remote protocol based unit test case, no need to update `DebugRemoteProtocolSop.md`.
  - I will check snapshots later.

## DETAILS

### Scope and interpretation

- This step changes composition point ownership, adds control opt-in and basic axis-locked scrolling, and synthesizes an optional click after an accepted sequence ends. Preserve the completed native, hosted, automation, and remote-protocol contracts. Inertia, pinch recognition, parent scroll handoff, text-selection handles, mobile text input, and a timer-triggered context menu remain outside this request, even where the broader plan discusses them.
- The shared GacUI implementation must automatically support GacJS through its existing point-input forwarding to Core. Keep scrolling, recognition, and deferred click behavior in the shared graphics-host/control implementation; no separate TypeScript scrolling implementation or new wire messages should be needed. Native and GacJS manual FullControlTest verification are both required after implementation.
- Read `point(Action|Sequence|Ended)` as `pointAction` and `pointSequenceEnded`; the explicit exclusion of `handled` from `pointSequenceStarted` takes precedence. In the control acceptance rule, read "when handled is false" as `acceptedComposition == nullptr`. `assignedComposition` means `acceptedComposition`, and `GetPointEventComposition` is the getter for the read-only `PointEventComposition` property.
- A regular fallback click is mouse Down followed by mouse Up. The reversed order in the original wording does not describe the intended click sequence. Long hold selects the right button at sequence End; it does not fire while the contact remains down.
- The existing `PointCapture`, `PointUncapture`, and `ResetPointCapture` helpers in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.h/.cpp` already provide composition capture. Adapt/reuse them with the new manager instead of adding another native capture API. Point capture remains independent of mouse capture. Synchronous deletion inside an input handler remains explicitly unsupported.

### Event ownership and control lifecycle

- `acceptedComposition` starts as `nullptr`. Deliver Started from the deepest hit composition through its parents, rebasing the sample and borrowed points for each receiver. The first non-null assignment wins and remains visible to subsequent handlers; later handlers cannot clear it or replace it with an ancestor. A selected composition must belong to the current graphics host. Continue the start query through the ancestors as requested; acceptance is the hint for their default behavior to skip.
- After the query, store that final composition as the sequence target. Send Action and End only to its event receiver, with coordinates relative to it; they no longer bubble. Set their source fields consistently with direct delivery. Do not retarget when a point moves outside, a second contact lands elsewhere, or an ancestor is scrollable. `handled` defaults to false for each Action/End and suppresses predefined behavior, not mandatory contact bookkeeping or terminal cleanup.
- Native `INativeWindowListener::PointSequenceStarted(firstPoint, bool& accept)` and its paired native End remain unchanged. Derive this host's local answer from the selected composition. A rejected composition query has no selected target and therefore no composition Action/End recipient; the native lifecycle and the existing rejected-input mouse fallback still run. Update tests that previously expected a rejected composition End or bubbling Action/End.
- Started currently inherits `handled` through `GuiPointEventArgs` and `GuiEventArgs`. Adjust the argument hierarchy and reflected bases so Started actually lacks that member, while preserving source fields, its `WindowPointInfo` sample, `eventType`, and borrowed `GetPoints()` access. Do not merely hide an inherited member. Replace the special sticky-`accept` handling in `<GacUI repo>/Source/Application/GraphicsCompositions/GuiGraphicsEventReceiver.h` with the selected-composition rule. Keep the existing no-reflected-constructor rule for arguments borrowing the host's list.
- Attach the three control adapters once to the persistent bounds composition, regardless of `AcceptPointEvents`. Route them through the protected virtual `OnPointSequenceStarted`, `OnPointAction`, and `OnPointSequenceEnded` hooks. The base start behavior claims its bounds only when opted in and the query is still unclaimed. `GuiScrollView` opts in by default; an explicitly accepting descendant takes precedence. Only the scroll view whose own bounds was selected performs its scrolling behavior.
- Make `GetPointEventComposition` reflect the manager's current selected target when that target is within this control's bounds subtree, otherwise return `nullptr`. This lets an ancestor observe a child's ownership without caching a pointer that would require a bubbled End to clear. After the start query it remains available through End callbacks, and becomes null when capture is cleared afterward. Do not forward extra Action/End events to ancestor controls to maintain this property.
- Keep cleanup outside overridable/handled-dependent behavior: successful End, canceled End, host detachment, and deferred target removal clear capture, the control-facing property, pending fallback, and sequence state. Preserve occupied sparse slots through canceled End and empty slots through successful End, clearing the borrowed list after callbacks. A setting change affects the next start query, not the already accepted sequence. Preserve existing lifetime hooks without adding support for deletion during a callback.

### Manager and scrolling state

- Put the host-owned point manager in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost_Point.h/.cpp`, following the Alt/Tab manager ownership pattern. It owns point samples, capture bookkeeping, movement/elapsed-time recognition, and the fallback decision. Keep Started dispatch/selection and ordinary mouse synthesis at the host boundary. `GuiScrollView` applies the recognized movement through its existing scroll APIs; keep control-type discovery and duplicated state machines out of `GuiGraphicsHost`.
- First point means `pointId == 0`, not the platform `primary` hint or the first currently occupied slot. Started initializes it once; the following first Down must not reset its baseline. Later contacts remain visible in the borrowed sparse list but never drive scrolling. After point 0 lifts, no other contact is promoted. Any additional Down permanently disqualifies the sequence from click fallback, even if the extra contact lifts before End.
- Treat the threshold as 8 logical GacUI coordinate units after current native conversion. Before locking, compare displacement from the initial position using radial distance (`dx * dx + dy * dy > 64`, with wide arithmetic). Exactly 8 does not lock. At the first crossing choose horizontal when `abs(dx) >= abs(dy)`, otherwise vertical; the equality rule gives a deterministic 45-degree tie. Keep that axis until End, including reversals or a return to the start.
- Evaluate point 0's position on every position-bearing action, including final Up, so a distant release without an intervening Update cannot become a tap. Updating gesture bookkeeping is independent of whether a control handler consumes an action. Once the threshold is crossed, fallback stays disabled even if no scrollbar can move.
- `GuiScrollView::OnPointSequenceStarted` invokes base acceptance, then records the starting point and starting view position when `arguments.acceptedComposition == boundsComposition`. Use the query argument here, since `GetPointEventComposition` reflects the final selection only after Started dispatch finishes. On an unhandled action, change only the locked axis to `initialViewPosition - displacementFromStart`, making content follow the finger. Use `SetViewPosition`/existing scrollbar range clamping in `<GacUI repo>/Source/Controls/GuiContainerControls.cpp`; do not apply total displacement repeatedly to the already changed position. Respect the control's enabled state and require the selected scrollbar to exist, be visible, and be enabled. An unavailable axis neither switches to the other axis nor hands input to a parent.
- Reset recognition for each accepted occurrence. Retain enough terminal information for the host to read `GetFallbackClick()` after End handlers, then consume/clear it so a later rejected, canceled, or unrelated End cannot reuse the result.

### Current conversion and fallback clicks

- Remove `pointsScale`, rather than moving the cached scale into the new manager. Keep canonical native/screen-space samples sufficient to reconstruct stationary contacts when the window moves or its scale changes; derive callback-local positions, contact bounds, and the borrowed list using the current `INativeWindow::Convert` and current composition bounds. Do not repeatedly scale already rounded logical coordinates. Recompute the first-point baseline in the same current coordinate space as the latest sample before measuring displacement.
- Retain the original start position independently of sparse slot 0, because normal Up empties that slot before End. At fallback time hit-test that original position using the current window conversion and layout, as requested. The original child composition or virtualized item is not a separately retained click target in this step.
- `fallbackClick` is an opt-in request, initially false. The unhandled default `GuiScrollView::OnPointSequenceEnded` requests it for its owned sequence; other accepting controls/composition handlers may opt in explicitly. A custom point surface such as the Touching demonstration keeps it false. A request cannot override cancellation, a crossed threshold, or multiple-contact history. `handled` prevents predefined controls from requesting fallback, but does not erase an explicit request already made by a custom handler.
- `GetFallbackClick()` returns a button only for an accepted, successfully ended sequence with a true final `fallbackClick`, exactly one Down in its history, and no threshold crossing. Use the existing monotonic microsecond sample timestamps: End minus Started below `500000` selects Left; at or above `500000` selects Right. No movement callback or timer is needed for a stationary hold. Every cancellation reason returns no button.
- Read the fallback decision and retain the required start sample, finish point cleanup, then perform one ordinary mouse click. Reuse/refactor the existing hover/hit-test, Down, Up, propagation, modifier, button-state, and capture machinery. Moving the logical mouse/hover to the hit position before Down is necessary: `GuiButton::OnMouseUp` in `<GacUI repo>/Source/Controls/GuiButtonControls.cpp` requires its hover state, and typed automation already moves before button operations. Preserve physical-mouse state and do not leave synthetic pressed/captured state behind.
- Keep this deferred accepted-sequence click separate from `ProcessIOPointFallback` in `<GacUI repo>/Source/Utilities/SharedServices/GuiSharedAutomationService.cpp`, which already implements a rejected sequence's immediate first-contact mouse stream. Do not produce both paths or change the remote protocol schema to carry composition-only fields.

### Reflection and existing consumers

- Update point argument registration in `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionEvents.cpp`, and register read/write `AcceptPointEvents` plus read-only `PointEventComposition` in the `GuiControl` registration in `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionControls.cpp`. Keep native acceptance and wire types unchanged.
- Migrate existing composition handlers from `accept` to assigning their intended composition, including `<GacUI repo>/Test/Resources/App/FullControlTest/TouchingTabPage.xml` and `<GacUI repo>/Test/GacUISrc/Playground/Resources/ResourcePointInput.xml`. Update affected tests and regenerate affected resources through their owning tools; do not modify generated C++ manually. Preserve the Touching page's custom gesture behavior without enabling click fallback.
- Add the manager files beside Alt/Tab in `<GacUI repo>/Test/GacUISrc/Source_GacUI_CoreApplication/Source_GacUI_CoreApplication.vcxitems` and its filters. Add any new test source to `<GacUI repo>/Test/GacUISrc/UnitTest/UnitTest.vcxproj` and its filters. Follow `<GacUI repo>/.github/Guidelines/SourceFileManagement.md` for inventory changes.

## VERIFICATION

### Routing, recognition, and lifecycle

- Extend the ordinary `GraphicsHostProtocol` coverage in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_GraphicsHost_Mouse.cpp`. Exercise actual `OnIOPointStarted`, point actions, and `OnIOPointEnded` through the existing protocol harness, not just direct calls to new virtual hooks. Retain sparse-slot, ordering, cancellation, automation, and physical-mouse regression coverage.
- Assert deepest/ancestor acceptance, first assignment winning, default false `AcceptPointEvents`, default true on `GuiScrollView`, explicit opt-out on a subsequent sequence, and an accepting child inside nested scroll views. Verify Started reaches ancestors while Action/End reach only the selected target. Check the getter on owner, ancestor, and unrelated controls, during End callbacks and after cleanup. Exercise handled/custom overrides without allowing them to bypass cleanup.
- Cover below, exactly at, and above the 8-unit threshold, horizontal/vertical/tied diagonal directions, reversal after lock, crossing then returning, and a final Up beyond the threshold without an Update. Check absent, hidden, disabled, and range-clamped scrollbars; an unavailable locked axis produces neither scrolling on another axis nor a fallback click.
- Cover A Down, B Down, A Up, B Update/Up and A Down, B Down/Up, A Up. Only A can scroll, B never becomes the scrolling contact, and neither sequence can click. Verify a fresh sequence starts cleanly after successful End, each cancellation reason, host detachment, and deferred target removal. Do not introduce a test requiring safe synchronous deletion inside a handler.
- Supply explicit monotonic timestamps for stationary holds just below and exactly at `500000` microseconds. Native `GetNativePointTimestamp` uses `std::chrono::steady_clock`; the existing date-time injection used by tooltip tests does not change it. Use the protocol samples' timestamps instead of sleeping or introducing another clock abstraction.
- Verify fallback false, no eligible button, canceled End, and consumed/handled behavior produce their specified outcomes. An eligible tap generates exactly one Down then Up with the expected button/modifiers at the start position, after point cleanup. Begin with the mouse elsewhere and verify a child button actually clicks, including its normal hover/pressed state. Verify rejected input still uses only its preexisting fallback path.
- Change scale during an accepted sequence through the existing `OnControllerScreenUpdated(ScreenConfig)` harness support, including nonuniform/fractional scale and stationary secondary contacts. Verify positions, contact bounds, sparse holes, displacement, and fallback hit location under current conversion; cover hosted offsets/window movement. Fixed-DPI tests alone do not verify removal of cached scaling.

### Useful rendered control tests

- Add remote-protocol snapshot cases with overflowing content in a real scrollable list and multiline text box. Reuse fixture patterns from `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_List_GuiVirtualTextList.cpp` and `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_Editor_Regression.cpp`. Assert view/scrollbar positions as well as meaningful before/after frames for dragging in both axes, reversal, and clamping.
- Verify dragging preserves list selection/text selection and causes no deferred item activation. Verify a short tap uses normal list selection or caret placement; for the right-button case, observe the ordinary right-button handler or an explicitly configured context menu. Do not assume every list or text box has a built-in context menu.
- Use `GacUIUnitTest_StartFast_WithResourceAsText` and flat `OnNextIdleFrame` callbacks, with frame names describing the preceding action's result. Every snapshot case must save useful rendered output. Keep event-only, no-op, timing-boundary, and reflection assertions in ordinary tests rather than creating artificial frames. Leave the generated snapshots available for the user's later inspection.

### Manual FullControlTest verification on native and GacJS

- Perform two live application runs after rebuilding the implementation: native FullControlTest using `CppTest`, and FullControlTest through GacJS connected to `RemotingTest_Core /FCT /Http` on Windows. A Core/native-renderer pair may serve as the native run instead. Follow `<GacUI repo>/.github/Guidelines/Running-GacUI.md` and the setup sections of `<GacUI repo>/.github/Jobs/DebugRemoteProtocolWithGacJS.md`; for native remoting, use `<GacUI repo>/.github/Jobs/DebugRemoteProtocolWithNativeRenderer.md`. This is a focused scrolling check in both renderers, not a request to repeat the full application/transport matrices in those guides.
- Open FullControlTest and choose an arbitrary existing scrollable control with overflowing content, such as a list or multiline text box. Use the same control/page in both runs where possible and record which was chosen. Exercise the existing control without adding a special demo or per-control touch handler; the shared `GuiScrollView` behavior should provide support automatically.
- Begin a single-contact touch/point drag inside the content, move beyond the threshold, and release. Scroll forward and backward, including reversing direction during a drag; cover horizontal movement too when the chosen control has horizontal overflow. Verify visible content and the scrollbar/view position move together along the locked axis, stop at bounds, and do not activate an item or change text selection merely from dragging. Follow with a short tap to check ordinary selection/caret behavior and that the completed drag left no stuck capture.
- Use the native application's automation service for point input when needed, including in a locked session; for a native remote renderer, send point input through its renderer endpoint. In GacJS, drive touch input or browser touch emulation through the rendered page so its acquisition and forwarding are exercised. Mouse-wheel scrolling, scrollbar-thumb dragging, ordinary mouse dragging, and Core-only `/IO` injection do not establish that this point-scrolling path works in GacJS. Check that the intended control scrolls while the containing browser page stays stationary and release does not generate a duplicate compatibility click.
- Observe the actual native/browser rendering after each gesture, using current automation/DOM state to corroborate the scroll result. Record the native app or renderer, browser, transport when applicable, selected control, input method, and outcome of each run. Distinguish automation/emulation from real touch hardware; an unavailable input method is not a passed check. These live checks supplement the unit-test snapshots the user will inspect later.

### Build and migration checks for implementation

- Extend `<GacUI repo>/Test/GacUISrc/UnitTest/TestReflectionTypeList.cpp` and relevant resource checks for `acceptedComposition`, `fallbackClick`, `handled` on Action/End and its absence on Started, and both control properties with the correct mutability. Ensure old composition `accept` use is migrated while native boolean acceptance remains intact.
- Follow `<GacUI repo>/Project.md` and the repository build/execution wrappers. Reflection changes require Debug Win32 build and `Metadata_Generate`, then Debug x64 build, `Metadata_Generate`, and `Metadata_Test`. Regenerate affected resources with `GacUI_Compiler`, inspect its error artifacts, and rebuild affected consumers. Run the relevant UnitTest coverage and inspect completed summaries and Debug memory-leak output. Do not hand-edit generated artifacts.
- Because migrating the existing FullControlTest handler changes an authored resource, keep its meaningful UIA expectations synchronized and run `<GacUI repo>/Test/UIA_CppTest.ps1` and `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1` as required by the project. Preserve the existing Touching behavior. The focused native and GacJS manual checks above are required in addition to automated verification. Keep `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md` unchanged.
- Keep authored changes, generated artifacts, and unit-test snapshots in their prescribed separate commits when implementing. This review changes only the task document; implementation, generation, builds, and runtime verification described above are future work.

## REVIEW COMMENTS

# UPDATES

## UPDATE

a small refactoring: I would like you to restore the inheritance, just simply ignore `handled` when processing `pointSequenceStarted`, it is acceptable, no need to remove `handled` from that event args.

## UPDATE

or you can let GuiControl::OnPointSequenceStarted checks handled and when it is true the acceptedComposition assignment will be skipped, that makes `handled` useful.

## UPDATE

I think this would be a way for a child control to stop a point experience acceptance, which actually adds a feature.

# TEST [CONFIRMED]

Exercise actual point protocol dispatch for ownership, sparse contacts, current conversion, recognition, fallback, and cleanup. Add rendered list and text-box scrolling cases. Verify reflection/resource migration, full required builds and metadata, existing UIA checks, and focused native/GacJS live point scrolling. The earlier task-review sentence records preparation; this investigate repro request executes the implementation.

# PROPOSALS

The baseline Debug x64 build passed with zero warnings/errors. The new protocol regression in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_GraphicsHost_Mouse.cpp` failed at `actions == 1`: an ordinary scroll view does not accept the first point. Existing source additionally confirms boolean acceptance, bubbling Action/End, and cached logical-coordinate rescaling.

- No.1 Move point ownership and recognition into a host-owned manager. [DENIED]
- No.2 Restore Started inheritance and let handled suppress default acceptance. [CONFIRMED]

## No.1 Move point ownership and recognition into a host-owned manager

Keep Started dispatch at the graphics host, with sticky first-composition selection. Make Started an independent argument type without handled; preserve Action/End handled behavior. Store canonical screen-space native samples in the manager and rebuild callback coordinates with current conversion. Deliver Action/End directly, recognize only slot zero with a radial threshold and permanent axis lock, and retain an explicit terminal fallback decision. Controls attach persistent adapters; scroll views use the shared recognition and existing range-clamped view APIs. Perform requested mouse clicks after point cleanup using the normal mouse pipeline, preserving independent mouse capture/button bookkeeping. Migrate reflection and XML consumers, regenerate outputs, and verify protocol, rendering and live applications.

### CODE CHANGE

Implemented `GuiPointActionManager` and its source inventories, direct Action/End routing, sticky Started selection without handled, current native conversion, control acceptance/properties/hooks, axis-locked scroll-view movement, and deferred mouse synthesis. Migrated reflected fields/properties and authored Touching/Playground handlers. Added ordinary protocol ownership, recognition, DPI, lifecycle, fallback and reflection coverage, plus rendered list/editor fixtures. Native boolean acceptance and remote wire formats remain unchanged.

The new protocol tests exposed an incorrect empty assignment to `Nullable<NativeMouseButton>`: debugger inspection showed a present Left value after reset despite two Downs. Explicit `Reset()` calls now clear the pending fallback. Multi-contact and terminal-state assertions cover this regression.

The synchronous fallback temporarily uses composition mouse capture without changing native capture, then restores the independent physical composition/button bookkeeping. Regression coverage holds Left or Mouse5 on another composition while tapping and verifies that the later physical Up still reaches its original capture target.

Resource generation found that `Workflow_GenerateEventHandler` recognized the standard `sender`/`arguments` names only through `GuiEventArgs` ancestry. Its type recognition now includes the independent Started query, with an ordinary regression for all three point handler signatures. This preserves existing authored event-handler syntax without reintroducing handled or changing reflection inheritance.

The list fixture uses an explicit item width in a test-only arranger so both axes really overflow. The editor fixture initializes a nonempty first-line selection and resets both scrollbar positions before layout: `SetText` moves the caret, and `SetViewPosition(0)` can return early while a scrollbar is disabled during setup. Ready-frame assertions check the resulting zero view position. Ordinary protocol coverage also checks a fractional-DPI threshold boundary alongside stationary-contact rebasing: eight converted logical units remain unlocked, while ten lock without requesting a click.

### DENIED BY USER

The follow-up rejects only the independent Started argument hierarchy and the requirement to remove inherited `handled`. Keep the implemented ownership, scrolling and fallback behavior. Restore the C++ and reflected `GuiPointEventArgs` base, and ignore `handled` for query traversal while default control acceptance follows No.2. The verification below is the historical result for the original implementation; No.2 records verification of the revised contract.

The original failing protocol regression now passes. The selected composition owns direct Action/End delivery and remains observable through End; mandatory cleanup runs independently of handled/default control behavior. Current native conversion preserves gesture geometry across DPI and window-origin changes. Recognition, scrollbar gating and deferred click eligibility agree with the threshold, axis, timing and multi-contact contracts. Ordinary mouse dispatch supplies normal hover/pressed/click behavior after point cleanup while preserving physical mouse capture.

GacUI_Compiler completed both architectures successfully without remaining resource error artifacts, and all affected consumers were rebuilt. The rebuilt Playground compiled and loaded ResourcePointInput through all nine phases; automation confirmed child-only Action/End delivery, parent acceptance, and clean completion, while UIA exposed the expected inspection controls.

The final Debug Win32 solution build completed with zero warnings/errors, followed by successful metadata generation. Its UnitTest run passed all 188 cases in the 10 selected files with no appended Debug memory-leak report. Both rendered point-scrolling fixtures passed their position, selection, tap and right-button assertions. The existing Raw/Properties compiler snapshots changed only declaration ordering during regeneration; they are retained with the snapshot outputs.

The final Debug x64 solution build completed with zero warnings/errors. Metadata_Generate and Metadata_Test exited successfully, and all four metadata text round trips still match byte for byte. The final x64 UnitTest run passed all 208 cases in the 12 selected files, including the rendered fixtures, with no appended Debug memory-leak report.

Both focused Touching UIA scenarios passed 226 assertions using rebuilt Debug x64 GDI applications. The shared assertions additionally verify captured Touching movement outside its surface, no deferred tab activation, and ordinary ListView point scrolling with preserved selection and bottom clamping. The launcher now strips a UTF-8 BOM before parsing saved project-user XML, and the C# runner initializes its UIA provider before the scenario's new UIA-based coordinate checks.

The native live run used rebuilt Debug x64 CppTest with GDI and automation point commands on the existing List > ListView. A staged run first selected Task, then exercised forward scrolling, reversal within the held contact, horizontal movement/reset, bottom clamping, restoration and a short Reminder tap. Windows Graphics Capture observations and Controls data confirmed rendered movement and scrollbar positions; Task retained its selected blue background through reversal, and the final tap visibly selected Reminder. Results and stage observations are retained in the ignored `<GacUI repo>/Test/Output/LiveNativeListView.json` and `<GacUI repo>/Test/Output/NativeLiveStagesEvidence.json`.

The live GacJS run used visible Chromium, HTTP transport to RemotingTest_Core `/FCT /Http`, and CDP touch emulation through the rendered page. All 18 assertions passed: Touching retains its selected target outside the surface; the existing List > ListView scrolls vertically, reverses within one held contact, clamps at the bottom, returns to the top, scrolls horizontally and restores its position. Selected Task rendering survives the drag, and a later short tap selects Reminder. Browser scroll coordinates stay at zero, the measured touch gestures emit no compatibility mouse/button wire events, and there are no page/protocol errors. Screenshots visibly confirm scrolling and selection. The ignored verifier and evidence are retained under `<GacJS repo>/Gaclib/website/entry/lib/live-touch-*`; no tracked GacJS implementation changes were needed. Verifier corrections accounted for DOM rendering, virtualized rows, and deliberate setup mouse events outside the measured touch intervals.

These live checks used native automation and browser touch emulation; they do not claim physical touch-hardware coverage. Owned test applications and browser processes were stopped, their endpoints were released, and the preexisting IIS server was retained. `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md` remains unchanged. Source review and whitespace checks pass; authored changes, generated metadata/resources, and unit-test snapshots are kept in separate commits.

## No.2 Restore Started inheritance and let handled suppress default acceptance

Restore `GuiPointSequenceStartedEventArgs` as a `GuiPointEventArgs` subclass in `<GacUI repo>/Source/Application/GraphicsCompositions/GuiGraphicsEventReceiver.h/.cpp`. Delegate constructors to the base with `GuiPointEventsType::Started`, retain only `acceptedComposition` as the new field, and inherit the borrowed list accessor. Restore `CLASS_MEMBER_BASE(GuiPointEventArgs)` in `<GacUI repo>/Source/Reflection/TypeDescriptors/GuiReflectionEvents.cpp`, removing duplicate registrations. Remove the special Started type check in `<GacUI repo>/Source/Compiler/WorkflowCodegen/GuiInstanceLoader_WorkflowInstallBindings.cpp`; ordinary `GuiEventArgs` ancestry supplies XML handler parameter names again.

Started dispatch continues through all handlers and ancestors regardless of `handled`, preserving sticky first-composition selection. Following the second update, add `!arguments.handled` to the default acceptance condition in `GuiControl::OnPointSequenceStarted` in `<GacUI repo>/Source/Application/Controls/GuiBasicControls.cpp`. A true value suppresses default acceptance but does not erase an existing selection or prevent a custom handler from explicitly selecting a composition. Extend the existing ordinary protocol test with these cases and a normal unhandled acceptance control. Update reflection assertions for inherited members, base conversion, source initialization and borrowed points. Preserve the no-reflected-constructor rule. Reconcile API comments, `<GacUI repo>/TODO_Touch_Step3.md` and the point-input design guidance.

### CODE CHANGE

Restored the C++ and reflected point-argument base, removed duplicated fields/accessor/registrations and the compiler special case, and added the handled guard to default control acceptance. Extended existing reflection and protocol cases to cover inherited access, continuing delivery, suppression of ancestor defaults, preserved selections and explicit custom acceptance. Updated the API and design/task documentation.

Verify with Debug Win32 and x64 solution builds and selected UnitTest suites, regenerate both metadata architectures and run Metadata_Test through the repository wrappers. Regenerate resource outputs with GacUI_Compiler so serialized Workflow references use the restored reflected base, rebuilding consumers if generated C++ changes. Inspect generated metadata, resources and snapshot differences. The earlier live checks remain historical; this focused hierarchy refactor is covered by the existing routing, scrolling, reflection and handler-generation regressions.

### CONFIRMED

Both Debug solution builds succeeded with zero warnings and errors. The Win32 UnitTest run passed 188/188 cases in 10/10 selected files; x64 passed 208/208 cases in 12/12 selected files. Neither run appended a Debug memory-leak report. The extended protocol regression confirms normal unhandled acceptance, continued handlers and ancestor queries after a child sets handled, suppression of automatic ancestor acceptance, preservation of an existing selection, explicit custom acceptance despite handled, and terminal cleanup. Reflection checks confirm inherited access and conversion, default source/handled initialization, borrowed points and conventional XML handler argument names.

Metadata_Generate succeeded for both architectures, and x64 Metadata_Test succeeded. All four metadata text round trips match byte for byte. The regenerated descriptors derive Started from GuiPointEventArgs and declare only acceptedComposition locally. With both metadata sets current, the x64 GacUI_Compiler completed its x86 and x64 resource passes successfully. Generated application resources and C++ have no tracked changes, and no resource error artifacts remain; no additional consumer rebuild is needed. Unit-test snapshots also remain unchanged. Source review and whitespace checks pass. Keep authored changes and regenerated metadata in separate commits.
