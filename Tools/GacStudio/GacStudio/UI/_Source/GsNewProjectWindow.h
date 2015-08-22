/***********************************************************************
Vczh Library++ 3.0
Developer: 陈梓瀚(vczh)
GacUI::NewProjectWindow

本文件使用Vczh GacUI Resource Code Generator工具自动生成
***********************************************************************/

#ifndef VCZH_GACUI_RESOURCE_CODE_GENERATOR_GacStudioUI_NewProjectWindow
#define VCZH_GACUI_RESOURCE_CODE_GENERATOR_GacStudioUI_NewProjectWindow

#include "GacStudioUIPartialClasses.h"

namespace ui
{
	class NewProjectWindow : public NewProjectWindow_<NewProjectWindow>
	{
		friend class NewProjectWindow_<NewProjectWindow>;
		friend struct vl::reflection::description::CustomTypeDescriptorSelector<NewProjectWindow>;
	protected:

		// #region CLASS_MEMBER_GUIEVENT_HANDLER (DO NOT PUT OTHER CONTENT IN THIS #region.)
		void buttonBrowse_Clicked(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void buttonCancel_Clicked(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void buttonCreate_Clicked(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		// #endregion CLASS_MEMBER_GUIEVENT_HANDLER
	public:
		NewProjectWindow(Ptr<vm::IStudioModel> ViewModel);
	};
}

#endif
