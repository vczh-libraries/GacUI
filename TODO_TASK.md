This is a task to make browser and potential future renderers could do user typing correctly.

## Task 1

Currently the DOM tree lacks of a mechanism to report which composition is editable.

Add to `GuiGraphicsComposition::(Get|Set)EditableAttributes`, the header file is already edited:
- It does nothing but a simple getter/setter.
- It does not trigger any event/
- It is not responsible to editing or any other feature.
- You should complete the reflection registration for it.

Currently only `GuiDocumentViewer` and `GuiDocumentLabel` will update its value, but it should be able to only change `GuiDocumentCommonInterface`:
- `documentControl->GetFocusableComposition` will be the target.
- Remove `focusableComposition` from `GuiDocumentCommonInterface::InstallDocumentViewer` and use `_sender->GetFocusableComopsition` instead.
- When a control got or lost focus, update `EditableAttributeValues::focused`. Currently disabling a control should automatically make it lost control so no need to handle enabling/disabling.
- `GuiDocumentEditMode::Editable` sets `EditableAttributeValue::editable` to true, other values set it to false.
- `EditableAttributeValue::featured` will always be true, as a hint that it belongs to a text box.

This new property is expected to completely have no side effect on core, only affecting remote protocol DOM tree content, the protocol definision is already edited.
To make a editable focusable composition into a dom tree, check if `EditableAttributeValues::featured` is true:
- If `featured` is true, `(ElementBoundary|RenderingDomContent)::editableAttribute` should copy `EditableAttributeValues`.
- If `featured` is false, `(ElementBoundary|RenderingDomContent)::editableAttribute` should be null.
- In order to make a visible featured composition always exist in the dom tree, `GuiRemoteGraphicsRenderTarget::AfterPushedClipper` is the key place:
  - Add a `Nullable<EditableAttributeValue> GetEditableAttributeFromGenerator` after hit test and cursor.
  - `if (arguments.hitTestResult || arguments.cursor)` will need to test `arguments.editableAttribute`.
- `GuiRemoteEventDomDiffConverter`, `GuiRemoteProtocolDomDiffConverter`, and `GuiRemoteProtocolSchema_FrameOperations.h` might need to be taken care of the make sure rendering commands -> dom -> dom diff -> recovered dom are all doing good.

To verify, add a remote protocol based unit test case, which is a window with multiple text boxes, enable/disable/focus something for a few times, and see if the DOM tree is properly updated:
- It does not easy to know which dom node binds to which control, but since a text box's focus composition is its bounds composition, so it is stable throughout the control's life time.
- You can first focus each one and see if each time it focus to a different dom node, and now you know their connections.
- Begin to perform other actions and see if all dom nodes are properly updated.
- `UnitTestRemoteProtocol::GetLastRenderingFrame()->renderingDom` has a dom, but each frame all dom nodes are different objects, you can tell the connection between frames by `RenderingDom::id`. Be awared that it only has visible nodes, so if a text box is hidden by itself or its any parent node, you can't find it in a dom tree.
- Since there is no actual meaningful change in `GuiRemoteRendererSingle`, so you can skip testing against test app features:
  - Make sure unit test cases cover.
  - Just launch `CppTest`, `CppTest_Metaonly` and `RemotingTest_Rendering_Win32`, find some text box and type something in them, to see if the code does not break, as a smoking test.
  - No need to run any other verification on test apps.

## Task 2

Implement browser text input in GacJS after Task 1. Core remains responsible for document contents, selection, editing commands and committed rendering. GacJS supplies a browser editing host and translates committed browser input into the existing remote protocol events. Apply this in the renderer library so HTTP and Wasm use the same implementation.

### DOM metadata and editing host lifetime

- Import Task 1's generated protocol through `yarn run import` and `yarn codegen` in `<GacJS repo>/Gaclib`. Never hand-edit `<GacJS repo>/Gaclib/gaclib/remote-protocol` or copied snapshot data.
- Carry `RenderingDomContent::editableAttribute` through full-tree construction, every relevant diff, and the recovered virtual DOM. The wire type is `ElementEditableAttribute`, containing `focused` and `editable`; `featured` is represented by a non-null attribute, not a third wire field.
- Keep a registry of visible tagged boundaries keyed by `RenderingDom::id`. This ID is stable while the editing boundary is visible. It identifies the owning editing region even when the text box contains multiple paragraph divs. Do not guess the owner from a paragraph span, caret color, text, CSS class, or nearest arbitrary div.
- The tag indicates focus belonging to that editing region; it does not identify the exact focused descendant. After applying a complete full tree or ordered diff batch, scan the resulting tree, including unchanged nodes, to derive the focused tagged boundary. Count logical DOM IDs once, regardless of clipping wrappers or paragraph descendants.
- **Crash immediately if more than one tagged boundary has `editableAttribute.focused === true`.** Throw an error identifying the conflicting IDs through the ordinary renderer fatal-error path. Check before filtering out read-only boundaries, and do not choose the first/deepest node, log and continue, or fall back to another input implementation. Multiple tagged or editable but unfocused text boxes are valid. Validate the completed batch, so a focus transfer does not fail because of its intermediate diff ordering.
- The active text-input target is the unique focused boundary only when its `editable` flag is also true; otherwise it is null. Read-only/selectable text, buttons and other Core focus targets must not activate browser editing. Missing metadata does not imply editable text.
- Bind the input implementation to the existing HTML div representing that tagged boundary. Preserve both its logical ID and its actual `HTMLElement` identity while visible. Avoid detaching/reinserting it, or an ancestor, merely to refresh children or bounds: retaining the same object after `replaceChildren()` can still lose focus/composition.
- Update `<GacJS repo>/Gaclib/gaclib/renderer/src/dom/virtualDom.ts`, `<GacJS repo>/Gaclib/gaclib/renderer/src/dom/virtualDomBuilding.ts` and `<GacJS repo>/Gaclib/gaclib/renderer/src/domRenderer/virtualDomRenderer.ts` accordingly. Audit clipping changes and root reconciliation, not only updates directly on the editing div.
- Do not add an input, textarea, hidden proxy, wrapper, or replacement editor subtree. Attributes, styles and browser input state can be attached to existing nodes. Core's DOM remains the authoritative rendering structure. Preserve unrelated attributes and restore the input bridge's changes when a host stops being eligible.
- Treat removal, hiding through an ancestor, loss of metadata/editability, renderer shutdown and connection replacement as ending the input session. Use a local session generation as well as the DOM ID to reject late composition/input callbacks, including after the same ID becomes visible again. A local generation is not a new protocol field.

### Shared input and focus handling

Implement one coordinator in the renderer, with EditContext and contenteditable adapters. Keep input state separate from paragraph layout and transport state; tests must be able to select an adapter without changing browser globals.

- Funnel committed text through one `OnIOChar` helper. Each event's `code` is one UTF-16 code unit; send surrogate pairs in order with consistent modifier flags. Do not truncate multi-character commits or send an entire string in one `code` field. This preserves the existing Core `OnIOChar` path, including conversion to UTF-32 on platforms that need it.
- While an adapter is active, remove the printable-character-to-`IOChar` path from `keydown`. Let browser text input run instead of unconditionally preventing its default. Keep ordinary Core key events, shortcuts, navigation and the legacy character path when no browser text adapter is active. Preserve reserved browser shortcuts and clear pressed-key state on focus loss.
- During composition, IME-owned keys such as candidate navigation, Enter and Escape must reach the IME without also editing/navigating Core. Account for `isComposing`, composition lifecycle state and legacy IME key codes. Balance forwarded key-down/key-up state when composition or focus changes.
- Handle browser editing intentions that arrive without keyboard events. Map backward/forward deletion and line/paragraph breaks to the existing Core key/character sequences; preserve Enter, Tab, navigation and shortcut behavior. Deduplicate a physical-key command and its corresponding `beforeinput`, and distinguish IME candidate acceptance from inserting a newline. Test soft-keyboard deletion at an empty browser capture buffer as well as after a commit.
- Keep preedit text browser-local. Track replacement ranges and the current composition buffer; updates such as `n` -> `ni` -> a chosen character must commit only the final result. An ordinary insertion also commits once. Normalize the different orders of `compositionend`, `beforeinput` and `input` with transaction state; do not deduplicate by text equality or a time window, since consecutive identical commits are legitimate.
- Commit at Core's current selection, allowing Core's existing character handling to replace selected text. The browser capture selection must not clear or modify Core's selection. Cover both a collapsed caret and a selected range, including forward/reversed and multi-paragraph selections. Cancellation must leave Core text and selection unchanged.
- Task 1 does not add a complete document/selection editing protocol. Do not infer selection endpoints from highlight colors or claim arbitrary reconversion/autocorrection ranges can be applied with `IOChar`. Start with insertion/replacement at Core's current selection and explicit editing commands. Disable optional browser spelling/autocorrection where possible; reject and reconcile an unmappable replacement rather than append it at the wrong place. Keep this limitation documented and test that it cannot corrupt Core text.
- Invalidate old composition ownership before processing a confirmed Core focus change. Never deliver an old host's delayed commit to the newly focused control. If an incoming Core edit invalidates the composition anchor, end that local session and reconcile from Core instead of applying stale offsets. The existing protocol has no input target ID: local bookkeeping cannot solve an unobserved Core focus change, so preserve transport ordering and test delayed focus updates explicitly.

Prepare each visible editable tagged host before a user gesture, even if it is not the currently focused Core region. On a genuine tap/click in such a region, use the existing DOM/hit-test mapping to focus that host synchronously within the trusted event handler. Send the ordinary pointer interaction to Core and reconcile with the next authoritative focus metadata. Do not await a remote rendering round trip before first making the browser host editable and focusing it. While confirmation is pending, the candidate adapter owns browser text input and suppresses legacy character synthesis, but holds text commits. Release them only if Core confirms the candidate; otherwise discard that session. Confirm against completed Core rendering/idle synchronization, including unchanged focus state, rather than waiting indefinitely for a focus-changing diff. A direct tap on an already-confirmed target needs no extra confirmation frame.

Integrate this with GacJS's existing touch ownership and compatibility-mouse suppression. A scroll, drag, long press or canceled contact must not be converted into a typing tap. Do not depend on a synthetic click that the touch adapter suppresses. Keep keyboard shortcuts that move focus working, and prevent website-level `screen.focus()` handlers from stealing focus back from an editing boundary.

When Core changes focus independently, synchronize the browser host and caret as far as browser policy allows. **Core focus metadata cannot guarantee that a phone's keyboard opens automatically.** In particular, an asynchronous Core message is not a new trusted user gesture. Retain the desired target so a subsequent user tap can activate it; do not synthesize activation or repeatedly blur/refocus to force a keyboard. WebKit's [programmatic-focus issue](https://bugs.webkit.org/show_bug.cgi?id=195884) distinguishes webpage behavior from native WKWebView embedding privileges.

### EditContext adapter

Use EditContext as the preferred adapter. It provides input independently of browser DOM editing; GacJS continues rendering Core's tree. Follow the [EditContext draft](https://www.w3.org/TR/edit-context/) and [Chrome implementation guidance](https://developer.chrome.com/blog/introducing-editcontext-api).

- Detect `typeof globalThis.EditContext === 'function'` and `'editContext' in HTMLElement.prototype`, then verify the methods used by the adapter on the constructed instance. Attach it to the tagged host, make that host focusable as needed, and avoid enabling contenteditable editing on the same host at the same time.
- Use `textupdate`, `compositionstart` and `compositionend` to maintain a local input buffer. Respect `updateRangeStart`, `updateRangeEnd` and selection offsets; `event.text` is a replacement, not necessarily newly appended text. Keep preedit changes out of `IOChar`, then pass the completed insertion through the shared commit helper exactly once.
- Use a capture buffer anchored at Core's caret/current selection, not a fictitious mirror of the complete document. Reset/rebase it only outside active composition, after consuming the relevant browser transaction. Verify a usable deletion intention is still received when the buffer has no committed text; any guard text needed inside EditContext stays in its JavaScript buffer and never enters Core or the rendered document.
- Provide control and selection bounds using the existing tagged host and Core caret information. Handle `characterboundsupdate` with `updateCharacterBounds`, and `textformatupdate` for composition presentation. Use existing nodes/styles or pseudo-elements for local preedit presentation; do not add a second editor subtree. Clear that presentation on completion/cancellation and never measure it as committed paragraph content.
- Use paragraph layout/caret data and `WindowNotifySetCaret` as appropriate; do not treat the mere presence of a caret as proof that a control is editable. Convert coordinates to browser client CSS pixels and update them after scrolling, wrapping, resize, DPI changes and viewport movement. Bounds for local preedit belong to its capture offsets, not unrelated offsets in another paragraph.
- Detach listeners and the EditContext on session teardown. Do not retain an active context attached to a removed host or let a late event revive it.

### contenteditable adapter

Use the same tagged boundary when EditContext is unavailable. This is the fallback for the tested WebKit and Firefox engines; it must support one owning host containing several paragraph divs.

- Detect `'contentEditable' in host`. Prefer `contenteditable="plaintext-only"` when assigning that value is accepted and reflected; otherwise use `contenteditable="true"` with the same interception logic. Check `'onbeforeinput' in host` and `typeof InputEvent === 'function'`, but treat API presence as preliminary: the browser tests must establish event behavior. If `beforeinput` is unavailable, an `input`/composition reconciliation route must pass the same tests before it counts as a usable adapter; otherwise retain the legacy route with its documented limitation.
- Establish a browser selection inside the appropriate existing paragraph using Core's caret position and the paragraph text-node mapping. Keep it separate from Core's potentially nonempty selection. Include empty paragraphs, inline objects and line boundaries; do not make an arbitrary leaf span the long-lived editing host. Suppress duplicate native caret painting while retaining the browser's editable selection.
- Listen to `beforeinput`, `input`, `compositionstart`, `compositionupdate` and `compositionend`. For cancelable ordinary edits, prevent the browser mutation and translate the intent into Core input. Treat plaintext paste as one text transaction and prevent rich HTML/drop formatting from changing the rendered tree. Do not emit both pasted text and a second clipboard command for the same action.
- **Do not rely on `preventDefault()` to stop IME edits.** Composition-related input can be noncancelable, as described by [Input Events Level 2](https://www.w3.org/TR/input-events-2/). Read the final transaction from the event/range state and, when necessary, the browser-mutated text; keep it out of Core until committed.
- Track the composition's text nodes/range and distinguish browser mutations from renderer mutations. Protect the active composition from unrelated full paragraph reconstruction, caret blinking and measurement work. Reconcile a conflicting Core content update by ending the affected session before applying it; do not leave a protocol response waiting for the user to finish composing.
- Restore browser-edited content from the latest Core-owned paragraph state after the transaction, without replacing the tagged host. Preserve/recover the browser selection against the current nodes. Never restore a stale `innerHTML` snapshot over a newer Core frame, clone the whole editor, or let browser normalization become the next Core document state.
- The browser may temporarily insert/remove/split nodes during a noncancelable composition, especially in an empty paragraph. This fallback cannot promise zero transient DOM mutations. Constrain and reconcile those changes so the resulting element structure is exactly Core's, with no persistent proxy or extra editing nodes. Explicitly test this limitation; a case requiring a permanent structural change is a failing fallback case, not permission to change the design.
- Audit `<GacJS repo>/Gaclib/gaclib/renderer/src/domRenderer/elementStyles_DocumentParagraph.ts`, particularly `initializeParagraph()` and its `replaceChildren()` calls. Continue measuring authoritative text, preserve active host identity, and verify all renderer-owned caret, inline-object and paragraph references after reconciliation.

### Virtual keyboard control and capability fallback

`navigator.virtualKeyboard` is an optional keyboard visibility/geometry facility; it is not a text-input adapter and does not make an ordinary div editable. Use it alongside either adapter only when the browser supports the necessary operations. Follow the [VirtualKeyboard draft](https://w3c.github.io/virtual-keyboard/).

- Detect a secure context, `navigator.virtualKeyboard`, callable `show`/`hide`, and the host's `virtualKeyboardPolicy` support. Detect geometry support independently. A present object or a successful void `show()` return does not prove that a keyboard opened.
- Keep native automatic keyboard behavior as the default. Support explicit control on eligible hosts using `virtualKeyboardPolicy="manual"` and a non-`none` input mode, focusing the host and calling `show()` from an eligible user gesture. Enable manual policy only for a supported configuration; do not suppress automatic display in an unsupported fallback. Verify EditContext-host eligibility instead of assuming every exposed API combination works together.
- The API requires user activation and permits additional platform restrictions; it is not a Safari activation bypass. When it is absent or explicit control is unusable, retain ordinary editable-host focus behavior.
- Observe `geometrychange`/`boundingRect` where available. Keep normal viewport resizing by default; do not globally set `overlaysContent=true` unless implementing and testing the corresponding layout ownership. Otherwise use the existing resize path and feature-detected `visualViewport` changes to keep the caret visible and update IME bounds without moving/detaching its host or applying a keyboard inset twice.
- On loss of an active editable target, end GacJS keyboard ownership. Call `hide()` only when GacJS owns an eligible manual-policy session, before discarding that ownership; do not dismiss a keyboard belonging to another input on the page.

Choose capabilities at runtime, without browser-name/version checks:

| Available capabilities | Input route | Keyboard route |
| --- | --- | --- |
| Working EditContext | EditContext on the tagged div | Native focus; optional VirtualKeyboard control |
| No working EditContext, usable contenteditable | contenteditable on the same tagged div | Native focus; optional VirtualKeyboard control |
| Neither adapter usable | Existing physical-keyboard handling | Report that IME/software input is unavailable |
| VirtualKeyboard alone | Does not change the input route | Cannot turn a noneditable div into a text target |

Limit any capability fallback to recognized unsupported construction/attachment cases. Unexpected exceptions and invalid protocol state must remain fatal, not become silent fallback. Never replay a partly committed transaction through another adapter. Expose the chosen adapter/capabilities to test diagnostics and provide a test seam to force fallback paths; do not require users to choose a browser mode.

### Automatic verification during implementation

These are instructions for the implementing agent. Run the checks, inspect their results and fix failures; do not hand them to the user as a manual checklist. Keep synthetic input, browser-engine input and actual OS IME/keyboard evidence distinct.

The earlier investigation reproduced the missing browser input target in Chromium with an iPhone-sized touch context and in Playwright WebKit: Core displayed a caret while the browser focused a noneditable div. Physical-key-style input worked, but `page.keyboard.insertText()` did not type into Core. A temporary EditContext experiment accepted Unicode and a Chromium CDP composition without changing the rendered structure. contenteditable probes in Chromium/Firefox demonstrated noncancelable composition mutations. These are preliminary experiments, not verification of the eventual implementation.

#### Harness and execution

1. After Task 1's protocol generation, run `yarn run import`, `yarn codegen`, `yarn build` and `yarn test` from `<GacJS repo>/Gaclib`, in that order. Follow repository build wrappers for native Core; do not call MSBuild directly. Do not edit generated protocol/snapshot output to make tests pass.
2. Add controlled browser tests under `<GacJS repo>/Gaclib/website/entry/test`, following `<GacJS repo>/Gaclib/website/entry/test/RawBrowser.test.ts`: serve the built renderer, drive real full/diff protocol messages, observe emitted events and rendered results, and use Playwright Chromium, WebKit and Firefox. Add these cases to the existing test scripts so future `yarn test` runs them. Install missing engines with `npx playwright install chromium webkit firefox` from `<GacJS repo>/Gaclib`.
3. Add focused live-Core coverage alongside `<GacJS repo>/Gaclib/website/entry/test/Testing_Protocol_SimpleTyping.js` and the caret suites. Run `RemotingTest_Core /FCT /Http` through the existing lifecycle harness. Exercise `Control / Document Editor (Ribbon)`'s Search text box and central editor, plus the Toolstrip editor for the multiple-paragraph/mode cases. Assert Core's resulting text as well as the browser view: an uncommitted contenteditable mutation alone must not pass.
4. Exercise the same renderer through Wasm with freshly generated matching artifacts when that build environment is available, following `<GacJS repo>/doc/Testing_Protocol.md`. Do not use an older public demo to claim Task 1 metadata works. Remember that building the website clears deployed assets; stage Wasm artifacts after the build.
5. Synchronize with renderer idle/caret callbacks, protocol completion and explicit event predicates. Use bounded failure timeouts, not fixed sleeps. Capture browser version/capabilities, adapter, active DOM ID/element, composition event order, emitted input and final text on failure. Check page errors and the existing fatal error mask.
6. When paragraph construction changes, verify the `InlineObjectWithCaret` snapshot through the existing snapshot viewer, together with the live editing checks. Snapshot playback must remain passive and must not focus an editing host or show a keyboard merely because a recorded node contains focus metadata.

#### Required assertions

- Full tree and diffs: multiple editable regions, no focused region, read-only focused region, focus transfer in either diff order, unchanged tagged nodes, metadata removal, subtree deletion, hidden ancestors and reappearance. Inject two focused tags in both full and incremental updates and require a fatal error containing both IDs, including when one is read-only. There must be no continued input or fallback after the error.
- Host lifetime: retain the actual active `HTMLElement` through unrelated diffs, paragraph updates, clipping, root bounds changes and caret blinks. Track node identity, focus, composition state and mutations. Require no additional proxy nodes and the expected Core element structure after each completed transaction.
- Input: Latin text, CJK, combining sequences, supplementary emoji and consecutive identical commits. Use `page.keyboard.insertText()` so success cannot depend on a printable `keydown`. Check exact ordered UTF-16 `IOChar` payloads and exactly-once final Core text. Test rapid commits before a rendering response and modifiers/shortcuts separately.
- Composition: in Chromium, use CDP `Input.imeSetComposition` to revise preedit and `Input.insertText` to commit; test cancellation as well. Require no preedit `IOChar`, one final commit and no newline from candidate acceptance. Exercise both the real EditContext path and forced contenteditable fallback. Label this browser-engine composition coverage, not an actual OS candidate-window test.
- Selection: repeat ordinary insertion and IME commit with no selection and with selected text in each editor. Require insertion at a collapsed caret and replacement of precisely the selected range. Include forward/reversed selections, a selection spanning paragraphs and cancellation that preserves the original selection/text. Browser selection setup must not change Core's selection first.
- Noncancelable fallback: real engine input must survive noncancelable events, final-input ordering differences, empty paragraphs and browser text-node changes. Use additional synthetic event-order tests for sequences unavailable through an engine's automation API, recording that distinction. Verify final Core rendering and structural reconciliation; dispatching a synthetic `CompositionEvent` alone does not establish that native IME works.
- Editing commands: soft deletion/Enter without `keydown`, physical deletion/Enter with matching input events, Tab, navigation, shortcut undo/redo, selection replacement, rejected range corrections and plaintext paste. Require one Core operation per action and no browser-created persistent paragraph/formatting nodes.
- Focus: genuine tap acquisition before a delayed Core response, rejection/redirect by Core, Core-driven focus switches without gestures, a switch during composition, hide/disable during composition, disconnect and late events. Verify pending text cannot go to the wrong confirmed region and page-level focus handlers do not steal the editing host. Include drag/scroll/long-press regressions.
- Keyboard API: test absent capabilities and an injected VirtualKeyboard test double for manual-policy calls, ownership, viewport/geometry changes and cleanup. Separately record whether the real API is exposed and callable. A test double or a zero-size headless keyboard rectangle is not evidence of physical keyboard display.

#### Browser expectations and verification limits

| Environment | Expected implementation and evidence |
| --- | --- |
| Playwright Chromium | Prefer EditContext when detected; force contenteditable in another run. Native engine text insertion and CDP composition can be verified. Record VirtualKeyboard availability independently. |
| Playwright WebKit | The tested engine had no EditContext or VirtualKeyboard, so exercise contenteditable and native engine text insertion. An iPhone device descriptor does not run iOS or its keyboard. |
| Playwright Firefox | The tested engine had no EditContext or VirtualKeyboard, so exercise contenteditable. Use Firefox-supported desktop/touch context settings; do not assume Chromium CDP or iPhone emulation support. If a newer engine exposes EditContext, test it as well as forced fallback. |
| Actual iPhone Safari / WebKit-based iPhone browsers | Expect the contenteditable path until capability detection says otherwise. A direct editing tap is the keyboard-opening path to verify; an arbitrary asynchronous Core focus change cannot promise keyboard display. |
| Actual Chromium/Firefox on Android, or desktop with an OS IME | Use detected capabilities and the platform's real keyboard/IME. Verify candidate selection, cancellation and resulting Core text; do not infer success from desktop viewport emulation. |

The preliminary installed engines were Chromium 145, WebKit 26 and Firefox 146.0.1; record the versions actually used by the implementation tests rather than asserting these versions or permanent support tables. WebKit's [EditContext implementation request](https://bugs.webkit.org/show_bug.cgi?id=269922) remains relevant background, not a substitute for runtime detection.

Headless browser tests can prove input events, focus, protocol delivery and reconciliation. They cannot prove that an iPhone software keyboard or an OS IME candidate window appeared. Attempt real OS IME verification in an available interactive browser session using the applicable automation/computer-use guidance. If the desktop is locked or no such device/session is available, record that specific coverage as unavailable, retain the automated results, and do not call simulated composition a physical IME pass. This limitation must not prevent running the other checks.

### Documentation and SOP updates

Update these documents with the implemented behavior and actual automated-test entry points:

- `<GacJS repo>/doc/Protocol.md`: nullable editing metadata, stable boundary identity, full/diff focus derivation, the fatal uniqueness invariant, and committed UTF-16 `IOChar` delivery versus composition-local state.
- `<GacJS repo>/doc/DOM.md`: append a final section titled exactly `## Responding to IME and Virtual Keyboard` and add it to the table of contents. Document host selection/lifetime, Core/browser focus synchronization, both input adapters, touch activation, VirtualKeyboard integration and mobile activation limits. Explain runtime capability detection and the fallback order from EditContext to contenteditable to legacy physical-keyboard handling, with VirtualKeyboard detected independently. Include typing-feature testing: how to exercise each adapter and force fallback, expected browser behavior, IME insertion with no selection and replacement with selected text, cancellation, and the limits of simulated input versus actual OS IME/keyboard verification. Link to `<GacJS repo>/doc/Testing_Protocol.md` for test entry points and `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md` for the authoritative feature operations. Also revise the existing keyboard mapping and root reconciliation descriptions.
- `<GacJS repo>/doc/DocumentParagraph.md`: browser-selection/caret mapping, active-composition handling during paragraph updates, IME bounds, local preedit presentation and reconciliation/measurement rules for contenteditable.
- `<GacJS repo>/doc/Testing_Protocol.md`: browser installation/run commands, controlled and live suites, capability diagnostics, forced-adapter coverage and the distinction between synthetic events, engine composition and actual OS IME/keyboard verification. Link to the feature-operation SOP rather than duplicating it.
- `<GacJS repo>/doc/Testing_Snapshot.md`: inspection of editing metadata and passive playback, including the paragraph/caret regression checks. Update `<GacJS repo>/doc/Projects.md` only if the implementation changes package or script organization.
- **Update `### 3. Type and Preserve Text in Two Editors` in `<GacUI repo>/.github/Jobs/DebugRemoteProtocolSop.md`.** When verifying GacJS, explicitly activate an actual IME and compose/choose/commit text in both the Search box and the central editor. Run each with no selected text and with existing text selected; require insertion at the caret or exact replacement of the selected range, respectively, with no missing, duplicated or reordered characters. Also cancel a composition and require unchanged text/selection. Switch to `List` and back and require both committed results to persist. Record the IME/browser used and any unavailable native coverage; direct Unicode insertion or synthetic composition alone does not satisfy this SOP step. Keep the matching live test operations synchronized with this requirement.
