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

			void GuiRibbonButtons::OnBeforeSwitchingView(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments)
			{
				SetButtonThemeName(responsiveView->GetViews()[arguments.itemIndex], button1);
				SetButtonThemeName(responsiveView->GetViews()[arguments.itemIndex], button2);
				SetButtonThemeName(responsiveView->GetViews()[arguments.itemIndex], button3);
			}

			void GuiRibbonButtons::SetButtonThemeName(compositions::GuiResponsiveCompositionBase* fixed, GuiControl* button)
			{
				if (fixed && button)
				{
					auto themeName = button->GetControlThemeName();
					vint type = -1;
					switch (themeName)
					{
					case ThemeName::RibbonLargeButton:
					case ThemeName::RibbonSmallButton:
					case ThemeName::ToolstripButton:
						type = 0;
						break;
					case ThemeName::RibbonLargeDropdownButton:
					case ThemeName::RibbonSmallDropdownButton:
					case ThemeName::ToolstripDropdownButton:
						type = 1;
						break;
					case ThemeName::RibbonLargeSplitButton:
					case ThemeName::RibbonSmallSplitButton:
					case ThemeName::ToolstripSplitButton:
						type = 0;
						break;
					}

					if (fixed == fixedLarge)
					{
						switch (type)
						{
						case 0: button->SetControlThemeName(ThemeName::RibbonLargeButton); break;
						case 1: button->SetControlThemeName(ThemeName::RibbonLargeDropdownButton); break;
						case 2: button->SetControlThemeName(ThemeName::RibbonLargeSplitButton); break;
						}
					}
					else if (fixed == fixedSmall)
					{
						switch (type)
						{
						case 0: button->SetControlThemeName(ThemeName::RibbonSmallButton); break;
						case 1: button->SetControlThemeName(ThemeName::RibbonSmallDropdownButton); break;
						case 2: button->SetControlThemeName(ThemeName::RibbonSmallSplitButton); break;
						}
					}
					else if (fixed == fixedIcon)
					{
						switch (type)
						{
						case 0: button->SetControlThemeName(ThemeName::ToolstripButton); break;
						case 1: button->SetControlThemeName(ThemeName::ToolstripDropdownButton); break;
						case 2: button->SetControlThemeName(ThemeName::ToolstripSplitButton); break;
						}
					}
				}
			}

			GuiRibbonButtons::GuiRibbonButtons(theme::ThemeName themeName, RibbonButtonSize _maxSize, RibbonButtonSize _minSize)
				:GuiControl(themeName)
				, maxSize(_maxSize)
				, minSize(_minSize)
			{
				responsiveView = new GuiResponsiveViewComposition();
				responsiveView->SetDirection(ResponsiveDirection::Horizontal);
				responsiveView->SetAlignmentToParent(Margin(0, 0, 0, 0));

				buttonContainer1 = new GuiControl(ThemeName::CustomControl);
				buttonContainer2 = new GuiControl(ThemeName::CustomControl);
				buttonContainer3 = new GuiControl(ThemeName::CustomControl);

				buttonContainer1->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				buttonContainer2->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				buttonContainer3->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));

				responsiveView->GetSharedControls().Add(buttonContainer1);
				responsiveView->GetSharedControls().Add(buttonContainer2);
				responsiveView->GetSharedControls().Add(buttonContainer3);
				responsiveView->BeforeSwitchingView.AttachMethod(this, &GuiRibbonButtons::OnBeforeSwitchingView);

				auto installButton = [&](GuiTableComposition* table, vint row, vint column, GuiControl* buttonContainer)
				{
					auto shared = new GuiResponsiveSharedComposition();
					shared->SetAlignmentToParent(Margin(0, 0, 0, 0));
					shared->SetShared(buttonContainer);

					auto cell = new GuiCellComposition();
					cell->SetSite(row, column, 1, 1);
					cell->AddChild(shared);

					table->AddChild(cell);
				};

				if (maxSize <= RibbonButtonSize::Large && RibbonButtonSize::Large <= minSize)
				{
					auto table = new GuiTableComposition();
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					table->SetRowsAndColumns(1, 3);
					table->SetCellPadding(2);
					table->SetBorderVisible(false);

					table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());
					table->SetColumnOption(1, GuiCellOption::MinSizeOption());
					table->SetColumnOption(2, GuiCellOption::MinSizeOption());

					installButton(table, 0, 0, buttonContainer1);
					installButton(table, 0, 1, buttonContainer2);
					installButton(table, 0, 2, buttonContainer3);

					fixedLarge = new GuiResponsiveFixedComposition();
					fixedLarge->AddChild(table);
					responsiveView->GetViews().Add(fixedLarge);
				}

				if (maxSize <= RibbonButtonSize::Small && RibbonButtonSize::Small <= minSize)
				{
					auto table = new GuiTableComposition();
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					table->SetRowsAndColumns(5, 1);
					table->SetCellPadding(2);
					table->SetBorderVisible(false);

					table->SetRowOption(0, GuiCellOption::MinSizeOption());
					table->SetRowOption(1, GuiCellOption::PercentageOption(1.0));
					table->SetRowOption(2, GuiCellOption::MinSizeOption());
					table->SetRowOption(3, GuiCellOption::PercentageOption(1.0));
					table->SetRowOption(4, GuiCellOption::MinSizeOption());
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());

					installButton(table, 0, 0, buttonContainer1);
					installButton(table, 2, 0, buttonContainer2);
					installButton(table, 4, 0, buttonContainer3);

					fixedSmall = new GuiResponsiveFixedComposition();
					fixedSmall->AddChild(table);
					responsiveView->GetViews().Add(fixedSmall);
				}

				if (maxSize <= RibbonButtonSize::Icon && RibbonButtonSize::Icon <= minSize)
				{
					auto table = new GuiTableComposition();
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					table->SetRowsAndColumns(5, 1);
					table->SetCellPadding(2);
					table->SetBorderVisible(false);

					table->SetRowOption(0, GuiCellOption::MinSizeOption());
					table->SetRowOption(1, GuiCellOption::PercentageOption(1.0));
					table->SetRowOption(2, GuiCellOption::MinSizeOption());
					table->SetRowOption(3, GuiCellOption::PercentageOption(1.0));
					table->SetRowOption(4, GuiCellOption::MinSizeOption());
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());

					installButton(table, 0, 0, buttonContainer1);
					installButton(table, 2, 0, buttonContainer2);
					installButton(table, 4, 0, buttonContainer3);

					fixedIcon = new GuiResponsiveFixedComposition();
					fixedIcon->AddChild(table);
					responsiveView->GetViews().Add(fixedIcon);
				}

				containerComposition->AddChild(responsiveView);
			}

			GuiRibbonButtons::~GuiRibbonButtons()
			{
			}

#define GUIRIBBON_ACCESSOR(INDEX) \
			GuiControl* GuiRibbonButtons::GetButton##INDEX() \
			{ \
				return button##INDEX; \
			} \
			GuiControl* GuiRibbonButtons::SetButton##INDEX(GuiControl* value) \
			{ \
				auto oldButton = button##INDEX; \
				if (button##INDEX) \
				{ \
					buttonContainer##INDEX->GetContainerComposition()->RemoveChild(button##INDEX->GetBoundsComposition()); \
				} \
				button##INDEX = value; \
				if (button##INDEX) \
				{ \
					button##INDEX->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0)); \
					buttonContainer##INDEX->GetContainerComposition()->AddChild(button##INDEX->GetBoundsComposition()); \
					SetButtonThemeName(responsiveView->GetCurrentView(), button##INDEX); \
				} \
				return button##INDEX; \
			} \

			GUIRIBBON_ACCESSOR(1)
			GUIRIBBON_ACCESSOR(2)
			GUIRIBBON_ACCESSOR(3)

#undef GUIRIBBON_ACCESSOR
		}
	}
}