/***********************************************************************
Vczh Library++ 3.0
Developer: 陈梓瀚(vczh)
GacUI::NewFileWindow

本文件使用Vczh GacUI Resource Code Generator工具自动生成
***********************************************************************/

#include "GacStudioUI.h"
#include "..\..\Studio\SolutionModel.h"

using namespace vl::filesystem;
using namespace vm;
using namespace vl::reflection::description;

namespace ui
{
	// #region CLASS_MEMBER_GUIEVENT_HANDLER (DO NOT PUT OTHER CONTENT IN THIS #region.)

	void NewFileWindow::buttonCancel_Clicked(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		Close();
	}

	void NewFileWindow::buttonCreate_Clicked(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto model = GetViewModel();
		auto fileFactory = UnboxValue<Ptr<IFileFactoryModel>>(listViewFileTemplate->GetSelectedItem());
		if (model->SafeExecute([=]()
			{
				model->AddNewFile(
					GetAction(),
					fileFactory,
					textBoxLocation->GetText(),
					textBoxFileName->GetText()
					);
			}))
		{
			Close();
		}
	}

	// #endregion CLASS_MEMBER_GUIEVENT_HANDLER

	NewFileWindow::NewFileWindow(Ptr<vm::IStudioModel> ViewModel, Ptr<vm::IStudioNewFileModel> OperationModel, Ptr<vm::IAddFileItemAction> Action)
	{
		InitializeComponents(ViewModel, OperationModel, Action);
	}
}
