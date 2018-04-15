#include "GuiRibbonControls.h"
#include "../Templates/GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace compositions;

/***********************************************************************
GuiRibbonTab
***********************************************************************/

			void GuiRibbonTab::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject();
				if (auto bhc = ct->GetBeforeHeadersContainer())
				{
					bhc->RemoveChild(beforeHeaders);
				}
				if (auto ahc = ct->GetAfterHeadersContainer())
				{
					ahc->RemoveChild(afterHeaders);
				}
			}

			void GuiRibbonTab::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject();
				if (auto bhc = ct->GetBeforeHeadersContainer())
				{
					bhc->AddChild(beforeHeaders);
				}
				if (auto ahc = ct->GetAfterHeadersContainer())
				{
					ahc->AddChild(afterHeaders);
				}
			}

			GuiRibbonTab::GuiRibbonTab(theme::ThemeName themeName)
				:GuiTab(themeName)
			{
				beforeHeaders = new GuiBoundsComposition();
				beforeHeaders->SetAlignmentToParent(Margin(0, 0, 0, 0));
				beforeHeaders->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

				afterHeaders = new GuiBoundsComposition();
				afterHeaders->SetAlignmentToParent(Margin(0, 0, 0, 0));
				afterHeaders->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			}

			GuiRibbonTab::~GuiRibbonTab()
			{
				if (!beforeHeaders->GetParent())
				{
					SafeDeleteComposition(beforeHeaders);
				}
				if (!afterHeaders->GetParent())
				{
					SafeDeleteComposition(afterHeaders);
				}
			}

			compositions::GuiGraphicsComposition* GuiRibbonTab::GetBeforeHeaders()
			{
				return beforeHeaders;
			}

			compositions::GuiGraphicsComposition* GuiRibbonTab::GetAfterHeaders()
			{
				return afterHeaders;
			}

/***********************************************************************
GuiRibbonTabPage
***********************************************************************/

			GuiRibbonTabPage::GuiRibbonTabPage(theme::ThemeName themeName)
				:GuiTabPage(themeName)
			{
				HighlightedChanged.SetAssociatedComposition(boundsComposition);
			}

			GuiRibbonTabPage::~GuiRibbonTabPage()
			{
			}

			bool GuiRibbonTabPage::GetHighlighted()
			{
				return highlighted;
			}

			void GuiRibbonTabPage::SetHighlighted(bool value)
			{
				if (highlighted != value)
				{
					highlighted = value;
					HighlightedChanged.Execute(GetNotifyEventArguments());
				}
			}

/***********************************************************************
GuiRibbonGroup
***********************************************************************/

			void GuiRibbonGroup::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiRibbonGroup::AfterControlTemplateInstalled_(bool initialize)
			{
				GetControlTemplateObject()->SetExpandable(expandable);
			}

			GuiRibbonGroup::GuiRibbonGroup(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
				ExpandableChanged.SetAssociatedComposition(boundsComposition);
				ExpandButtonClicked.SetAssociatedComposition(boundsComposition);
			}

			GuiRibbonGroup::~GuiRibbonGroup()
			{
			}

			bool GuiRibbonGroup::GetExpandable()
			{
				return expandable;
			}

			void GuiRibbonGroup::SetExpandable(bool value)
			{
				if (expandable != value)
				{
					expandable = value;
					GetControlTemplateObject()->SetExpandable(expandable);
					ExpandableChanged.Execute(GetNotifyEventArguments());
				}
			}
		}
	}
}