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
			using namespace theme;

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
GuiRibbonGroupCollection
***********************************************************************/

			bool GuiRibbonGroupCollection::QueryInsert(vint index, GuiRibbonGroup* const& value)
			{
				return !value->GetBoundsComposition()->GetParent();
			}

			void GuiRibbonGroupCollection::AfterInsert(vint index, GuiRibbonGroup* const& value)
			{
				value->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));

				auto item = new GuiStackItemComposition();
				item->AddChild(value->GetBoundsComposition());

				tabPage->stack->InsertStackItem(index, item);
			}

			void GuiRibbonGroupCollection::AfterRemove(vint index, vint count)
			{
				auto item = tabPage->stack->GetStackItems()[index];
				tabPage->stack->RemoveChild(item);

				item->RemoveChild(item->Children()[0]);
				delete item;
			}

			GuiRibbonGroupCollection::GuiRibbonGroupCollection(GuiRibbonTabPage* _tabPage)
				:tabPage(_tabPage)
			{
			}

			GuiRibbonGroupCollection::~GuiRibbonGroupCollection()
			{
			}

/***********************************************************************
GuiRibbonTabPage
***********************************************************************/

			GuiRibbonTabPage::GuiRibbonTabPage(theme::ThemeName themeName)
				:GuiTabPage(themeName)
				, groups(this)
			{
				stack = new GuiStackComposition();
				stack->SetDirection(GuiStackComposition::Horizontal);
				stack->SetAlignmentToParent(Margin(2, 2, 2, 2));
				stack->SetPadding(2);
				stack->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

				responsiveStack = new GuiResponsiveStackComposition();
				responsiveStack->SetDirection(ResponsiveDirection::Horizontal);
				responsiveStack->SetAlignmentToParent(Margin(0, 0, 0, 0));
				responsiveStack->AddChild(stack);

				responsiveContainer = new GuiResponsiveContainerComposition();
				responsiveContainer->SetAlignmentToParent(Margin(0, 0, 0, 0));
				responsiveContainer->SetResponsiveTarget(responsiveStack);

				containerComposition->AddChild(responsiveContainer);

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

			collections::ObservableListBase<GuiRibbonGroup*>& GuiRibbonTabPage::GetGroups()
			{
				return groups;
			}

/***********************************************************************
GuiRibbonGroupItemCollection
***********************************************************************/

			bool GuiRibbonGroupItemCollection::QueryInsert(vint index, GuiControl* const& value)
			{
				return !value->GetBoundsComposition()->GetParent();
			}

			void GuiRibbonGroupItemCollection::AfterInsert(vint index, GuiControl* const& value)
			{
				value->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));

				auto item = new GuiStackItemComposition();
				item->AddChild(value->GetBoundsComposition());

				group->stack->InsertStackItem(index, item);
			}

			void GuiRibbonGroupItemCollection::AfterRemove(vint index, vint count)
			{
				auto item = group->stack->GetStackItems()[index];
				group->stack->RemoveChild(item);

				item->RemoveChild(item->Children()[0]);
				delete item;
			}

			GuiRibbonGroupItemCollection::GuiRibbonGroupItemCollection(GuiRibbonGroup* _group)
				:group(_group)
			{
			}

			GuiRibbonGroupItemCollection::~GuiRibbonGroupItemCollection()
			{
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
				, items(this)
			{
				stack = new GuiStackComposition();
				stack->SetDirection(GuiStackComposition::Horizontal);
				stack->SetAlignmentToParent(Margin(0, 0, 0, 0));
				stack->SetPadding(2);
				stack->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

				responsiveStack = new GuiResponsiveStackComposition();
				responsiveStack->SetDirection(ResponsiveDirection::Horizontal);
				responsiveStack->SetAlignmentToParent(Margin(0, 0, 0, 0));
				responsiveStack->AddChild(stack);

				containerComposition->AddChild(responsiveStack);

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

			collections::ObservableListBase<GuiControl*>& GuiRibbonGroup::GetItems()
			{
				return items;
			}

/***********************************************************************
GuiRibbonButtons
***********************************************************************/

			GuiRibbonButtons::GuiRibbonButtons(theme::ThemeName themeName, RibbonButtonSize _maxSize, RibbonButtonSize _minSize)
				:GuiControl(themeName)
				, maxSize(_maxSize)
				, minSize(_minSize)
			{
				responsiveView = new GuiResponsiveViewComposition();
				responsiveView->SetDirection(ResponsiveDirection::Horizontal);
				responsiveView->SetAlignmentToParent(Margin(0, 0, 0, 0));

				button1 = new GuiToolstripButton(ThemeName::CustomControl);
				button2 = new GuiToolstripButton(ThemeName::CustomControl);
				button3 = new GuiToolstripButton(ThemeName::CustomControl);

				responsiveView->GetSharedControls().Add(button1);
				responsiveView->GetSharedControls().Add(button2);
				responsiveView->GetSharedControls().Add(button3);

				if (maxSize <= RibbonButtonSize::Large && RibbonButtonSize::Large <= minSize)
				{
				}

				if (maxSize <= RibbonButtonSize::Small && RibbonButtonSize::Small <= minSize)
				{
				}

				if (maxSize <= RibbonButtonSize::Icon && RibbonButtonSize::Icon <= minSize)
				{
				}

				containerComposition->AddChild(responsiveView);
			}

			GuiRibbonButtons::~GuiRibbonButtons()
			{
			}

#define GUIRIBBON_ACCESSOR(INDEX) \
			GuiToolstripCommand* GuiRibbonButtons::GetCommand##INDEX() \
			{ \
				return button##INDEX->GetCommand(); \
			} \
			void GuiRibbonButtons::SetCommand##INDEX(GuiToolstripCommand* value) \
			{ \
				button##INDEX->SetCommand(value); \
			} \
			GuiToolstripMenu* GuiRibbonButtons::GetSubMenu##INDEX() \
			{ \
				return button##INDEX->EnsureToolstripSubMenu(); \
			} \

			GUIRIBBON_ACCESSOR(1)
			GUIRIBBON_ACCESSOR(2)
			GUIRIBBON_ACCESSOR(3)

#undef GUIRIBBON_ACCESSOR
		}
	}
}