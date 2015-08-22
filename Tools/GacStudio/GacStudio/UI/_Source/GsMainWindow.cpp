/***********************************************************************
Vczh Library++ 3.0
Developer: 陈梓瀚(vczh)
GacUI::MainWindow

本文件使用Vczh GacUI Resource Code Generator工具自动生成
***********************************************************************/

#include "GacStudioUI.h"
#include "..\..\Studio\SolutionModel.h"

using namespace vl::collections;

namespace ui
{
	// #region CLASS_MEMBER_GUIEVENT_HANDLER (DO NOT PUT OTHER CONTENT IN THIS #region.)

	void MainWindow::cellSplitter_leftButtonDown(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiMouseEventArgs& arguments)
	{
		splitterDragging = true;
		splitterPosition = cellSplitter->GetGlobalBounds().Left() + arguments.x;
	}

	void MainWindow::cellSplitter_leftButtonUp(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiMouseEventArgs& arguments)
	{
		splitterDragging = false;
	}

	void MainWindow::cellSplitter_mouseMove(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiMouseEventArgs& arguments)
	{
		if (splitterDragging)
		{
			vint delta = cellSplitter->GetGlobalBounds().Left() + arguments.x - splitterPosition;
			splitterPosition += delta;

			auto option = tableMain->GetColumnOption(0);
			option.absolute += delta;
			tableMain->SetColumnOption(0, option);
			tableMain->ForceCalculateSizeImmediately();
		}
	}

	void MainWindow::commandFileAddExistingFiles_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto action = GetViewModel()->GetWorkingItem().Cast<vm::IAddFileItemAction>();
		if (!action) return;
		auto window = new AddExistingFilesWindow(GetViewModel(), GetViewModel()->CreateAddExistingFilesModel(action), action);
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();
		window->ShowModalAndDelete(this, [](){});
	}

	void MainWindow::commandFileAddNewFile_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto action = GetViewModel()->GetWorkingItem().Cast<vm::IAddFileItemAction>();
		if (!action) return;
		auto window = new NewFileWindow(GetViewModel(), GetViewModel()->CreateNewFileModel(), action);
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();
		window->ShowModalAndDelete(this, [](){});
	}

	void MainWindow::commandFileCloseSolution_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		GetViewModel()->CloseSolution();
	}

	void MainWindow::commandFileExit_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		Close();
	}

	void MainWindow::commandFileNewProject_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto window = new NewProjectWindow(GetViewModel());
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();
		window->ShowModalAndDelete(this, [](){});
	}

	void MainWindow::commandFileOpenProject_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto model = GetViewModel();
		List<WString> fileNames;
		vint filterIndex = 0;
		if (GetCurrentController()->DialogService()->ShowFileDialog(
			GetNativeWindow(),
			fileNames,
			filterIndex,
			INativeDialogService::FileDialogOpen,
			L"Open Solution",
			L"",
			L"",
			L"gacsln.xml",
			L"Gac Studio Solution (*.gacsln.xml)|*.gacsln.xml",
			(INativeDialogService::FileDialogOptions)(
				INativeDialogService::FileDialogDereferenceLinks |
				INativeDialogService::FileDialogFileMustExist
			)))
		{
			model->OpenSolution(fileNames[0]);
		}
	}

	void MainWindow::commandFileOpenWith_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto action = GetViewModel()->GetWorkingItem().Cast<vm::IOpenInEditorItemAction>();
		if (!action) return;
		// todo
	}

	void MainWindow::commandFileOpen_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto action = GetViewModel()->GetWorkingItem().Cast<vm::IOpenInEditorItemAction>();
		if (!action) return;
		// todo
	}

	void MainWindow::commandFileRemove_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto action = GetViewModel()->GetWorkingItem().Cast<vm::IRemoveItemAction>();
		if (!action) return;

		auto model = GetViewModel();
		model->SafeExecute([=]()
		{
			model->RemoveFile(action, model->GetWorkingItem());
		});
	}

	void MainWindow::commandFileRename_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto action = GetViewModel()->GetWorkingItem().Cast<vm::IRenameItemAction>();
		if (!action) return;
		auto window = new RenameFileWindow(GetViewModel(), GetViewModel()->GetWorkingItem(), action);
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();
		window->ShowModalAndDelete(this, [](){});
	}

	void MainWindow::commandFileSaveAll_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		// todo
	}

	void MainWindow::commandFileSave_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		// todo
	}

	void MainWindow::commandHelpAbout_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto window = new AboutWindow(GetViewModel());
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();
		window->ShowModalAndDelete(this, [](){});
	}

	void MainWindow::treeViewSolutionItem_NodeRightButtonUp(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiNodeMouseEventArgs& arguments)
	{
		toolstripMenuSolutionItem->ShowPopup(treeViewSolutionItem, Point(arguments.x, arguments.y));
	}

	void MainWindow::treeViewSolutionItem_SelectionChanged(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto item = description::UnboxValue<Ptr<vm::ISolutionItemModel>>(treeViewSolutionItem->GetSelectedItem());
		commandFileAddNewFile->SetEnabled(item.Cast<vm::IAddFileItemAction>());
		commandFileAddExistingFiles->SetEnabled(item.Cast<vm::IAddFileItemAction>());
		commandFileOpen->SetEnabled(item.Cast<vm::IOpenInEditorItemAction>());
		commandFileOpenWith->SetEnabled(item.Cast<vm::IOpenInEditorItemAction>());
		commandFileRename->SetEnabled(item.Cast<vm::IRenameItemAction>());
		commandFileRemove->SetEnabled(item.Cast<vm::IRemoveItemAction>());
		GetViewModel()->NotifySelectedSolutionItem(item);
	}

	// #endregion CLASS_MEMBER_GUIEVENT_HANDLER

	MainWindow::MainWindow(Ptr<vm::IStudioModel> ViewModel)
	{
		InitializeComponents(ViewModel);
		cellSplitter->SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::SizeWE));
	}
}
