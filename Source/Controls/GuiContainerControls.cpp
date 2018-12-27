#include "GuiContainerControls.h"

/* CodePack:BeginIgnore() */
#ifndef VCZH_DEBUG_NO_REFLECTION
/* CodePack:ConditionOff(VCZH_DEBUG_NO_REFLECTION, ../Reflection/TypeDescriptors/GuiReflectionPlugin.h) */
#include "../Reflection/TypeDescriptors/GuiReflectionPlugin.h"
#endif
/* CodePack:EndIgnore() */

namespace vl
{
	namespace presentation
	{
		using namespace compositions;

		namespace controls
		{

/***********************************************************************
GuiTabPage
***********************************************************************/

			bool GuiTabPage::IsAltAvailable()
			{
				return false;
			}

			GuiTabPage::GuiTabPage(theme::ThemeName themeName)
				:GuiCustomControl(themeName)
			{
			}

			GuiTabPage::~GuiTabPage()
			{
				FinalizeAggregation();
			}

			GuiTab* GuiTabPage::GetOwnerTab()
			{
				return tab;
			}

/***********************************************************************
GuiTabPageList
***********************************************************************/

			bool GuiTabPageList::QueryInsert(vint index, GuiTabPage* const& value)
			{
				return !items.Contains(value) && value->tab == nullptr;
			}

			void GuiTabPageList::AfterInsert(vint index, GuiTabPage* const& value)
			{
				value->tab = tab;
				value->SetVisible(false);
				value->boundsComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				tab->containerComposition->AddChild(value->boundsComposition);

				if (!tab->selectedPage)
				{
					tab->SetSelectedPage(value);
				}
			}

			void GuiTabPageList::BeforeRemove(vint index, GuiTabPage* const& value)
			{
				tab->containerComposition->RemoveChild(value->boundsComposition);
				value->tab = nullptr;

				if (items.Count() == 0)
				{
					tab->SetSelectedPage(nullptr);
				}
				else if (tab->selectedPage == value)
				{
					tab->SetSelectedPage(items[0]);
				}
			}

			GuiTabPageList::GuiTabPageList(GuiTab* _tab)
				:tab(_tab)
			{
			}

			GuiTabPageList::~GuiTabPageList()
			{
			}

/***********************************************************************
GuiTab::CommandExecutor
***********************************************************************/

			GuiTab::CommandExecutor::CommandExecutor(GuiTab* _tab)
				:tab(_tab)
			{
			}

			GuiTab::CommandExecutor::~CommandExecutor()
			{
			}

			void GuiTab::CommandExecutor::ShowTab(vint index, bool setFocus)
			{
				tab->SetSelectedPage(tab->GetPages().Get(index));
				if (setFocus)
				{
					tab->SetFocus();
				}
			}

/***********************************************************************
GuiTab
***********************************************************************/

			void GuiTab::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject(false);
				if (!ct) return;

				ct->SetCommands(nullptr);
				ct->SetTabPages(nullptr);
				ct->SetSelectedTabPage(nullptr);
			}

			void GuiTab::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				ct->SetCommands(commandExecutor.Obj());
				ct->SetTabPages(tabPages.GetWrapper());
				ct->SetSelectedTabPage(selectedPage);
			}

			void GuiTab::OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments)
			{
				if (arguments.eventSource == focusableComposition)
				{
					if (auto ct = GetControlTemplateObject(false))
					{
						vint index = tabPages.IndexOf(selectedPage);
						if (index != -1)
						{
							auto hint = ct->GetTabOrder();
							vint tabOffset = 0;
							switch (hint)
							{
							case TabPageOrder::LeftToRight:
								if (arguments.code == VKEY::_LEFT) tabOffset = -1;
								else if (arguments.code == VKEY::_RIGHT) tabOffset = 1;
								break;
							case TabPageOrder::RightToLeft:
								if (arguments.code == VKEY::_LEFT) tabOffset = 1;
								else if (arguments.code == VKEY::_RIGHT) tabOffset = -1;
								break;
							case TabPageOrder::TopToBottom:
								if (arguments.code == VKEY::_UP) tabOffset = -1;
								else if (arguments.code == VKEY::_DOWN) tabOffset = 1;
								break;
							case TabPageOrder::BottomToTop:
								if (arguments.code == VKEY::_UP) tabOffset = 1;
								else if (arguments.code == VKEY::_DOWN) tabOffset = -1;
								break;
							default:;
							}

							if (tabOffset != 0)
							{
								arguments.handled = true;
								index += tabOffset;
								if (index < 0) index = 0;
								else if (index >= tabPages.Count()) index = tabPages.Count() - 1;

								SetSelectedPage(tabPages[index]);
							}
						}
					}
				}
			}

			GuiTab::GuiTab(theme::ThemeName themeName)
				:GuiControl(themeName)
				, tabPages(this)
			{
				commandExecutor = new CommandExecutor(this);
				SetFocusableComposition(boundsComposition);

				boundsComposition->GetEventReceiver()->keyDown.AttachMethod(this, &GuiTab::OnKeyDown);
			}

			GuiTab::~GuiTab()
			{
			}

			collections::ObservableList<GuiTabPage*>& GuiTab::GetPages()
			{
				return tabPages;
			}

			GuiTabPage* GuiTab::GetSelectedPage()
			{
				return selectedPage;
			}

			bool GuiTab::SetSelectedPage(GuiTabPage* value)
			{
				if (!value)
				{
					if (tabPages.Count() == 0)
					{
						selectedPage = nullptr;
					}
				}
				else if (value->GetOwnerTab() == this)
				{
					if (selectedPage == value)
					{
						return true;
					}

					selectedPage = value;
					FOREACH(GuiTabPage*, tabPage, tabPages)
					{
						tabPage->SetVisible(tabPage == selectedPage);
					}
				}
				if (auto ct = GetControlTemplateObject(false))
				{
					ct->SetSelectedTabPage(selectedPage);
				}
				SelectedPageChanged.Execute(GetNotifyEventArguments());
				return selectedPage == value;
			}

/***********************************************************************
GuiScrollView
***********************************************************************/

			void GuiScrollView::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject(false);
				if (!ct) return;

				if (auto scroll = ct->GetHorizontalScroll())
				{
					scroll->PositionChanged.Detach(hScrollHandler);
				}
				if (auto scroll = ct->GetVerticalScroll())
				{
					scroll->PositionChanged.Detach(vScrollHandler);
				}
				ct->GetEventReceiver()->horizontalWheel.Detach(hWheelHandler);
				ct->GetEventReceiver()->verticalWheel.Detach(vWheelHandler);
				ct->BoundsChanged.Detach(containerBoundsChangedHandler);

				hScrollHandler = nullptr;
				vScrollHandler = nullptr;
				hWheelHandler = nullptr;
				vWheelHandler = nullptr;
				containerBoundsChangedHandler = nullptr;
				supressScrolling = false;
			}

			void GuiScrollView::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				if (auto scroll = ct->GetHorizontalScroll())
				{
					hScrollHandler = scroll->PositionChanged.AttachMethod(this, &GuiScrollView::OnHorizontalScroll);
				}
				if (auto scroll = ct->GetVerticalScroll())
				{
					vScrollHandler = scroll->PositionChanged.AttachMethod(this, &GuiScrollView::OnVerticalScroll);
				}
				hWheelHandler = ct->GetEventReceiver()->horizontalWheel.AttachMethod(this, &GuiScrollView::OnHorizontalWheel);
				vWheelHandler = ct->GetEventReceiver()->verticalWheel.AttachMethod(this, &GuiScrollView::OnVerticalWheel);
				containerBoundsChangedHandler = ct->BoundsChanged.AttachMethod(this, &GuiScrollView::OnContainerBoundsChanged);
				CalculateView();
			}

			void GuiScrollView::UpdateDisplayFont()
			{
				GuiControl::UpdateDisplayFont();
				CalculateView();
			}

			void GuiScrollView::OnContainerBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				InvokeOrDelayIfRendering([=]()
				{
					CalculateView();
				});
			}

			void GuiScrollView::OnHorizontalScroll(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(!supressScrolling)
				{
					CallUpdateView();
				}
			}

			void GuiScrollView::OnVerticalScroll(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(!supressScrolling)
				{
					CallUpdateView();
				}
			}

			void GuiScrollView::OnHorizontalWheel(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(!supressScrolling)
				{
					if (auto scroll = GetControlTemplateObject(true)->GetHorizontalScroll())
					{
						if (scroll->GetEnabled())
						{
							vint position = scroll->GetPosition();
							vint move = scroll->GetSmallMove();
							position -= move * arguments.wheel / 60;
							scroll->SetPosition(position);
						}
					}
				}
			}

			void GuiScrollView::OnVerticalWheel(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(!supressScrolling && GetVisuallyEnabled())
				{
					if (auto scroll = GetControlTemplateObject(true)->GetVerticalScroll())
					{
						if (scroll->GetEnabled())
						{
							vint position = scroll->GetPosition();
							vint move = scroll->GetSmallMove();
							position -= move * arguments.wheel / 60;
							scroll->SetPosition(position);
						}
					}
				}
			}

			void GuiScrollView::CallUpdateView()
			{
				Rect viewBounds=GetViewBounds();
				UpdateView(viewBounds);
			}

			bool GuiScrollView::AdjustView(Size fullSize)
			{
				auto ct = GetControlTemplateObject(true);
				auto hScroll = ct->GetHorizontalScroll();
				auto vScroll = ct->GetVerticalScroll();
				Size viewSize = ct->GetContainerComposition()->GetBounds().GetSize();

				auto hVisible = hScroll ? hScroll->GetVisible() : false;
				auto vVisible = vScroll ? vScroll->GetVisible() : false;

				if (hScroll)
				{
					if (fullSize.x <= viewSize.x)
					{
						hScroll->SetVisible(horizontalAlwaysVisible);
						hScroll->SetEnabled(false);
						hScroll->SetPosition(0);
					}
					else
					{
						hScroll->SetVisible(true);
						hScroll->SetEnabled(true);
						hScroll->SetTotalSize(fullSize.x);
						hScroll->SetPageSize(viewSize.x);
					}
				}

				if (vScroll)
				{
					if (fullSize.y <= viewSize.y)
					{
						vScroll->SetVisible(verticalAlwaysVisible);
						vScroll->SetEnabled(false);
						vScroll->SetPosition(0);
					}
					else
					{
						vScroll->SetVisible(true);
						vScroll->SetEnabled(true);
						vScroll->SetTotalSize(fullSize.y);
						vScroll->SetPageSize(viewSize.y);
					}
				}

				auto hVisible2 = hScroll ? hScroll->GetVisible() : false;
				auto vVisible2 = vScroll ? vScroll->GetVisible() : false;
				return hVisible != hVisible2 || vVisible != vVisible2;
			}

			GuiScrollView::GuiScrollView(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
				containerComposition->BoundsChanged.AttachMethod(this, &GuiScrollView::OnContainerBoundsChanged);
			}

			vint GuiScrollView::GetSmallMove()
			{
				return GetDisplayFont().size * 2;
			}

			Size GuiScrollView::GetBigMove()
			{
				return GetViewSize();
			}
			
			GuiScrollView::~GuiScrollView()
			{
			}

			void GuiScrollView::CalculateView()
			{
				auto ct = GetControlTemplateObject(true);
				if (!supressScrolling)
				{
					Size fullSize = QueryFullSize();
					while (true)
					{
						bool flagA = AdjustView(fullSize);
						bool flagB = AdjustView(fullSize);
						supressScrolling = true;
						CallUpdateView();
						supressScrolling = false;

						Size newSize = QueryFullSize();
						if (fullSize == newSize)
						{
							vint smallMove = GetSmallMove();
							Size bigMove = GetBigMove();
							if (auto scroll = ct->GetHorizontalScroll())
							{
								scroll->SetSmallMove(smallMove);
								scroll->SetBigMove(bigMove.x);
							}
							if (auto scroll = ct->GetVerticalScroll())
							{
								scroll->SetSmallMove(smallMove);
								scroll->SetBigMove(bigMove.y);
							}

							if (!flagA && !flagB)
							{
								break;
							}
						}
						else
						{
							fullSize = newSize;
						}
					}
				}
			}

			Size GuiScrollView::GetViewSize()
			{
				Size viewSize = GetControlTemplateObject(true)->GetContainerComposition()->GetBounds().GetSize();
				return viewSize;
			}

			Rect GuiScrollView::GetViewBounds()
			{
				return Rect(GetViewPosition(), GetViewSize());
			}

			Point GuiScrollView::GetViewPosition()
			{
				auto ct = GetControlTemplateObject(true);
				auto hScroll = ct->GetHorizontalScroll();
				auto vScroll = ct->GetVerticalScroll();
				return Point(hScroll ? hScroll->GetPosition() : 0, vScroll ? vScroll->GetPosition() : 0);
			}

			void GuiScrollView::SetViewPosition(Point value)
			{
				auto ct = GetControlTemplateObject(true);
				if (auto hScroll = ct->GetHorizontalScroll())
				{
					hScroll->SetPosition(value.x);
				}
				if (auto vScroll = ct->GetVerticalScroll())
				{
					vScroll->SetPosition(value.y);
				}
			}

			GuiScroll* GuiScrollView::GetHorizontalScroll()
			{
				return GetControlTemplateObject(true)->GetHorizontalScroll();
			}

			GuiScroll* GuiScrollView::GetVerticalScroll()
			{
				return GetControlTemplateObject(true)->GetVerticalScroll();
			}

			bool GuiScrollView::GetHorizontalAlwaysVisible()
			{
				return horizontalAlwaysVisible;
			}

			void GuiScrollView::SetHorizontalAlwaysVisible(bool value)
			{
				if (horizontalAlwaysVisible != value)
				{
					horizontalAlwaysVisible = value;
					CalculateView();
				}
			}

			bool GuiScrollView::GetVerticalAlwaysVisible()
			{
				return verticalAlwaysVisible;
			}

			void GuiScrollView::SetVerticalAlwaysVisible(bool value)
			{
				if (verticalAlwaysVisible != value)
				{
					verticalAlwaysVisible = value;
					CalculateView();
				}
			}

/***********************************************************************
GuiScrollContainer
***********************************************************************/

			Size GuiScrollContainer::QueryFullSize()
			{
				return containerComposition->GetBounds().GetSize();
			}

			void GuiScrollContainer::UpdateView(Rect viewBounds)
			{
				auto leftTop = Point(-viewBounds.x1, -viewBounds.y1);
				containerComposition->SetBounds(Rect(leftTop, Size(0, 0)));
			}

			GuiScrollContainer::GuiScrollContainer(theme::ThemeName themeName)
				:GuiScrollView(themeName)
			{
				containerComposition->SetAlignmentToParent(Margin(-1, -1, -1, -1));
				UpdateView(Rect(0, 0, 0, 0));
			}

			GuiScrollContainer::~GuiScrollContainer()
			{
			}

			bool GuiScrollContainer::GetExtendToFullWidth()
			{
				return extendToFullWidth;
			}

			void GuiScrollContainer::SetExtendToFullWidth(bool value)
			{
				if (extendToFullWidth != value)
				{
					extendToFullWidth = value;
					auto margin = containerComposition->GetAlignmentToParent();
					if (value)
					{
						containerComposition->SetAlignmentToParent(Margin(0, margin.top, 0, margin.bottom));
					}
					else
					{
						containerComposition->SetAlignmentToParent(Margin(-1, margin.top, -1, margin.bottom));
					}
				}
			}

			bool GuiScrollContainer::GetExtendToFullHeight()
			{
				return extendToFullHeight;
			}

			void GuiScrollContainer::SetExtendToFullHeight(bool value)
			{
				if (extendToFullHeight != value)
				{
					extendToFullHeight = value;
					auto margin = containerComposition->GetAlignmentToParent();
					if (value)
					{
						containerComposition->SetAlignmentToParent(Margin(margin.left, 0, margin.right, 0));
					}
					else
					{
						containerComposition->SetAlignmentToParent(Margin(margin.left, -1, margin.right, -1));
					}
				}
			}
		}
	}
}