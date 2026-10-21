# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

execute GacUI/TODO_Touch_Step4.md , commit and push all local changes

Details are in [TODO_Touch.md](./TODO_Touch.md), but this document has a much bigger scope, so keep the target scope in this request, uncovered parts will be handled in future requests. Here are completed works:
- `TODO_Touch_Step1.md`
- `TODO_Touch_Step2.md`
- `TODO_Touch_Step3.md`
- `TODO_Touch_StepR1.md`
- `TODO_Touch_StepR2.md`

There is a known issue when handling mouse events on a composition, the handler deletes the composition, and GacUI will crash. This is expected, and point events will be handled just like how mouse events are handled, it will have the same issue, and this is also expected. When implementing point events, you are going to figure out if any code for mouse events could be reused for propogating and capturing. GacUI will offer a pair of new capture/release functions for point events, they work like how captures are done to mouse events.

This request is about updating GacJS and wGac as well as refreshing the WASM demo in the website.

- Make sure WASM works with new test pages: `RawClock`, `Touching` and `TouchingList`.
  - Run `.github/Jobs/job.rpWasm.prompt.md`.
  - Make sure `DebugRemoteProtocolSop.md` has test instructions about `Touching` and `TouchingList` test pages.
- Port to wGac, it doesn't support touch experience so it should only offer an empty implementation when needed.
  - Run `.github/Jobs/job.rpXPlat.prompt.md`.
  - Fix this file to mention that `wGac` and `iGac` does not support touch experience, so that touch verification could be skipped.
- Update website's WASM demo.
  - The website is just published so all files should already sync.
  - Instead of running the complete publishing, we should just replace WASM demo related files to make the change small.
- commit and push all local changes.

## DETAILS

### Scope and existing implementation

- The completed follow-ups named above are `<GacUI repo>/TODO_Touch_Step3R1.md` and `<GacUI repo>/TODO_Touch_Step3R2.md`. The visible FullControlTest page called `RawClock` in this request is `Misc > Raw Clock`, implemented in `<GacUI repo>/Test/Resources/App/FullControlTest/RawElementTabPage.xml`.
- Preserve the completed point acquisition, capture, routing, gesture recognition, automatic click fallback, and directly reflected `gesture` variant. The opening capture paragraph is historical context; this step does not introduce another capture API, restore the old `Gesture` getter, or make synchronous composition deletion inside an input handler safe. Current contracts are documented in `<GacUI repo>/.github/KnowledgeBase/KB_GacUI_Design_PointInput.md`.
- This step completes Wasm integration and verification, wGac compatibility, the requested SOP corrections, and a targeted public demo refresh. Broader touch features in `<GacUI repo>/TODO_Touch.md`, such as inertia, selection handles, mobile text input, and additional native touch providers, remain outside this request.

### GacJS and Wasm integration

- The browser renderer already acquires touch, and `<GacJS repo>/Gaclib/website/entry/assets/fullControlTestRaw.js` already paints both `FullControlTest.Clock` and `FullControlTest.Touching`. The Wasm FCT entry page already passes its installer to `startWasmPage` in `<GacJS repo>/Gaclib/website/entry/src/wasmPage.ts`. Reuse these paths and investigate actual failures instead of introducing a second painter or browser gesture recognizer. TouchingList uses ordinary controls and requires no raw painter.
- Keep gesture recognition and list deletion in Core. The existing ordered point messages must pass through the Wasm worker/channel adapter with their explicit Started/End boundaries, consecutive event indices, sparse contact identities, cancellation reasons, and decimal-string 64-bit fields intact. Do not introduce a Wasm-specific point schema or convert full-width identifiers through JavaScript numbers.
- Ensure the Wasm builds consume the current x86 generated resources and matching framework dependencies from the completed variant work. Follow owning generation/release tools if anything is stale; do not repair generated C++, metadata, or imported files by hand. All three Wasm apps use no-reflection generated C++; the browser view-model host remains specific to RVMT.
- Extend the existing Wasm suite in `<GacJS repo>/Gaclib/website/entry/test/Testing_Wasm.js` and applicable Firefox checks in `<GacJS repo>/Gaclib/website/entry/test/Testing_Wasm_Firefox.js`. The existing `<GacUI repo>/Test/Touching_Browser.mjs` supplies useful operations, but it depends on a native HTTP Core and its automation endpoint. Adapt its checks to the Wasm page's renderer, rendered state, and worker-channel observations; running that script unchanged is not Wasm verification.

### SOP placement and coverage

- Audit the page-specific operations already present in `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md`. Keep every FullControlTest operation under `Complete Control Showcase (/FCT)`, and retain RPT/RVMT operations under their own applications. In particular, `Verify Touching and Rejected Touch` is currently inside the `/RPT` section even though its `Misc > Touching` page belongs to FullControlTest. Move it into `/FCT`, repair numbering and references, and preserve the remaining RPT replacement/close sequence.
- Add explicit `/FCT` procedures for `Misc > Raw Clock`, `Misc > Touching`, and `Misc > TouchingList`, using the operations and expected results below. Raw Clock and TouchingList currently have no page-specific coverage in this SOP. Touching needs its existing procedure moved and its applicability updated for Wasm.
- Per the follow-up scope clarification, other FullControlTest pages not currently mentioned in the SOP may be skipped. Do not expand this task into documenting or testing every showcase page. Preserve and correctly place the existing documented checks; only the three explicitly requested pages require new coverage.
- Keep feature operations in the SOP and process setup/test matrices in their existing job/topology documents. State which lifecycle checks apply: HTTP/native renderer replacement preserves a live Core; the Wasm pages own their Core and use shutdown/page Reload instead. The public website demo currently restarts through browser refresh, rather than the development site's Reload button.
- Interpret “Fix this file” in the wGac item as `<GacUI repo>/.github/Jobs/job.rpXPlat.prompt.md`. State that wGac and iGac native providers do not support touch experience and may skip their touch-specific checks. This exception does not cover GacJS running on Linux/macOS or Wasm browser input. Raw Clock and existing mouse/keyboard checks remain applicable to native GUI renderers.
- Keep the corresponding canonical instructions in `<Tools repo>/Copilot/Jobs/DebugRemoteProtocolSop.md` and `<Tools repo>/Copilot/Jobs/job.rpXPlat.prompt.md` synchronized with the working copies. Update `<Tools repo>/Copilot/Jobs/job.rpWasm.prompt.md` and its GacUI copy if needed to connect the required Wasm checks to the revised SOP. Use repository-qualified paths, and keep run-specific results in test cards rather than reusable instructions.

### wGac compatibility

- Follow `<GacUI repo>/.github/Jobs/job.rpXPlat.prompt.md` on Linux. Bring the current GacUI release and dependency snapshots forward through their owning tools, then run `<wGac repo>/import.sh`, `<wGac repo>/syncProj.sh`, and `<wGac repo>/build.sh` in order. Preserve the source ownership rules for imported libraries and generated app resources.
- Add only compatibility declarations/empty implementations actually required by the updated interfaces or mirrored test apps. The point callbacks on `INativeWindowListener`, declared in `<GacUI repo>/Source/NativeWindow/GuiNativeWindow.h`, already have default empty implementations in its `.cpp`; do not add an acquisition subsystem merely because those callbacks exist. Preserve existing mouse behavior and Raw Clock rendering.
- The Linux job does not require implementing or running iGac. Document the same native-touch exclusion for iGac without reporting macOS verification. Keep excluded native touch items identifiable in the test cards with the stated reason and no passing result; synthetic Core input does not establish native hardware support.

### Targeted website demo refresh

- The public destination is `<vczh-libraries.github.io repo>/wasm-fct`. The three GacJS development pages are a separate deployment. Refresh only the public FCT demo and its necessary authored packaging inputs; do not run the full website/document publication or copy the entire GacJS test site over the published site.
- There is a concrete launcher gap: `<WebsiteSource repo>/packages/website/assets/wasm-fct/demo.js` creates and starts its renderer without installing the shared raw painters. Install `installFullControlTestRawElements` before protocol delivery and retain its cleanup for normal shutdown, failure, and page disposal. Reuse the GacJS helper rather than copying its drawing implementation into the website launcher.
- `<WebsiteSource repo>/packages/website/scripts/Copy-WASM.sh` currently copies browser bundles and the Wasm runtime, but omits `<GacJS repo>/Gaclib/website/entry/lib/dist/fullControlTestRaw.js`. Extend that packaging inventory so the authored demo's relative import is reproducible on future publishes. Keep the website's existing loading UI and scoped service worker.
- Stage a consistent set from the final verified builds: WasmFCT's `app.mjs`, `app.wasm`, and `app.worker.js`; the GacJS browser bundles required by the current packaging script (`gacui.js`, `wasm.js`, `http.js`, `rvm.js`, `wasm-worker.js`, and their emitted source maps); the shared raw helper; and the authored demo assets that changed. Recalculate `<vczh-libraries.github.io repo>/wasm-fct/app-size.json` from the decoded byte size of the exact deployed Wasm binary.
- Keep `app.worker.js` (Emscripten's pthread worker) distinct from `wasm-worker.js` (the GacJS application host). Preserve the generated module and its default factory unchanged. Resolve demo-local imports beneath the public demo directory, without depending on development-site root files.
- Finish builds before final staging. GacJS builds clear its entry output, and WebsiteSource builds clear its prepared output; `<WebsiteSource repo>/packages/website/scripts/Copy-WASM.sh` itself rebuilds GacJS. If a later build invalidates a verified deployment, recopy and reverify the final artifacts. Keep Wasm outputs ignored in GacUI/GacJS; the public site's deployed runtime files are tracked publishing artifacts and must be committed there.

## VERIFICATION

### Builds and test records

- Execute `<GacUI repo>/.github/Jobs/job.rpWasm.prompt.md`, including all three Wasm apps, their native compile/link checks, Chromium/Firefox coverage, and the existing shutdown/RPC regressions. Build each Wasm project with the prescribed `-bw -o` wrapper command; rebuild Wasm after a native full build cleans its output. Initialize and maintain `<GacUI repo>/.github/TaskLogs/TestMatrix_Wasm.md` as the job requires.
- In `<GacJS repo>/Gaclib`, run `yarn install`, `yarn run import`, `yarn codegen`, `yarn build`, and `yarn test`. Run `<GacJS repo>/copy-wasm.sh` after the final build, serve the entry package in an interactive CLI session, and run `npm run test-wasm`. A normal Linux `yarn test` skips the Windows-only HTTP end-to-end suite and does not replace the Wasm suite.
- Follow the cross-platform job's applicable native, GacJS, and separate TUI matrices, with the native-touch exclusion and the SOP page-scope clarification above. Verify wGac standalone and hosted FCT, the native remote renderer, and the existing non-touch regressions. Record results in `<wGac repo>/TestMatrix_NativeRenderer.md`, `<wGac repo>/TestMatrix_GacJS.md`, and `<wGac repo>/TestMatrix_Tui.md`.
- Apply `<GacUI repo>/Project.md` build/unit-test and metadata/resource-generation requirements if implementation changes trigger them. Update corresponding GacJS/website behavior documentation when necessary. This task-document review itself runs no application builds or runtime tests.

### Required FullControlTest page checks

1. **Raw Clock:** Navigate to `Misc > Raw Clock`. Require one visible canvas painting the received `HH:mm:ss` state and changing clock-hand pixels as Core updates that state. Check hide/show, resize, palette refresh, and shutdown/restart without duplicate canvases or retained observers. The painter must not invent an independent ticking clock. Retain the existing Wasm clock regression rather than replacing it with a raw-data-only assertion.
2. **Touching:** Require the sky-blue rectangle and yellow contact circles. Verify one-contact translation; two-contact translation, rotation, and scaling; three-contact pause; and membership changes that establish a new baseline without a jump. Exercise A Down, B Down, A Up, C Down, B Up, C Up, including reused browser pointer IDs. Observe one Started/End pair, consecutive indices, distinct Down-order slots, and actual transformed pixels. Normal capture release after A's Up must not cancel B. Cancellation clears markers once, generates no successful click, and permits a clean subsequent sequence.
3. **TouchingList:** Use Clear and Add 5 Items to establish known item identities/counts. Verify vertical scrolling, a left swipe revealing the measured red `DELETE IT` area, clamping at its maximum, immediate reversal, and partial-release reset. Right-only movement does not reveal/delete. Successful single-contact completion at maximum removes exactly the original item after dispatch; crossing the threshold on final Up is included. Cancellation, an extra contact, a start outside a row, and an incomplete swipe must not remove an item. Clear during a pending interaction and scrolling/recycling rows must not redirect deferred deletion or retain another row's reveal. Verify remaining identities and visible layout, not merely a changed count.
4. **Browser ownership and ordinary input:** Drive touch through browser input injection on the actual Wasm renderer. Check capture outside the surface, nested labels/paragraphs/raw canvases, child DOM replacement, and no outer-page pan/zoom or duplicate compatibility mouse activation. Keep physical mouse, wheel, and keyboard behavior working. Verify one activation on an ordinary rejected-touch control and no secondary-contact promotion. Preserve accepted-list tap/scroll and selectable/editor text behavior from the completed Core implementation.
5. **Lifetime and browser coverage:** Repeat relevant interactions after resize, theme refresh, and application restart. On development Wasm pages, verify Exit/Force Exit, canceled RPT Exit, worker termination, and Reload gating/reset as required by the Wasm job. Do not substitute HTTP renderer takeover for these checks. Use Chromium's browser-level touch injection for acquisition coverage; distinguish any Firefox DOM-synthetic checks from browser-native input. Record unavailable physical/mobile hardware checks explicitly instead of inferring them from synthetic input or a successful build.

### SOP and public deployment checks

- Compare the SOP before and after the edit: all previously documented FullControlTest page operations must be under `/FCT`, all three requested pages must have navigation/actions/observable results, and RPT/RVMT instructions must still refer to controls in their own applications. Check numbering, links, native-touch exclusions, and Wasm lifecycle applicability. Other previously unmentioned pages remain optional for this task.
- Check the explicit GacJS copy step's missing/empty input and destination validation, compare all nine deployed development runtime files with their three corresponding build outputs, and retain the job's module/worker-loading checks.
- Before updating the published directory, verify every required staged file and relative dependency exists. Compare the public runtime and browser assets with the final verified inputs, confirm the exact Wasm size manifest, and inspect the public-repository diff to ensure it is confined to the demo directory. Keep unrelated published pages and documentation unchanged.
- Serve the prepared public demo through its real website route and service-worker flow, then repeat Raw Clock, Touching, TouchingList, ordinary input, Exit/Force Exit, and browser-refresh checks. A pass on the development Wasm page is insufficient because the public launcher is different. Check download progress, isolation, worker loading, missing requests, console errors, and teardown in fresh and previously visited browser contexts.
- After committing/pushing the implementation and public demo, wait for the public site's deployment and follow `Web > Start WASM Now!` in a fresh browser context. Repeat the three page checks on the deployed demo and record actual evidence or access limitations. Commit/push changes in each affected repository, separating authored changes, generated/imported files, and snapshots where required. The present review commits only this task file.

## REVIEW COMMENTS

# TEST

Execute the Wasm, native renderer, GacJS and terminal matrices, then verify the targeted public demo locally and after deployment. Record actual results and unavailable hardware checks in the test cards.

# PROPOSALS

## No.1 Complete Step 4 integration and verification

### CODE CHANGE

Implemented shared public raw-painter installation/cleanup and reproducible packaging; extended real Wasm worker/input/pixel/list/lifetime regressions and full-width channel payload coverage. Preserved Core point semantics. Rebuilt all three native/Wasm projects, refreshed wGac through owning tools, and verified native/HTTP application matrices. The terminal matrix records separate byte-replay, compositor-input and desktop limitations.

A repeated live second-host rejection exposed RVM renderer cleanup before startup. The guarded cleanup and lifecycle regression preserve the original connection error. Final builds, unit suites and development/local/live public browser suites pass; exact evidence is in the test cards.

The public change is confined to the FCT demo. `bec531fff` is deployed, both browser engines passed through the real Web link, and the live Wasm hash matches staging.

Later user steering: “I would like you to completely delete Tools/Copilot/Jobs folder and push, as these documents are supposed to be in GacUI/.github/Jobs”. Deleted the folder and pushed `27b9058`; updated the task's ownership instruction so it will not recreate those copies. This supersedes the copied original synchronization request above.
