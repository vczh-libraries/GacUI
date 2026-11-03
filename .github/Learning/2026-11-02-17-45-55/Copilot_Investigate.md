# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- Apply momentum scrolling on list and document, including selection.
  - Move `#include "GuiGraphicsHost_Point.h"` to `GuiGraphicsHost.h` and remove `class GuiPointActionManager;` from header.
  - Define `vl::presentation::compositions::MomentumScrolling`, inheriting from `GuiComponent`, add reflection with `MomentumScrolling*` constructor:
    - It could listen on multiple controls via multiple calls to `AttachControl` and `DetachControl`:
      - In `AttachControl` it grabs a `GuiControl::GetDisposedFlag`, when it is disposed, detach a control automatically. But it can't call `DetachControl` inside as the control is already gone.
      - `AttachControl` listen to events on `GuiControl::GetPointEventComposition`.
      - `DetachControl` remove event listening, stop and delete the animation object dedicated for the control.
    - Overriding of `GuiComponent::Detach` should stop and delete all animation objects.
    - When an control is attached:
      - It does not touch the `AcceptPointEvents` property, this property should be set explicitly somewhere (except `GuiScrollView` or any sub controls because it is by default set).
      - It always select scrolling gesture.
      - It triggers continuing `pointGesture` events in an animation object after `pointSequenceEnded` if the scrolling gesture happened after the last `pointSequenceStarted`:
        - The animation stops when `pointSequenceStarted` happens.
        - The animation act as a momentum scrolling by keep triggering `pointGesture`.
        - It remembers all `pointGesture` for scrolling and its timestamps, after the sequence ending:
          - It calculates the average speed in the last 0.5 second, all `pointGesture` before the last 0.5 second will be discarded (except the last one before 0.5 second which contributes to the samping of average speed).
          - It continues to to trigger `pointGesture` with a slow down for each 0.1 second.
          - The default deceleration should be, if the last 0.5 second scrolls 160 logical px, it should stops after 1 second.
          - The deceleration is controlled by the `Deceleration` property with `double`, when it is 1 it is the default deceleration, and it is more/less than 1 it stops faster/slower linearly.
      - States should be recorded per control.
  - In then `TouchingList` tab in `FullControlTest` and the `GacUI_Windows/TouchingList` tutorial in `Release` repo:
    - A check box called `Momentum Scrolling` should be added before the `Clear` button, it is by default unchecked:
    - It controls `MomentumScrolling::(Attach|Detach)Control` so checking it enables momentum scrolling.

### DETAILS

- Implementation references: `<GacUI repo>/Source/Application/GraphicsHost/GuiGraphicsHost_Point.h/.cpp`, `<GacUI repo>/Source/Application/Controls/GuiBasicControls.h/.cpp`, `<GacUI repo>/Source/Application/Controls/GuiInstanceRootObject.h/.cpp`, and `<GacUI repo>/Source/Controls/GuiContainerControls.cpp`. Follow `<GacUI repo>/.github/KnowledgeBase/KB_GacUI_Design_PointInput.md` for the existing acceptance, capture and gesture contract.
- API correction: `GuiControl::GetPointEventComposition()` currently queries active capture inside the control subtree; it returns null while idle and after End. It is not a persistent event-subscription target. Subscribe to the control's persistent bounds composition, where its built-in point handlers are installed, and preserve the getter's capture-query meaning. Observe only sequences owned by that bounds composition; a child that accepts its own sequence must not also start parent momentum. A Started query through the bounds should still stop that control's previous animation, even if the new sequence is rejected for text selection.
- `GuiScrollView::OnPointGesture` currently requires live capture at its bounds and would ignore every post-End momentum event. Update this guard to accept correctly targeted component-generated gestures after capture has cleared, while retaining enabled/visible scrollbar checks, axis locking, range clamping and sole-owner routing. Set both event source fields to the target composition. Do not fake a new native sequence, retain capture after End, or add remote-protocol messages for inertia.
- The component selects scrolling offers without marking them handled merely for observing them. Existing control and application handlers must still consume the movement. Record real scrolling even when an earlier handler has marked it handled, and exclude the component's own synthetic events from its samples. Synthetic events start unhandled so normal consumers can apply them. Do not select manipulation or override a different accepted composition. No change to `AcceptPointEvents` is needed when attaching or detaching.
- Interpret selection support as preserving selected list items and document ranges while the viewport scrolls. `<GacUI repo>/Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.cpp` deliberately rejects point acceptance over Selectable/Editable document content so immediate mouse compatibility can perform range selection. Preserve that behavior; ViewOnly content and scrollable chrome can initiate momentum. Attaching this component does not turn a rejected selection drag into a scrolling sequence.
- Declare `MomentumScrolling` in the point header with `GuiComponent` and `Description<MomentumScrolling>` ancestry. The reflected constructor is parameterless and returns `MomentumScrolling*`, not a constructor taking that pointer. Register the type, base, `AttachControl`, `DetachControl`, and read/write `Deceleration` in the existing composition reflection/type-list files under `<GacUI repo>/Source/Reflection/TypeDescriptors`. The component is owned by one `GuiInstanceRootObject` through `AddComponent`; control attachments do not own controls. Use the existing UI-thread animation facilities rather than worker threads.
- Store listener handles, the retained disposed flag, real-sequence samples, playback state and animation ownership separately for each control. Duplicate attachment must not install duplicate listeners; detaching a control that is not attached is a no-op. One control's restart/detachment must not stop another control's animation.
- `GuiDisposedFlag` is a passive flag, not a disposal event. Check it before every access to retained control/composition pointers, including cleanup. Retire disposed entries on animation ticks and component operations; an idle disposed entry can remain inert until the next operation or component teardown. Do not introduce a polling thread just to retire idle bookkeeping. For a disposed target, release its bookkeeping without calling `DetachControl` or accessing its destroyed event receiver. For a live target, detach its listeners normally. Component `Detach` and destruction must invalidate all callbacks as well as stop/release animations.
- Cancellation and any extra contact invalidate momentum for that sequence, matching the scrolling recognizer's permanent stop after a second Down. Start inertia only after successful End with usable scrolling samples and nonzero velocity. Stop/reset on the next Started, explicit detachment or target disposal. Attaching midway through an existing sequence should wait for the next Started. Do not allow paused/stale animations to resume against a replacement control or graphics host.
- Event handlers may uncheck the option or detach another control during a generated gesture. Keep the executing state alive until dispatch returns and recheck its validity before further work. `<GacUI repo>/Source/Application/Controls/GuiInstanceRootObject.cpp` removes stopped animations by index after `Run`; avoid invalidating that iteration through synchronous `KillAnimation` calls from the same callback. Use stopped state and safe retirement, or fix the scheduler if its mutation contract must be expanded. Removing a receiving composition during input remains deferred, as with existing point handlers.
- Sampling and deceleration:
  - `GuiPointGestureEventArgs` has no timestamp. Cache the corresponding raw `pointAction` timestamp, which is delivered before recognition, and seed cumulative displacement at Started. Include final-Up movement and the stationary interval through the End timestamp. Input timestamps are monotonic microseconds; derive velocity entirely within that input clock, without subtracting a renderer timestamp from the local animation clock.
  - Let `tEnd` be the successful End time and `tStart` the later of Started and `tEnd - 0.5 seconds`. Compute signed average velocity from cumulative displacement over this interval. Retain the last sample before its left boundary for interpolation, and treat time after the last movement as stationary. Prune old history while recording, retaining the boundary predecessor, so memory does not grow with the whole gesture. Coalesce equal-time samples and never divide by zero; a zero-duration or zero-displacement interval produces no inertia.
  - Default `Deceleration` to `1.0` and require a finite value greater than zero, using the usual contract failure for invalid values. Use constant deceleration magnitude `320 * Deceleration` logical pixels per second squared, opposing the signed initial velocity. Thus 160 logical pixels over 0.5 seconds means 320 pixels/second and stops after 1 second at the default setting; `2.0` stops in 0.5 seconds and `0.5` in 2 seconds.
  - Emit continuing scrolling increments at 0.1-second elapsed boundaries, plus the final partial interval when speed reaches zero. Integrate the linearly decreasing speed over each interval, carry fractional logical-pixel residue, and clamp at zero without reversing direction. The default example adds 160 logical pixels after release before scrollbar clamping. Base progress on elapsed animation time rather than timer-callback count, including delayed callbacks. Capture the deceleration setting when playback starts so a property change applies to the next playback.
- Update the authored demos at `<GacUI repo>/Test/Resources/App/FullControlTest/TouchingListTabPage.xml` and `<Release repo>/Tutorial/GacUI_Windows/TouchingList/UI/TouchingListWindow.xml`. Own one component in each root and attach/detach the list from the initially unchecked `Momentum Scrolling` checkbox before `Clear`. Preserve horizontal swipe reveal/reset and the queued, identity-based deletion at real End; synthetic gestures must neither resurrect an ended row swipe nor delete an additional/recycled row. Regenerate their generated C++ and the tutorial resource `<Release repo>/Tutorial/GacUI_Windows/UIRes/TouchingList.bin` through the existing generators.

### VERIFICATION

- Extend the existing point-scrolling coverage in `<GacUI repo>/Test/GacUISrc/UnitTest/TestControls_PointScrolling.cpp`. The `PointProtocolInput` helper in `<GacUI repo>/Test/GacUISrc/UnitTest/TestPointInput.h` currently reuses action timestamps; provide explicit sample timing and a controllable animation clock for the new cases. Do not use wall-clock sleeps to assert velocity or stopping time.
- Use ordinary unit tests for sampling and lifecycle behavior: short sequences, exact/interpolated 0.5-second boundaries, older discarded samples, a stationary tail, equal timestamps, direction reversal, both axes/signs, zero velocity, fractional distances, delayed animation ticks, and deceleration values `0.5`, `1.0`, `2.0` plus invalid values. Assert the default 320-pixels/second example stops at one second and travels 160 additional logical pixels before clamping.
- Verify attachment before the first touch, duplicate attachment, detach/reattach, multiple independent controls, no synthetic feedback, no scrolling after uncheck, new Started interrupting inertia, cancellation, second-contact suppression, child capture, disabled/absent scrollbars, boundaries, template refresh, host changes, control disposal while idle and animating, root/component teardown, and detachment from within a generated event. Check for stale callbacks and debug memory leaks.
- Use useful rendering snapshots for visible post-Up scrolling with capture already cleared, preserved list selection, document ViewOnly/chrome scrolling, and unchanged Selectable/Editable range selection. Keep non-rendering assertions in ordinary tests and name frames after the already-completed actions.
- Follow `<GacUI repo>/Project.md` and the repository build/execution wrappers: build Debug Win32 and x64 and run their `Metadata_Generate` projects, run x64 `Metadata_Test`, run `GacUI_Compiler` for the XML changes, repeat the required metadata steps after compiler generation, rebuild affected consumers, and run the full `UnitTest` project. Check generated compiler errors rather than hand-editing generated files. Verify native code in no-reflection, metadata-only and full-reflection consumers, plus interpreted Workflow through `GacUI_Host`.
- Extend `<GacUI repo>/Test/UIA_CppTest_Shared.cs` and run `<GacUI repo>/Test/UIA_CppTest.ps1` and `<GacUI repo>/Test/UIA_CppTest_Metaonly.ps1` with `-Scenario Touching -SkipBuild` after successful builds. Check checkbox role/name, initial unchecked state, position before Clear, toggling, continuing movement after Up, interruption on Down/uncheck, list/document selection, swipe deletion and Clear/repopulation during playback. Exercise interpreted Workflow with the shared wrapper's `-Application GacUI_Host -AsPort 8892` option, and build/run the Release tutorial too.
- Verify remote renderer input through `<GacUI repo>/Test/Touching_Remote.ps1` and the existing browser fixture when available, observing viewport movement after release and stopping on fresh contact. Separate Core injection, native-renderer injection, browser acquisition and physical touch results. Record unavailable platform/hardware coverage explicitly; native wGac/iGac do not currently acquire touch.

- Update knowledge base about remote protocol in `GacUI` repo:
  - Simply mention that core and native renderer could both easily control date time, locale, file system implementation via `Inject(DateTime|Locale|FileSystem)Impl`.

### DETAILS

- The canonical remote guidance is `<Tools repo>/Copilot/KnowledgeBase/KB_GacUI_Design_RemoteProtocolCoreArchitecture.md` and `<Tools repo>/Copilot/KnowledgeBase/KB_GacUI_Design_RemoteProtocolRendererAndSerialization.md`. Update the canonical text and keep the corresponding `<GacUI repo>/.github/KnowledgeBase` copies synchronized without broad repository initialization during publication.
- Briefly distinguish replacing a process's underlying services through `InjectDateTimeImpl`, `InjectLocaleImpl` and `filesystem::InjectFileSystemImpl` from enabling remote injection through configuration/capability flags. Both Core and a native renderer can install their own underlying implementations. Preserve existing documentation of opt-in defaults and fallback behavior.
- Also qualify the existing no-inertia statement in `<Tools repo>/Copilot/KnowledgeBase/KB_GacUI_Design_PointInput.md` and its GacUI copy: ordinary scroll views retain their current behavior, while the optional `MomentumScrolling` component adds continuation. Document its stable subscription target, capture-query distinction and selection behavior consistently with the implementation.

### VERIFICATION

- Cross-check service API names and namespaces against the imported Vlpp/VlppOS declarations and the remote controller/renderer implementation. Confirm canonical and copied guidance agree, use repository-relative references, and do not describe direct service injection as automatically opting into the remote capability.

- Update website:
  - Mention `GuiRemoteControllerConfig::inject(DateTime|Locale|FileSystem)`.
  - Mention `ControllerGlobalConfig::inject(DateTime|Locale|FileSystem)`.
  - Mention `GuiPointSequenceStartedEventArgs::generate(Left|Right)ButtonClick`.
  - Mention `MomentumScrolling` component.

### DETAILS

- Edit the authored articles `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/modes/remote_core.xml`, `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/modes/remote_client.xml`, `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/components/compositions/events.xml`, and component documentation/navigation. Qualify the current no-inertia statement in `<WebsiteSource repo>/packages/website-doc2/src/articles/gacui/components/controls/container/scrollcontainer.xml` and link the optional component. Follow the website repository's own instructions and maintain its exported manual.
- Document all three flags separately. `GuiRemoteControllerConfig` defaults them to false; the native renderer's `GuiRemoteRendererSingle::GlobalConfig` defaults its capabilities to true, while the demos explicitly disable them. Both sides must enable a capability for remote use. DateTime/Locale fall back to predecessor implementations; an enabled but unavailable remote filesystem exposes an empty root. Do not change those behaviors as part of this documentation task.
- Explain both click flags default true and are configured during Started for that sequence. Left-only permits both short and long eligible presses to become left clicks; right-only suppresses short taps; disabling both flags suppresses both deferred clicks. Preserve movement/multiple-contact/cancellation exclusions, and explain these flags do not change physical mouse input or rejected-sequence mouse compatibility.
- Explain component ownership, attaching multiple controls, explicit `AcceptPointEvents` configuration for non-scroll views, detachment/disposal, selection compatibility, and the positive `Deceleration` setting with the numeric example. Include an XML example using the reflected parameterless raw-pointer constructor and root-owned component lifetime.

### VERIFICATION

- Follow `<WebsiteSource repo>/AGENTS.md` for website build/tests, both package-download validations in sequence, markdown export and browser inspection. Check navigation and real links, type/member references, the new component example, and readable exported markdown. Ensure docs no longer imply that all scrolling is momentum-enabled by default.

- Commit and push all local changes before publishing the website, you have to skip `job:copilotInitAll` during publishing.

### DETAILS

- During execution, commit and push changes in every affected repository before publication. Keep manually maintained files, generated files and unit-test snapshots in their required separate commits. This review only prepares the task specification; implementation, generation and publication belong to task execution.
- `<WebsiteSource repo>/job.publish.prompt.md` currently invokes the Windows `copilotInitAll` job twice; skip both invocations as requested. Preserve the remaining publication steps, including manual export and the corresponding canonical Tools manual/index updates, then commit and push publication outputs too.
- Preserve the publication order: website build, then `<WebsiteSource repo>/packages/website/scripts/Copy-WASM.sh` in the configured Linux/macOS environment, with no subsequent build that clears the copied runtime. Preserve the existing published `<vczh-libraries.github.io repo>/wasm-fct` unless replacement is separately requested; documentation updates do not require replacing it.

### VERIFICATION

- Before publishing, verify all intended commits have reached their remote branches. After publishing, verify the deployment result and updated live pages, then confirm publication-generated changes have also been committed and pushed. Record skipped initialization explicitly without claiming those jobs ran.

## REVIEW COMMENTS

# UPDATES

# TEST [CONFIRMED]

Deterministic point-input and animation-clock regressions will cover sampling, deceleration, per-control lifetime, routing and selection. Rendering snapshots, generated/interpreted demos, native renderer input and documentation/publication checks follow the verification requirements above.

- Baseline Debug x64 solution build passed with zero warnings/errors. The ordinary reflection regression runs inside the initialized compiler-native harness, confirms GuiComponent is registered, and fails because MomentumScrolling is absent. This establishes the missing API independently of snapshot rendering.

# PROPOSALS

- No.1 Per-control sampled continuation through the existing UI animation scheduler [CONFIRMED]

## No.1 Per-control sampled continuation through the existing UI animation scheduler

Declare the reflected component in the point header. Subscribe at persistent bounds and record only sequences owned there, using raw monotonic timestamps and cumulative signed displacement. Interpolate the trailing half-second interval, include stationary End time, and integrate constant deceleration at elapsed 100-ms boundaries. Each control has its own retained disposal flag, handlers, samples and animation state. Stop on Started, extra contacts, cancellation, detachment and host changes. Dispose bookkeeping without touching destroyed receivers.

Use stopped animation state for safe retirement during dispatch; do not synchronously mutate the root animation array. Keep an executing state's owning reference through event callbacks. Permit targeted post-End scroll-view gestures while preserving scrollbar checks and child ownership. Own the demo component in each root and toggle attachment from an initially unchecked checkbox. Preserve existing swipe-end identity handling.

### CODE CHANGE

The ordinary sampling/lifetime tests and list continuation snapshots pass. Debugging the document fixture showed its caret setup had already scrolled the viewport (250 pixels after the 160-pixel drag); explicitly reset the initial viewport before the gesture, as the existing selection fixture does.

Implemented the component, composition reflection registration, targeted scroll-view dispatch, safe event-handler traversal, authored checkbox demos, deterministic input-clock/animation-clock tests and post-Up selection snapshots. Initial compilation corrected test-only collection spelling; the first runtime run exposed an invalid Unknown theme in the ordinary fixture, now replaced with CustomControl. Verification includes deterministic ordinary tests, useful post-Up snapshots, both metadata architectures, compiler regeneration, full unit suite, generated/interpreted showcases, tutorial, native-renderer/browser coverage and website validation/publication. Publication skips both broad initialization jobs and preserves the deployed Wasm demo.
Additional ordinary regressions cover absent/disabled/hidden scrollbars, disabled controls, wrong event sources, clamping, child acceptance, late attachment and idle disposal. The post-template selection snapshot also exercises Selectable mode; the initial selection snapshot exercises Editable mode.

Review identified two refinements: a Started handler installed after the component may accept the bounds, so retain the Started seed while acceptance is undecided and only consume correctly targeted raw actions/gestures. Compute captured stopping duration by dividing before scaling, then integrate normalized speed, avoiding overflow for valid extreme finite deceleration values.

Extend the shared UI Automation scenario with a horizontal full swipe while momentum is enabled. After real End removes the selected identity, wait through generated horizontal events and assert that exactly one row was removed and no recycled row exposes a red panel.

The first native Win32 automation run timed out in the pre-existing two-contact rotation check, before momentum. Add actual/expected transform diagnostics to that check, then reproduce and diagnose before proceeding.

Diagnostics showed the first point at (20,19), the next at (49,39), and the final transform (-193,359), rather than the fixture's assumed (-190,360). Native Windows logical/native conversions truncate at the current DPI. Compute the fixture's analytic translation, scaling and quarter-turn expectations from those converted coordinates, retaining the existing one-pixel translation and 0.001 axis tolerances; no recognizer change is needed.

The corrected transform checks pass. The same DPI quantization can leave a swipe exactly one pixel short of the deletion threshold; use the existing full-reveal fixture's 50-pixel overshoot for both final-Up deletion checks, while keeping exact threshold/clamp assertions in the deterministic unit tests.

Numerical review reproduced a rounding edge: velocity 10 pixels/second and the representable multiplier immediately below 0.3125 yield a duration just above 0.1 seconds. The first interval rounds half a pixel to one and carries -0.5; the almost-zero final interval can round that carry to -1. Clamp each rounded increment to the initial velocity's direction, as required by the no-reversal contract, and add a deterministic two-sign regression spanning the 100-ms and 101-ms ticks.

CDB reproduced the new assertion failure in Debug x64 before the correction: at time 0.101, velocity was -10, the interval ended at 0.10000000000000001943, integrated distance was zero, and the emitted increment was +1. The regression rejected that reverse step. The correction clamps the rounded increment to the captured velocity's sign while retaining fractional residue.

The remote matrix passed Pipe and Http with both native backends. MiniHttp then rejected the fixture's first POST with HTTP 417: the existing VlppOS HTTP parser deliberately rejects Expect headers, while Windows PowerShell adds Expect: 100-continue by default. Disable that handshake on the fixture's endpoint ServicePoint before POSTing; preserve the server's existing protocol contract and rerun both MiniHttp backends.

Windows OS input acquisition passed, but its subsequent repeat of the remote scenario missed continuation after release. The fixture performs an expensive full Controls read between the final move and Up, then another settling read and delay before beginning its continuation observation. This changes the sampled velocity and can consume the short playback window. Move the baseline observation before the timed flick, use a 200-pixel flick with two 100-ms intervals, and observe immediately after its completion. Deterministic unit tests retain exact velocity, final-Up and stopping-time assertions; the integration fixture verifies real viewport continuation and interruption without including snapshot latency in its input sampling.

### CONFIRMED

Select this proposal as the implementation. Sampling uses only the raw input clock, while elapsed animation integration uses its own clock. Deterministic tests establish the required distance and timing, and generated/interpreted/native/remote/browser consumers establish actual control routing, selection preservation and lifecycle behavior. The component remains optional, owns no attached control, and safely retires stopped animations and detached listeners. Review found and corrected the representable half-pixel reversal edge. Publication is separately pending the external environment described below; this confirmation does not claim that deployment has occurred.

- The full unfiltered Debug Win32 suite passed 1,951/1,951 cases in 96/96 files, without a debug leak dump. This run preceded the final rounding correction; final focused runs on both architectures cover that correction. The initial focused Win32 run passed 102/102 cases.
- Complete two-architecture resource regeneration finished without UI compiler error files. Final Debug Win32 and x64 builds passed with zero warnings/errors, and CDB confirmed both signs of the new rounding regression pass after the correction. Final focused runs passed 168/168 cases in 10/10 files on x64 and 105/105 cases in 3/3 files on Win32, without debug leak dumps.
- The required post-generation sequence completed: Win32 build and Metadata_Generate, x64 build and Metadata_Generate, then x64 Metadata_Test. One x64 generation attempt failed opening Reflection64.bin for writing while other checks overlapped. CDB confirmed FileStream::Write received a closed stream, with a null implementation, at the full-metadata output. The file was available when inspected; a serialized rerun with all consumers stopped passed both generation and Metadata_Test. No metadata or reflection source change was needed.
- The Release tutorial and the website's reflected XML example generated successfully for both architectures with the current GacGen/GacBuild tools. The cached older GacGen failed against current metadata; CDB traced that incompatibility to the old Workflow emitter. Rebuilding the current tools resolved generation without changing their sources.
- The Release tutorial rebuilt successfully in Debug x64 after the final correction. UI Automation verified the initial unchecked checkbox, its position before Clear, toggling, list selection and scrolling, Clear/Add behavior, and normal shutdown.
- Complete Win32 Touching automation passed 8,337 assertions in both no-reflection and full-reflection native consumers, and 8,336 assertions in the interpreted Workflow host. A full-reflection theme-navigation timeout during a concurrent large build passed on rerun. Both required named x64 wrappers, UIA_CppTest and UIA_CppTest_Metaonly, also passed 8,337 assertions each and shut down normally.
- All six native remote combinations passed: Pipe, Http and MiniHttp with Direct2D and GDI. These exercised Core and renderer listener injection, actual rendered rectangle/contact pixels, post-Up momentum, fresh-Down interruption, identity-based swipe removal, list selection, document selection before/after template replacement, and normal shutdown.
- Chromium CDP touch acquisition passed with both Http and MiniHttp. Outgoing protocol events, actual canvas pixels, momentum after release and interruption on fresh touch, selection rendering, swipe identity removal, unchanged outer-page scrolling, compatibility-mouse suppression and template replacement all passed. Both runs reported no browser errors and shut down Core normally. The momentum screenshot shows the checkbox before Clear and the scrolled list.
- Windows OS injection through the Http/Direct2D native renderer passed WM_POINTER one-contact translation, multiple contacts with the original lifting first, canceled Up, rejected-touch compatibility mouse forwarding and independent unpromoted Windows mouse injection. Physical touch hardware, other-monitor DPI, and native Linux/macOS execution remain unverified; native wGac/iGac do not currently acquire touch. These results are separate from Core/listener injection and Chromium CDP acquisition.
- After removing full-tree snapshot latency from the timed flick, the complete Http/Direct2D run with NativeInjection passed again, including momentum, interruption, list/document selection, template replacement and graceful shutdown. The earlier six-combination matrix and both browser transports passed independently.
- Website build, all three test targets, sequential main/docs downloads, markdown export, and Chromium inspection of all changed articles passed. Navigation links to the new component page work, and the exported example compiled. Canonical and copied remote/point knowledge agree.
- Publication prerequisites are incomplete on this host: `wsl --list --quiet` reports that WSL is not installed, and no container runtime or configured SSH host is available. The requested Linux/macOS Copy-WASM step and deployment remain pending. Both broad initialization invocations were skipped, as requested. The publishing repository and its existing Wasm folder were left unchanged.

Remaining publication work requires the configured Linux/macOS build environment: complete the prescribed website build then Copy-WASM sequence, perform sequential main/docs downloads and markdown export without another website build, publish while preserving the existing wasm-fct folder, and verify CI and live pages. Then replace the canonical Tools manual and its final index section from the exported markdown and push those outputs. The local website export and validation are complete, but the canonical manual/index publication is pending this ordered workflow. No broad copilotInitAll invocation is required or permitted for this request.

Authored website documentation is pushed in `<WebsiteSource repo>` commit `b35948b`. Canonical knowledge changes are pushed in `<Tools repo>` commits `359689d` and `66922a7`. The tutorial and regenerated distribution are pushed in `<Release repo>` commits `15eaeac6` and `588871c1`. GacUI implementation, generated files and snapshots are retained in separate commits following this investigation confirmation.
