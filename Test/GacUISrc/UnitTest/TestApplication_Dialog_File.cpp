#include "TestControls.h"
#include "TestControls_List.h"
#include "TestItemProviders.h"
#include "../../../Source/Utilities/FakeServices/Dialogs/Source/GuiFakeDialogServiceUI.h"

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

	Ptr<FileItemMock> CreateFileItemRoot_WithImages()
	{
		auto root = CreateFileItemRoot();
		AddFile(root, L"pic.bmp");
		AddFile(root, L"pic.png");
		AddFile(root, L"pic.jpg");
		return root;
	}

	GacUIUnitTest_Installer CreateInstaller(Ptr<FileSystemMock>& fsMock)
	{
		return {
			.initialize = [&]() { fsMock = Ptr(new FileSystemMock(CreateFileItemRoot())); },
			.finalize = [&]() { fsMock = {}; }
		};
	}

	GacUIUnitTest_Installer CreateInstallerWithImages(Ptr<FileSystemMock>& fsMock)
	{
		return {
			.initialize = [&]() { fsMock = Ptr(new FileSystemMock(CreateFileItemRoot_WithImages())); },
			.finalize = [&]() { fsMock = {}; }
		};
	}
}
using namespace gacui_unittest_template;

namespace gacui_file_dialog_template
{
	class FileNameSorter : public list::DataSorterBase
	{
	public:
		vint Compare(const Value& row1, const Value& row2) override
		{
			auto name1 = UnboxValue<Ptr<IFileDialogFile>>(row1)->GetName();
			auto name2 = UnboxValue<Ptr<IFileDialogFile>>(row2)->GetName();
			return name1 < name2 ? -1 : name1 > name2 ? 1 : 0;
		}
	};

	class FileSentinelFilter : public list::DataFilterBase
	{
	public:
		bool Filter(const Value& row) override
		{
			return UnboxValue<Ptr<IFileDialogFile>>(row)->GetName() != L"a.txt";
		}
	};

	GuiWindow* GetOwnedFileDialogWindow(GuiWindow* owner)
	{
		auto ownerWindow = owner->GetNativeWindow();
		auto result = From(GetApplication()->GetWindows())
			.Where([=](GuiWindow* window)
			{
				auto native = window->GetNativeWindow();
				return native && native->GetParent() == ownerWindow;
			})
			.First(nullptr);
		TEST_ASSERT(result != nullptr);
		return result;
	}

	void PressFileDialogButton(UnitTestRemoteProtocol* protocol, GuiWindow* window, const WString& text)
	{
		auto button = FindControlByText<GuiButton>(window, text);
		auto location = protocol->LocationOf(button);
		GetApplication()->InvokeInMainThread(window, [=]() { protocol->LClick(location); });
	}

	GuiWindow* TryGetOpeningFileDialog()
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
			.First(nullptr);
	}

	GuiWindow* GetOpeningFileDialog()
	{
		auto openingDialog = TryGetOpeningFileDialog();
		TEST_ASSERT(openingDialog != nullptr);
		return openingDialog;
	}

	Ptr<IFileDialogViewModel> GetOpeningFileDialogViewModel()
	{
		return FindObjectByName<gaclib_controls::FilePickerControl>(GetOpeningFileDialog(), L"filePickerControl")->GetViewModel();
	}

	GuiWindow* TryGetOpeningMessageDialog()
	{
		auto fileDialogWindow = TryGetOpeningFileDialog();
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
			.First(nullptr);
	}

	GuiWindow* GetOpeningMessageDialog()
	{
		auto openingDialog = TryGetOpeningMessageDialog();
		TEST_ASSERT(openingDialog != nullptr);
		return openingDialog;
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

	void PressMessageButton(UnitTestRemoteProtocol* protocol, const WString& buttonText)
	{
		auto window = GetOpeningMessageDialog();
		auto button = FindControlByText<GuiButton>(window, buttonText);
		auto location = protocol->LocationOf(button);
		GetApplication()->InvokeInMainThread(window, [=]()
		{
			protocol->LClick(location);
		});
	}

	void PressMessageOK(UnitTestRemoteProtocol* protocol)
	{
		PressMessageButton(protocol, L"OK");
	}

	void PressMessageCancel(UnitTestRemoteProtocol* protocol)
	{
		PressMessageButton(protocol, L"Cancel");
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
          <Cell Site="row:4 column:0">
            <Button Text="Open PromptCreateFile">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                dialogOpen.Options = INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile;
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
        </Table>
      </Window>
    </Instance>
  </Instance>
</Resource>
)GacUISrc";

	const auto resourceFileDialogs_FilterExtensions = LR"GacUISrc(
<Resource>
  <Instance name="MainWindowResource">
    <Instance ref.Class="gacuisrc_unittest::MainWindow">
      <Window ref.Name="self" Text="Dialog File Filter Extensions" ClientSize="x:640 y:480">
        <OpenFileDialog ref.Name="dialogOpen" Title="FileDialog" Filter="All Files (*.*)|*.*|Text Files (*.txt)|*.txt|Image Files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg|Wildcard ?xt (*.?xt)|*.?xt|Wildcard ?yt (*.?yt)|*.?yt|Wildcard *xt+*yt (*.*xt;*.*yt)|*.*xt;*.*yt|Wildcard *yt+*zt (*.*yt;*.*zt)|*.*yt;*.*zt" Directory=""/>
        <SaveFileDialog ref.Name="dialogSave" Title="FileDialog" Filter="All Files (*.*)|*.*|Text Files (*.txt)|*.txt|Image Files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg|Wildcard ?xt (*.?xt)|*.?xt|Wildcard ?yt (*.?yt)|*.?yt|Wildcard *xt+*yt (*.*xt;*.*yt)|*.*xt;*.*yt|Wildcard *yt+*zt (*.*yt;*.*zt)|*.*yt;*.*zt" Directory=""/>
        <Table AlignmentToParent="left:0 top:0 right:0 bottom:0" CellPadding="5">
          <att.Rows>
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
            <Button Text="Open Filter Extensions">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                dialogOpen.DefaultExtension = "txt";
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
            <Button Text="Open Filter Extensions MustExist">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                dialogOpen.Options = INativeDialogService::FileDialogOptions::FileDialogFileMustExist;
                dialogOpen.DefaultExtension = "txt";
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
            <Button Text="Save Filter Extensions PromptCreateFile">
              <att.BoundsComposition-set AlignmentToParent="left:0 top:0 right:0 bottom:0"/>
              <ev.Clicked-eval><![CDATA[{
                dialogSave.Options = INativeDialogService::FileDialogOptions::FileDialogPromptCreateFile;
                dialogSave.DefaultExtension = "txt";
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
		TEST_CASE(L"Create retry and confirmed recursive deletion preserve the directory and typed name")
		{
			Ptr<FileSystemMock> fsMock;
			Ptr<FileItemMock> root;
			auto childName = WString(L"\u65B0\u5EFA Folder");
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen")->SetDirectory(L"/A");
					PressFileDialogButton(protocol, window, L"Open DefaultOptions");
				});
				protocol->OnNextIdleFrame(L"Directory loaded", [=]()
				{
					TypeFile(protocol, L"independent.txt");
					PressButton(protocol, L"New Folder");
				});
				protocol->OnNextIdleFrame(L"New folder input", [=]()
				{
					auto modal = GetOwnedFileDialogWindow(GetOpeningFileDialog());
					TEST_ASSERT(modal->GetText() == L"New Folder");
					TEST_ASSERT(!FindObjectByName<GuiSinglelineTextBox>(modal, L"folderName")->GetAcceptTabInput());
					FindObjectByName<GuiSinglelineTextBox>(modal, L"folderName")->SetText(L"AA");
					PressFileDialogButton(protocol, modal, L"OK");
				});
				protocol->OnNextIdleFrame(L"Existing folder error", [=]()
				{
					auto modal = GetOwnedFileDialogWindow(GetOpeningFileDialog());
					auto error = GetOwnedFileDialogWindow(modal);
					auto message = L"In folder \"/A\", \"AA\" folder cannot be created because it already exists.";
					FindControlByText<GuiLabel>(error, message);
					auto vm = GetOpeningFileDialogViewModel();
					TEST_ASSERT(vm->GetIsMutatingFiles());
					List<WString> paths;
					paths.Add(L"/B");
					TEST_ASSERT(!vm->TryConfirm(GetOpeningFileDialog(), From(paths)));
					TEST_ASSERT(vm->GetSelectedFolder()->GetFullPath() == L"/A");
					PressFileDialogButton(protocol, error, L"OK");
				});
				protocol->OnNextIdleFrame(L"Retry retains entered name", [=]()
				{
					auto modal = GetOwnedFileDialogWindow(GetOpeningFileDialog());
					auto input = FindObjectByName<GuiSinglelineTextBox>(modal, L"folderName");
					TEST_ASSERT(input->GetText() == L"AA");
					input->SetText(L"..");
					PressFileDialogButton(protocol, modal, L"OK");
				});
				protocol->OnNextIdleFrame(L"Parent traversal rejected", [=]()
				{
					auto modal = GetOwnedFileDialogWindow(GetOpeningFileDialog());
					auto error = GetOwnedFileDialogWindow(modal);
					FindControlByText<GuiLabel>(error, L"In folder \"/A\", \"..\" folder cannot be created.");
					PressFileDialogButton(protocol, error, L"OK");
				});
				protocol->OnNextIdleFrame(L"Corrected child name", [=]()
				{
					auto modal = GetOwnedFileDialogWindow(GetOpeningFileDialog());
					FindObjectByName<GuiSinglelineTextBox>(modal, L"folderName")->SetText(childName);
					PressFileDialogButton(protocol, modal, L"OK");
				});
				protocol->OnNextIdleFrame(L"Created directory and refreshed views", [=]()
				{
					auto window = GetOpeningFileDialog();
					TEST_ASSERT(Folder(FilePath(L"/A") / childName).Exists());
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"filePickerControl", L"textBox");
					TEST_ASSERT(textBox->GetText() == L"independent.txt");
					auto folderA = root->children[L"A"];
					AddFile(folderA->children[childName], L"nested.txt");
					ClickFile(protocol, childName);
					TypeFile(protocol, L"/B");
					PressButton(protocol, L"Delete");
				});
				protocol->OnNextIdleFrame(L"Confirmation uses selected row rather than typed path", [=]()
				{
					auto prompt = GetOwnedFileDialogWindow(GetOpeningFileDialog());
					FindControlByText<GuiLabel>(prompt, WString(L"Are you sure to delete the following items in folder \"/A\"?\n") + childName);
					PressFileDialogButton(protocol, prompt, L"Cancel");
				});
				protocol->OnNextIdleFrame(L"Canceled deletion preserves recursive contents", [=]()
				{
					TEST_ASSERT(File(FilePath(L"/A") / childName / L"nested.txt").Exists());
					PressButton(protocol, L"Delete");
				});
				protocol->OnNextIdleFrame(L"Confirmed selected directory", [=]()
				{
					PressFileDialogButton(protocol, GetOwnedFileDialogWindow(GetOpeningFileDialog()), L"OK");
				});
				protocol->OnNextIdleFrame(L"Deleted directory and retained independent filename", [=]()
				{
					TEST_ASSERT(!Folder(FilePath(L"/A") / childName).Exists());
					TEST_ASSERT(Folder(FilePath(L"/B/BA")).Exists());
					auto window = GetOpeningFileDialog();
					auto tree = FindObjectByName<GuiBindableTreeView>(window, L"filePickerControl", L"treeView");
					auto current = UnboxValue<Ptr<IFileDialogFolder>>(tree->GetSelectedItem());
					TEST_ASSERT(current && current->GetFullPath() == L"/A");
					TEST_ASSERT(!current->TryGetFolder(childName));
					auto child = current->TryGetFolder(L"AA");
					TEST_ASSERT(child && child->GetIndex() == 0 && child->GetParent() == current);
					auto textBox = FindObjectByName<GuiSinglelineTextBox>(window, L"filePickerControl", L"textBox");
					TEST_ASSERT(textBox->GetText() == L"/B");
					TEST_ASSERT(!FindControlByText<GuiButton>(window, L"Delete")->GetEnabled());
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Canceled", [=]()
				{
					GetApplication()->GetMainWindow()->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/CreateAndDelete"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				{
					.initialize = [&]()
					{
						root = CreateFileItemRoot();
						fsMock = Ptr(new FileSystemMock(root));
					},
					.finalize = [&]() { fsMock = {}; root = {}; }
				}
			);
		});

		TEST_CASE(L"Deletion continues after failures and rejects linked subtrees before mutation")
		{
			Ptr<FileSystemMock> fsMock;
			Ptr<FileItemMock> root;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen")->SetDirectory(L"/A");
					PressFileDialogButton(protocol, window, L"Open MultipleSelection");
				});
				protocol->OnNextIdleFrame(L"Opened multiple-selection dialog", [=]()
				{
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(GetOpeningFileDialog(), L"filePickerControl", L"dataGrid");
					dataGrid->SetAdditionalFilter(Ptr(new FileSentinelFilter));
					dataGrid->GetColumns()[0]->SetSorter(Ptr(new FileNameSorter));
					auto view = dynamic_cast<IDataGridView*>(dataGrid->GetItemProvider()->RequestView(WString::Unmanaged(IDataGridView::Identifier)));
					view->SortByColumn(0, false);
				});
				protocol->OnNextIdleFrame(L"Sorted and filtered file rows", [=]()
				{
					ClickFile(protocol, L"blocked.txt");
					ClickFile(protocol, L"linked", true, false, false);
					ClickFile(protocol, L"unsafe", true, false, false);
					ClickFile(protocol, L"z-delete.txt", true, false, false);
					ClickFile(protocol, L"a-empty", true, false, false);
					ClickFile(protocol, L"0-delete.txt", true, false, false);
					PressButton(protocol, L"Delete");
				});
				protocol->OnNextIdleFrame(L"Confirm mixed selection", [=]()
				{
					auto prompt = GetOwnedFileDialogWindow(GetOpeningFileDialog());
					FindControlByText<GuiLabel>(prompt, L"Are you sure to delete the following items in folder \"/A\"?\nz-delete.txt\nunsafe\nlinked\nblocked.txt\na-empty\n0-delete.txt");
					PressFileDialogButton(protocol, prompt, L"OK");
				});
				protocol->OnNextIdleFrame(L"Combined failure report after later successful deletion", [=]()
				{
					TEST_ASSERT(File(FilePath(L"/A/blocked.txt")).Exists());
					TEST_ASSERT(!File(FilePath(L"/A/z-delete.txt")).Exists());
					TEST_ASSERT(!File(FilePath(L"/A/0-delete.txt")).Exists());
					TEST_ASSERT(!Folder(FilePath(L"/A/a-empty")).Exists());
					TEST_ASSERT(File(FilePath(L"/A/unsafe/keep.txt")).Exists());
					TEST_ASSERT(Folder(FilePath(L"/A/unsafe/link")).Exists());
					TEST_ASSERT(Folder(FilePath(L"/A/linked")).Exists());
					TEST_ASSERT(File(FilePath(L"/A/a.txt")).Exists());
					auto prompt = GetOwnedFileDialogWindow(GetOpeningFileDialog());
					FindControlByText<GuiLabel>(prompt, L"The following items failed to delete in folder \"/A\":\nunsafe\nlinked\nblocked.txt");
					PressFileDialogButton(protocol, prompt, L"OK");
				});
				protocol->OnNextIdleFrame(L"Failure report dismissed and selection cleared", [=]()
				{
					auto window = GetOpeningFileDialog();
					TEST_ASSERT(!FindControlByText<GuiButton>(window, L"Delete")->GetEnabled());
					TEST_ASSERT(FindObjectByName<GuiSinglelineTextBox>(window, L"filePickerControl", L"textBox")->GetText() == WString::Empty);
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Canceled", [=]()
				{
					GetApplication()->GetMainWindow()->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/DeleteFailures"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				{
					.initialize = [&]()
					{
						root = CreateFileItemRoot();
						auto folder = root->children[L"A"];
						AddFile(folder, L"blocked.txt");
						AddFile(folder, L"z-delete.txt");
						AddFile(folder, L"0-delete.txt");
						AddFolder(folder, L"a-empty");
						AddFolder(folder, L"linked");
						auto unsafe = AddFolder(folder, L"unsafe");
						AddFolder(unsafe, L"link");
						AddFile(unsafe, L"keep.txt");
						fsMock = Ptr(new FileSystemMock(root));
						fsMock->blockedPaths.Add(L"/A/blocked.txt");
						fsMock->linkedPaths.Add(L"/A/linked");
						fsMock->linkedPaths.Add(L"/A/unsafe/link");
					},
					.finalize = [&]() { fsMock = {}; root = {}; }
				}
			);
		});

		TEST_CASE(L"Pending deletion keeps captured targets while navigation and refresh change the list")
		{
			Ptr<FileSystemMock> fsMock;
			Ptr<IFileDialogFolder> folderA;
			Ptr<IFileDialogFolder> deletedFolder;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen")->SetDirectory(L"/A");
					PressFileDialogButton(protocol, window, L"Open DefaultOptions");
				});
				protocol->OnNextIdleFrame(L"Selected directory before confirmation", [=, &folderA, &deletedFolder]()
				{
					auto vm = GetOpeningFileDialogViewModel();
					folderA = vm->GetSelectedFolder();
					deletedFolder = folderA->TryGetFolder(L"AA");
					ClickFile(protocol, L"AA");
					PressButton(protocol, L"Delete");
				});
				protocol->OnNextIdleFrame(L"Confirmation remains owned while the model navigates", [=]()
				{
					auto vm = GetOpeningFileDialogViewModel();
					TEST_ASSERT(vm->GetIsMutatingFiles());
					FindControlByText<GuiLabel>(GetOwnedFileDialogWindow(GetOpeningFileDialog()), L"Are you sure to delete the following items in folder \"/A\"?\nAA");
					vm->SetSelectedFolder(vm->GetRootFolder()->TryGetFolder(L"B"));
					vm->RefreshFiles();
					vm->RefreshFiles();
					TEST_ASSERT(!vm->GetCanModifyFolder());
					vm->CreateFolder(GetOpeningFileDialog(), vm->GetSelectedFolder(), L"duplicate", WString::Empty, WString::Empty, [](bool) { TEST_ASSERT(false); });
					Func<WString(const WString&, const WString&)> forbiddenPrompt = [](const WString&, const WString&) { TEST_ASSERT(false); return WString::Empty; };
					vm->DeleteFiles(GetOpeningFileDialog(), From(vm->GetFiles()), forbiddenPrompt, forbiddenPrompt);
				});
				protocol->OnNextIdleFrame(L"Confirm original captured directory after replacement list loaded", [=]()
				{
					TEST_ASSERT(GetOpeningFileDialogViewModel()->GetSelectedFolder()->GetFullPath() == L"/B");
					PressFileDialogButton(protocol, GetOwnedFileDialogWindow(GetOpeningFileDialog()), L"OK");
				});
				protocol->OnNextIdleFrame(L"Completion preserves the other directory and detaches deleted tree node", [=, &folderA, &deletedFolder]()
				{
					auto vm = GetOpeningFileDialogViewModel();
					TEST_ASSERT(!Folder(FilePath(L"/A/AA")).Exists());
					TEST_ASSERT(Folder(FilePath(L"/B/BA")).Exists());
					TEST_ASSERT(vm->GetSelectedFolder()->GetFullPath() == L"/B");
					TEST_ASSERT(!vm->GetIsLoadingFiles() && !vm->GetIsMutatingFiles());
					TEST_ASSERT(vm->GetFiles().Count() == 1 && vm->GetFiles()[0]->GetName() == L"BA");
					TEST_ASSERT(!folderA->TryGetFolder(L"AA"));
					TEST_ASSERT(!deletedFolder->GetParent() && deletedFolder->GetIndex() == -1);
					auto tree = FindObjectByName<GuiBindableTreeView>(GetOpeningFileDialog(), L"filePickerControl", L"treeView");
					TEST_ASSERT(UnboxValue<Ptr<IFileDialogFolder>>(tree->GetSelectedItem()) == vm->GetSelectedFolder());
					vm->SetSelectedFolder(folderA);
					vm->RefreshFiles();
				});
				protocol->OnNextIdleFrame(L"Return does not restore deleted entries", [=]()
				{
					auto vm = GetOpeningFileDialogViewModel();
					TEST_ASSERT(vm->GetFiles().Count() == 2);
					TEST_ASSERT(!vm->GetSelectedFolder()->TryGetFolder(L"AA"));
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Canceled", [=]() { GetApplication()->GetMainWindow()->Hide(); });
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/PendingNavigation"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Folder mutation commands are available in an ordinary directory")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					dialog->SetDirectory(L"/A");
					auto button = FindControlByText<GuiButton>(window, L"Open DefaultOptions");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]() { protocol->LClick(location); });
				});
				protocol->OnNextIdleFrame(L"Folder mutation commands", [=]()
				{
					auto window = GetOpeningFileDialog();
					auto create = FindControlByText<GuiButton>(window, L"New Folder");
					auto remove = FindControlByText<GuiButton>(window, L"Delete");
					TEST_ASSERT(create->GetEnabled());
					TEST_ASSERT(!remove->GetEnabled());
					auto vm = GetOpeningFileDialogViewModel();
					vm->SetSelectedFolder(vm->GetRootFolder());
				});
				protocol->OnNextIdleFrame(L"Synthetic root disables both mutation commands", [=]()
				{
					auto window = GetOpeningFileDialog();
					auto vm = GetOpeningFileDialogViewModel();
					TEST_ASSERT(vm->GetSelectedFolder()->GetFullPath() == WString::Empty);
					TEST_ASSERT(!vm->GetCanModifyFolder());
					TEST_ASSERT(!FindControlByText<GuiButton>(window, L"New Folder")->GetEnabled());
					TEST_ASSERT(!FindControlByText<GuiButton>(window, L"Delete")->GetEnabled());
					TEST_ASSERT(!vm->CanDeleteFiles(From(vm->GetFiles())));
					vm->CreateFolder(window, vm->GetRootFolder(), L"invalid-root-child", WString::Empty, WString::Empty, [](bool) { TEST_ASSERT(false); });
					Func<WString(const WString&, const WString&)> forbiddenPrompt = [](const WString&, const WString&) { TEST_ASSERT(false); return WString::Empty; };
					vm->DeleteFiles(window, From(vm->GetFiles()), forbiddenPrompt, forbiddenPrompt);
					TEST_ASSERT(!vm->GetIsMutatingFiles());
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Canceled", [=]()
				{
					GetApplication()->GetMainWindow()->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MutationCommands"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Initial file names survive asynchronous folder loading")
		{
			for (auto save : { false,true })
			{
				Ptr<FileSystemMock> fsMock;
				GacUIUnitTest_SetGuiMainProxy([&](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
				{
					auto initialFileName = save ? WString(L"new.txt") : FilePath(L"/A/a.txt").GetFullPath();
					protocol->OnNextIdleFrame(L"Ready", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto dialog = FindObjectByName<GuiFileDialogBase>(window, save ? L"dialogSave" : L"dialogOpen");
						dialog->SetFileName(initialFileName);
						auto button = FindControlByText<GuiButton>(window, save ? L"Save PromptCreateFile" : L"Open DefaultOptions");
						auto location = protocol->LocationOf(button);
						GetApplication()->InvokeInMainThread(window, [=]() { protocol->LClick(location); });
					});
					protocol->OnNextIdleFrame(L"Initial name", [=]()
					{
						auto textBox = FindObjectByName<GuiSinglelineTextBox>(GetOpeningFileDialog(), L"filePickerControl", L"textBox");
						TEST_ASSERT(textBox->GetText() == initialFileName);
						protocol->LClick(protocol->LocationOf(textBox));
					});
					protocol->OnNextIdleFrame(L"Loaded name", [=]()
					{
						auto textBox = FindObjectByName<GuiSinglelineTextBox>(GetOpeningFileDialog(), L"filePickerControl", L"textBox");
						TEST_ASSERT(textBox->GetText() == initialFileName);
						PressCancel(protocol);
					});
					protocol->OnNextIdleFrame(L"Canceled", [=]()
					{
						auto window = GetApplication()->GetMainWindow();
						auto dialog = FindObjectByName<GuiFileDialogBase>(window, save ? L"dialogSave" : L"dialogOpen");
						TEST_ASSERT(dialog->GetFileName() == initialFileName);
						window->Hide();
					});
				});
				GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
					save ? L"Application/Dialog_File/InitialName_Save" : L"Application/Dialog_File/InitialName_Open",
					WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
					resourceFileDialogs,
					CreateInstaller(fsMock)
				);
			}
		});

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
					PressMessageOK(protocol);
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

		TEST_CASE(L"Open and Select an absolute file from its parent folder")
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
					TypeFile(protocol, FilePath(L"/A/a.txt").GetFullPath());
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
					PressMessageOK(protocol);
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
					PressMessageOK(protocol);
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
					PressMessageOK(protocol);
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

	TEST_CATEGORY(L"File Dialog Message Box Interactions")
	{
		TEST_CASE(L"File must exist: Open + non-existing file shows error and stays open")
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
					TypeFile(protocol, L"not-exist.txt");
				});
				protocol->OnNextIdleFrame(L"Typed: not-exist.txt", [=]()
				{
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageOK(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message OK", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Cancel", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 0);
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MessageBoxes_FileMustExist_Error_StaysOpen"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Prompt create file (open): Cancel keeps open; OK commits")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Open PromptCreateFile");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Clicked: Open PromptCreateFile", [=]()
				{
					TypeFile(protocol, L"new-created.txt");
				});
				protocol->OnNextIdleFrame(L"Typed: new-created.txt", [=]()
				{
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message Cancel", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open again", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageOK(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message OK", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() == nullptr);

					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 1);
					TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/new-created.txt").GetFullPath());
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MessageBoxes_PromptCreateFile_Open_CancelThenOK"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Prompt overwrite file (save): Cancel keeps open; OK commits")
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
				protocol->OnNextIdleFrame(L"Clicked: Save PromptOverwriteFile", [=]()
				{
					TypeFile(protocol, L"root.txt");
				});
				protocol->OnNextIdleFrame(L"Typed: root.txt", [=]()
				{
					PressSave(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Save", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message Cancel", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressSave(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Save again", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageOK(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message OK", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() == nullptr);

					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiSaveFileDialog>(window, L"dialogSave");
					TEST_ASSERT(dialog->GetFileName() == FilePath(L"/root.txt").GetFullPath());
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MessageBoxes_PromptOverwriteFile_Save_CancelThenOK"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Multiple selection not enabled: type a;b shows error and stays open")
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
					TypeFile(protocol, L"README;root.txt");
				});
				protocol->OnNextIdleFrame(L"Typed: README;root.txt", [=]()
				{
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageOK(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message OK", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Cancel", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 0);
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MessageBoxes_MultipleSelectionNotEnabled_Error_StaysOpen"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"File expected but folder selected: type README;A shows error and stays open")
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
					TypeFile(protocol, L"README;A");
				});
				protocol->OnNextIdleFrame(L"Typed: README;A", [=]()
				{
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Open", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() != nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressMessageOK(protocol);
				});
				protocol->OnNextIdleFrame(L"Clicked: Message OK", [=]()
				{
					TEST_ASSERT(TryGetOpeningMessageDialog() == nullptr);
					TEST_ASSERT(TryGetOpeningFileDialog() != nullptr);
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Pressed: Cancel", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 0);
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/MessageBoxes_FileExpected_Error_StaysOpen"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				CreateInstaller(fsMock)
			);
		});
	});

	TEST_CATEGORY(L"File Dialog Filter Extensions")
	{
		TEST_CASE(L"ParseFileDialogFilter")
		{
			using FilterDesc = FakeDialogServiceBase::FilterDesc;
			collections::List<FilterDesc> descs;

			// Empty filter string
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Empty, descs);
			TEST_ASSERT(descs.Count() == 0);

			// Single filter with simple extension
			descs.Clear();
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Unmanaged(L"Text Files (*.txt)|*.txt"), descs);
			TEST_ASSERT(descs.Count() == 1);
			TEST_ASSERT(descs[0].name == L"Text Files (*.txt)");
			TEST_ASSERT(descs[0].filter == L"*.txt");
			TEST_ASSERT(descs[0].regexFilterCode == L"^(/.*.txt)$");
			TEST_ASSERT(descs[0].defaultExtensionOverride.Value() == L"txt");

			// All files wildcard: *.* has no default extension
			descs.Clear();
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Unmanaged(L"All Files (*.*)|*.*"), descs);
			TEST_ASSERT(descs.Count() == 1);
			TEST_ASSERT(descs[0].name == L"All Files (*.*)");
			TEST_ASSERT(descs[0].filter == L"*.*");
			TEST_ASSERT(descs[0].regexFilterCode == L"^(/.*./.*)$");
			TEST_ASSERT(!descs[0].defaultExtensionOverride);

			// Star-only filter: * has no default extension
			descs.Clear();
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Unmanaged(L"All Files|*"), descs);
			TEST_ASSERT(descs.Count() == 1);
			TEST_ASSERT(descs[0].name == L"All Files");
			TEST_ASSERT(descs[0].filter == L"*");
			TEST_ASSERT(!descs[0].defaultExtensionOverride);

			// Multiple filters separated by |
			descs.Clear();
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Unmanaged(L"All Files (*.*)|*.*|Text Files (*.txt)|*.txt"), descs);
			TEST_ASSERT(descs.Count() == 2);
			TEST_ASSERT(descs[0].name == L"All Files (*.*)");
			TEST_ASSERT(descs[0].filter == L"*.*");
			TEST_ASSERT(!descs[0].defaultExtensionOverride);
			TEST_ASSERT(descs[1].name == L"Text Files (*.txt)");
			TEST_ASSERT(descs[1].filter == L"*.txt");
			TEST_ASSERT(descs[1].defaultExtensionOverride.Value() == L"txt");

			// Multi-wildcard filter: first qualifying part wins
			descs.Clear();
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Unmanaged(L"Image Files|*.bmp;*.png;*.jpg"), descs);
			TEST_ASSERT(descs.Count() == 1);
			TEST_ASSERT(descs[0].name == L"Image Files");
			TEST_ASSERT(descs[0].filter == L"*.bmp;*.png;*.jpg");
			TEST_ASSERT(descs[0].defaultExtensionOverride.Value() == L"bmp");

			// Multi-wildcard where first part doesn't qualify but second does
			descs.Clear();
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Unmanaged(L"Mixed|*.?xt;*.txt"), descs);
			TEST_ASSERT(descs.Count() == 1);
			TEST_ASSERT(descs[0].name == L"Mixed");
			TEST_ASSERT(descs[0].filter == L"*.?xt;*.txt");
			TEST_ASSERT(descs[0].defaultExtensionOverride.Value() == L"txt");

			// *.?xt: question mark in extension, no default
			descs.Clear();
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Unmanaged(L"Wildcard ?xt|*.?xt"), descs);
			TEST_ASSERT(descs.Count() == 1);
			TEST_ASSERT(descs[0].name == L"Wildcard ?xt");
			TEST_ASSERT(descs[0].filter == L"*.?xt");
			TEST_ASSERT(!descs[0].defaultExtensionOverride);

			// *.?yt: question mark in extension, no default
			descs.Clear();
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Unmanaged(L"Wildcard ?yt|*.?yt"), descs);
			TEST_ASSERT(descs.Count() == 1);
			TEST_ASSERT(descs[0].name == L"Wildcard ?yt");
			TEST_ASSERT(descs[0].filter == L"*.?yt");
			TEST_ASSERT(!descs[0].defaultExtensionOverride);

			// *.*xt;*.*yt: star in extension parts, no default for either
			descs.Clear();
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Unmanaged(L"Wildcard *xt+*yt|*.*xt;*.*yt"), descs);
			TEST_ASSERT(descs.Count() == 1);
			TEST_ASSERT(descs[0].name == L"Wildcard *xt+*yt");
			TEST_ASSERT(descs[0].filter == L"*.*xt;*.*yt");
			TEST_ASSERT(!descs[0].defaultExtensionOverride);

			// *.*yt;*.*zt: star in extension parts, no default for either
			descs.Clear();
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Unmanaged(L"Wildcard *yt+*zt|*.*yt;*.*zt"), descs);
			TEST_ASSERT(descs.Count() == 1);
			TEST_ASSERT(descs[0].name == L"Wildcard *yt+*zt");
			TEST_ASSERT(descs[0].filter == L"*.*yt;*.*zt");
			TEST_ASSERT(!descs[0].defaultExtensionOverride);

			// Full filter string with all filters including new wildcards
			descs.Clear();
			FakeDialogServiceBase::ParseFileDialogFilter(WString::Unmanaged(
				L"All Files (*.*)|*.*|Text Files (*.txt)|*.txt|Image Files (*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg"
				L"|Wildcard ?xt (*.?xt)|*.?xt|Wildcard ?yt (*.?yt)|*.?yt"
				L"|Wildcard *xt+*yt (*.*xt;*.*yt)|*.*xt;*.*yt|Wildcard *yt+*zt (*.*yt;*.*zt)|*.*yt;*.*zt"
			), descs);
			TEST_ASSERT(descs.Count() == 7);
			// 0: All Files *.*
			TEST_ASSERT(descs[0].name == L"All Files (*.*)");
			TEST_ASSERT(!descs[0].defaultExtensionOverride);
			// 1: Text Files *.txt
			TEST_ASSERT(descs[1].name == L"Text Files (*.txt)");
			TEST_ASSERT(descs[1].defaultExtensionOverride.Value() == L"txt");
			// 2: Image Files *.bmp;*.png;*.jpg -> first qualifying = bmp
			TEST_ASSERT(descs[2].name == L"Image Files (*.bmp;*.png;*.jpg)");
			TEST_ASSERT(descs[2].defaultExtensionOverride.Value() == L"bmp");
			// 3: *.?xt -> no default
			TEST_ASSERT(descs[3].name == L"Wildcard ?xt (*.?xt)");
			TEST_ASSERT(!descs[3].defaultExtensionOverride);
			// 4: *.?yt -> no default
			TEST_ASSERT(descs[4].name == L"Wildcard ?yt (*.?yt)");
			TEST_ASSERT(!descs[4].defaultExtensionOverride);
			// 5: *.*xt;*.*yt -> no default
			TEST_ASSERT(descs[5].name == L"Wildcard *xt+*yt (*.*xt;*.*yt)");
			TEST_ASSERT(!descs[5].defaultExtensionOverride);
			// 6: *.*yt;*.*zt -> no default
			TEST_ASSERT(descs[6].name == L"Wildcard *yt+*zt (*.*yt;*.*zt)");
			TEST_ASSERT(!descs[6].defaultExtensionOverride);

			// Verify regex filter matching for some patterns
			{
				// *.txt matches "file.txt" but not "file.doc"
				auto m1 = descs[1].regexFilter->MatchHead(WString::Unmanaged(L"file.txt"));
				TEST_ASSERT(m1 && m1->Result().Length() == 8);
				auto m2 = descs[1].regexFilter->MatchHead(WString::Unmanaged(L"file.doc"));
				TEST_ASSERT(!m2 || m2->Result().Length() != 8);
			}
			{
				// *.bmp;*.png;*.jpg matches "pic.bmp", "pic.png", "pic.jpg" but not "pic.doc"
				auto m1 = descs[2].regexFilter->MatchHead(WString::Unmanaged(L"pic.bmp"));
				TEST_ASSERT(m1 && m1->Result().Length() == 7);
				auto m2 = descs[2].regexFilter->MatchHead(WString::Unmanaged(L"pic.png"));
				TEST_ASSERT(m2 && m2->Result().Length() == 7);
				auto m3 = descs[2].regexFilter->MatchHead(WString::Unmanaged(L"pic.jpg"));
				TEST_ASSERT(m3 && m3->Result().Length() == 7);
				auto m4 = descs[2].regexFilter->MatchHead(WString::Unmanaged(L"pic.doc"));
				TEST_ASSERT(!m4 || m4->Result().Length() != 7);
			}
			{
				// *.?xt matches "x.txt" (? matches t) but not "x.abcxt" (? is single char)
				auto m1 = descs[3].regexFilter->MatchHead(WString::Unmanaged(L"x.txt"));
				TEST_ASSERT(m1 && m1->Result().Length() == 5);
				// *.?xt matches "x.txt" but not "x.abxt" (length mismatch for single ?)
				// Actually *.?xt -> regex ^(/.*\./.xt)$ so "x.abxt" would need /.=a then xt but remaining is bxt
				// Wait: "x.abxt" -> /.*="x", \.=".", /.="a", then "xt" but remaining is "bxt" -> no match
				// But /.* can match more: /.*="x.a", \.="b", /.="x", "xt"="t" -> no, "t" != "xt"
				// Correct: /.*="x.ab", but then \. needs a literal dot which "x" is not. This is tricky with backtracking.
				// Let's just test known cases.
			}
			{
				// *.?yt does NOT match "x.txt" (? matches t, then yt but remaining is xt)
				auto m1 = descs[4].regexFilter->MatchHead(WString::Unmanaged(L"x.txt"));
				TEST_ASSERT(!m1 || m1->Result().Length() != 5);
			}
			{
				// *.*xt;*.*yt matches "x.txt" (via *.*xt since txt ends with xt)
				auto m1 = descs[5].regexFilter->MatchHead(WString::Unmanaged(L"x.txt"));
				TEST_ASSERT(m1 && m1->Result().Length() == 5);
				// *.*xt;*.*yt matches "x.tyt" (via *.*yt)
				auto m2 = descs[5].regexFilter->MatchHead(WString::Unmanaged(L"x.tyt"));
				TEST_ASSERT(m2 && m2->Result().Length() == 5);
				// *.*xt;*.*yt does not match "x.tzt"
				auto m3 = descs[5].regexFilter->MatchHead(WString::Unmanaged(L"x.tzt"));
				TEST_ASSERT(!m3 || m3->Result().Length() != 5);
			}
			{
				// *.*yt;*.*zt does NOT match "x.txt"
				auto m1 = descs[6].regexFilter->MatchHead(WString::Unmanaged(L"x.txt"));
				TEST_ASSERT(!m1 || m1->Result().Length() != 5);
				// *.*yt;*.*zt matches "x.tyt"
				auto m2 = descs[6].regexFilter->MatchHead(WString::Unmanaged(L"x.tyt"));
				TEST_ASSERT(m2 && m2->Result().Length() == 5);
				// *.*yt;*.*zt matches "x.tzt"
				auto m3 = descs[6].regexFilter->MatchHead(WString::Unmanaged(L"x.tzt"));
				TEST_ASSERT(m3 && m3->Result().Length() == 5);
			}
		});

		TEST_CASE(L"*.* uses dialog default extension")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Open Filter Extensions MustExist");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					ChooseFilter(protocol, 1);
				});
				protocol->OnNextIdleFrame(L"Filter: Text Files", [=]()
				{
					ChooseFilter(protocol, 0);
				});
				protocol->OnNextIdleFrame(L"Filter: All Files", [=]()
				{
					TypeFile(protocol, L"root2");
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Confirmed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 1);
					TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/root2.txt").GetFullPath());
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/FilterExtensions_AllFiles_DefaultExtension"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs_FilterExtensions,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Multi-wildcard filter lists supported extensions")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Open Filter Extensions");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					ChooseFilter(protocol, 2);
				});
				protocol->OnNextIdleFrame(L"Filter: Image Files", [=]()
				{
					const wchar_t* expected[] =
					{
						L"A",
						L"B",
						L"pic.bmp",
						L"pic.jpg",
						L"pic.png",
					};

					auto dialogWindow = GetOpeningFileDialog();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
					AssertDataGridItems(dataGrid, expected);
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
				WString::Unmanaged(L"Application/Dialog_File/FilterExtensions_MultiWildcard_List"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs_FilterExtensions,
				CreateInstallerWithImages(fsMock)
			);
		});

		TEST_CASE(L"Multi-wildcard filter invents open extension from first qualifying wildcard")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Open Filter Extensions MustExist");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					ChooseFilter(protocol, 2);
				});
				protocol->OnNextIdleFrame(L"Filter: Image Files", [=]()
				{
					TypeFile(protocol, L"pic");
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Confirmed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 1);
					TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/pic.bmp").GetFullPath());
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/FilterExtensions_MultiWildcard_Open"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs_FilterExtensions,
				CreateInstallerWithImages(fsMock)
			);
		});

		TEST_CASE(L"Multi-wildcard filter uses first qualifying extension for save")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Save Filter Extensions PromptCreateFile");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					ChooseFilter(protocol, 2);
				});
				protocol->OnNextIdleFrame(L"Filter: Image Files", [=]()
				{
					TypeFile(protocol, L"newfile");
					PressSave(protocol);
				});
				protocol->OnNextIdleFrame(L"Confirmed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiSaveFileDialog>(window, L"dialogSave");
					TEST_ASSERT(dialog->GetFileName() == FilePath(L"/newfile.bmp").GetFullPath());
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/FilterExtensions_MultiWildcard_Save"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs_FilterExtensions,
				CreateInstallerWithImages(fsMock)
			);
		});

		TEST_CASE(L"Wildcard filter uses matching dialog default extension for open")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Open Filter Extensions MustExist");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					// *.?xt filter (index 3): no defaultExtensionOverride, but "txt" matches the filter
					ChooseFilter(protocol, 3);
				});
				protocol->OnNextIdleFrame(L"Filter: Wildcard ?xt", [=]()
				{
					TypeFile(protocol, L"root2");
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Confirmed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiOpenFileDialog>(window, L"dialogOpen");
					TEST_ASSERT(dialog->GetFileNames().Count() == 1);
					TEST_ASSERT(dialog->GetFileNames()[0] == FilePath(L"/root2.txt").GetFullPath());
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/FilterExtensions_WildcardMatchDefault_Open"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs_FilterExtensions,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Wildcard filter ignores non-matching dialog default extension for open")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Open Filter Extensions MustExist");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					// *.?yt filter (index 4): no defaultExtensionOverride, "txt" does NOT match the filter
					ChooseFilter(protocol, 4);
				});
				protocol->OnNextIdleFrame(L"Filter: Wildcard ?yt", [=]()
				{
					TypeFile(protocol, L"root2");
					PressOpen(protocol);
				});
				protocol->OnNextIdleFrame(L"Missing file prompt", [=]()
				{
					// "root2" without extension, filter doesn't match default "txt", so no extension added
					// File "root2" does not exist -> file must exist error
					PressMessageOK(protocol);
				});
				protocol->OnNextIdleFrame(L"Dismissed", [=]()
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
				WString::Unmanaged(L"Application/Dialog_File/FilterExtensions_WildcardNoMatchDefault_Open"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs_FilterExtensions,
				CreateInstaller(fsMock)
			);
		});

		TEST_CASE(L"Wildcard filter uses matching dialog default extension for save")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Save Filter Extensions PromptCreateFile");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					// *.*xt;*.*yt filter (index 5): no defaultExtensionOverride, "txt" matches *.*xt
					ChooseFilter(protocol, 5);
				});
				protocol->OnNextIdleFrame(L"Filter: Wildcard *xt+*yt", [=]()
				{
					TypeFile(protocol, L"newfile");
					PressSave(protocol);
				});
				protocol->OnNextIdleFrame(L"Confirmed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiSaveFileDialog>(window, L"dialogSave");
					TEST_ASSERT(dialog->GetFileName() == FilePath(L"/newfile.txt").GetFullPath());
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/FilterExtensions_WildcardMatchDefault_Save"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs_FilterExtensions,
				CreateInstallerWithImages(fsMock)
			);
		});

		TEST_CASE(L"Wildcard filter ignores non-matching dialog default extension for save")
		{
			Ptr<FileSystemMock> fsMock;
			GacUIUnitTest_SetGuiMainProxy([&fsMock](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
			{
				protocol->OnNextIdleFrame(L"Ready", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto button = FindControlByText<GuiButton>(window, L"Save Filter Extensions PromptCreateFile");
					auto location = protocol->LocationOf(button);
					GetApplication()->InvokeInMainThread(window, [=]()
					{
						protocol->LClick(location);
					});
				});
				protocol->OnNextIdleFrame(L"Show Dialog", [=]()
				{
					// *.*yt;*.*zt filter (index 6): no defaultExtensionOverride, "txt" does NOT match
					ChooseFilter(protocol, 6);
				});
				protocol->OnNextIdleFrame(L"Filter: Wildcard *yt+*zt", [=]()
				{
					TypeFile(protocol, L"newfile");
					PressSave(protocol);
				});
				protocol->OnNextIdleFrame(L"Confirmed", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					auto dialog = FindObjectByName<GuiSaveFileDialog>(window, L"dialogSave");
					// No extension added because "txt" doesn't match *.*yt;*.*zt
					TEST_ASSERT(dialog->GetFileName() == FilePath(L"/newfile").GetFullPath());
					window->Hide();
				});
			});
			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/FilterExtensions_WildcardNoMatchDefault_Save"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs_FilterExtensions,
				CreateInstallerWithImages(fsMock)
			);
		});
	});

	TEST_CATEGORY(L"File Dialog Scroll Reset On Navigation")
	{
		TEST_CASE(L"Scroll resets to first item after navigating into a folder")
		{
			Ptr<FileSystemMock> fsMock;

			auto createScrollTestRoot = []()
			{
				auto root = Ptr(new FileItemMock);
				auto folderA = AddFolder(root, L"A");
				for (vint i = 1; i <= 100; i++)
				{
					auto name = itow(i);
					while (name.Length() < 3) name = WString::Unmanaged(L"0") + name;
					auto sub = AddFolder(folderA, name);
					if (i == 100)
					{
						for (vint j = 1; j <= 100; j++)
						{
							auto fileName = itow(j);
							while (fileName.Length() < 3) fileName = WString::Unmanaged(L"0") + fileName;
							AddFile(sub, fileName);
						}
					}
				}
				return root;
			};

			GacUIUnitTest_SetGuiMainProxy([&fsMock, createScrollTestRoot](UnitTestRemoteProtocol* protocol, IUnitTestContext*)
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
				protocol->OnNextIdleFrame(L"Entered: A", [=]()
				{
					auto dialogWindow = GetOpeningFileDialog();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
					TEST_ASSERT(dataGrid->GetItemProvider()->Count() == 100);
					dataGrid->EnsureItemVisible(99);
				});
				protocol->OnNextIdleFrame(L"Scrolled to end", [=]()
				{
					auto dialogWindow = GetOpeningFileDialog();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
					// Verify item 0 is NOT visible (scrolled away)
					TEST_ASSERT(dataGrid->GetArranger()->GetVisibleStyle(0) == nullptr);
					DbClickFile(protocol, L"100");
				});
				protocol->OnNextIdleFrame(L"Entered: 100", [=]()
				{
					auto dialogWindow = GetOpeningFileDialog();
					auto dataGrid = FindObjectByName<GuiBindableDataGrid>(dialogWindow, L"filePickerControl", L"dataGrid");
					TEST_ASSERT(dataGrid->GetItemProvider()->Count() == 100);
					// After navigating into a new folder, item 0 should be visible
					TEST_ASSERT(dataGrid->GetArranger()->GetVisibleStyle(0) != nullptr);
					PressCancel(protocol);
				});
				protocol->OnNextIdleFrame(L"Cancel", [=]()
				{
					auto window = GetApplication()->GetMainWindow();
					window->Hide();
				});
			});

			GacUIUnitTest_StartFast_WithResourceAsText<darkskin::Theme>(
				WString::Unmanaged(L"Application/Dialog_File/ScrollResetOnNavigation"),
				WString::Unmanaged(L"gacuisrc_unittest::MainWindow"),
				resourceFileDialogs,
				{
					.initialize = [&, createScrollTestRoot]() { fsMock = Ptr(new FileSystemMock(createScrollTestRoot())); },
					.finalize = [&]() { fsMock = {}; }
				}
			);
		});
	});
}
