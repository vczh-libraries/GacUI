# !!!PLANNING!!!

# UPDATES

# EXECUTION PLAN

## STEP 1: Make caret-click stable for multiline editors

The existing `RunTextBoxSmokeTest<TTextBox>` uses a mouse click to place the caret before typing (`"Mouse typed at start"`).

For multiline controls (everything except `<DocumentTextBox/>`), the control is expected to fill the window vertically (`bottom:5`), so clicking at `ratioY=0.5` can land in an empty area depending on layout/padding/scroll state.

Change the caret click to target the first line deterministically: click near the top-left of the editor content at `(ratioX=0.0, ratioY=0.0, offsetX=2, offsetY=8)`.

Planned code change in `Test/GacUISrc/UnitTest/TestControls_Editor.cpp` (inside `TEST_CASE(L"Typing")` / frame `"Backspace/Delete applied"`):

				auto caretPoint = protocol->LocationOf(textBox, 0.0, 0.0, 2, 8);
				protocol->LClick(caretPoint);

Why:
- Ensures the click always hits the first line across multiline controls.
- Keeps the test frame count unchanged and avoids artificial UI changes.

## STEP 2: Add XML resources for each target control

Add one resource XML string per control type so each smoke test runs in its own well-defined window and produces comparable logs.

Rules:
- Keep `ref.Name="textBox"` so the helper can locate the control.
- For multiline controls (`<MultilineTextBox/>`, `<DocumentLabel/>`, `<DocumentViewer/>`), set `AlignmentToParent="left:5 top:5 right:5 bottom:5"`.
- For `<DocumentTextBox/>` (singleline), keep `bottom:-1`.
- Set `EditMode="Editable"` on `<DocumentTextBox/>`, `<DocumentLabel/>`, and `<DocumentViewer/>`.

Planned code to add in `Test/GacUISrc/UnitTest/TestControls_Editor.cpp` (near the existing `resource`):

	const auto resource_SinglelineTextBox = LR"GacUISrc(
	<Resource>
	  <Instance name="MainWindowResource">
	    <Instance ref.Class="gacuisrc_unittest::MainWindow">
	      <Window ref.Name="self" Text="GuiSinglelineTextBox Test" ClientSize="x:480 y:320">
	        <SinglelineTextBox ref.Name="textBox" Text="Initial text">
	          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
	        </SinglelineTextBox>
	      </Window>
	    </Instance>
	  </Instance>
	</Resource>
	)GacUISrc";

	const auto resource_MultilineTextBox = LR"GacUISrc(
	<Resource>
	  <Instance name="MainWindowResource">
	    <Instance ref.Class="gacuisrc_unittest::MainWindow">
	      <Window ref.Name="self" Text="GuiMultilineTextBox Test" ClientSize="x:480 y:320">
	        <MultilineTextBox ref.Name="textBox" Text="Initial text">
	          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
	        </MultilineTextBox>
	      </Window>
	    </Instance>
	  </Instance>
	</Resource>
	)GacUISrc";

	const auto resource_DocumentTextBox = LR"GacUISrc(
	<Resource>
	  <Instance name="MainWindowResource">
	    <Instance ref.Class="gacuisrc_unittest::MainWindow">
	      <Window ref.Name="self" Text="GuiDocumentTextBox Test" ClientSize="x:480 y:320">
	        <DocumentTextBox ref.Name="textBox" Text="Initial text" EditMode="Editable">
	          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
	        </DocumentTextBox>
	      </Window>
	    </Instance>
	  </Instance>
	</Resource>
	)GacUISrc";

	const auto resource_DocumentLabel = LR"GacUISrc(
	<Resource>
	  <Instance name="MainWindowResource">
	    <Instance ref.Class="gacuisrc_unittest::MainWindow">
	      <Window ref.Name="self" Text="GuiDocumentLabel Test" ClientSize="x:480 y:320">
	        <DocumentLabel ref.Name="textBox" Text="Initial text" EditMode="Editable">
	          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
	        </DocumentLabel>
	      </Window>
	    </Instance>
	  </Instance>
	</Resource>
	)GacUISrc";

	const auto resource_DocumentViewer = LR"GacUISrc(
	<Resource>
	  <Instance name="MainWindowResource">
	    <Instance ref.Class="gacuisrc_unittest::MainWindow">
	      <Window ref.Name="self" Text="GuiDocumentViewer Test" ClientSize="x:480 y:320">
	        <DocumentViewer ref.Name="textBox" Text="Initial text" EditMode="Editable">
	          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:5"/>
	        </DocumentViewer>
	      </Window>
	    </Instance>
	  </Instance>
	</Resource>
	)GacUISrc";

Why:
- Makes failures comparable because each control’s visuals live under stable log roots.
- Avoids adding conditional logic in the test helper.

## STEP 3: Add `TEST_CATEGORY` invocations per control type

Add one `TEST_CATEGORY` per control type so log paths are clearly separated while reusing the same helper.

Notes:
- `<DocumentTextBox/>` is a virtual XML type created as a `GuiDocumentLabel`-backed control, so it should be tested with `RunTextBoxSmokeTest<GuiDocumentLabel>(...)` while using the folder name `GuiDocumentTextBox`.

Planned code change in `Test/GacUISrc/UnitTest/TestControls_Editor.cpp` (replace/extend the existing category block):

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		RunTextBoxSmokeTest<GuiSinglelineTextBox>(resource_SinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox"));
	});

	TEST_CATEGORY(L"GuiMultilineTextBox")
	{
		RunTextBoxSmokeTest<GuiMultilineTextBox>(resource_MultilineTextBox, WString::Unmanaged(L"GuiMultilineTextBox"));
	});

	TEST_CATEGORY(L"GuiDocumentTextBox")
	{
		RunTextBoxSmokeTest<GuiDocumentLabel>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));
	});

	TEST_CATEGORY(L"GuiDocumentLabel")
	{
		RunTextBoxSmokeTest<GuiDocumentLabel>(resource_DocumentLabel, WString::Unmanaged(L"GuiDocumentLabel"));
	});

	TEST_CATEGORY(L"GuiDocumentViewer")
	{
		RunTextBoxSmokeTest<GuiDocumentViewer>(resource_DocumentViewer, WString::Unmanaged(L"GuiDocumentViewer"));
	});

Why:
- Keeps the output folders stable and easy to compare: `Controls/Editor/<ControlName>/{Basic|Typing}`.
- Keeps the smoke-test surface identical across editor variants.

## STEP 4: Test plan (no new test cases beyond this extension)

Run existing unit test build and execution tasks.

Verification checklist:
- All 5 categories produce both `Basic` and `Typing` logs under `Controls/Editor/...`.
- Assertions in `RunTextBoxSmokeTest` pass for every control type (especially `GetText()` transitions in `Typing`).
- For the multiline controls (`GuiMultilineTextBox`, `GuiDocumentLabel`, `GuiDocumentViewer`), the `Mouse typed at start` frame shows caret placement on the first line (no unexpected edits at the end / no click in blank space).

# !!!FINISHED!!!
