# !!!PLANNING!!!

# UPDATES

## UPDATE

It is not a good idea to write `RunSinglelineTextBox` in this way. Instead I just want you to inline all TEST_CASE in the `RunSinglelineTextBoxTestCases` function. As there are not much to share between test cases. And therefore you don't need that `appName` variable.

# EXECUTION PLAN

## STEP 1: Add minimal UI resource and outer category

Update `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox_Key.cpp` to have the same file-level structure as `Test/GacUISrc/UnitTest/TestControls_Editor_GuiSinglelineTextBox.cpp`, so all future key-behavior categories can be added in one place.

Proposed `TEST_FILE` and resource:

	const auto resourceSinglelineTextBox = LR"GacUISrc(
	<Resource>
	  <Instance name="MainWindowResource">
	    <Instance ref.Class="gacuisrc_unittest::MainWindow">
	      <Window ref.Name="self" Text="GuiSinglelineTextBox Key Test" ClientSize="x:480 y:320">
	        <SinglelineTextBox ref.Name="textBox">
	          <att.BoundsComposition-set AlignmentToParent="left:5 top:5 right:5 bottom:-1"/>
	        </SinglelineTextBox>
	      </Window>
	    </Instance>
	  </Instance>
	</Resource>
	)GacUISrc";

	TEST_CATEGORY(L"GuiSinglelineTextBox")
	{
		RunSinglelineTextBoxTestCases(resourceSinglelineTextBox, WString::Unmanaged(L"GuiSinglelineTextBox"));
	}

Why:
- The outer category name becomes stable (`GuiSinglelineTextBox`) while inner categories/cases grow over time.
- Keeping the UI minimal avoids speculative config and keeps future key tests focused on input behavior.

## STEP 2: Inline all TEST_CASE in RunSinglelineTextBoxTestCases

Implement `static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)` and place all `TEST_CATEGORY` / `TEST_CASE` blocks directly inside it (no separate `RunSinglelineTextBox` helper).

For each `TEST_CASE`:
- Build the test log identity inline (no `appName` variable) using:
  - `WString::Unmanaged(L"Controls/Editor/") + controlName + WString::Unmanaged(L"/Key/") + categoryName + WString::Unmanaged(L"_") + caseName`
- Call `GacUIUnitTest_SetGuiMainProxy(...)` to schedule:
  - A baseline setup idle-frame: find `GuiDocumentLabel` by name `textBox`, focus it, and ensure initial text like `L"0123456789"` is present.
  - The actual test actions/assertions (key presses / typing) in subsequent frames.
- Call `GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(...)` with the inline identity, window type, and `resource`.

Why:
- Avoids over-abstracting: there is little to share across test cases besides "boot + find + baseline", and the test scripts will diverge quickly.
- Keeps each `TEST_CASE` self-contained and readable when later categories (navigation/clipboard/undo/typing) are added.

## STEP 3: Add a smoke test category

Inside `RunSinglelineTextBoxTestCases(...)`, add a minimal `Scaffold` category with one case to validate the resource wiring and the basic key/typing plumbing.

Proposed content:

	static void RunSinglelineTextBoxTestCases(const wchar_t* resource, const WString& controlName)
	{
		TEST_CATEGORY(L"Scaffold")
		{
			TEST_CASE(L"Smoke")
			{
				GacUIUnitTest_SetGuiMainProxy([=](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
					protocol->OnNextIdleFrame(L"Init", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
						textBox->SetFocused();
						textBox->SelectAll();
						protocol->TypeString(L"0123456789");
					});

					protocol->OnNextIdleFrame(L"Run", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto textBox = FindObjectByName<GuiDocumentLabel>(window, L"textBox");
						TEST_ASSERT(textBox->GetText() == L"0123456789");
						protocol->KeyPress(VKEY::KEY_LEFT);
						protocol->TypeString(L"X");
						TEST_ASSERT(textBox->GetText() == L"012345678X9");
						window->Hide();
					});
				});

				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					WString::Unmanaged(L"Controls/Editor/")
					+ controlName
					+ WString::Unmanaged(L"/Key/Scaffold_Smoke"),
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resource
				);
			});
		});
	}

Why:
- Exercises the complete path: load resource, find control by name as `GuiDocumentLabel`, baseline initialization, key input via `UnitTestRemoteProtocol`, and orderly shutdown.
- Ensures the file provides one runnable test before future keyboard categories are added.

## STEP 4: Test plan and verification checklist

Test cases (to be present after implementation):
- `GuiSinglelineTextBox/Scaffold/Smoke` verifies the scaffold works and produces a log path under `Controls/Editor/GuiSinglelineTextBox/Key/Scaffold_Smoke`.

Manual verification checklist (for reviewing logs/snapshots):
- Confirm the generated test identity/path matches `Controls/Editor/<controlName>/Key/<CATEGORY>_<CASE>`.
- Confirm baseline initialization happens before key input: the first assertions see `L"0123456789"` and subsequent typing changes it deterministically.

# !!!FINISHED!!!
