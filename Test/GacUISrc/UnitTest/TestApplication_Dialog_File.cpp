#include "TestControls.h"
#include "TestControls_List.h"

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
	void PressButton(UnitTestRemoteProtocol* protocol, const WString& buttonText)
	{
		auto window = From(GetApplication()->GetWindows())
			.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
			.First();
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
		auto window = From(GetApplication()->GetWindows())
			.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
			.First();
		auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"filePickerControl", L"textBox");
		textBox->SetText(fileName);
	}

	void ClickFileInternal(UnitTestRemoteProtocol* protocol, const WString& fileName, bool doubleClick)
	{
		auto window = From(GetApplication()->GetWindows())
			.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
			.First();
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
			LClickListItem(protocol, dataGrid, fileItemIndex);
		}
	}

	void ClickFile(UnitTestRemoteProtocol* protocol, const WString& fileName)
	{
		ClickFileInternal(protocol, fileName, false);
	}

	void DbClickFile(UnitTestRemoteProtocol* protocol, const WString& fileName)
	{
		ClickFileInternal(protocol, fileName, true);
	}

	void ChooseFilter(UnitTestRemoteProtocol* protocol, vint filterIndex)
	{
		auto window = From(GetApplication()->GetWindows())
			.Where([](GuiWindow* w) { return w->GetText() == L"FileDialog"; })
			.First();
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
      <Window ref.Name="self" Text="Dialog File Test" ClientSize="x:640 y:640">
        <OpenFileDialog ref.Name="dialogOpen" Title="FileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>
        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" CellPadding="5">
          <att.Rows>
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
                if (dialogOpen.ShowDialog())
                {
                  self.Text = "Opened";
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
					auto window = GetApplication()->GetMainWindow();;
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

		TEST_CASE(L"Open and Select")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();;
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
	});
}