# !!!PLANNING!!!

# UPDATES

## UPDATE

# UPDATE
I would like to propose a change in the `Proposed code` section in the planning document.  To make the test code clear, you can follow `AssertItems` in [TestItemProviders.h](Test/GacUISrc/UnitTest/TestItemProviders.h) , to assert a list of string literals against the data grid, and write your own version while reusing AssertCallbacks from that file directly.  WaitForDataGridTexts is not necessary, you should just assert items on the next frame, where those texts should be ready.

## UPDATE

# UPDATE
you don't need `AssertDataGridItemsOnNextFrame`, just call `AssertDataGridItems` in the next frame. making a function like this is not necessary.

## UPDATE

# UPDATE
in `Navigation + Listing: Root -> A -> AA` you cannot store dataGrid as this function will end even before the first frame is executed. Just get all controls in every frame independently. Check all other test cases.

## UPDATE

# UPDATE
`protocol->OnNextIdleFrame` can never be nested

## UPDATE

# UPDATE
any frame should make visible UI change except the last one. so assertion cannot be a single frame. for example, these two frames
```
			protocol->OnNextIdleFrame(L"Show Dialog", [=]()
			{
				auto dialogWindow = From(GetApplication()->GetWindows())
					.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
					.First();
				auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
				AssertDataGridItems(dataGrid, expectedAll);
			});

			protocol->OnNextIdleFrame(L"Choose Filter: Text Files", [=]()
			{
				ChooseFilter(protocol, 1);
			});
```
should combine. check all other test cases. by the way, variables like expectedAll and expectedTxt can just be defined inside a frame if it is not used across multiple frames. and you are not going to [=] an array anyway so just move them back in

# EXECUTION PLAN

## STEP 1: Add reusable helpers to assert the data grid listing

Change:

- Update `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` to reuse `gacui_unittest_template::AssertCallbacks` from `Test/GacUISrc/UnitTest/TestItemProviders.h` and add helpers in `namespace gacui_file_dialog_template`:
	- Add `#include "TestItemProviders.h"`.
	- Assert a list of string literals against the current visible item texts from `GuiBindableDataGrid::GetItemProvider()` using `IItemProvider::GetTextValue(i)`.
	- In test flows, call `protocol->OnNextIdleFrame(...)` directly to assert on the next idle frame after actions that refresh the listing (open dialog / navigation / filter change).

Proposed code (to be placed near existing helpers like `ClickFileInternal`):

	template<vint Count>
	void AssertDataGridItems(GuiBindableDataGrid* dataGrid, const wchar_t* (&expected)[Count])
	{
		auto itemProvider = dataGrid->GetItemProvider();
		collections::List<WString> actual;
		for (vint i = 0; i < itemProvider->Count(); i++)
		{
			actual.Add(itemProvider->GetTextValue(i));
		}
		gacui_unittest_template::AssertCallbacks(actual, expected);
	}

Why:

- The task requires assertions to be based on the displayed text in the data grid, not on folder/file types.
- A `const wchar_t*[]`-based helper mirrors `AssertItems` in `TestItemProviders.h`, keeping test code short while preserving `AssertCallbacks` diagnostics.
- For these cases, an extra `OnNextIdleFrame(...)` after an action that refreshes the listing is sufficient; avoid ad-hoc “wait until matches” loops in test code.
- Do not nest `protocol->OnNextIdleFrame(...)` calls; schedule all frames at the same scope like existing dialog tests in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`.
- Do not cache dialog controls (e.g. `GuiBindableDataGrid*`) in variables that outlive the current idle-frame callback; always re-find controls inside each `OnNextIdleFrame(...)` body.
- Avoid capturing C++ arrays in lambdas; define `expected...` arrays inside each frame when possible.

## STEP 2: Add listing tests for root and filtered root

Change:

- Add a new `TEST_CASE` under the dialog category in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` that:
	- Opens the dialog.
	- Asserts the initial root listing text order is “folders (alphabetical), then files (alphabetical)”.
	- Switches filter to `Text Files (*.txt)` and asserts the new visible listing is “folders (alphabetical), then remaining `*.txt` files (alphabetical)”.
	- Cancels, asserting `GetFileNames().Count() == 0`.
- Ensure that every `protocol->OnNextIdleFrame(...)` except the last one triggers a visible UI change (e.g. click, filter change, cancel). If an idle frame only performs assertions, merge it with the next action frame.
- Define `expected...` arrays inside a frame when they are not reused across frames.

Expected text sequences (based on `CreateFileItemRoot()` + dialog behavior):

- Root, All Files:
	- `L"A"`, `L"B"`, `L"README"`, `L"root.txt"`
- Root, Text Files (`*.txt`):
	- `L"A"`, `L"B"`, `L"root.txt"`

Proposed test flow (sketch):

	TEST_CASE(L"Listing: Root + Filter")
	{
		Ptr<FileSystemMock> fsMock;

		GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindControlByText<GuiButton>(window, L"Open DefaultOptions");
				auto location = protocol->LocationOf(button);
				GetApplication()->InvokeInMainThread(window, [=]()
				{
					protocol->LClick(location);
				});
			});

			protocol->OnNextIdleFrame(L"Show Dialog + Choose Filter: Text Files", [=]()
			{
				const wchar_t* expectedAll[] =
				{
					L"A",
					L"B",
					L"README",
					L"root.txt",
				};

				auto dialogWindow = From(GetApplication()->GetWindows())
					.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
					.First();
				auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
				AssertDataGridItems(dataGrid, expectedAll);
				ChooseFilter(protocol, 1);
			});

			protocol->OnNextIdleFrame(L"Assert Root Filtered Listing", [=]()
			{
				const wchar_t* expectedTxt[] =
				{
					L"A",
					L"B",
					L"root.txt",
				};

				auto dialogWindow = From(GetApplication()->GetWindows())
					.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
					.First();
				auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
				AssertDataGridItems(dataGrid, expectedTxt);
				PressCancel(protocol);
			});

			protocol->OnNextIdleFrame(L"Cancel", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
				TEST_ASSERT(dialog->GetFileNames().Count() == 0);
				window->Hide();
			});
		});

		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Application/Dialog_File/Listing_Root_Filter"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resourceFileDialogs,
			CreateInstaller(fsMock)
		);
	}

Why:

- Validates the most fundamental contract: the dialog renders the correct folder/file listing in deterministic order, and filtering hides only non-matching files while keeping folder rows visible.
- Addresses the update requirement: validating the listing by reading item text from the data grid.

## STEP 3: Add navigation + listing tests for nested folders

Change:

- Add a new `TEST_CASE` that navigates through folders using double-click in the data grid and asserts listing changes after each navigation:
	- Root → `A` (double-click `L"A"`).
	- `A` → `AA` (double-click `L"AA"`).
	- Apply `Text Files (*.txt)` inside `A` to verify filtering stability across navigation.
	- Cancel at the end and assert `GetFileNames().Count() == 0`.
- Ensure that every `protocol->OnNextIdleFrame(...)` except the last one triggers a visible UI change. Merge assertion-only idle frames with the next action frame.
- Define `expected...` arrays inside a frame when they are not reused across frames.

Expected text sequences:

- Folder `A`, All Files:
	- `L"AA"`, `L"a.txt"`, `L"noext"`
- Folder `A`, Text Files:
	- `L"AA"`, `L"a.txt"`
- Folder `AA`, All Files:
	- `L"deep.bin"`

Proposed test flow (sketch):

	TEST_CASE(L"Navigation + Listing: Root -> A -> AA")
	{
		Ptr<FileSystemMock> fsMock;

		GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
		{
			protocol->OnNextIdleFrame(L"Ready", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto button = FindControlByText<GuiButton>(window, L"Open DefaultOptions");
				auto location = protocol->LocationOf(button);
				GetApplication()->InvokeInMainThread(window, [=]()
				{
					protocol->LClick(location);
				});
			});

			protocol->OnNextIdleFrame(L"Show Dialog + DbClick: A", [=]()
			{
				const wchar_t* expectedRoot[] =
				{
					L"A",
					L"B",
					L"README",
					L"root.txt",
				};

				auto dialogWindow = From(GetApplication()->GetWindows())
					.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
					.First();
				auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
				AssertDataGridItems(dataGrid, expectedRoot);
				DbClickFile(protocol, L"A");
			});

			protocol->OnNextIdleFrame(L"Assert A Listing + Choose Filter: Text Files", [=]()
			{
				const wchar_t* expectedA[] =
				{
					L"AA",
					L"a.txt",
					L"noext",
				};

				auto dialogWindow = From(GetApplication()->GetWindows())
					.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
					.First();
				auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
				AssertDataGridItems(dataGrid, expectedA);
				ChooseFilter(protocol, 1);
			});

			protocol->OnNextIdleFrame(L"Assert A Filtered Listing + DbClick: AA", [=]()
			{
				const wchar_t* expectedATxt[] =
				{
					L"AA",
					L"a.txt",
				};

				auto dialogWindow = From(GetApplication()->GetWindows())
					.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
					.First();
				auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
				AssertDataGridItems(dataGrid, expectedATxt);
				DbClickFile(protocol, L"AA");
			});

			protocol->OnNextIdleFrame(L"Assert AA Listing", [=]()
			{
				const wchar_t* expectedAA[] =
				{
					L"deep.bin",
				};

				auto dialogWindow = From(GetApplication()->GetWindows())
					.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
					.First();
				auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
				AssertDataGridItems(dataGrid, expectedAA);
				PressCancel(protocol);
			});

			protocol->OnNextIdleFrame(L"Cancel", [=]()
			{
				auto window = GetApplication()->GetMainWindow();
				auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
				TEST_ASSERT(dialog->GetFileNames().Count() == 0);
				window->Hide();
			});
		});

		GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
			WString::Unmanaged(L"Application/Dialog_File/Navigation_Listing_Root_A_AA"),
			WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
			resourceFileDialogs,
			CreateInstaller(fsMock)
		);
	}

Why:

- Ensures double-click navigation triggers a folder change and refreshes listing content accordingly.
- Validates ordering and filtering at multiple levels, preventing regressions when view model refresh logic changes.

# TEST PLAN

- Run unit tests in `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp` and ensure:
	- Existing “Open and Close” / “Open and Select” cases remain stable.
	- New listing assertions always compare the full `GetTextValue(i)` sequences (folders first, then filtered files), per the update requirement.
	- Navigation steps do not rely on folder/file type detection; only the rendered item texts are compared.

# !!!FINISHED!!!
