#include "GuiRibbonControls.h"
#include "../Templates/GuiThemeStyleFactory.h"
#include "../../GraphicsComposition/GuiGraphicsStackComposition.h"
#include "../../GraphicsComposition/GuiGraphicsTableComposition.h"
#include "../../GraphicsComposition/GuiGraphicsResponsiveComposition.h"
#include "../../GraphicsComposition/GuiGraphicsSharedSizeComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace reflection::description;
			using namespace collections;
			using namespace compositions;
			using namespace theme;
			using namespace templates;

/***********************************************************************
GuiRibbonTab
***********************************************************************/

			void GuiRibbonTab::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject(false);
				if (!ct) return;

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
				auto ct = GetControlTemplateObject(true);
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
				for (vint i = 0; i < count; i++)
				{
					auto item = tabPage->stack->GetStackItems()[index];
					tabPage->stack->RemoveChild(item);

					item->RemoveChild(item->Children()[0]);
					delete item;
				}
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
				for (vint i = 0; i < count; i++)
				{
					auto item = group->stack->GetStackItems()[index];
					group->stack->RemoveChild(item);

					item->RemoveChild(item->Children()[0]);
					delete item;
				}
			}

			GuiRibbonGroupItemCollection::GuiRibbonGroupItemCollection(GuiRibbonGroup* _group)
				:group(_group)
			{
			}

			GuiRibbonGroupItemCollection::~GuiRibbonGroupItemCollection()
			{
			}

/***********************************************************************
GuiRibbonGroup::CommandExecutor
***********************************************************************/

			GuiRibbonGroup::CommandExecutor::CommandExecutor(GuiRibbonGroup* _group)
				:group(_group)
			{
			}

			GuiRibbonGroup::CommandExecutor::~CommandExecutor()
			{
			}

			void GuiRibbonGroup::CommandExecutor::NotifyExpandButtonClicked()
			{
				group->ExpandButtonClicked.Execute(group->GetNotifyEventArguments());
			}

/***********************************************************************
GuiRibbonGroupMenu
***********************************************************************/

			class GuiRibbonGroupMenu : public GuiMenu, public Description<GuiRibbonGroupMenu>
			{
			private:
				IGuiMenuService::Direction GetPreferredDirection()override
				{
					return IGuiMenuService::Horizontal;
				}

				bool IsActiveState()override
				{
					return false;
				}

			public:
				GuiRibbonGroupMenu(theme::ThemeName themeName, GuiControl* _owner)
					:GuiMenu(themeName, _owner)
				{
				}
			};

/***********************************************************************
GuiRibbonGroup
***********************************************************************/

			void GuiRibbonGroup::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject(false);
				if (!ct) return;

				ct->SetCommands(nullptr);
			}

			void GuiRibbonGroup::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				ct->SetExpandable(expandable);
				ct->SetCollapsed(responsiveView->GetCurrentView() == responsiveFixedButton);
				ct->SetCommands(commandExecutor.Obj());
				dropdownButton->SetControlTemplate(ct->GetLargeDropdownButtonTemplate());
				dropdownMenu->SetControlTemplate(ct->GetSubMenuTemplate());
			}

			void GuiRibbonGroup::OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				dropdownMenu->GetBoundsComposition()->SetPreferredMinSize(Size(0, containerComposition->GetBounds().Height()));
			}

			void GuiRibbonGroup::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				dropdownButton->SetText(GetText());
			}

			void GuiRibbonGroup::OnBeforeSubMenuOpening(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (responsiveView->GetViews().Contains(responsiveFixedButton))
				{
					auto currentDropdown = dropdownMenu;
					if (items.Count() == 1)
					{
						if (auto provider = items[0]->QueryTypedService<IGuiMenuDropdownProvider>())
						{
							if (auto menu = provider->ProvideDropdownMenu())
							{
								currentDropdown = menu;
							}
						}
					}
					dropdownButton->SetSubMenu(currentDropdown, false);
				}
			}

			void GuiRibbonGroup::OnBeforeSwitchingView(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments)
			{
				if (auto ct = GetControlTemplateObject(false))
				{
					ct->SetCollapsed(arguments.itemIndex == 1);
				}

				if (arguments.itemIndex == 0)
				{
					while (responsiveStack->LevelDown());
					dropdownMenu->GetContainerComposition()->RemoveChild(stack);
					responsiveStack->AddChild(stack);
					dropdownButton->SetSubMenu(nullptr, false);
				}
				else
				{
					while (responsiveStack->LevelUp());
					responsiveStack->RemoveChild(stack);
					dropdownMenu->GetContainerComposition()->AddChild(stack);
				}
			}

			GuiRibbonGroup::GuiRibbonGroup(theme::ThemeName themeName)
				:GuiControl(themeName)
				, items(this)
			{
				commandExecutor = new CommandExecutor(this);
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
				}
				{
					dropdownButton = new GuiToolstripButton(theme::ThemeName::RibbonLargeDropdownButton);
					dropdownButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));

					responsiveFixedButton = new GuiResponsiveFixedComposition();
					responsiveFixedButton->SetAlignmentToParent(Margin(0, 0, 0, 0));
					responsiveFixedButton->AddChild(dropdownButton->GetBoundsComposition());

					dropdownMenu = new GuiRibbonGroupMenu(theme::ThemeName::Menu, dropdownButton);
				}

				responsiveView = new GuiResponsiveViewComposition();
				responsiveView->SetAlignmentToParent(Margin(0, 0, 0, 0));
				responsiveView->GetViews().Add(responsiveStack);

				containerComposition->AddChild(responsiveView);

				ExpandableChanged.SetAssociatedComposition(boundsComposition);
				ExpandButtonClicked.SetAssociatedComposition(boundsComposition);
				LargeImageChanged.SetAssociatedComposition(boundsComposition);

				TextChanged.AttachMethod(this, &GuiRibbonGroup::OnTextChanged);
				boundsComposition->BoundsChanged.AttachMethod(this, &GuiRibbonGroup::OnBoundsChanged);
				responsiveView->BeforeSwitchingView.AttachMethod(this, &GuiRibbonGroup::OnBeforeSwitchingView);
				dropdownButton->BeforeSubMenuOpening.AttachMethod(this, &GuiRibbonGroup::OnBeforeSubMenuOpening);
			}

			GuiRibbonGroup::~GuiRibbonGroup()
			{
				if (!responsiveView->GetViews().Contains(responsiveFixedButton))
				{
					SafeDeleteComposition(responsiveFixedButton);
				}
				delete dropdownMenu;
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
					GetControlTemplateObject(true)->SetExpandable(expandable);
					ExpandableChanged.Execute(GetNotifyEventArguments());
				}
			}

			Ptr<GuiImageData> GuiRibbonGroup::GetLargeImage()
			{
				return largeImage;
			}

			void GuiRibbonGroup::SetLargeImage(Ptr<GuiImageData> value)
			{
				if (largeImage != value)
				{
					largeImage = value;
					dropdownButton->SetLargeImage(value);
					LargeImageChanged.Execute(GetNotifyEventArguments());

					if (value)
					{
						if (!responsiveView->GetViews().Contains(responsiveFixedButton))
						{
							responsiveView->GetViews().Add(responsiveFixedButton);
						}
					}
					else
					{
						if (responsiveView->GetViews().Contains(responsiveFixedButton))
						{
							responsiveView->GetViews().Remove(responsiveFixedButton);
						}
					}
				}
			}

			collections::ObservableListBase<GuiControl*>& GuiRibbonGroup::GetItems()
			{
				return items;
			}

/***********************************************************************
GuiRibbonIconLabel
***********************************************************************/

			void GuiRibbonIconLabel::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiRibbonIconLabel::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				ct->SetImage(image);
			}

			GuiRibbonIconLabel::GuiRibbonIconLabel(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
				ImageChanged.SetAssociatedComposition(boundsComposition);
			}

			GuiRibbonIconLabel::~GuiRibbonIconLabel()
			{
			}

			Ptr<GuiImageData> GuiRibbonIconLabel::GetImage()
			{
				return image;
			}

			void GuiRibbonIconLabel::SetImage(Ptr<GuiImageData> value)
			{
				if (image != value)
				{
					image = value;
					GetControlTemplateObject(true)->SetImage(image);
					ImageChanged.Execute(GetNotifyEventArguments());
				}
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

						stack->InsertStackItem(index, item);
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

			void GuiRibbonButtons::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiRibbonButtons::AfterControlTemplateInstalled_(bool initialize)
			{
				FOREACH(GuiControl*, button, buttons)
				{
					SetButtonThemeName(responsiveView->GetCurrentView(), button);
				}
			}

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
					case ThemeName::RibbonSmallIconLabel:
					case ThemeName::RibbonIconLabel:
						type = 3;
						break;
					default:;
					}

					if (type != -1)
					{
						ThemeName themeName = ThemeName::Unknown;
						TemplateProperty<GuiToolstripButtonTemplate> controlTemplate;

						if (fixed == views[(vint)RibbonButtonSize::Large])
						{
							switch (type)
							{
							case 0: themeName = ThemeName::RibbonLargeButton; break;
							case 1: themeName = ThemeName::RibbonLargeDropdownButton; break;
							case 2: themeName = ThemeName::RibbonLargeSplitButton; break;
							case 3: themeName = ThemeName::RibbonSmallIconLabel; break;
							}
						}
						else if (fixed == views[(vint)RibbonButtonSize::Small])
						{
							switch (type)
							{
							case 0: themeName = ThemeName::RibbonSmallButton; break;
							case 1: themeName = ThemeName::RibbonSmallDropdownButton; break;
							case 2: themeName = ThemeName::RibbonSmallSplitButton; break;
							case 3: themeName = ThemeName::RibbonSmallIconLabel; break;
							}
						}
						else if (fixed == views[(vint)RibbonButtonSize::Icon])
						{
							switch (type)
							{
							case 0: themeName = ThemeName::ToolstripButton; break;
							case 1: themeName = ThemeName::ToolstripDropdownButton; break;
							case 2: themeName = ThemeName::ToolstripSplitButton; break;
							case 3: themeName = ThemeName::RibbonIconLabel; break;
							}
						}

						if (auto ct = GetControlTemplateObject(false))
						{
							if (fixed == views[(vint)RibbonButtonSize::Large])
							{
								switch (type)
								{
								case 0: controlTemplate = ct->GetLargeButtonTemplate(); break;
								case 1: controlTemplate = ct->GetLargeDropdownButtonTemplate(); break;
								case 2: controlTemplate = ct->GetLargeSplitButtonTemplate(); break;
								case 3: controlTemplate = ct->GetSmallIconLabelTemplate(); break;
								}
							}
							else if (fixed == views[(vint)RibbonButtonSize::Small])
							{
								switch (type)
								{
								case 0: controlTemplate = ct->GetSmallButtonTemplate(); break;
								case 1: controlTemplate = ct->GetSmallDropdownButtonTemplate(); break;
								case 2: controlTemplate = ct->GetSmallSplitButtonTemplate(); break;
								case 3: controlTemplate = ct->GetSmallIconLabelTemplate(); break;
								}
							}
							else if (fixed == views[(vint)RibbonButtonSize::Icon])
							{
								switch (type)
								{
								case 0: controlTemplate = ct->GetIconButtonTemplate(); break;
								case 1: controlTemplate = ct->GetIconDropdownButtonTemplate(); break;
								case 2: controlTemplate = ct->GetIconSplitButtonTemplate(); break;
								case 3: controlTemplate = ct->GetIconLabelTemplate(); break;
								}
							}
						}

						button->SetControlThemeNameAndTemplate(themeName, controlTemplate);
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

				auto sharedSizeRootComposition = new GuiSharedSizeRootComposition();
				sharedSizeRootComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				sharedSizeRootComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				sharedSizeRootComposition->AddChild(responsiveView);

				containerComposition->AddChild(sharedSizeRootComposition);
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

			void GuiRibbonToolstrips::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiRibbonToolstrips::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				for (vint i = 0; i < ARRLEN(toolbars); i++)
				{
					toolbars[i]->SetControlTemplate(ct->GetToolbarTemplate());
				}
			}

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

				vint count = viewIndex == 0 ? 2 : 3;

				if (groups.Count() <= count)
				{
					auto containers = viewIndex == 0 ? longContainers : shortContainers;
					for (vint i = 0; i < groups.Count(); i++)
					{
						containers[i]->GetToolstripItems().Add(groups[i]);
					}
				}
				else if (count == 3)
				{
#define DELTA(POSTFIX) (abs(count1##POSTFIX - count2##POSTFIX) + abs(count2##POSTFIX - count3##POSTFIX) + abs(count3##POSTFIX - count1##POSTFIX))
#define DEFINE_COUNT(POSTFIX, OFFSET_FIRST, OFFSET_LAST) \
					vint count1##POSTFIX = count1_o + (OFFSET_FIRST); \
					vint count2##POSTFIX = count2_o - (OFFSET_FIRST) - (OFFSET_LAST); \
					vint count3##POSTFIX = count3_o + (OFFSET_LAST)
#define MIN(a, b) (a)<(b)?(a):(b)

					vint firstGroupCount = 0;
					vint lastGroupCount = 0;

					vint count1_o = 0;
					vint count2_o = From(groups)
						.Select([](GuiToolstripGroup* group) {return group->GetToolstripItems().Count(); })
						.Aggregate([](vint a, vint b) {return a + b; });
					vint count3_o = 0;
					vint delta_o = DELTA(_o);

					while (firstGroupCount + lastGroupCount < groups.Count())
					{
						auto newFirstGroup = groups[firstGroupCount];
						auto newLastGroup = groups[groups.Count() - lastGroupCount - 1];

						DEFINE_COUNT(_f, newFirstGroup->GetToolstripItems().Count(), 0);
						vint delta_f = DELTA(_f);

						DEFINE_COUNT(_l, 0, newLastGroup->GetToolstripItems().Count());
						vint delta_l = DELTA(_l);

						vint delta = MIN(delta_o, MIN(delta_f, delta_l));
						if (delta == delta_f)
						{
							firstGroupCount++;
							count1_o = count1_f;
							count2_o = count2_f;
							count3_o = count3_f;
							delta_o = delta_f;
						}
						else if (delta == delta_l)
						{
							lastGroupCount++;
							count1_o = count1_l;
							count2_o = count2_l;
							count3_o = count3_l;
							delta_o = delta_l;
						}
						else
						{
							break;
						}
					}

					vint minMiddle = firstGroupCount;
					vint maxMiddle = groups.Count() - lastGroupCount - 1;
					for (vint j = 0; j < groups.Count(); j++)
					{
						shortContainers[
							j < minMiddle ? 0 :
							j>maxMiddle ? 2 :
							1
						]->GetToolstripItems().Add(groups[j]);
					}

#undef MIN
#undef DEFINE_COUNT
#undef DELTA
				}
				else if (count == 2)
				{
					vint firstGroupCount = groups.Count();
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
								firstGroupCount = i;
								break;
							}

							count1 = count1_2;
							count2 = count2_2;
							delta = delta_2;
						}
					}

					for (vint j = 0; j < groups.Count(); j++)
					{
						longContainers[j < firstGroupCount ? 0 : 1]->GetToolstripItems().Add(groups[j]);
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

				vint toolbarIndex = 0;
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
						auto toolbar = new GuiToolstripToolBar(theme::ThemeName::ToolstripToolBar);
						toolbar->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
						toolbars[toolbarIndex++] = toolbar;

						auto cell = new GuiCellComposition();
						cell->SetSite(j * 2 + 1, 0, 1, 1);
						cell->AddChild(toolbar->GetBoundsComposition());
						table->AddChild(cell);

						auto container = new GuiToolstripGroupContainer(theme::ThemeName::CustomControl);
						toolbar->GetToolstripItems().Add(container);
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

/***********************************************************************
GuiRibbonGallery::CommandExecutor
***********************************************************************/

			GuiRibbonGallery::CommandExecutor::CommandExecutor(GuiRibbonGallery* _gallery)
				:gallery(_gallery)
			{
			}

			GuiRibbonGallery::CommandExecutor::~CommandExecutor()
			{
			}

			void GuiRibbonGallery::CommandExecutor::NotifyScrollUp()
			{
				gallery->RequestedScrollUp.Execute(gallery->GetNotifyEventArguments());
			}

			void GuiRibbonGallery::CommandExecutor::NotifyScrollDown()
			{
				gallery->RequestedScrollDown.Execute(gallery->GetNotifyEventArguments());
			}

			void GuiRibbonGallery::CommandExecutor::NotifyDropdown()
			{
				gallery->RequestedDropdown.Execute(gallery->GetNotifyEventArguments());
			}

/***********************************************************************
GuiRibbonGallery
***********************************************************************/

			void GuiRibbonGallery::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject(false);
				if (!ct) return;

				ct->SetCommands(nullptr);
			}

			void GuiRibbonGallery::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				ct->SetCommands(commandExecutor.Obj());
				ct->SetScrollUpEnabled(scrollUpEnabled);
				ct->SetScrollDownEnabled(scrollDownEnabled);
			}

			GuiRibbonGallery::GuiRibbonGallery(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
				commandExecutor = new CommandExecutor(this);

				ScrollUpEnabledChanged.SetAssociatedComposition(boundsComposition);
				ScrollDownEnabledChanged.SetAssociatedComposition(boundsComposition);
				RequestedScrollUp.SetAssociatedComposition(boundsComposition);
				RequestedScrollDown.SetAssociatedComposition(boundsComposition);
				RequestedDropdown.SetAssociatedComposition(boundsComposition);
			}

			GuiRibbonGallery::~GuiRibbonGallery()
			{
			}

			bool GuiRibbonGallery::GetScrollUpEnabled()
			{
				return scrollUpEnabled;
			}

			void GuiRibbonGallery::SetScrollUpEnabled(bool value)
			{
				if (scrollUpEnabled != value)
				{
					scrollUpEnabled = value;
					GetControlTemplateObject(true)->SetScrollUpEnabled(value);
				}
			}

			bool GuiRibbonGallery::GetScrollDownEnabled()
			{
				return scrollDownEnabled;
			}

			void GuiRibbonGallery::SetScrollDownEnabled(bool value)
			{
				if (scrollDownEnabled != value)
				{
					scrollDownEnabled = value;
					GetControlTemplateObject(true)->SetScrollDownEnabled(value);
				}
			}

/***********************************************************************
GuiRibbonToolstripMenu
***********************************************************************/

			void GuiRibbonToolstripMenu::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject(false);
				if (!ct) return;

				if (auto cc = ct->GetContentComposition())
				{
					cc->RemoveChild(contentComposition);
				}
			}

			void GuiRibbonToolstripMenu::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				if (auto cc = ct->GetContentComposition())
				{
					cc->AddChild(contentComposition);
				}
			}

			GuiRibbonToolstripMenu::GuiRibbonToolstripMenu(theme::ThemeName themeName, GuiControl* owner)
				:GuiToolstripMenu(themeName, owner)
			{
				contentComposition = new GuiBoundsComposition();
				contentComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				contentComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			}

			GuiRibbonToolstripMenu::~GuiRibbonToolstripMenu()
			{
				if (!contentComposition->GetParent())
				{
					SafeDeleteComposition(contentComposition);
				}
			}

			compositions::GuiGraphicsComposition* GuiRibbonToolstripMenu::GetContentComposition()
			{
				return contentComposition;
			}
		}
	}
}
