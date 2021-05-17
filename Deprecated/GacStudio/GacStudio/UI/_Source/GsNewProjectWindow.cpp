/***********************************************************************
Vczh Library++ 3.0
Developer: 陈梓瀚(vczh)
GacUI::NewProjectWindow

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

	void NewProjectWindow::buttonBrowse_Clicked(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
	}

	void NewProjectWindow::buttonCancel_Clicked(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		Close();
	}

	void NewProjectWindow::buttonCreate_Clicked(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments)
	{
		auto model = GetViewModel();
		auto projectFactory = UnboxValue<Ptr<IProjectFactoryModel>>(listViewProjectTemplate->GetSelectedItem());
		if (model->SafeExecute([=]()
			{
				model->AddNewProject(
					comboSolution->GetSelectedIndex() == 0,
					projectFactory,
					textBoxProjectName->GetText(),
					textBoxLocation->GetText(),
					textBoxSolutionName->GetText()
					);
			}))
		{
			Close();
		}
	}

	// #endregion CLASS_MEMBER_GUIEVENT_HANDLER

	NewProjectWindow::NewProjectWindow(Ptr<vm::IStudioModel> ViewModel)
	{
		InitializeComponents(ViewModel);
	}
}
