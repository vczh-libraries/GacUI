- you have to follow `REPO-ROOT/.github/Guidelines/Coding.md` when coding.
- you have to run unit test to make sure your change works.
- commit and push after finishing the work.

Cursor on text box scroll bars are still I-beam.
Does not repro in document box, could be darkskin issue, not a control issue.
To repro this issue, you could use `Playground` with the automation service, if the automation service is not turned on, you can also fix it.

## REVIEW COMMENTS

### Clarify Expected Cursor And Scope [CLOSED]

**review comment**: The task says the cursor on text box scroll bars is "still I-beam", but it does not explicitly say what cursor is expected. Please clarify whether every scrollbar part in the affected text box should force `Arrow` (buttons, track, handle, and empty corner) while only the editable document area keeps `IBeam`. Also clarify the affected control scope: in DarkSkin, `MultilineTextBox` and `DocumentViewer` both use `darkskin::DocumentViewerTemplate`, while `SinglelineTextBox` and `DocumentTextBox` use `darkskin::DocumentTextBoxTemplate`. This matters because a template-level fix may change `DocumentViewer` too, despite the note saying the issue does not repro in document box.

The document box behavior is correct, only when the cursor is on where can type it could be a I-beam. Scroll bars are for grabbing so there should use default cursor.

### Require A Regression Check For Cursor Metadata [CLOSED]

**review comment**: The task gives a manual repro path through Playground, but does not state how the fix should be verified beyond running unit tests. Cursor state is already represented in the remote rendering DOM and there are existing cursor tests around compositions and graphics host hit testing, so please specify whether the execution should add a focused unit/snapshot/DOM assertion for the DarkSkin text-box scrollbar cursor, or whether manual Playground automation verification is intentionally enough. If the fix changes DarkSkin XML under `Test/Resources/App/DarkSkin`, the execution should also run `GacUI_Compiler` and inspect generated output before running the required unit test.

Put a text box and document box on Playground with scroll bar enabled, and you can run the app, use the automation service to read the UI. It should directly tell you what cursor should be on a composition. If you can't see it, check the source code of automation service and add it if missing. To implement this, you can omit if it is not set in a composition to reduce the JSON size, as cursors are inherited, if a composition does not specify one, it uses the one fron its direct or indirect parent. If the whole "report line" does not specify, it means default. Not specifying means `nullptr`, and when it is not null, you can read the enum. The enum item name will be what you need to write to the JSON in automationn service.
