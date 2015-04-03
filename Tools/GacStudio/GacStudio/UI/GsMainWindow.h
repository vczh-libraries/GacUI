/***********************************************************************
Vczh Library++ 3.0
Developer: 陈梓瀚(vczh)
GacUI::MainWindow

本文件使用Vczh GacUI Resource Code Generator工具自动生成
***********************************************************************/

#ifndef VCZH_GACUI_RESOURCE_CODE_GENERATOR_GacStudioUI_MainWindow
#define VCZH_GACUI_RESOURCE_CODE_GENERATOR_GacStudioUI_MainWindow

#include "GacStudioUIPartialClasses.h"

namespace ui
{
	class MainWindow : public MainWindow_<MainWindow>
	{
		friend class MainWindow_<MainWindow>;
		friend struct vl::reflection::description::CustomTypeDescriptorSelector<MainWindow>;
	protected:
		vint							splitterPosition = -1;
		bool							splitterDragging = false;

		// #region CLASS_MEMBER_GUIEVENT_HANDLER (DO NOT PUT OTHER CONTENT IN THIS #region.)
		void cellSplitter_leftButtonDown(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiMouseEventArgs& arguments);
		void cellSplitter_leftButtonUp(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiMouseEventArgs& arguments);
		void cellSplitter_mouseMove(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiMouseEventArgs& arguments);
		void commandFileAddExistingFiles_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandFileAddNewFile_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandFileCloseSolution_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandFileExit_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandFileNewProject_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandFileOpenProject_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandFileOpenWith_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandFileOpen_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandFileRemove_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandFileRename_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandFileSaveAll_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandFileSave_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void commandHelpAbout_Executed(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		void treeViewSolutionItem_NodeRightButtonUp(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiNodeMouseEventArgs& arguments);
		void treeViewSolutionItem_SelectionChanged(GuiGraphicsComposition* sender, vl::presentation::compositions::GuiEventArgs& arguments);
		// #endregion CLASS_MEMBER_GUIEVENT_HANDLER
	public:
		MainWindow(Ptr<vm::IStudioModel> ViewModel);
	};
}

#endif
