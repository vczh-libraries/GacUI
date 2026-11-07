# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- Currently a short pressing touch generates left button click:
  - If two generated left button click happens less than 0.5 second, the second one should generates left double click.
  - The actual meaning of "left double click" means the sequence of `MouseDown`, `MouseDbClick`, `MouseDoubleClick`.
  - Since a generated right click requires pressing more than 0.5 second so it is not impossible to generate a right double click, so only processing left double click is totally fine.
  - You can use a `Nullable<DateTime>` to record the time of the last generated left click, when double click is generated reset it to empty, it is easier to check.
- `PointSequenceEndInfo::timestamp` should have a comment to clarify its unit.
- To verify, run `job:rpWindows` but:
  - No need to generate matrix card.
  - Only pick `Touching`, `TouchingList`, `Document Editor` tab pages.
  - Verify against `CppTest`, `RemotingTest_Rendering_Win32` and `GacJS`, for regression.
  - Additionally on `Document Editor` tab page, you can try to insert an image from file system, using any existing image in the repo:
    - When the file dialog is opened, try to send fast sequences of point events to simulate some double clicks, on folder/file name for navigation.
    - This is for verifying the new feature.
- Update KB page and the website to simply mention the left double click conversion.
- Publish the website, but when running `job:copilotInitAll`, skip learning.

## DETAILS

### Conversion and timing

- Extend `GuiGraphicsHost::PointSequenceEnded` in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.cpp`, where accepted point sequences already produce deferred mouse input. Make the decision after gesture selection and public point End have completed and the actual fallback button is known. This shared path serves hosted and remote applications; no new recognizer, reflected event or remote-protocol message is needed.
- The event names above need a factual clarification: there is no `MouseDbClick` callback. The existing native convention in `<GacUI repo>/Source/PlatformProviders/Windows/WinNativeWindow.cpp` is `MouseDown`, `MouseDoubleClick`, then `MouseUp`. Keep the preceding `MouseMoving`; insert exactly one left `MouseDoubleClick` between Down and Up for the second generated click, with the left-button flag still set. Keep ordinary Down/Up behavior, including the final release.
- Store the last unpaired generated left click as a host-local `Nullable<DateTime>` in `GuiGraphicsHost`, declared in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost.h`. Sample `DateTime::UtcTime()` at click generation and compare `osMilliseconds`, not platform-specific `osInternal` or point timestamps. Pair when the elapsed time is nonnegative and strictly less than 500 milliseconds, including equal timestamps. At 500 milliseconds or later, generate a single click and replace the remembered time.
- Clear the remembered time when consuming a pair, so three rapid clicks produce single/double/single and four produce single/double/single/double. Commit the history update before generated mouse callbacks can pump nested input. Keep it across ordinary point cleanup and subsequent Started events; clear it when the host replaces or detaches its native window in `SetNativeWindow`. Different graphics hosts must not share history.
- Apply the requested time-only rule to actual generated left clicks; do not add a composition-identity, position or operating-system double-click setting requirement. A sequence that produces no deferred click must not prime or consume this history. Physical mouse/pen input, rejected-sequence mouse compatibility and generated right clicks keep their existing behavior and do not feed the left-click history.
- Preserve the current gesture rules in `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost_Point.cpp`: a long press starts at **500000 microseconds**, inclusive. With right-click generation disabled and left-click generation enabled, an eligible long press can also generate a left click and therefore participates in the same pairing rule. Selected gestures, disabled click generation, cancellation, extra contacts and movement outside the existing click radius must retain their suppression behavior.
- Preserve original modifiers and start-position targeting using current layout/conversion. Point capture must already be cleared before generated mouse events; existing independent physical mouse capture and button state must survive the complete synthetic click.
- Add a comment to `PointSequenceEndInfo::timestamp` in `<GacUI repo>/Source/NativeWindow/GuiNativeWindow.h`: microseconds from a monotonic clock, unrelated to wall-clock time, matching `WindowPointInfo_::timestamp`. This is the point-sequence timing domain, separate from the generated-click `DateTime` comparison.

### Documentation and publishing

- Briefly document the conversion, strict half-second interval and pair reset in `<GacUI repo>/.github/KnowledgeBase/KB_GacUI_Design_PointInput.md`, under "Four fixed recognizers and click fallback", and in `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/components/compositions/events.xml`, under "Clicks and Existing Controls". Preserve the existing distinction between deferred clicks and rejected-sequence mouse compatibility.
- Follow `<WebsiteSource repo>/job.publish.prompt.md` for website build, sequential static downloads, Markdown export, publication and synchronization. Propagate the KB update to `<Tools repo>/Copilot/KnowledgeBase/KB_GacUI_Design_PointInput.md` through the documented synchronization flow; regenerate manual copies rather than editing them directly.
- Apply "skip learning" to **both** publishing-flow invocations of `job:copilotInitAll`. As defined in `<Tools repo>/Jobs/job.Windows.copilotInitAll.prompt.md`, start at "Merge Knowledge Base", then perform "Sync Back Knowledge Base and Instructions". Preserve the existing deployed `wasm-fct` folder under the publishing job's default rule; this task does not request a WASM rebuild.

## VERIFICATION

### Deterministic regression coverage

- Extend the ordinary graphics-host tests in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_GraphicsHost_Mouse.cpp`. Reuse the injectable UTC-clock approach from `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls.h` for exact click intervals. `<GacUI repo>/Test/GacUISrc/UnitTest/TestPointInput.h` controls point timestamps only; changing those timestamps alone does not advance the generated-click clock.
- Assert the full ordered mouse event sequence for a first click and for a pair separated by 0, 499, 500 and 501 milliseconds. The second click gets a double-click event only for the first two intervals. Verify three/four rapid clicks, expired-history replacement, separate hosts and native-window rebinding.
- Assert the left-button flag during the double-click callback, one matching Up per Down, original modifiers and targeting, cleared point capture during generated callbacks, and preserved independent physical button/capture state. Retain the existing modal/reentrant-input regression, including opening, closing and reopening a dialog using touch before mouse input.
- Cover actual left fallback after a long press with right-click generation disabled. Reuse the existing press/fallback cases to verify that selected gestures, click flags, canceled sequences, extra contacts and excessive movement do not generate or prime a double click. Verify generated right clicks and ordinary physical mouse double clicks retain their behavior.
- Build Debug x64 using `<GacUI repo>/.github/Scripts/copilotBuild.ps1` from `<GacUI repo>/Test/GacUISrc`, then run UnitTest with `<GacUI repo>/.github/Scripts/copilotExecute.ps1` following `<GacUI repo>/.github/Guidelines/Running-UnitTest.md`. Ensure the relevant mouse and point-scrolling test files are selected without clearing unrelated existing filters. Inspect completed build/test logs, including the final leak report. Follow `<GacUI repo>/Project.md` generation requirements if reflection or XML resources are changed during implementation.

### Scoped Windows application verification

- Follow `<GacUI repo>/.github/Jobs/job.rpWindows.prompt.md` and its native-renderer/GacJS procedures, limited to FullControlTest and the requested three page groups. Cover `CppTest` with Direct2D/GDI; `RemotingTest_Core /FCT` with `RemotingTest_Rendering_Win32` over Pipe, Http and MiniHttp with Direct2D/GDI; and GacJS in Windows Chromium over Http and MiniHttp. Document Editor includes both Ribbon and Toolstrip variants. The explicit targets here override any older "only CppTest" wording in the shared Document Editor SOP.
- Do not create or update matrix cards. Exclude unrelated pages, other showcase applications, TUI and WASM verification. Record executed configurations, observations, failures/fixes and any unavailable acquisition checks in the investigation log at `<GacUI repo>/.github/TaskLogs/Copilot_Investigate.md`.
- On Touching, retain dragging, rotation, scaling, cancellation and capture checks. On TouchingList, retain scrolling, swipe reveal/deletion, recycled-row state and momentum checks. These gestures must not accidentally produce double-click activation. On both editors, retain text selection, short-press preservation of an existing selection, long-press selection/context menus and ordinary mouse operation. A consumed document press must not become a generated click.
- Use automation-service point input for `CppTest`; for native remote tests, exercise the renderer's IO endpoint as well as Core-side dispatch where the SOP requires it. GacJS needs browser touch input to cover acquisition and forwarding. Existing helpers in `<GacUI repo>/Test/Touching_Remote.ps1` and `<GacUI repo>/Test/Touching_Browser.mjs` provide setup/regression coverage but do not already prove the new file-dialog behavior. Keep synthetic point injection, browser/native acquisition and physical hardware evidence distinct.
- In each requested application path and editor variant, open Insert Image and use the GacUI file dialog. Keep remote filesystem injection disabled so the remote Core can access the repository image, for example `<GacUI repo>/Test/Resources/App/FullControlTest/ToolbarImages/s_Font.png`.
- After obtaining current folder/file label coordinates, send two complete one-contact Down/Up sequences rapidly, without a slow inspection between them. Do not use a mouse double-click command as evidence of touch conversion. In native remote tests inject through the renderer; in GacJS use browser touch input. Automation timestamps are assigned when commands execute, so exact threshold assertions belong in the deterministic tests above.
- Confirm a single tap selects a row, a double tap on a folder navigates exactly once while leaving the dialog open, and a double tap on an image confirms it, closes the dialog and inserts exactly one image. Refresh coordinates after navigation. Undo and Redo must remove and restore that one insertion. Also verify two taps spaced beyond the interval do not activate the folder/file, and opening/canceling/reopening the dialog remains functional.

### Published documentation

- Complete the website repository's required build/checks, verify the generated article and Markdown contain the brief conversion note, then publish and wait for the publishing repository's CI. Inspect the live article for the updated text. Verify synchronized KB/manual copies and commit/push all affected repositories as required by the publishing flow, with learning skipped throughout.

## REVIEW COMMENTS

No unresolved review comments.

# TEST [CONFIRMED]

- Add ordinary graphics-host regression coverage in `<GacUI repo>/Test/GacUISrc/UnitTest/TestRemote_GraphicsHost_Mouse.cpp`, using the injectable UTC clock from `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls.h`. Compare ordered Move/Down/DoubleClick/Up callbacks at 0, 499, 500 and 501 milliseconds; cover pair reset, expiry replacement, host isolation, native-window rebinding and reentrant callbacks.
- Preserve point capture cleanup, modifiers, original-position targeting and independent physical mouse capture/buttons. Extend the existing press/fallback cases to check that suppressed sequences cannot prime or consume a pair, while actual left fallback from long press participates and generated right clicks remain independent.
- Build Debug x64 and run the selected unit tests through the repository wrappers, inspect completed logs and the final leak report. The initial checkout has no UnitTest debugger argument file or filters.
- Execute the requested FullControlTest scope: CppTest Direct2D/GDI; native remote Pipe/Http/MiniHttp with Direct2D/GDI; GacJS Chromium Http/MiniHttp. Check Touching, TouchingList and both document editors, including touch-only file-dialog folder navigation, one image insertion, Undo/Redo, slow taps and reopen/cancel. Record observations here, without matrix cards, and distinguish synthetic, acquisition and hardware evidence.
- Build and publish the website, verify the live article and generated Markdown, preserve the deployed wasm-fct folder, and synchronize KB/manual copies with learning skipped in both copilotInitAll invocations.

## Baseline reproduction

- Debug x64 build completed with zero warnings and zero errors. The first deterministic UTC-pairing test compiled successfully. The unfiltered UnitTest wrapper run is still progressing; no existing test filters were changed.
- CppTest Direct2D, launched through the CLI wrapper, reproduced the missing conversion through its automation-service point endpoint. In the Ribbon Insert Image dialog, two rapid complete one-contact sequences on the current `ToolbarImages` label left the selected folder at its parent. The bounded assertion for navigation failed. Single selection, taps separated by 600 ms, real local filesystem access, editor text selection/context menus, and touch-only open/cancel/reopen succeeded first.
- `<GacUI repo>/Test/Touching_Editors.mjs` retains this observable regression for the native and browser paths. This is synthetic point-dispatch evidence, not physical touch hardware evidence.

# PROPOSALS

- No.1 Pair actual generated left clicks in their graphics host

## No.1 Pair actual generated left clicks in their graphics host

After public point End and fallback selection, sample the injectable UTC clock only for an actual deferred left click. Compare `osMilliseconds` with the host's last unpaired click, accepting nonnegative elapsed time strictly below 500 ms. Consume a pair before callbacks; otherwise remember the new time. Insert `MouseDoubleClick` after Down with the left flag set, preserving the existing Up and capture/button restoration. Reset history only on an actual native-window binding change. Keep point timestamps and gesture recognition separate.

### CODE CHANGE

- Add a protected host-local `Nullable<DateTime>` and update it before generated mouse callbacks in `PointSequenceEnded`; clear it in the changed-binding branch of `SetNativeWindow`.
- Document the monotonic microsecond unit of `PointSequenceEndInfo::timestamp`.
- Extend deterministic timing, exclusion, capture and modal regressions. Extend existing application helpers with a scoped mode and shared editor/file-dialog checks for CppTest, native renderer and browser acquisition.
- Add a brief conversion note to the point-input KB and website article, then build, publish and synchronize both documented rounds with learning skipped and deployed WASM files preserved.
