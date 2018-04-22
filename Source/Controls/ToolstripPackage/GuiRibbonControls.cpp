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
GuiRibbonButtonsItemCollection
***********************************************************************/

			bool GuiRibbonButtonsItemCollection::QueryInsert(vint index, GuiControl* const& value)
			{
				return !value->GetBoundsComposition()->GetParent();
			}

			void GuiRibbonButtonsItemCollection::AfterInsert(vint index, GuiControl* const& value)
			{
				buttons->responsiveView->GetSharedControls().Add(value);
				buttons->SetButtonThemeName(buttons->responsiveView->GetCurrentView(), value);

				for (vint i = 0; i < sizeof(buttons->views) / sizeof(*buttons->views); i++)
				{
					if (auto view = buttons->views[i])
					{
						auto stack = dynamic_cast<GuiStackComposition*>(view->Children()[0]);

						auto shared = new GuiResponsiveSharedComposition();
						shared->SetAlignmentToParent(Margin(0, 0, 0, 0));
						shared->SetShared(value);

						auto item = new GuiStackItemComposition();
						item->AddChild(shared);

						stack->InsertChild(index, item);
					}
				}
			}

			void GuiRibbonButtonsItemCollection::BeforeRemove(vint index, GuiControl* const& value)
			{
				CHECK_FAIL(L"GuiRibbonButtonsItemCollection::BeforeRemove(vint, GuiControl* const&)#Controls are not allowed to be removed from GuiRibbonButtons.");
			}

			GuiRibbonButtonsItemCollection::GuiRibbonButtonsItemCollection(GuiRibbonButtons* _buttons)
				:buttons(_buttons)
			{
			}

			GuiRibbonButtonsItemCollection::~GuiRibbonButtonsItemCollection()
			{
			}

/***********************************************************************
GuiRibbonButtons
***********************************************************************/

			void GuiRibbonButtons::OnBeforeSwitchingView(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments)
			{
				FOREACH(GuiControl*, button, buttons)
				{
					SetButtonThemeName(responsiveView->GetViews()[arguments.itemIndex], button);
				}
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
						type = 2;
						break;
					}

					if (fixed == views[(vint)RibbonButtonSize::Large])
					{
						switch (type)
						{
						case 0: button->SetControlThemeName(ThemeName::RibbonLargeButton); break;
						case 1: button->SetControlThemeName(ThemeName::RibbonLargeDropdownButton); break;
						case 2: button->SetControlThemeName(ThemeName::RibbonLargeSplitButton); break;
						}
					}
					else if (fixed == views[(vint)RibbonButtonSize::Small])
					{
						switch (type)
						{
						case 0: button->SetControlThemeName(ThemeName::RibbonSmallButton); break;
						case 1: button->SetControlThemeName(ThemeName::RibbonSmallDropdownButton); break;
						case 2: button->SetControlThemeName(ThemeName::RibbonSmallSplitButton); break;
						}
					}
					else if (fixed == views[(vint)RibbonButtonSize::Icon])
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
				, buttons(this)
			{
				responsiveView = new GuiResponsiveViewComposition();
				responsiveView->SetDirection(ResponsiveDirection::Horizontal);
				responsiveView->SetAlignmentToParent(Margin(0, 0, 0, 0));
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

				for (vint i = 0; i < sizeof(views) / sizeof(*views); i++)
				{
					if ((vint)maxSize <= i && i <= (vint)minSize)
					{
						auto stack = new GuiStackComposition();
						stack->SetAlignmentToParent(Margin(0, 0, 0, 0));
						stack->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						stack->SetDirection(i == 0 ? GuiStackComposition::Horizontal : GuiStackComposition::Vertical);

						views[i] = new GuiResponsiveFixedComposition();
						views[i]->AddChild(stack);
						responsiveView->GetViews().Add(views[i]);
					}
				}

				containerComposition->AddChild(responsiveView);
			}

			GuiRibbonButtons::~GuiRibbonButtons()
			{
			}

			collections::ObservableListBase<GuiControl*>& GuiRibbonButtons::GetButtons()
			{
				return buttons;
			}

/***********************************************************************
GuiRibbonToolstripsGroupCollection
***********************************************************************/

			bool GuiRibbonToolstripsGroupCollection::QueryInsert(vint index, GuiToolstripGroup* const& value)
			{
				return !value->GetBoundsComposition()->GetParent();
			}

			void GuiRibbonToolstripsGroupCollection::AfterInsert(vint index, GuiToolstripGroup* const& value)
			{
				toolstrips->RearrangeToolstripGroups();
			}

			void GuiRibbonToolstripsGroupCollection::AfterRemove(vint index, vint count)
			{
				toolstrips->RearrangeToolstripGroups();
			}

			GuiRibbonToolstripsGroupCollection::GuiRibbonToolstripsGroupCollection(GuiRibbonToolstrips* _toolstrips)
				:toolstrips(_toolstrips)
			{
			}

			GuiRibbonToolstripsGroupCollection::~GuiRibbonToolstripsGroupCollection()
			{
			}

/***********************************************************************
GuiRibbonToolstrips
***********************************************************************/

#define ARRLEN(X) sizeof(X) / sizeof(*X)

			void GuiRibbonToolstrips::OnBeforeSwitchingView(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments)
			{
				RearrangeToolstripGroups(arguments.itemIndex);
			}

			void GuiRibbonToolstrips::RearrangeToolstripGroups(vint viewIndex)
			{
				static_assert(ARRLEN(longContainers) == 2, "");
				static_assert(ARRLEN(shortContainers) == 3, "");

				if (viewIndex == -1)
				{
					viewIndex = responsiveView->GetViews().IndexOf(responsiveView->GetCurrentView());
				}

				for (vint i = 0; i < ARRLEN(longContainers); i++)
				{
					longContainers[i]->GetToolstripItems().Clear();
				}
				for (vint i = 0; i < ARRLEN(shortContainers); i++)
				{
					shortContainers[i]->GetToolstripItems().Clear();
				}

				auto containers = viewIndex == 0 ? longContainers : shortContainers;
				vint count = viewIndex == 0 ? 2 : 3;

				if (groups.Count() <= count)
				{
					for (vint i = 0; i < groups.Count(); i++)
					{
						containers[i]->GetToolstripItems().Add(groups[i]);
					}
				}
				else if (count == 3)
				{
				}
				else if (count == 2)
				{
					vint count1 = 0;
					vint count2 = From(groups)
						.Select([](GuiToolstripGroup* group) {return group->GetToolstripItems().Count(); })
						.Aggregate([](vint a, vint b) {return a + b; });
					vint delta = abs(count2 - count1);

					for (vint i = 0; i < groups.Count(); i++)
					{
						auto groupCount = groups[i]->GetToolstripItems().Count();
						vint count1_2 = count1 + groupCount;
						vint count2_2 = count2 - groupCount;
						vint delta_2 = abs(count2_2 - count1_2);
						if (delta < delta_2)
						{
							for (vint j = 0; j < groups.Count(); j++)
							{
								shortContainers[j <= i ? 0 : 1]->GetToolstripItems().Add(groups[j]);
							}
							break;
						}
					}
				}
			}

			GuiRibbonToolstrips::GuiRibbonToolstrips(theme::ThemeName themeName)
				:GuiControl(themeName)
				, groups(this)
			{
				responsiveView = new GuiResponsiveViewComposition();
				responsiveView->SetDirection(ResponsiveDirection::Horizontal);
				responsiveView->SetAlignmentToParent(Margin(0, 0, 0, 0));
				responsiveView->BeforeSwitchingView.AttachMethod(this, &GuiRibbonToolstrips::OnBeforeSwitchingView);

				for (vint i = 0; i < sizeof(views) / sizeof(*views); i++)
				{
					auto containers = i == 0 ? longContainers : shortContainers;
					vint count = i == 0 ? 2 : 3;

					auto table = new GuiTableComposition();
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					
					table->SetRowsAndColumns(count * 2 + 1, 1);
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());
					table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
					for (vint j = 0; j < count; j++)
					{
						table->SetRowOption(j * 2 + 1, GuiCellOption::MinSizeOption());
						table->SetRowOption(j * 2 + 2, GuiCellOption::PercentageOption(1.0));
					}

					for (vint j = 0; j < count; j++)
					{
						auto toolstrip = new GuiToolstripToolBar(theme::ThemeName::ToolstripToolBar);
						toolstrip->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));

						auto cell = new GuiCellComposition();
						cell->SetSite(j * 2 + 1, 1, 1, 1);
						cell->AddChild(toolstrip->GetBoundsComposition());

						auto container = new GuiToolstripGroupContainer(theme::ThemeName::CustomControl);
						toolstrip->GetToolstripItems().Add(container);
						containers[j] = container;
					}

					views[i] = new GuiResponsiveFixedComposition();
					views[i]->AddChild(table);
					responsiveView->GetViews().Add(views[i]);
				}

				containerComposition->AddChild(responsiveView);
			}

			GuiRibbonToolstrips::~GuiRibbonToolstrips()
			{
			}

			collections::ObservableListBase<GuiToolstripGroup*>& GuiRibbonToolstrips::GetGroups()
			{
				return groups;
			}

#undef ARRLEN
		}
	}
}