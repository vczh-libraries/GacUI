# !!!EXECUTION!!!

# UPDATES

## User Update Spotted

- `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`: The `Navigation + Listing: Root -> A -> AA` test case no longer directly asserts `deep.bin` immediately after navigating into `AA`.
	- It now first asserts an empty listing under the persisted `Text Files (*.txt)` filter, then switches back to `All Files (*.*)` before asserting `deep.bin`.

# EXECUTION PLAN

## STEP 1: Add reusable helpers to assert the data grid listing [DONE]

Target file: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`.

Add `TestItemProviders.h` so the test can reuse `gacui_unittest_template::AssertCallbacks`:

	#include "TestControls.h"
	#include "TestControls_List.h"
	#include "TestItemProviders.h"

In `namespace gacui_file_dialog_template`, add this helper between `ClickFileInternal(...)` and `ClickFile(...)`:

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

## STEP 2: Add listing tests for root and filtered root [DONE]

Target file: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`.

In the existing `TEST_CATEGORY(L"File Dialog Navigation")`, append this test case:

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

## STEP 3: Add navigation + listing tests for nested folders [DONE]

Target file: `Test/GacUISrc/UnitTest/TestApplication_Dialog_File.cpp`.

In the existing `TEST_CATEGORY(L"File Dialog Navigation")`, append this test case:

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

## STEP 4: Verify [DONE]

- Built unit tests using the `Build Unit Tests` task (per execution prompt: compile only, do not run unit tests).
- Confirmed the listing assertions compare `GetTextValue(i)` in exact order: folders (alphabetical) followed by files (alphabetical, and filtered by the selected filter).

# FIXING ATTEMPTS

## Fixing attempt No.1

The new test case `Navigation + Listing: Root -> A -> AA` failed on the last frame.
`AssertDataGridItems` expected `deep.bin` but the data grid item provider reported 0 items.

Cause analysis:
- The test chooses filter index 1 (`Text Files (*.txt)`) while in folder `A`.
- The file dialog keeps the selected filter when navigating into folder `AA`.
- Folder `AA` contains only `deep.bin`, which does not match the `*.txt` filter, so the expected listing should be empty.

Fix:
- Update the test to first assert the empty listing in `AA` under the `Text Files` filter, then switch back to filter index 0 (`All Files`), and finally assert that `deep.bin` appears.

Why this should work:
- It aligns expectations with the intended behavior that file filters persist across navigation and apply to files.

# !!!FINISHED!!!

# !!!VERIFIED!!!


# Comparing to User Edit

- In navigation/listing tests, the selected filter persists across folder navigation, so a folder can legitimately show an empty list (e.g. navigating into a folder with only `deep.bin` while `Text Files (*.txt)` is selected).

