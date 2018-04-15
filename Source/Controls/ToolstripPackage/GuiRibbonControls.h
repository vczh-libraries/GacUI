/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIRIBBONCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIRIBBONCONTROLS

#include "GuiToolstripMenu.h"
#include "../GuiContainerControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
Ribbon Containers
***********************************************************************/

			class GuiRibbonTab : public GuiTab, public Description<GuiRibbonTab>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonTabTemplate, GuiRibbonTab)
			protected:
				compositions::GuiBoundsComposition*		beforeHeaders = nullptr;
				compositions::GuiBoundsComposition*		afterHeaders = nullptr;
			public:
				GuiRibbonTab(theme::ThemeName themeName);
				~GuiRibbonTab();

				compositions::GuiGraphicsComposition*	GetBeforeHeaders();
				compositions::GuiGraphicsComposition*	GetAfterHeaders();
			};

			class GuiRibbonTabPage : public GuiTabPage, public Description<GuiRibbonTabPage>
			{
			protected:
				bool									highlighted = false;

			public:
				GuiRibbonTabPage(theme::ThemeName themeName);
				~GuiRibbonTabPage();

				compositions::GuiNotifyEvent			HighlightedChanged;

				bool									GetHighlighted();
				void									SetHighlighted(bool value);
			};

			class GuiRibbonGroup : public GuiControl, public Description<GuiRibbonGroup>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonGroupTemplate, GuiRibbonGroup)
			protected:
				bool									expandable = false;

			public:
				GuiRibbonGroup(theme::ThemeName themeName);
				~GuiRibbonGroup();

				compositions::GuiNotifyEvent			ExpandableChanged;
				compositions::GuiNotifyEvent			ExpandButtonClicked;

				bool									GetExpandable();
				void									SetExpandable(bool value);
			};

/***********************************************************************
Ribbon Controls
***********************************************************************/
		}
	}
}

#endif
