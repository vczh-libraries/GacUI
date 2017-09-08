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

			GuiTabPage::GuiTabPage(IStyleController* _styleController)
				:GuiCustomControl(_styleController)
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

			GuiTab::GuiTab(IStyleController* _styleController)
				:GuiControl(_styleController)
				, styleController(_styleController)
				, tabPages(this)
			{
				commandExecutor = new CommandExecutor(this);
				styleController->SetCommandExecutor(commandExecutor.Obj());
				styleController->SetTabPages(tabPages.GetWrapper());
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
				styleController->SetSelectedTabPage(selectedPage);
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
					auto scroll = styleController->GetHorizontalScroll();
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
					auto scroll = styleController->GetVerticalScroll();
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

			GuiScrollView::GuiScrollView(IStyleController* _styleController)
				:GuiControl(_styleController)
				, styleController(_styleController)
				, supressScrolling(false)
			{
				commandExecutor = new CommandExecutor(this);
				styleController->SetCommandExecutor(commandExecutor.Obj());
				styleController->GetInternalContainerComposition()->BoundsChanged.AttachMethod(this, &GuiScrollView::OnContainerBoundsChanged);
				styleController->GetHorizontalScroll()->PositionChanged.AttachMethod(this, &GuiScrollView::OnHorizontalScroll);
				styleController->GetVerticalScroll()->PositionChanged.AttachMethod(this, &GuiScrollView::OnVerticalScroll);
				styleController->GetBoundsComposition()->GetEventReceiver()->horizontalWheel.AttachMethod(this, &GuiScrollView::OnHorizontalWheel);
				styleController->GetBoundsComposition()->GetEventReceiver()->verticalWheel.AttachMethod(this, &GuiScrollView::OnVerticalWheel);
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
				if(!supressScrolling)
				{
					Size fullSize = QueryFullSize();
					while(true)
					{
						styleController->AdjustView(fullSize);
						styleController->AdjustView(fullSize);
						supressScrolling = true;
						CallUpdateView();
						supressScrolling = false;

						Size newSize=QueryFullSize();
						if (fullSize == newSize)
						{
							vint smallMove = GetSmallMove();
							styleController->GetHorizontalScroll()->SetSmallMove(smallMove);
							styleController->GetVerticalScroll()->SetSmallMove(smallMove);
							Size bigMove = GetBigMove();
							styleController->GetHorizontalScroll()->SetBigMove(bigMove.x);
							styleController->GetVerticalScroll()->SetBigMove(bigMove.y);
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
				Size viewSize=styleController->GetInternalContainerComposition()->GetBounds().GetSize();
				return viewSize;
			}

			Rect GuiScrollView::GetViewBounds()
			{
				Point viewPosition=
					Point(
						styleController->GetHorizontalScroll()->GetPosition(),
						styleController->GetVerticalScroll()->GetPosition()
						);
				Size viewSize=GetViewSize();
				return Rect(viewPosition, viewSize);
			}

			GuiScroll* GuiScrollView::GetHorizontalScroll()
			{
				return styleController->GetHorizontalScroll();
			}

			GuiScroll* GuiScrollView::GetVerticalScroll()
			{
				return styleController->GetVerticalScroll();
			}

			bool GuiScrollView::GetHorizontalAlwaysVisible()
			{
				return styleController->GetHorizontalAlwaysVisible();
			}

			void GuiScrollView::SetHorizontalAlwaysVisible(bool value)
			{
				styleController->SetHorizontalAlwaysVisible(value);
			}

			bool GuiScrollView::GetVerticalAlwaysVisible()
			{
				return styleController->GetVerticalAlwaysVisible();
			}

			void GuiScrollView::SetVerticalAlwaysVisible(bool value)
			{
				styleController->SetVerticalAlwaysVisible(value);
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

			GuiScrollContainer::GuiScrollContainer(IStyleController* styleController)
				:GuiScrollView(styleController)
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