# !!!TASK!!!

# PROBLEM DESCRIPTION

## TASK No.9: Run smoke tests for more editor text boxes

Extend the existing editor smoke tests in #file:TestControls_Editor.cpp so the same `RunTextBoxSmokeTest<TTextBox>(resource, controlName)` flow (Basic + Typing) runs against the other `GuiDocumentCommonInterface`-based editor controls.

This task is only about expanding the smoke-test coverage (reusing the existing shared test-case helper) and preparing the XML resources for each control type so failures can be compared via consistent log paths.

### what to be done

- In #file:TestControls_Editor.cpp, add additional resource XML strings (or a single combined resource that can host one control at a time) and add outer `TEST_CATEGORY` blocks to call `RunTextBoxSmokeTest` for:
	- `<MultilineTextBox/>` mapped to `GuiMultilineTextBox`.
	- `<DocumentTextBox/>` mapped to `GuiDocumentLabel`.
	- `<DocumentLabel/>` mapped to `GuiDocumentLabel`.
	- `<DocumentViewer/>` mapped to `GuiDocumetViewer`.
- Follow the latest frame learnings (from TASK No.8 / Typing) when adding new smoke-test invocations: merge focus + input when possible; avoid adding `window->SetText(...)` just to force a render; if a step doesn’t naturally trigger a render, merge it into a frame that performs real input (or restructure so each intermediate frame does something that will change UI); keep the last frame hide-only when practical, but merge it when it avoids artificial UI changes.
- Ensure the last 3 types have `EditMode="Editable"` set in the XML so the existing typing-based smoke tests are meaningful.
- Keep log paths consistent with the existing pattern in `RunTextBoxSmokeTest`, so the output folders differentiate control types cleanly (e.g. `Controls/Editor/<ControlName>/Basic` and `.../Typing`).

### rationale

- These editor controls share a large portion of `GuiDocumentCommonInterface` behavior; running the same smoke tests across them quickly detects regressions in shared code paths.
- Doing this after the key-test categories are planned keeps scope contained: it reuses a proven smoke-test helper and adds only the minimum XML/control scaffolding needed.

# UPDATES

## UPDATE

In the 4 mentioned text box controls, only <DocumentTextBox> is a singleline text box, others are all multiline text boxes. That means, you must set bottom:5 instead of -1 so that it expands and fill the whole window. Therefore you should change the clicking position in the test case, to click (0.0, 0.0, 2, 8) so that it hit the first line.

# INSIGHTS AND REASONING

## What already exists (baseline)

- The smoke tests live in #file:TestControls_Editor.cpp and are already factored as a reusable template function `RunTextBoxSmokeTest<TTextBox>(resource, controlName)`.
	- It is constrained to `TTextBox` being both `GuiControl` and `GuiDocumentCommonInterface`, so it is intended to cover the “document-based editor controls” family.
	- It currently runs for `GuiSinglelineTextBox` only, with log output rooted at `Controls/Editor/<ControlName>/{Basic|Typing}`.
- The test harness is explicitly frame-based via `UnitTestRemoteProtocol::OnNextIdleFrame`. The knowledge base notes that each `OnNextIdleFrame` represents one rendering frame and logs UI state frame-by-frame, so test structure needs to respect “frame causes next-frame changes” patterns (#file:.github/KnowledgeBase/KB_GacUI_Design_PlatformInitialization.md).

## Target controls and how they map

The four “other” controls requested by this task all derive from (or are virtual types backed by) `GuiDocumentCommonInterface`, so they should be compatible with the existing smoke test helper:

- `GuiMultilineTextBox` is a concrete control type (declared alongside the single-line textbox and document label/viewer) in #file:Source/Controls/TextEditorPackage/GuiDocumentViewer.h.
- `GuiDocumentViewer` is a concrete scrollable viewer control in #file:Source/Controls/TextEditorPackage/GuiDocumentViewer.h.
- `GuiDocumentLabel` is a concrete non-scroll-container label control in #file:Source/Controls/TextEditorPackage/GuiDocumentViewer.h.
- `GuiDocumentTextBox` is a *virtual type* registered by the XML instance loader: it is created as a `GuiDocumentLabel`-backed control but with a different theme (`theme::ThemeName::DocumentTextBox`) in #file:Source/Compiler/InstanceLoaders/GuiInstanceLoader_Document.cpp.

Design implication: for `<DocumentTextBox/>` and `<DocumentLabel/>`, the runtime object is compatible with `FindObjectByName<GuiDocumentLabel>(...)`, so the smoke test should be invoked as `RunTextBoxSmokeTest<GuiDocumentLabel>(...)` while varying only the XML tag/template.

## Proposed design (high-level)

### 1) Keep `RunTextBoxSmokeTest` unchanged and reuse it

The helper already expresses the smoke-test intent (Basic + Typing) and already sets up deterministic time via `TooltipTimer` per test. The cleanest design is to reuse it as-is and expand coverage by adding more “outer categories” in the same file.

Benefits:
- Minimal change surface (low risk) while increasing regression coverage.
- Consistent log-path behavior is guaranteed because the helper owns the path formatting.

Update note (from latest update): because 3 of the 4 target controls are multi-line (everything except `<DocumentTextBox/>`), the click used to focus the editor should be adjusted to target the first text line deterministically (click near the top-left of the content, e.g. `(ratioX=0.0, ratioY=0.0, offsetX=2, offsetY=8)`), instead of relying on a vertically-centered click which may land in empty space.

### 2) Add one resource XML per control type (preferred)

Each control needs a distinct root tag (`<MultilineTextBox/>`, `<DocumentTextBox/>`, `<DocumentLabel/>`, `<DocumentViewer/>`). A per-control XML string keeps the tests explicit, avoids runtime switching logic, and keeps future diffs localized.

Shared invariants across these XMLs:
- The tested control uses `ref.Name="textBox"` so the helper’s `FindObjectByName<TTextBox>(..., L"textBox")` continues to work.
- Bounds are anchored to parent so mouse location helpers (`protocol->LocationOf(...)`) remain meaningful.
	- For multi-line controls (`<MultilineTextBox/>`, `<DocumentLabel/>`, `<DocumentViewer/>`), set `AlignmentToParent` with a non-negative bottom margin (e.g. `bottom:5` instead of `-1`) so the editor expands and fills the window.
	- For the single-line `<DocumentTextBox/>`, keep the single-line layout (it does not need to fill the whole window).

### 3) Ensure editability where needed

`GuiDocumentCommonInterface` maintains an `editMode` field and exposes `GetEditMode()/SetEditMode(...)` in #file:Source/Controls/TextEditorPackage/GuiDocumentCommonInterface.h. For controls that are view-only by default, the XML should set `EditMode="Editable"` so the existing Typing smoke test is meaningful.

Per the task requirement:
- Keep `<MultilineTextBox/>` as-is (it is a textbox control).
- Set `EditMode="Editable"` on `<DocumentTextBox/>`, `<DocumentLabel/>`, and `<DocumentViewer/>`.

### 4) Add one `TEST_CATEGORY` per control type to keep logs separated

Add outer categories in `TEST_FILE` (still in #file:TestControls_Editor.cpp), each invoking the smoke helper with:
- the right `TTextBox` type parameter
- the matching resource XML string
- a `controlName` string that becomes the folder segment (e.g. `GuiMultilineTextBox`, `GuiDocumentTextBox`, `GuiDocumentLabel`, `GuiDocumentViewer`)

This preserves the “compare failures via consistent folder structure” requirement.

## Risks / open questions

- The task text mentions `GuiDocumetViewer` (typo) as the mapping target. Source code defines `GuiDocumentViewer` (#file:Source/Controls/TextEditorPackage/GuiDocumentViewer.h). The design should use the correct type name in code and use `GuiDocumentViewer` as the `controlName` to match the class.
- `RunTextBoxSmokeTest` includes a mouse click to focus the editor before typing. For multi-line editors that fill the window, a top-left click with a small offset (e.g. `(0.0, 0.0, 2, 8)`) is expected to be more stable (targets the first line) than a centered click (which can land on empty space depending on view state, padding, and scroll position).

# !!!FINISHED!!!
