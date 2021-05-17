/***********************************************************************
Vczh Library++ 3.0
Developer: 陈梓瀚(vczh)
GacUI::NewFileWindow

本文件使用Vczh GacUI Resource Code Generator工具自动生成
***********************************************************************/

#ifndef VCZH_GACUI_RESOURCE_CODE_GENERATOR_GacStudioUI_NewFileWindow
#define VCZH_GACUI_RESOURCE_CODE_GENERATOR_GacStudioUI_NewFileWindow

#include "GacStudioUIPartialClasses.h"

namespace ui
{
	class NewFileWindow : public NewFileWindow_<NewFileWindow>
	{
		friend class NewFileWindow_<NewFileWindow>;
		friend struct vl::reflection::description::CustomTypeDescriptorSelector<NewFileWindow>;
	protected:

		// #region CLASS_MEMBER_GUIEVENT_HANDLER (DO NOT PUT OTHER CONTENT IN THIS #region.)
		void buttonCancel_Clicked(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void buttonCreate_Clicked(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		// #endregion CLASS_MEMBER_GUIEVENT_HANDLER
	public:
		NewFileWindow(Ptr<vm::IStudioModel> ViewModel, Ptr<vm::IStudioNewFileModel> OperationModel, Ptr<vm::IAddFileItemAction> Action);
	};
}

#endif
