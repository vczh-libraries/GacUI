# !!!EXECUTION!!!

# UPDATES

## UPDATE

XML in C++ string literal should use 2 spaces for indentation instead of tab

## User Update Spotted

- The current code in Test/GacUISrc/UnitTest/TestControls_Editor.cpp still contains `resource_DocumentTextBox` and still has a `TEST_CATEGORY(L"GuiDocumentTextBox")`.
- This differs from the plan and Fixing attempt No.1 above, which says the `resource_DocumentTextBox` and `TEST_CATEGORY(L"GuiDocumentTextBox")` were removed.
- In the current code, `TEST_CATEGORY(L"GuiDocumentTextBox")` calls `RunTextBoxSmokeTest<GuiDocumentLabel>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));` (template type and category/name are not aligned).

# EXECUTION PLAN

## STEP 1: Make caret-click stable for multiline editors [DONE]

File: Test/GacUISrc/UnitTest/TestControls_Editor.cpp

In RunTextBoxSmokeTest<TTextBox>, find TEST_CASE(L"Typing") and the frame:

OnNextIdleFrame(L"Backspace/Delete applied", ...)

Replace the caret click location so it targets the first line deterministically.

Replace these two lines:

				auto caretPoint = protocol->LocationOf(textBox, 0.0, 0.5, 2, 0);
				protocol->LClick(caretPoint);

With:

				auto caretPoint = protocol->LocationOf(textBox, 0.0, 0.0, 2, 8);
				protocol->LClick(caretPoint);

## STEP 2: Add XML resources for each target control [DONE]

File: Test/GacUISrc/UnitTest/TestControls_Editor.cpp

In TEST_FILE, replace the existing resource definition:

	const auto resource = LR"GacUISrc(
	...
	)GacUISrc";

With the following resource definitions (insert as a consecutive block):

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

## STEP 3: Add TEST_CATEGORY invocations per control type [DONE]

File: Test/GacUISrc/UnitTest/TestControls_Editor.cpp

In TEST_FILE, replace the existing category block:

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		RunTextBoxSmokeTest<GuiSinglelineTextBox>(resource, WString::Unmanaged(L"GuiSinglelineTextBox"));
	});

With:

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
		RunTextBoxSmokeTest<GuiDocumentTextBox>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));
	});

	TEST_CATEGORY(L"GuiDocumentLabel")
	{
		RunTextBoxSmokeTest<GuiDocumentLabel>(resource_DocumentLabel, WString::Unmanaged(L"GuiDocumentLabel"));
	});

	TEST_CATEGORY(L"GuiDocumentViewer")
	{
		RunTextBoxSmokeTest<GuiDocumentViewer>(resource_DocumentViewer, WString::Unmanaged(L"GuiDocumentViewer"));
	});

## STEP 4: Test plan

Run the VS Code task: Build and Run Unit Tests.

Verification checklist:

- All 5 categories produce both Basic and Typing logs under Controls/Editor/... in the unit test output.
- Assertions in RunTextBoxSmokeTest pass for every control type (especially GetText() transitions in Typing).
- For multiline controls (GuiMultilineTextBox, GuiDocumentLabel, GuiDocumentViewer), the "Mouse typed at start" frame shows the caret landed on the first line (no click in blank space).

# FIXING ATTEMPTS

## Fixing attempt No.1

Why original change didn’t work:

- `GuiDocumentTextBox` is not a valid control type in this repo (it is not present in the runtime reflection metadata), so `RunTextBoxSmokeTest<GuiDocumentTextBox>(...)` fails to compile with an undeclared identifier / invalid template argument.

What you changed:

- Removed the `resource_DocumentTextBox` XML resource and the `TEST_CATEGORY(L"GuiDocumentTextBox")` block in `Test/GacUISrc/UnitTest/TestControls_Editor.cpp`.

Why it should fix the build:

- The unit test file now only instantiates `RunTextBoxSmokeTest<TTextBox>` for existing control types, eliminating the compile-time type error.

## Fixing attempt No.2

Why original change didn’t work:

- `GuiDocumentTextBox / Typing` failed at the `Mouse typed at start` frame.
- The `DocumentTextBox` in `resource_DocumentTextBox` was aligned with `bottom:-1`, so the control height could be too small.
- The test clicks at `ratioY=0.0` with `offsetY=8`, which can land outside the editable client area when the control is short, causing the caret not to move to the beginning and the inserted `"Start "` to go to an unexpected position.

What you changed:

- In `Test/GacUISrc/UnitTest/TestControls_Editor.cpp`, changed `resource_DocumentTextBox` bounds to `AlignmentToParent="left:5 top:5 right:5 bottom:5"` so the `DocumentTextBox` occupies a stable height.

Why it should fix the test:

- The click point used by the test is now guaranteed to be inside the control, so the caret move + insertion should be deterministic and `textBox->GetText()` should become `L"Start Replac"`.

## Fixing attempt No.3

Why original change didn’t work:

- `GuiDocumentTextBox / Typing` still failed at the `Mouse typed at start` frame.
- Even with a deterministic click location, clicking near the top-left of `DocumentTextBox` doesn’t always place the caret at the very beginning of the document, so typing `"Start "` can insert at an unexpected position.

What you changed:

- In `Test/GacUISrc/UnitTest/TestControls_Editor.cpp`, after clicking the caret location, added `protocol->KeyPress(VKEY::KEY_HOME, true, false, false);` (Ctrl+Home) to force the caret to the beginning before typing `"Start "`.

Why it should fix the test:

- `Ctrl+Home` deterministically moves the caret to the start of the document across editor implementations, making the subsequent insertion result stable.

# !!!FINISHED!!!

# !!!VERIFIED!!!

# Comparing to User Edit

- `resource_DocumentTextBox` and `TEST_CATEGORY(L"GuiDocumentTextBox")` exist in `Test/GacUISrc/UnitTest/TestControls_Editor.cpp`, despite Fixing attempt No.1 describing their removal. This suggests you prefer keeping a dedicated smoke-test entry for the XML virtual type `<DocumentTextBox/>`, even though the runtime control can be treated as `GuiDocumentLabel`.
- In `TEST_CATEGORY(L"GuiDocumentTextBox")`, the instantiation is `RunTextBoxSmokeTest<GuiDocumentLabel>(resource_DocumentTextBox, WString::Unmanaged(L"GuiDocumentTextBox"));`, confirming the intended pattern: the template parameter reflects the runtime C++ type while `controlName`/category reflects the XML control concept.
- `resource_DocumentTextBox` keeps `AlignmentToParent="... bottom:-1"` (instead of the Fixing attempt No.2 change to `bottom:5`). The preference appears to be: keep the single-line layout as originally designed and rely on deterministic caret movement (`Ctrl+Home` after the click) rather than changing layout just to make the click land reliably.
