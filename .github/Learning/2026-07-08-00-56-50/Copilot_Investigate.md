# !!!INVESTIGATE!!!

# PROBLEM DESCRIPTION

- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

Cursor on text box scroll bars are still I-beam.
Does not repro in document box, could be darkskin issue, not a control issue.
To repro this issue, you could use `Playground` with the automation service, if the automation service is not turned on, you can also fix it.

## REVIEW COMMENTS

### Clarify Expected Cursor And Scope

**review comment**: The task says the cursor on text box scroll bars is "still I-beam", but it does not explicitly say what cursor is expected. Please clarify whether every scrollbar part in the affected text box should force `Arrow` (buttons, track, handle, and empty corner) while only the editable document area keeps `IBeam`. Also clarify the affected control scope: in DarkSkin, `MultilineTextBox` and `DocumentViewer` both use `darkskin::DocumentViewerTemplate`, while `SinglelineTextBox` and `DocumentTextBox` use `darkskin::DocumentTextBoxTemplate`. This matters because a template-level fix may change `DocumentViewer` too, despite the note saying the issue does not repro in document box.

The document box behavior is correct, only when the cursor is on where can type it could be a I-beam. Scroll bars are for grabbing so there should use default cursor.

### Require A Regression Check For Cursor Metadata

**review comment**: The task gives a manual repro path through Playground, but does not state how the fix should be verified beyond running unit tests. Cursor state is already represented in the remote rendering DOM and there are existing cursor tests around compositions and graphics host hit testing, so please specify whether the execution should add a focused unit/snapshot/DOM assertion for the DarkSkin text-box scrollbar cursor, or whether manual Playground automation verification is intentionally enough. If the fix changes DarkSkin XML under `Test/Resources/App/DarkSkin`, the execution should also run `GacUI_Compiler` and inspect generated output before running the required unit test.

Put a text box and document box on Playground with scroll bar enabled, and you can run the app, use the automation service to read the UI. It should directly tell you what cursor should be on a composition. If you can't see it, check the source code of automation service and add it if missing. To implement this, you can omit if it is not set in a composition to reduce the JSON size, as cursors are inherited, if a composition does not specify one, it uses the one fron its direct or indirect parent. If the whole "report line" does not specify, it means default. Not specifying means `nullptr`, and when it is not null, you can read the enum. The enum item name will be what you need to write to the JSON in automationn service.

# UPDATES

## UPDATE

your fix have been committed, but it added a new skin. The buggy skin is only used by GuiMultilineTextBox, which means you should just fix the original one instead of adding a new one.

## UPDATE

I have reorganize your commits a little bit to revert just a few unrelated changes. I wonder is it possible to only fix Template_Document.xml, so that you don't need to touch GuiSharedAutomationService_Controls.cpp and GuiDocumentCommonInterface.cpp. And by saying only fix Template_Document.xml, I would like you to explore a fix that only uses the XML description part, instead of inserting Workflow script directly. I believe it should be doable.

## UPDATE

two work items to finish

1. The original fix has a problem. It looks too much like a patch. But the idea way is that, scroll bar's container should not be assigned with the IBeam cursor, it is the content to be assigned with the IBeam cursor, by implementing it in this way, the fix looks correct and the bug is fixed at the root cause. If you don't think the issue is in the skin, you might want to take a look at the control. But given that document skins work just right, so there might be some other issues that are not discovered. I would like to see a fix really hit on the root cause.

2. You didn't perform the test on Playground, you should do  it.

# TEST [CONFIRMED]

Add a DarkSkin regression test for a `MultilineTextBox` and a `DocumentViewer` with horizontal and vertical scroll bars forced visible.

The test verifies:
- the multiline text box document area resolves to `IBeam`;
- the multiline text box bounds has no explicit cursor, and its horizontal scroll bar, vertical scroll bar, and scroll corner resolve to the composition-level default cursor (`nullptr`);
- the document viewer document area and scroll bars do not resolve to `IBeam`, preserving the existing correct document-box scope.

This is expected to fail before the root-cause fix because `GuiMultilineTextBox` sets `IBeam` before the template is installed, leaving the temporary control bounds cursor inherited by the DarkSkin scroll area. After the fix, `IBeam` is cleared from the temporary area and reapplied only to the real document content mouse area.

Confirmed by building `Test/GacUISrc/GacUISrc.sln` with `copilotBuild.ps1` and running `UnitTest` with `copilotExecute.ps1`. The new `ScrollableTextBoxCursor` test fails on the current generated DarkSkin output at `cursor == expected`, proving the text box scroll area resolves to the inherited `IBeam` instead of the default cursor.

# PROPOSALS

- No.1 Text-box-specific DarkSkin cursor ownership [DENIED]
- No.2 Original DarkSkin document viewer template cursor ownership [DENIED]
- No.3 Declarative scrollbar cursor ownership in the original template [DENIED]
- No.4 Move document edit cursor with the content mouse area [CONFIRMED]

## No.1 Text-box-specific DarkSkin cursor ownership

Create a new DarkSkin template resource for `MultilineTextBox` instead of sharing `DocumentViewerTemplate`. Keep `DocumentViewer` mapped to the existing template so the document-box behavior does not change.

The new `MultilineTextBoxTemplate` should use the same visual tree as `DocumentViewerTemplate`, but it should explicitly set cursor ownership in the text-box skin:
- the scroll-view look resolves to `Arrow`, covering scroll bars and the scroll corner;
- the document container cell resolves to `IBeam`, so only the editable text area keeps the editing cursor.

This keeps the actual behavior fix in the text-box skin, not in the shared scroll-bar skin and not in the document viewer control. Because the Playground automation service is currently missing cursor metadata in its control dump, add explicit composition cursor reporting as verification support: omit the `cursor` JSON field when `AssociatedCursor` is `nullptr`, and write the system cursor enum name when it is set.

### CODE CHANGE

- Added `darkskin::MultilineTextBoxTemplate` in `Test/Resources/App/DarkSkin/Template_Document.xml`.
  - It keeps the same document-viewer visual tree.
  - It assigns `Arrow` to the `CommonScrollViewLook`, so scroll bars and the scroll corner stop inheriting the control bounds `IBeam`.
  - It assigns `IBeam` to the document container cell, so the editable document area keeps the text cursor.
- Changed `Test/Resources/App/DarkSkin/Index.xml` so only `MultilineTextBox` uses `darkskin::MultilineTextBoxTemplate`; `DocumentViewer` continues to use `darkskin::DocumentViewerTemplate`.
- Added `cursor` output to `AutomationService::DumpComposition` when a composition has an explicit `AssociatedCursor`, using the system cursor enum name and omitting the field when the associated cursor is `nullptr`.
- Made `GacUI_Compiler` unload an already-registered resource by name before loading the freshly compiled resource, so the full regeneration pass can run when generated resource-loader plugins have preloaded the same resource name.

### DENIED BY USER

The proposal fixes the cursor behavior, but it adds a new `darkskin::MultilineTextBoxTemplate` skin whose only purpose is to duplicate `darkskin::DocumentViewerTemplate` and add cursor assignments. The user rejected this shape because the buggy DarkSkin template entry is only used by `GuiMultilineTextBox` in the affected path, so the fix should update the existing original template resource instead of introducing a parallel skin class.

## No.2 Original DarkSkin document viewer template cursor ownership

Remove the added `darkskin::MultilineTextBoxTemplate` resource and map `MultilineTextBox` back to `darkskin::DocumentViewerTemplate`. Add the cursor ownership assignment to the original `DocumentViewerTemplateResource` visual tree so the `CommonScrollViewLook` resolves to `Arrow`, covering scroll bars and the scroll corner.

Keep the editable document-area `IBeam` in the control-side cursor lifecycle: when `GuiDocumentCommonInterface` replaces the mouse area after a template install, clear the cursor from the old mouse area and reapply the cursor implied by the current edit mode to the new mouse area. This preserves the existing view-only `DocumentViewer` scope while allowing the original DarkSkin template to stop scrollbar inheritance from leaking the multiline text-box `IBeam`.

Regenerate DarkSkin generated sources with `GacUI_Compiler`, then rebuild and run the unit test.

### CODE CHANGE

- Remove `darkskin::MultilineTextBoxTemplate` from `Test/Resources/App/DarkSkin/Template_Document.xml`.
- Change `Test/Resources/App/DarkSkin/Index.xml` so `MultilineTextBox` uses `darkskin::DocumentViewerTemplate` again.
- Add `Arrow` to the original `DocumentViewerTemplateResource` `CommonScrollViewLook`.
- Update `GuiDocumentCommonInterface::ReplaceMouseArea` so edit-mode cursor state is moved from the old mouse area to the new mouse area when templates are installed or uninstalled.
- Regenerated DarkSkin generated C++ and binary resources.
- Updated unit-test snapshots for the cursor metadata changes produced by the original template and mouse-area cursor ownership.

### DENIED BY USER

`GacUI_Compiler` succeeds after rebuilding the stale compiler executable and regenerates DarkSkin without leaving any `*.UI.error.txt` or `*.UI.errors.txt` files.

`copilotBuild.ps1` succeeds after regeneration:
- 0 warnings;
- 0 errors.

`copilotExecute.ps1 -Mode UnitTest -Executable UnitTest` succeeds:
- passed test files: 84/84;
- passed test cases: 1687/1687;
- no memory leak output is appended to `Execute.log`.

The regression test confirms that `GuiMultilineTextBox` keeps `IBeam` on the editable document area while its horizontal scroll bar, vertical scroll bar, and scroll corner resolve to `Arrow`. The `DocumentViewer` guard still does not resolve to `IBeam` in view-only document area or scroll bars. The generated DarkSkin output no longer contains `darkskin::MultilineTextBoxTemplate`, so the fix is on the original `darkskin::DocumentViewerTemplate` resource and the control cursor lifecycle instead of a duplicated skin.

The user rejected this shape because it still changes `GuiSharedAutomationService_Controls.cpp` and `GuiDocumentCommonInterface.cpp`, and because the template cursor assignment was implemented through a `ref.Ctor` Workflow script block. The preferred direction is to keep the behavioral fix in the original `Template_Document.xml` description, without a new skin and without control-side cursor lifecycle changes.

## No.3 Declarative scrollbar cursor ownership in the original template

Keep `MultilineTextBox` and `DocumentViewer` mapped to the original `darkskin::DocumentViewerTemplate`. Do not add a new template class and do not use a `ref.Ctor` Workflow script block.

The existing `GuiMultilineTextBox` control already assigns `IBeam` to the control bounds before the template is installed, while `GuiDocumentViewer` does not. The XML fix should therefore avoid assigning any `IBeam` in the shared template. Instead, it should assign `Arrow` cursors only to the parts of the original template that must not inherit the text-box cursor:
- the horizontal scrollbar bounds;
- the vertical scrollbar bounds;
- a 20x20 bottom-right corner overlay that is visible only when both scrollbars are visible.

This keeps the editable document area inheriting the control's existing cursor behavior, while forcing scrollbar parts and the scroll corner to resolve to the default cursor.

### CODE CHANGE

- Remove the `ref.Ctor` script from `Test/Resources/App/DarkSkin/Template_Document.xml`.
- Add XML property assignments in `Template_Document.xml` that set `Arrow` on the horizontal scrollbar bounds, vertical scrollbar bounds, and a conditional bottom-right corner overlay.
- Revert the prior `GuiDocumentCommonInterface.cpp` cursor lifecycle change and the prior `GuiSharedAutomationService_Controls.cpp` cursor JSON output change so the proposal is no longer dependent on C++ changes.

### DENIED BY USER

`GacUI_Compiler` succeeds with the XML-only cursor assignments and leaves no `*.UI.error.txt` or `*.UI.errors.txt` files.

`copilotBuild.ps1` succeeds:
- 0 warnings;
- 0 errors.

`copilotExecute.ps1 -Mode UnitTest -Executable UnitTest` succeeds:
- passed test files: 84/84;
- passed test cases: 1687/1687.

The generated DarkSkin code now sets `Arrow` on the two scrollbar bounds and the conditional bottom-right overlay from XML property expressions. There is no new skin, no `ref.Ctor` block, no `GuiDocumentCommonInterface.cpp` cursor lifecycle dependency, and no `GuiSharedAutomationService_Controls.cpp` cursor dump dependency.

The user rejected this shape because forcing `Arrow` onto the scrollbar containers is still treating the symptom. The root cause should be that the scroll-view container should never inherit `IBeam` in the first place: only the editable text content area should explicitly own `IBeam`, leaving scroll bars to use the default cursor naturally.

## No.4 Move document edit cursor with the content mouse area

Keep `MultilineTextBox` and `DocumentViewer` mapped to the original `darkskin::DocumentViewerTemplate`, and remove the DarkSkin XML cursor assignments added only for the scrollbars and scroll corner.

Fix the control-side root cause in `GuiDocumentCommonInterface`: the current cursor should belong to the document mouse area owned by the content region. During `GuiMultilineTextBox` construction, `SetEditMode(Editable)` runs before the template is installed, so the first document mouse area is the temporary parent under `boundsComposition`. When the real template is installed, `ReplaceMouseArea` should clear the cursor from the old temporary area and reapply the same cursor to the new document mouse area. This makes the content area own `IBeam` and prevents the control bounds or scrollbar container from inheriting it.

The regression test should assert that the text box content area resolves to `IBeam`, the control bounds does not explicitly own `IBeam`, and the scroll bars and scroll corner resolve to the default cursor (`nullptr` at the composition level). The Playground test should run against a resource containing both a multiline text box and a document box with scroll bars visible.

### CODE CHANGE

- Added `documentCursor` to `GuiDocumentCommonInterface` so the control remembers the cursor that belongs to the document interaction area.
- Changed `GuiDocumentCommonInterface::ReplaceMouseArea` to clear the remembered cursor from the old mouse area before detaching mouse handlers, and to reapply the remembered cursor to the new mouse area after template installation.
- Changed `GuiDocumentCommonInterface::UpdateCursor` to update both the remembered cursor and the current mouse area.
- Changed `GuiDocumentCommonInterface::OnMouseLeave` to reset a view-only hyperlink cursor when the mouse leaves the document area.
- Removed the DarkSkin XML `Arrow` cursor assignments from `DocumentViewerTemplateResource`, returning the shared template to the original declarative structure without scrollbar-specific cursor patches.
- Updated the `ScrollableTextBoxCursor` regression test to assert that the text-box bounds has no explicit cursor, the content area resolves to `IBeam`, and the scrollbars plus scroll corner resolve to the composition-level default cursor (`nullptr`).
- Updated Playground to load `ResourceDocument.xml` and force both `DocumentViewer` and `MultilineTextBox` scrollbars visible for automation verification.
- Added explicit `cursor` output to the Playground `/Controls` composition dump, emitted only when a composition has an `AssociatedCursor`, so inherited/default cursor remains omitted.
- Added `ReloadResource` to `GacUI_Compiler` so the full x86/x64 generation pass unloads an already-loaded resource name before loading the freshly generated binary.

### CONFIRMED

`GacUI_Compiler` succeeds after rebuilding the compiler with `ReloadResource` support and leaves no `*.UI.error.txt` or `*.UI.errors.txt` files.

`copilotBuild.ps1` succeeds after regeneration and after the automation-service cursor dump change:
- 0 warnings;
- 0 errors.

`copilotExecute.ps1 -Mode UnitTest -Executable UnitTest` succeeds after the final build:
- passed test files: 84/84;
- passed test cases: 1687/1687;
- no memory leak output is appended to `Execute.log`.

Playground was run from `Test/GacUISrc/Playground` so `ResourceDocument.xml` resolves correctly, and `http://localhost:8888/Automation/Playground/Controls` was captured. The dump confirms:
- the `MultilineTextBox` content composition has `cursor: "IBeam"` at bounds `(403,436)-(767,567)`;
- the `MultilineTextBox` control bounds, `HScroll`, `VScroll`, scrollbar buttons, and scroll corner omit `cursor`, meaning default/inherited `nullptr` rather than an explicit `IBeam`;
- the editable `DocumentViewer` content has `cursor: "IBeam"` while its scrollbars also omit `cursor`.

This confirms the root cause is fixed: the edit cursor now belongs to the document content mouse area, not the scroll-view container or the control bounds.
