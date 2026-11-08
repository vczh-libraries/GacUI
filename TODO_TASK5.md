Fix some issue from the previous execution of `TODO_TASK3.md`:
- Repro in `Document Editor (Ribbon|Toolstrip)` tabs:
  - Need to change the design, where short press on a selected area of a text box, it should act like trivial left click, which moves the caret, instead of doing nothing.
  - Unlike trivial left click, short press after the end of a paragraph or below the last paragraph does not move the caret. It should.
  - When the context menu is opened, short press in other area of the text box cannot close the context menu, while clicking outside of the text box can. It shyould close the context menu
  - I think in order to deliver the same experience, handling both the short press generated left/right/double click and trivial click should go the same path, `GuiDocumentCommonInterface` probably need to fix like that.
    - One of the reason might be two click is handled in different compositions, if this is the case:
      - The background composition should pay attention about composition fields in event arguments, tell if the click is directly generated from gesture or not. When clicking on a scroll bar, `eventSource` and `compositionSource` will be different. Generated clicks or any point/gesture events except `pointSequenceStarted` should have `eventSource` and `compositionSource` pointing to the same composition, which is the `sender`.
      - Different compositions have different offset, deal with x/y carefully.
      - Eventually clicks should deliver to text box's original event handler, but I don't require two compositions to use the same event handler because gesture generated clicks might need extra care. But in this case it should call the original event handler directly instead of triggering the event.
      - "original event handler" means the ones who initially reacts to trivial/real clicks.
  - Around `DocumentEditorBase.xml`'s `ShortcutBuilder-bind` code:
    - It originall changes because the shortcut is not working in `Document Editor (Toolstrip)`.
    - It is expected, both ribbon and toolstrip use the same set of shortcut key, so only ribbon is expected to respond to them.
    - Please revert the original change around `ShortcutBuilder-bind` code, you can git blame to know the original implementation.
    - If anything in `DebugRemoteProtocolSop.md` mentioning checks shortcut keys, only do it on ribbon tab page, and don't do it on toolstrip tab page.

### DETAILS

- The old behavior is explicit in `GuiDocumentCommonInterface::OnPointGesture` in `<GacUI repo>/Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp`: an inside-selection short press selects/handles the gesture without moving the caret. Remove that special consumption and reuse the original mouse handlers for ordinary caret placement, modifiers and hyperlink behavior. Preserve the existing long-press extension from a collapsed caret and right-click fallback when a range already exists.
- Merely removing that branch is insufficient at the active selection endpoint: the original `OnMouseDown` skips `Move` when the hit equals `GetCaretEnd()`. An unmodified left click or short press must also collapse a nonempty range in that case. Correct the shared behavior rather than adding a separate gesture-only selection algorithm.
- Point handlers are installed on persistent control bounds, while `ReplaceMouseArea` installs mouse handlers on a template-dependent composition. If generated clicks reach the bounds, adapt coordinates into the current mouse area's space before directly calling the original handlers. Preserve button/modifier fields and avoid executing the composition event again or delivering one click twice. Account for document margins, viewport scrolling, label offsets and template replacement. Blank space after paragraph text or below the final paragraph means space inside the document viewport; use the ordinary mouse path's caret clamping there.
- Correct the source-field premise: equality of `eventSource` and `compositionSource` alone does not identify a generated click. Physical input can also have equal sources. A scrollbar event can have both fields identifying the same descendant, neither equal to the background handler's `sender`. Test both fields against the receiving composition for direct delivery, and retain child ownership/original-hit checks so scrollbar chrome and embedded controls do not position the document caret.
- `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.cpp` currently synthesizes fallback clicks through ordinary hit testing and bubbling. The requested direct source/target contract therefore needs an explicit routing change; preserve the existing `pointSequenceStarted` query behavior. Audit descendant click consumers when changing fallback delivery: list-item handlers in `<GacUI repo>/Source/Controls/ListControlPackage/GuiListControls.cpp` and embedded buttons must still receive their activation/double-click behavior. Preserve accepted-sequence cleanup, independent physical mouse capture and one delivery per mouse phase.
- Popup dismissal also crosses a window boundary. `GuiHostedController::GetSelectedWindow_MouseDown` in `<GacUI repo>/Source/PlatformProviders/Hosted/GuiHostedController.cpp` closes unrelated hosted popups; host-local generated clicks currently bypass that route. Reuse the applicable dismissal behavior for an eligible generated click so it closes the context menu and performs its caret action once. Calling document handlers alone does not address this. Preserve menu/submenu interaction; do not dismiss every popup on raw point-down before knowing whether the sequence becomes a click, scroll or canceled gesture.
- In `<GacUI repo>/Test/Resources/App/FullControlTest/DocumentEditorBase.xml`, commit `4cd8d7720f` introduced the nine focus-dependent `ShortcutBuilder-bind` attributes. Restore only those attributes from its parent to literal `ShortcutBuilder` values for Ctrl+Z/Y/C/X/V/A and Ctrl+Shift+P/S/E. Preserve unrelated context-menu and persistence changes. Update the executable editor checks in `<GacUI repo>/Test/UIA_CppTest_Shared.cs` too: Toolstrip must use menu/toolbar commands where the helper currently sends shortcuts; Ribbon retains shortcut activation checks.
- Update `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md` sections 3.1, 3.2 and the existing touch-selection description. Remove the inside-selection preservation workaround and distinguish selection-preserving dismissal such as Escape from a document click that moves the caret. Section 3.2 should explicitly omit shortcut activation inherited from 3.1 while retaining equivalent Toolstrip command checks.

### VERIFICATION

- On both requested editor tabs, load `<GacUI repo>/Test/TestDocument.bin` and compare actual caret endpoints/focus after a physical left click and an unmodified short press: ordinary text, inside forward/reversed selections, both endpoints including the active endpoint, after paragraph text and below the final paragraph. Repeat the blank-space cases with a scrolled viewport and nonzero control offsets. Isolate single-click cases from the existing generated-double-click time window.
- Open the context menu by long press over an existing selection. Short press another document position and require the menu to close and the caret to move exactly once. Verify reopening, menu/submenu commands, Escape and outside-editor dismissal, and selection-preserving right clicks. Holding alone must not open the menu; long press completes on release.
- Preserve long-press range extension, Shift-click, physical dragging, ViewOnly hyperlinks and disabled-control behavior. Check scrollbar/embedded-control hits do not move the editor caret, selected child controls retain their own sequence, and cancellation/extra contacts do not produce fallback clicks. Repeat representative cases after palette/template replacement. Check generated left/right/double-click phase counts and both source fields against the intended receiver; leave ordinary physical mouse bubbling and its source fields unchanged.
- Update existing document-press expectations in `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_PointScrolling.cpp`, including both viewer and label families, instead of retaining the old inside-selection assertions. Extend `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_GraphicsHost_Mouse.cpp` for changed fallback routing, coordinate conversion and descendant activation. Use ordinary tests for handler-only behavior and snapshot tests only when they capture useful rendering results.
- Confirm the nine restored shortcut definitions match the historical literals. Exercise shortcut effects only on Ribbon; exercise the corresponding Toolstrip commands through its UI. Keep integration interactions within the two requested editor pages, including any reused SOP steps.

Fix some issue from the previous execution of `TODO_TASK1.md`:
- Repro in `TouchingList` tabs:
- `MomentumScrolling` seems to have a lower damping, I would like to:
  - Double the deceleration, so that it stops faster.
  - Originally it calculates the momentum scrolling starting speed in 0.5 seconds of moving:
    - Verify again to make sure: the calculation of the average speed consider the timeing gap between the last scrolling gesture and `pointSequenceEnded`:
      - If the figure rests for 1 seconds before releasing, the starting speed of momentum scrolling should be 0.
      - If the figure releases right after scrolling, you will have enough scrolling gesture to calculate the starting speed.
    - Change it to 0.25 second.
  - If you think adding a timestamp to `pointSequenceEnded` is needed, you need to fix at least:
    - `GuiNativeWindow.h`
    - Remote protocol
    - Automation Service
    - Any platform provider message redirection, e.g. hosted mode
    - To add timestamp to all point events.
- `MomentumScrolling::states` should be a `Dictionary<GuiControl*, xxx>` for high performance look ups.

### DETAILS

- The review clarification confirms that faster stopping is the goal and deceleration should be doubled. `MomentumScrolling` in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost_Point.h/.cpp` currently defaults `Deceleration` to `1.0` and computes a constant magnitude of `320 * Deceleration` logical pixels/second squared. Set the default multiplier to `2.0`, retaining its positive-multiplier semantics and base factor. At equal release velocity, the default stopping time and continuation distance are halved; existing explicitly configured multipliers retain their meaning. Update the default-value API comment and tests.
- Change the momentum history window to `250000` microseconds consistently in both `MomentumScrollingState::Record` and `OnEnded`, preferably sharing one named constant. Preserve the boundary predecessor/interpolation, equal-time sample coalescing, final-Up movement and unchanged-position sample at End. Use the shorter actual duration for sequences younger than the window. Do not change the separate `500000`-microsecond long-press threshold or generated-double-click interval.
- End timestamps already exist: `PointSequenceEndInfo::timestamp` in `<GacUI repo>/Source/NativeWindow/GuiNativeWindow.h` flows through `GuiPointSequenceEndedEventArgs` in `<GacUI repo>/Source/Application/GraphicsCompositions/GuiGraphicsEventReceiver.h`. Started/Action arguments also inherit timestamped point samples. The native, hosted, automation and remote paths already supply/forward these values; `<GacUI repo>/Source/PlatformProviders/Remote/Protocol/Protocol_IO.txt` carries End timestamps as unsigned decimal strings. No timestamp API/schema extension is needed merely to implement the requested sampling change.
- The current `OnEnded` already includes stationary time through End when computing average velocity. Verify that contract with the shorter window rather than replacing the release timestamp with the last movement timestamp or the playback clock. Gesture arguments themselves need no new timestamp: momentum records the preceding raw Action timestamp and the explicit End timestamp. Preserve the separate animation clock, fractional integration and 0.1-second playback boundaries.
- `<GacUI repo>/TODO_TASK1.md` is absent in the current checkout. Use the existing implementation, point-input knowledge base and `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_PointScrolling.cpp` as the baseline; do not reconstruct a second momentum implementation from the missing task.

### VERIFICATION

- Use deterministic point timestamps and a controlled animation clock in the existing unit tests. A constant-speed movement of 160 logical pixels over 0.5 seconds, released immediately, has a final-quarter-second velocity of 320 pixels/second. With default `Deceleration == 2.0`, require 80 additional pixels over 0.5 seconds before clamping, allowing only the existing integral-pixel rounding. Verify explicit multipliers still scale stopping time inversely.
- Separate immediate release from stationary release: move 160 pixels over 0.25 seconds, then release immediately, after 0.125 seconds of rest, after 0.25 seconds of rest and after one second of rest. Expected starting speeds are respectively 640, 320, 0 and 0 pixels/second under the existing linear interpolation rule. Include a nonzero gap between final Up and explicit End in a direct sequence test so the End timestamp itself is exercised.
- Test movement in the first quarter second followed by reversal in the final quarter second, proving the older direction no longer biases launch velocity. Cover a shorter-than-window sequence, both axes/signs, window-boundary interpolation and duplicate timestamps. Update old half-second/default-speed assertions in `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_PointScrolling.cpp`; preserve the purpose of fractional-tail and delayed-tick regressions when adjusting their fixtures.
- On TouchingList, exercise the initially disabled momentum option off/on/off, a quick flick, a stationary hold before release, reversal and range clamping. Preserve selection, immediate interruption by a new contact, cancellation/extra-contact suppression and detachment. Horizontal swipe deletion must still happen once at the real End, never during continuation.
- Inspect the timestamp path through hosted and native-remote input with the existing schema. If implementation reveals a genuine missing propagation step, fix that path and apply the conditional protocol/GacJS verification below; do not add a parallel wall-clock fallback or infer hardware coverage from synthetic input.

- To verify, following job:rpWindows but:
  - No need to generate test matrix card.
  - Only verify `CppTest`, `RemotingTest_Rendering_Win32`.
  - Only verify `GacJS` if remote protocol is touched.
  - Only perform tests on 3 tabs mentioned above.

- Update KB pages and website to update stale facts caused by this task, if anything is originally not mentioned, no need to add.
- Commit and push all local changes, and then publish the website, skip learning in `job:copilotInitAll`.

## DETAILS

- The implementation's shared verification scope is `CppTest` plus `RemotingTest_Core /FCT` with `RemotingTest_Rendering_Win32`, interacting only with Document Editor (Ribbon), Document Editor (Toolstrip) and TouchingList. Retain the applicable Direct2D/GDI and Pipe/Http/MiniHttp combinations from the Windows job/SOP, but omit its matrix-card creation, other showcase tabs, other applications and RPT/RVMT scenarios. Add GacJS only when remote protocol implementation/schema changes; the sampling-window change alone does not require it. The explicit application limit also excludes the otherwise standard Metaonly UIA run.
- Correct existing stale point-input/selection and momentum facts in the knowledge-base index/design page and website articles. For the shared copies, update `<Tools repo>/Copilot/KnowledgeBase/Index_GacUI.md` and `KB_GacUI_Design_PointInput.md` through instruction synchronization. Website sources are `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/components/controls/editor/home.xml`, `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/components/compositions/events.xml` and `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/components/components/momentumscrolling.xml`. Update existing examples/defaults/routing statements; do not add unrelated undocumented topics. GacUI's SOP is maintained in `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md`.
- Follow `<WebsiteSource repo>/job.publish.prompt.md` during implementation, applying the requested learning skip to both `copilotInitAll` invocations, beginning at Merge Knowledge Base. Generate/export manual Markdown through that workflow and synchronize the copies. Commit/push the implementation checkpoint before publication, then commit/push remaining publication/synchronization changes. Preserve authored/generated/snapshot commit separation required by `<GacUI repo>/Project.md`.

## VERIFICATION

These are acceptance checks for executing the task; this review does not implement it or run its jobs.

- Use the repository build/run wrappers. Regenerate FullControlTest after the authored XML rollback with `GacUI_Compiler`, perform its required Debug Win32/x64 metadata generation and `Metadata_Test`, rebuild affected consumers and run the focused source regressions. If protocol schema changes, run `Metadata_UpdateProtocol` and the corresponding GacJS generation before consumer checks. Inspect full logs for failures, resource diagnostics and Debug leaks; do not edit generated outputs by hand.
- Run the scoped application checks through automation-service IO first, then applicable UIA/native input. For remote cases, distinguish Core injection from renderer injection, which additionally verifies protocol forwarding. Reuse/update the existing editor and TouchingList helpers with focused scenario selection so they do not silently visit unrelated tabs or require Toolstrip shortcuts. Record unsupported desktop-dependent operations separately from successful synthetic checks. If GacJS is required, exercise the same three pages with browser input over the applicable HTTP transports and verify unchanged timestamp values/order end to end.
- Check the revised selection contract, 0.25-second window, doubled default deceleration and numeric examples in the KB, rendered website, exported manuals and published pages. Complete publication build/tests and CI verification, inspect synchronization diffs, and commit/push all resulting task changes. Report only coverage actually executed.

## REVIEW COMMENTS

No unresolved review comments. The deceleration correction follows the user's review clarification; the remaining implementation choices and acceptance checks are specified above.
