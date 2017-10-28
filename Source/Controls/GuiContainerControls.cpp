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
				if (HasControlTemplateObject())
				{
					GetControlTemplateObject()->SetSelectedTabPage(selectedPage);
				}
				SelectedPageChanged.Execute(GetNotifyEventArguments());
				return selectedPage == value;
			}

/***********************************************************************
GuiScrollView
***********************************************************************/

			void GuiScrollView::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject();
				ct->GetHorizontalScroll()->PositionChanged.Detach(hScrollHandler);
				ct->GetVerticalScroll()->PositionChanged.Detach(vScrollHandler);
				ct->GetEventReceiver()->horizontalWheel.Detach(hWheelHandler);
				ct->GetEventReceiver()->verticalWheel.Detach(vWheelHandler);

				hScrollHandler = nullptr;
				vScrollHandler = nullptr;
				hWheelHandler = nullptr;
				vWheelHandler = nullptr;
				supressScrolling = false;
			}

			void GuiScrollView::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject();
				hScrollHandler = ct->GetHorizontalScroll()->PositionChanged.AttachMethod(this, &GuiScrollView::OnHorizontalScroll);
				vScrollHandler = ct->GetVerticalScroll()->PositionChanged.AttachMethod(this, &GuiScrollView::OnVerticalScroll);
				hWheelHandler = ct->GetEventReceiver()->horizontalWheel.AttachMethod(this, &GuiScrollView::OnHorizontalWheel);
				vWheelHandler = ct->GetEventReceiver()->verticalWheel.AttachMethod(this, &GuiScrollView::OnVerticalWheel);
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

			void GuiScrollView::AdjustView(Size fullSize)
			{
				auto ct = GetControlTemplateObject();
				auto hScroll = ct->GetHorizontalScroll();
				auto vScroll = ct->GetVerticalScroll();

				Size viewSize = containerComposition->GetBounds().GetSize();
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

			GuiScrollView::GuiScrollView(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
				containerComposition->BoundsChanged.AttachMethod(this, &GuiScrollView::OnContainerBoundsChanged);
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
						AdjustView(fullSize);
						AdjustView(fullSize);
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
				Size viewSize = GetControlTemplateObject()->GetContainerComposition()->GetBounds().GetSize();
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