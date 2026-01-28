#include "TestControls.h"
#include "TestControls_List.h"
#include "TestItemProviders.h"

namespace gacui_unittest_template
{
	Ptr<FileItemMock> AddFolder(Ptr<FileItemMock> parent, const WString& name)
	{
		auto item = Ptr(new FileItemMock);
		item->isFile = false;
		parent->children.Add(name, item);
		return item;
	}

	Ptr<FileItemMock> AddFile(Ptr<FileItemMock> parent, const WString& name)
	{
		auto item = Ptr(new FileItemMock);
		item->isFile = true;
		parent->children.Add(name, item);
		return item;
	}

	Ptr<FileItemMock> CreateFileItemRoot()
	{
		auto root = Ptr(new FileItemMock);

		AddFile(root, L"root.txt");
		AddFile(root, L"root2.txt");
		AddFile(root, L"README");

		auto folderA = AddFolder(root, L"A");
		auto folderB = AddFolder(root, L"B");

		AddFile(folderA, L"a.txt");
		AddFile(folderA, L"noext");

		auto folderAA = AddFolder(folderA, L"AA");
		AddFile(folderAA, L"deep.bin");

		AddFolder(folderB, L"BA");

		return root;
	}

	GacUIUnitTest_Installer CreateInstaller(Ptr<FileSystemMock>& fsMock)
	{
		return {
			.initialize = [&]() { fsMock = Ptr(new FileSystemMock(CreateFileItemRoot())); },
			.finalize = [&]() { fsMock = {}; }
		};
	}
}
using namespace gacui_unittest_template;

namespace gacui_file_dialog_template
{
	GuiWindow* GetOpeningFileDialog()
	{
		auto mainWindow = GetApplication()->GetMainWindow();
		auto mainNativeWindow = mainWindow ? mainWindow->GetNativeWindow() : nullptr;
		return From(GetApplication()->GetWindows())
			.Where([](GuiWindow* w)
			{
				return w->GetText() == L"FileDialog";
			})
			.Where([=](GuiWindow* w)
			{
				auto nativeWindow = w->GetNativeWindow();
				if (!nativeWindow) return true;
				auto parent = nativeWindow->GetParent();
				return parent == nullptr || parent == mainNativeWindow;
			})
			.First();
	}

	GuiWindow* GetOpeningMessageDialog()
	{
		auto fileDialogWindow = GetOpeningFileDialog();
		auto fileDialogNativeWindow = fileDialogWindow ? fileDialogWindow->GetNativeWindow() : nullptr;
		return From(GetApplication()->GetWindows())
			.Where([=](GuiWindow* w)
			{
				if (w == fileDialogWindow) return false;
				auto nativeWindow = w->GetNativeWindow();
				if (!nativeWindow) return false;
				if (fileDialogNativeWindow) return nativeWindow->GetParent() == fileDialogNativeWindow;
				return nativeWindow->GetParent() != nullptr;
			})
			.First();
	}

	void PressButton(UnitTestRemoteProtocol* protocol, const WString& buttonText)
	{
		auto window = GetOpeningFileDialog();
		auto button = FindControlByText<GuiButton>(window, buttonText);
		auto location = protocol->LocationOf(button);
		GetApplication()->InvokeInMainThread(window, [=]()
		{
			protocol->LClick(location);
		});
	}

	void PressOpen(UnitTestRemoteProtocol* protocol)
	{
		PressButton(protocol, L"Open");
	}

	void PressSave(UnitTestRemoteProtocol* protocol)
	{
		PressButton(protocol, L"Save");
	}

	void PressCancel(UnitTestRemoteProtocol* protocol)
	{
		PressButton(protocol, L"Cancel");
	}

	void TypeFile(UnitTestRemoteProtocol* protocol, const WString& fileName)
	{
		auto window = GetOpeningFileDialog();
		auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"filePickerControl", L"textBox");
		textBox->SetText(fileName);
	}

	void ClickFileInternal(UnitTestRemoteProtocol* protocol, const WString& fileName, bool ctrl, bool shift, bool alt, bool doubleClick)
	{
		auto window = GetOpeningFileDialog();
		auto dataGrid = FindObjectByName<GuiBindableDataGrid>(window, L"filePickerControl", L"dataGrid");
		auto itemProvider = dataGrid->GetItemProvider();

		vint fileItemIndex = -1;
		for (vint i = 0; i < itemProvider->Count(); i++)
		{
			if (itemProvider->GetTextValue(i) == fileName)
			{
				fileItemIndex = i;
				break;
			}
		}

		TEST_ASSERT(fileItemIndex != -1);
		if (doubleClick)
		{
			LDBClickListItem(protocol, dataGrid, fileItemIndex);
		}
		else
		{
			LClickListItem(protocol, dataGrid, ctrl, shift, alt, fileItemIndex);
		}
	}

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

	void ClickFile(UnitTestRemoteProtocol* protocol, const WString& fileName)
	{
		ClickFileInternal(protocol, fileName, false, false, false, false);
	}

	void DbClickFile(UnitTestRemoteProtocol* protocol, const WString& fileName)
	{
		ClickFileInternal(protocol, fileName, false, false, false, true);
	}

	void ClickFile(UnitTestRemoteProtocol* protocol, const WString& fileName, bool ctrl, bool shift, bool alt)
	{
		ClickFileInternal(protocol, fileName, ctrl, shift, alt, false);
	}

	void ChooseFilter(UnitTestRemoteProtocol* protocol, vint filterIndex)
	{
		auto window = GetOpeningFileDialog();
		auto comboBox = FindObjectByName<GuiComboBoxListControl>(window, L"filePickerControl", L"comboBox");
		comboBox->SetSelectedIndex(filterIndex);
	}
}
using namespace gacui_file_dialog_template;

TEST_FILE
{
	const auto resourceFileDialogs = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Dialog File Test" ClientSize="x:640 y:480">
        <OpenFileDialog ref.Name="dialogOpen" Title="FileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>
        <SaveFileDialog ref.Name="dialogSave" Title="FileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>
        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" CellPadding="5">
          <att.Rows>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Rows>
          <att.Columns>
            <_>composeType:MinSize</_>
            <_>composeType:Percentage percentage:1.0</_>
          </att.Columns>

          <Cell Site="row:0 column:0">
            <Button Text="Open DefaultOptions">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                dialogOpen.Options = INativeDialogService::FileDialogOptions::FileDialogFileMustExist;
                if (dialogOpen.ShowDialog())
                {
                  var output : string = "";
                  for (fileName in dialogOpen.FileNames)
                  {
                    if (output != "")
                    {
                      output = output & ";";
                    }
                    output = output & fileName;
                  }
                  self.Text = output;
                }
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>
          <Cell Site="row:1 column:0">
            <Button Text="Open MultipleSelection">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                dialogOpen.Options = INativeDialogService::FileDialogOptions::FileDialogFileMustExist | INativeDialogService::FileDialogOptions::FileDialogAllowMultipleSelection;
                if (dialogOpen.ShowDialog())
                {
                  var output : string = "";
                  for (fileName in dialogOpen.FileNames)
                  {
                    if (output != "")
                    {
                      output = output & ";";
                    }
                    output = output & fileName;
                  }
                  self.Text = output;
                }
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>
          <Cell Site="row:2 column:0">
            <Button Text="Save PromptCreateFile">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                dialogSave.Options = INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile;
                if (dialogSave.ShowDialog())
                {
                  self.Text = dialogSave.FileName;
                }
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>
          <Cell Site="row:3 column:0">
            <Button Text="Save PromptOverwriteFile">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                dialogSave.Options = INativeDialogService::FileDialogOptions::FileDialogPromptOverwriteFile;
                if (dialogSave.ShowDialog())
                {
                  self.Text = dialogSave.FileName;
                }
              }]]></ev.Clicked-eval>
            </Button>
          </Cell>
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	TEST_CATEGORY(L"File Dialog Navigation")
	{
		TEST_CASE(L"Open and Close")
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
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					TypeFile(protocol, L"A");
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Open: A", [=]()
				{
					ChooseFilter(protocol, 1);
				});
				protocol->OnNextIdleFrame(L"Filter: Text Files", [=]()
				{
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
				WString::Unmanaged(L"Application/Dialog_File/OpenAndClose"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Save PromptCreateFile: Open and Close")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Save PromptCreateFile");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					TypeFile(protocol, L"new.txt");
				});
				protocol->OnNextIdleFrame(L"Typed", [=]()
				{
					PressSave(protocol);
				});
				protocol->OnNextIdleFrame(L"Confirmed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiSaveFileDialog>(window, L"dialogSave");
					TEST_ASSERT(dialog->GetFileName() == FilePath(L"/new.txt").GetFullPath());
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/SavePromptCreateFile_OpenAndClose"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Save PromptOverwriteFile: Open and Close")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Save PromptOverwriteFile");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					ClickFile(protocol, L"root.txt");
				});
				protocol->OnNextIdleFrame(L"Selected", [=]()
				{
					PressSave(protocol);
				});
				protocol->OnNextIdleFrame(L"Popped up overwrite prompt", [=]()
				{
					auto messageWindow = GetOpeningMessageDialog();
					auto buttonOk = FindControlByText<GuiButton>(messageWindow, L"OK");
					auto location = protocol->LocationOf(buttonOk);
					GetApplication()->InvokeInMainThread(messageWindow, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Confirmed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiSaveFileDialog>(window, L"dialogSave");
					TEST_ASSERT(dialog->GetFileName() == FilePath(L"/root.txt").GetFullPath());
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/SavePromptOverwriteFile_OpenAndClose"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Open and Select")
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
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					DbClickFile(protocol, L"A");
				});
				protocol->OnNextIdleFrame(L"DbClick: A", [=]()
				{
					ClickFile(protocol, L"a.txt");
				});
				protocol->OnNextIdleFrame(L"Click: a.txt", [=]()
				{
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Open", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 1);
					TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/A/a.txt").GetFullPath());
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/OpenAndSelect"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

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
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					const wchar_t* expectedAll[] =
					{
						L"A",
						L"B",
						L"README",
						L"root.txt",
						L"root2.txt",
					};

					auto dialogWindow = GetOpeningFileDialog();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
					AssertDataGridItems(dataGrid, expectedAll);
					ChooseFilter(protocol, 1);
				});
				protocol->OnNextIdleFrame(L"Choose Filter: Text Files", [=]()
				{
					const wchar_t* expectedTxt[] =
					{
						L"A",
						L"B",
						L"root.txt",
						L"root2.txt",
					};

					auto dialogWindow = GetOpeningFileDialog();
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
		});

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
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					const wchar_t* expectedRoot[] =
					{
						L"A",
						L"B",
						L"README",
						L"root.txt",
						L"root2.txt",
					};

					auto dialogWindow = GetOpeningFileDialog();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
					AssertDataGridItems(dataGrid, expectedRoot);
					DbClickFile(protocol, L"A");
				});
				protocol->OnNextIdleFrame(L"DbClick: A", [=]()
				{
					const wchar_t* expectedA[] =
					{
						L"AA",
						L"a.txt",
						L"noext",
					};

					auto dialogWindow = GetOpeningFileDialog();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
					AssertDataGridItems(dataGrid, expectedA);
					ChooseFilter(protocol, 1);
				});
				protocol->OnNextIdleFrame(L"Choose Filter: Text Files", [=]()
				{
					const wchar_t* expectedATxt[] =
					{
						L"AA",
						L"a.txt",
					};

					auto dialogWindow = GetOpeningFileDialog();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
					AssertDataGridItems(dataGrid, expectedATxt);
					DbClickFile(protocol, L"AA");
				});
				protocol->OnNextIdleFrame(L"DbClick: AA", [=]()
				{
					auto dialogWindow = GetOpeningFileDialog();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
					TEST_ASSERT(dataGrid->GetItemProvider()->Count() == 0);
					ChooseFilter(protocol, 0);
				});
				protocol->OnNextIdleFrame(L"Choose Filter: All Files", [=]()
				{
					const wchar_t* expectedAA[] =
					{
						L"deep.bin",
					};

					auto dialogWindow = GetOpeningFileDialog();
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
		});
	});

	TEST_CATEGORY(L"File Dialog Multiple Selection")
	{
		TEST_CASE(L"Multiple selection: All Files (root) -> selection string -> output list")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Open MultipleSelection");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Clicked: Open MultipleSelection", [=]()
				{
					ClickFile(protocol, L"README");
				});
				protocol->OnNextIdleFrame(L"Selected: README", [=]()
				{
					ClickFile(protocol, L"root.txt", true, false, false);
				});
				protocol->OnNextIdleFrame(L"Ctrl-Selected: root.txt", [=]()
				{
					auto dialogWindow = GetOpeningFileDialog();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(dialogWindow, L"filePickerControl", L"textBox");
					auto selectionText = textBox->GetText();
					TEST_ASSERT(selectionText == WString::Unmanaged(L"\"README\";\"root.txt\""));
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					auto&& fileNames = dialog->GetFileNames();
					TEST_ASSERT(fileNames.Count() == 2);
					TEST_ASSERT(fileNames.Contains(FilePath(L"/README").GetFullPath()));
					TEST_ASSERT(fileNames.Contains(FilePath(L"/root.txt").GetFullPath()));
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MultipleSelection_Root_AllFiles"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Multiple selection: Text Files filter (root) -> selection string -> output list")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Open MultipleSelection");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Clicked: Open MultipleSelection", [=]()
				{
					ChooseFilter(protocol, 1);
				});
				protocol->OnNextIdleFrame(L"Selected Filter: Text Files", [=]()
				{
					ClickFile(protocol, L"root.txt");
				});
				protocol->OnNextIdleFrame(L"Selected: root.txt", [=]()
				{
					ClickFile(protocol, L"root2.txt", true, false, false);
				});
				protocol->OnNextIdleFrame(L"Ctrl-Selected: root2.txt", [=]()
				{
					auto dialogWindow = GetOpeningFileDialog();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(dialogWindow, L"filePickerControl", L"textBox");
					auto selectionText = textBox->GetText();
					TEST_ASSERT(selectionText == WString::Unmanaged(L"\"root.txt\";\"root2.txt\""));
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					auto&& fileNames = dialog->GetFileNames();
					TEST_ASSERT(fileNames.Count() == 2);
					TEST_ASSERT(fileNames.Contains(FilePath(L"/root.txt").GetFullPath()));
					TEST_ASSERT(fileNames.Contains(FilePath(L"/root2.txt").GetFullPath()));
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MultipleSelection_Root_TextFiles"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Ctrl-click behaves like click when multiple selection is disabled")
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
				protocol->OnNextIdleFrame(L"Clicked: Open DefaultOptions", [=]()
				{
					ClickFile(protocol, L"README");
				});
				protocol->OnNextIdleFrame(L"Selected: README", [=]()
				{
					ClickFile(protocol, L"root.txt", true, false, false);
				});
				protocol->OnNextIdleFrame(L"Ctrl-Clicked: root.txt", [=]()
				{
					auto dialogWindow = GetOpeningFileDialog();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(dialogWindow, L"filePickerControl", L"textBox");
					auto selectionText = textBox->GetText();
					TEST_ASSERT(selectionText == WString::Unmanaged(L"\"root.txt\""));
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					auto&& fileNames = dialog->GetFileNames();
					TEST_ASSERT(fileNames.Count() == 1);
					TEST_ASSERT(fileNames[0] == FilePath(L"/root.txt").GetFullPath());
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/SingleSelection_CtrlClickBehavesLikeClick"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});
	});

	TEST_CATEGORY(L"File Dialog Typed Selection")
	{
		TEST_CASE(L"Typed selection: All Files no extension (fail then pass)")
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
				protocol->OnNextIdleFrame(L"Attempt 1: All Files + README.txt (fail)", [=]()
				{
					ChooseFilter(protocol, 0);
					TypeFile(protocol, L"README.txt");
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Popped up message dialog", [=]()
				{
					auto messageWindow = GetOpeningMessageDialog();
					auto buttonOK = FindControlByText<GuiButton>(messageWindow, L"OK");
					auto location = protocol->LocationOf(buttonOK);
					GetApplication()->InvokeInMainThread(messageWindow, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Attempt 2: All Files + README (pass)", [=]()
				{
					TypeFile(protocol, L"README");
				});
				protocol->OnNextIdleFrame(L"Typed", [=]()
				{
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Confirmed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 1);
					TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/README").GetFullPath());
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/TypedSelection_AllFiles_NoExt_FailThenPass"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Typed selection: Text Files no extension (fail then pass)")
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
				protocol->OnNextIdleFrame(L"Attempt 1: Text Files + README (fail)", [=]()
				{
					ChooseFilter(protocol, 1);
					TypeFile(protocol, L"README");
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Popped up message dialog", [=]()
				{
					auto messageWindow = GetOpeningMessageDialog();
					auto buttonOK = FindControlByText<GuiButton>(messageWindow, L"OK");
					auto location = protocol->LocationOf(buttonOK);
					GetApplication()->InvokeInMainThread(messageWindow, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Attempt 2: Text Files + root (pass)", [=]()
				{
					TypeFile(protocol, L"root");
				});
				protocol->OnNextIdleFrame(L"Typed", [=]()
				{
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Confirmed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 1);
					TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/root.txt").GetFullPath());
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/TypedSelection_TextFiles_NoExt_FailThenPass"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Typed selection: Text Files with extension (fail then pass)")
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
				protocol->OnNextIdleFrame(L"Attempt 1: Text Files + README.txt (fail)", [=]()
				{
					ChooseFilter(protocol, 1);
					TypeFile(protocol, L"README.txt");
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Popped up message dialog", [=]()
				{
					auto messageWindow = GetOpeningMessageDialog();
					auto buttonOK = FindControlByText<GuiButton>(messageWindow, L"OK");
					auto location = protocol->LocationOf(buttonOK);
					GetApplication()->InvokeInMainThread(messageWindow, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Attempt 2: Text Files + root.txt (pass)", [=]()
				{
					TypeFile(protocol, L"root.txt");
				});
				protocol->OnNextIdleFrame(L"Typed", [=]()
				{
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Confirmed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 1);
					TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/root.txt").GetFullPath());
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/TypedSelection_TextFiles_WithExt_FailThenPass"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});
	});
}