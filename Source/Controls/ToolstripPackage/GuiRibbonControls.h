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

			class GuiRibbonTabPage;
			class GuiRibbonGroup;

			class GuiRibbonTab : public GuiTab, public Description<GuiRibbonTab>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonTabTemplate, GuiTab)
			protected:
				compositions::GuiBoundsComposition*					beforeHeaders = nullptr;
				compositions::GuiBoundsComposition*					afterHeaders = nullptr;
			public:
				GuiRibbonTab(theme::ThemeName themeName);
				~GuiRibbonTab();

				compositions::GuiGraphicsComposition*				GetBeforeHeaders();
				compositions::GuiGraphicsComposition*				GetAfterHeaders();
			};

			class GuiRibbonGroupCollection : public collections::ObservableListBase<GuiRibbonGroup*>
			{
			protected:
				GuiRibbonTabPage*									tabPage = nullptr;

				bool												QueryInsert(vint index, GuiRibbonGroup* const& value)override;
				void												AfterInsert(vint index, GuiRibbonGroup* const& value)override;
				void												AfterRemove(vint index, vint count)override;

			public:
				GuiRibbonGroupCollection(GuiRibbonTabPage* _tabPage);
				~GuiRibbonGroupCollection();
			};

			class GuiRibbonTabPage : public GuiTabPage, public Description<GuiRibbonTabPage>
			{
				friend class GuiRibbonGroupCollection;
			protected:
				bool												highlighted = false;
				GuiRibbonGroupCollection							groups;
				compositions::GuiResponsiveStackComposition*		responsiveStack = nullptr;
				compositions::GuiStackComposition*					stack = nullptr;

			public:
				GuiRibbonTabPage(theme::ThemeName themeName);
				~GuiRibbonTabPage();

				compositions::GuiNotifyEvent						HighlightedChanged;

				bool												GetHighlighted();
				void												SetHighlighted(bool value);

				collections::ObservableListBase<GuiRibbonGroup*>&	GetGroups();
			};

			class GuiRibbonGroup : public GuiControl, public Description<GuiRibbonGroup>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonGroupTemplate, GuiControl)
			protected:
				bool												expandable = false;

			public:
				GuiRibbonGroup(theme::ThemeName themeName);
				~GuiRibbonGroup();

				compositions::GuiNotifyEvent						ExpandableChanged;
				compositions::GuiNotifyEvent						ExpandButtonClicked;

				bool												GetExpandable();
				void												SetExpandable(bool value);
			};

/***********************************************************************
Ribbon Controls
***********************************************************************/
		}
	}
}

#endif
