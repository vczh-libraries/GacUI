#include "TestControls.h"

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

TEST_FILE
{
	const auto resourceFileDialogs = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Dialog File Test" ClientSize="x:640 y:640">
        <OpenFileDialog ref.Name="dialogOpen" Title="OpenFileDialog" Filter="All Files (*.*)|*|Text Files (*.txt)|*.txt" Directory=""/>
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
					auto window = From(GetApplication()->GetWindows())
						.Where([](GuiWindow* w) { return w->GetText() == L"OpenFileDialog"; })
						.First();
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"filePickerControl", L"textBox");
					auto button = FindControlByText<GuiButton>(window, L"Open");
					auto location = protocol->LocationOf(button);

					textBox->SetText(L"A");
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Open: A", [=]()
				{
					auto window = From(GetApplication()->GetWindows())
						.Where([](GuiWindow* w) { return w->GetText() == L"OpenFileDialog"; })
						.First();
					auto comboBox = FindObjectByName<GuiComboBoxListControl>(window, L"filePickerControl", L"comboBox");
					comboBox->SetSelectedIndex(1);
				});
				protocol->OnNextIdleFrame(L"Filter: Text Files", [=]()
				{
					auto window = From(GetApplication()->GetWindows())
						.Where([](GuiWindow* w) { return w->GetText() == L"OpenFileDialog"; })
						.First();
					auto button = FindControlByText<GuiButton>(window, L"Cancel");
					auto location = protocol->LocationOf(button);
					protocol->LClick(location);
				});
				protocol->OnNextIdleFrame(L"Cancel", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
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
	});
}