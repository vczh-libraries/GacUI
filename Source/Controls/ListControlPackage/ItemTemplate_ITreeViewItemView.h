/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_ITEMTEMPLATE_ITREEVIEWITEMVIEW
#define VCZH_PRESENTATION_CONTROLS_ITEMTEMPLATE_ITREEVIEWITEMVIEW

#include "GuiListControls.h"

namespace vl::presentation::controls::tree
{
/***********************************************************************
DefaultTreeItemTemplate
***********************************************************************/

	class DefaultTreeItemTemplate : public list::PredefinedListItemTemplate<templates::GuiTreeItemTemplate>
	{
	protected:
		GuiSelectableButton*					expandingButton = nullptr;
		compositions::GuiTableComposition*		table = nullptr;
		elements::GuiImageFrameElement*			imageElement = nullptr;
		elements::GuiSolidLabelElement*			textElement = nullptr;

		void									OnInitialize()override;
		void									OnRefresh()override;
		void									OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
		void									OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
		void									OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
		void									OnExpandingChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
		void									OnExpandableChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
		void									OnLevelChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
		void									OnImageChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
		void									OnExpandingButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
		void									OnExpandingButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
	public:
		DefaultTreeItemTemplate();
		~DefaultTreeItemTemplate();
	};
}

#endif