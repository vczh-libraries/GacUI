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
				value->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				tab->GetContainerComposition()->AddChild(value->GetBoundsComposition());

				if (!tab->selectedPage)
				{
					tab->SetSelectedPage(value);
				}
			}

			void GuiTabPageList::BeforeRemove(vint index, GuiTabPage* const& value)
			{
				tab->GetContainerComposition()->RemoveChild(value->GetBoundsComposition());
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
GuiTab
***********************************************************************/

			void GuiTab::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject();
				ct->SetCommands(nullptr);
				ct->SetTabPages(nullptr);
				ct->SetSelectedTabPage(nullptr);
			}

			void GuiTab::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject();
				ct->SetCommands(commandExecutor.Obj());
				ct->SetTabPages(tabPages.GetWrapper());
				ct->SetSelectedTabPage(selectedPage);
			}

			GuiTab::CommandExecutor::CommandExecutor(GuiTab* _tab)
				:tab(_tab)
			{
			}

			GuiTab::CommandExecutor::~CommandExecutor()
			{
			}

			void GuiTab::CommandExecutor::ShowTab(vint index)
			{
				tab->SetSelectedPage(tab->GetPages().Get(index));
			}

			GuiTab::GuiTab(theme::ThemeName themeName)
				:GuiControl(themeName)
				, tabPages(this)
			{
				commandExecutor = new CommandExecutor(this);
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
				else if (value->tab == this)
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
				GetControlTemplateObject()->SetSelectedTabPage(selectedPage);
				SelectedPageChanged.Execute(GetNotifyEventArguments());
				return selectedPage == value;
			}

/***********************************************************************
GuiScrollView::CommandExecutor
***********************************************************************/

			GuiScrollView::CommandExecutor::CommandExecutor(GuiScrollView* _scrollView)
				:scrollView(_scrollView)
			{
			}

			GuiScrollView::CommandExecutor::~CommandExecutor()
			{
			}

			void GuiScrollView::CommandExecutor::CalculateView()
			{
				scrollView->CalculateView();
			}

/***********************************************************************
GuiScrollView
***********************************************************************/

			void GuiScrollView::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject();
				ct->SetCommands(nullptr);
				ct->GetInternalContainerComposition()->BoundsChanged.Detach(containerBoundsChangedHandler);
				ct->GetHorizontalScroll()->PositionChanged.Detach(hScrollHandler);
				ct->GetVerticalScroll()->PositionChanged.Detach(vScrollHandler);
				ct->GetEventReceiver()->horizontalWheel.Detach(hWheelHandler);
				ct->GetEventReceiver()->verticalWheel.Detach(vWheelHandler);

				containerBoundsChangedHandler = nullptr;
				hScrollHandler = nullptr;
				vScrollHandler = nullptr;
				hWheelHandler = nullptr;
				vWheelHandler = nullptr;
				supressScrolling = false;
			}

			void GuiScrollView::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject();
				ct->SetCommands(commandExecutor.Obj());
				containerBoundsChangedHandler = ct->GetInternalContainerComposition()->BoundsChanged.AttachMethod(this, &GuiScrollView::OnContainerBoundsChanged);
				hScrollHandler = ct->GetHorizontalScroll()->PositionChanged.AttachMethod(this, &GuiScrollView::OnHorizontalScroll);
				vScrollHandler = ct->GetVerticalScroll()->PositionChanged.AttachMethod(this, &GuiScrollView::OnVerticalScroll);
				hWheelHandler = ct->GetEventReceiver()->horizontalWheel.AttachMethod(this, &GuiScrollView::OnHorizontalWheel);
				vWheelHandler = ct->GetEventReceiver()->verticalWheel.AttachMethod(this, &GuiScrollView::OnVerticalWheel);

				if (initialize)
				{
					horizontalAlwaysVisible = ct->GetHorizontalAlwaysVisible();
					verticalAlwaysVisible = ct->GetVerticalAlwaysVisible();
				}
				else
				{
					ct->SetHorizontalAlwaysVisible(horizontalAlwaysVisible);
					ct->SetVerticalAlwaysVisible(verticalAlwaysVisible);
				}
				CalculateView();
			}

			void GuiScrollView::OnContainerBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				CalculateView();
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
					auto scroll = GetControlTemplateObject()->GetHorizontalScroll();
					vint position = scroll->GetPosition();
					vint move = scroll->GetSmallMove();
					position -= move*arguments.wheel / 60;
					scroll->SetPosition(position);
				}
			}

			void GuiScrollView::OnVerticalWheel(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(!supressScrolling && GetVisuallyEnabled())
				{
					auto scroll = GetControlTemplateObject()->GetVerticalScroll();
					vint position = scroll->GetPosition();
					vint move = scroll->GetSmallMove();
					position -= move*arguments.wheel / 60;
					scroll->SetPosition(position);
				}
			}

			void GuiScrollView::CallUpdateView()
			{
				Rect viewBounds=GetViewBounds();
				UpdateView(viewBounds);
			}

			GuiScrollView::GuiScrollView(theme::ThemeName themeName)
				:GuiControl(themeName)
				, supressScrolling(false)
			{
				commandExecutor = new CommandExecutor(this);
			}

			vint GuiScrollView::GetSmallMove()
			{
				return GetFont().size * 2;
			}

			Size GuiScrollView::GetBigMove()
			{
				return GetViewSize();
			}
			
			GuiScrollView::~GuiScrollView()
			{
			}

			void GuiScrollView::SetFont(const FontProperties& value)
			{
				GuiControl::SetFont(value);
				CalculateView();
			}

			void GuiScrollView::CalculateView()
			{
				auto ct = GetControlTemplateObject();
				if(!supressScrolling)
				{
					Size fullSize = QueryFullSize();
					while(true)
					{
						ct->AdjustView(fullSize);
						ct->AdjustView(fullSize);
						supressScrolling = true;
						CallUpdateView();
						supressScrolling = false;

						Size newSize=QueryFullSize();
						if (fullSize == newSize)
						{
							vint smallMove = GetSmallMove();
							ct->GetHorizontalScroll()->SetSmallMove(smallMove);
							ct->GetVerticalScroll()->SetSmallMove(smallMove);
							Size bigMove = GetBigMove();
							ct->GetHorizontalScroll()->SetBigMove(bigMove.x);
							ct->GetVerticalScroll()->SetBigMove(bigMove.y);
							break;
						}
						else
						{
							fullSize=newSize;
						}
					}
				}
			}

			Size GuiScrollView::GetViewSize()
			{
				Size viewSize = GetControlTemplateObject()->GetInternalContainerComposition()->GetBounds().GetSize();
				return viewSize;
			}

			Rect GuiScrollView::GetViewBounds()
			{
				auto ct = GetControlTemplateObject();
				Point viewPosition=
					Point(
						ct->GetHorizontalScroll()->GetPosition(),
						ct->GetVerticalScroll()->GetPosition()
						);
				Size viewSize=GetViewSize();
				return Rect(viewPosition, viewSize);
			}

			GuiScroll* GuiScrollView::GetHorizontalScroll()
			{
				return GetControlTemplateObject()->GetHorizontalScroll();
			}

			GuiScroll* GuiScrollView::GetVerticalScroll()
			{
				return GetControlTemplateObject()->GetVerticalScroll();
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
					GetControlTemplateObject()->SetHorizontalAlwaysVisible(value);
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
					GetControlTemplateObject()->SetVerticalAlwaysVisible(value);
				}
			}

/***********************************************************************
GuiScrollContainer
***********************************************************************/

			void GuiScrollContainer::OnControlContainerBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				CalculateView();
			}

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
				containerComposition = new GuiBoundsComposition();
				containerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				containerComposition->BoundsChanged.AttachMethod(this, &GuiScrollContainer::OnControlContainerBoundsChanged);

				GuiControl::containerComposition->AddChild(containerComposition);
				GuiControl::containerComposition = containerComposition;

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
					if (value)
					{
						containerComposition->SetAlignmentToParent(Margin(0, -1, 0, -1));
					}
					else
					{
						containerComposition->SetAlignmentToParent(Margin(-1, -1, -1, -1));
					}
				}
			}
		}
	}
}