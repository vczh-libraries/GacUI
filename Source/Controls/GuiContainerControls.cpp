#include "GuiContainerControls.h"

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

			bool GuiTabPage::AssociateTab(GuiTab* _owner)
			{
				if(owner)
				{
					return false;
				}
				else
				{
					owner=_owner;
					PageInstalled.Execute(containerControl->GetNotifyEventArguments());
					return true;
				}
			}

			bool GuiTabPage::DeassociateTab(GuiTab* _owner)
			{
				if(owner && owner==_owner)
				{
					PageUninstalled.Execute(containerControl->GetNotifyEventArguments());
					owner=0;
					return true;
				}
				else
				{
					return false;
				}
			}

			compositions::GuiGraphicsComposition* GuiTabPage::GetAltComposition()
			{
				return containerControl->GetContainerComposition();
			}

			compositions::IGuiAltActionHost* GuiTabPage::GetPreviousAltHost()
			{
				return previousAltHost;
			}

			void GuiTabPage::OnActivatedAltHost(compositions::IGuiAltActionHost* previousHost)
			{
				previousAltHost = previousHost;
			}

			void GuiTabPage::OnDeactivatedAltHost()
			{
				previousAltHost = 0;
			}

			void GuiTabPage::CollectAltActions(collections::Group<WString, compositions::IGuiAltAction*>& actions)
			{
				IGuiAltActionHost::CollectAltActionsFromControl(containerControl, actions);
			}

			GuiTabPage::GuiTabPage()
				:containerControl(0)
				,owner(0)
				,previousAltHost(0)
			{
				containerControl = new GuiControl(new GuiControl::EmptyStyleController());
				containerControl->GetBoundsComposition()->SetAlignmentToParent(Margin(2, 2, 2, 2));
				
				AltChanged.SetAssociatedComposition(containerControl->GetBoundsComposition());
				TextChanged.SetAssociatedComposition(containerControl->GetBoundsComposition());
				PageInstalled.SetAssociatedComposition(containerControl->GetBoundsComposition());
				PageUninstalled.SetAssociatedComposition(containerControl->GetBoundsComposition());
			}

			GuiTabPage::~GuiTabPage()
			{
				if (!containerControl->GetParent())
				{
					delete containerControl;
				}
			}

			compositions::GuiGraphicsComposition* GuiTabPage::GetContainerComposition()
			{
				return containerControl->GetContainerComposition();
			}

			GuiTab* GuiTabPage::GetOwnerTab()
			{
				return owner;
			}

			const WString& GuiTabPage::GetAlt()
			{
				return alt;
			}

			bool GuiTabPage::SetAlt(const WString& value)
			{
				if (!IGuiAltAction::IsLegalAlt(value)) return false;
				if(owner)
				{
					owner->styleController->SetTabAlt(owner->tabPages.IndexOf(this), text, this);
				}
				if (alt != value)
				{
					alt = value;
					AltChanged.Execute(containerControl->GetNotifyEventArguments());
				}
				return true;
			}

			const WString& GuiTabPage::GetText()
			{
				return text;
			}

			void GuiTabPage::SetText(const WString& value)
			{
				if(text!=value)
				{
					text=value;
					if(owner)
					{
						owner->styleController->SetTabText(owner->tabPages.IndexOf(this), text);
					}
					TextChanged.Execute(containerControl->GetNotifyEventArguments());
				}
			}

			bool GuiTabPage::GetSelected()
			{
				return owner->GetSelectedPage()==this;
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

			vint GuiTab::GetAltActionCount()
			{
				return tabPages.Count();
			}

			compositions::IGuiAltAction* GuiTab::GetAltAction(vint index)
			{
				return styleController->GetTabAltAction(index);
			}

			GuiTab::GuiTab(IStyleController* _styleController)
				:GuiControl(_styleController)
				,styleController(_styleController)
				,selectedPage(0)
			{
				commandExecutor=new CommandExecutor(this);
				styleController->SetCommandExecutor(commandExecutor.Obj());
			}

			GuiTab::~GuiTab()
			{
				for(vint i=0;i<tabPages.Count();i++)
				{
					delete tabPages[i];
				}
			}

			IDescriptable* GuiTab::QueryService(const WString& identifier)
			{
				if (identifier == IGuiAltActionContainer::Identifier)
				{
					return (IGuiAltActionContainer*)this;
				}
				else
				{
					return GuiControl::QueryService(identifier);
				}
			}

			GuiTabPage* GuiTab::CreatePage(vint index)
			{
				GuiTabPage* page=new GuiTabPage();
				if(CreatePage(page, index))
				{
					return page;
				}
				else
				{
					delete page;
					return 0;
				}
			}

			bool GuiTab::CreatePage(GuiTabPage* page, vint index)
			{
				if(index>=0 && index>=tabPages.Count())
				{
					index=tabPages.Count()-1;
				}
				else if(index<-1)
				{
					index=-1;
				}

				if(page->AssociateTab(this))
				{
					index=index==-1?tabPages.Add(page):tabPages.Insert(index, page);
					GetContainerComposition()->AddChild(page->GetContainerComposition());
					styleController->InsertTab(index);
					styleController->SetTabText(index, page->GetText());
					styleController->SetTabAlt(index, page->GetAlt(), page);
				
					if(!selectedPage)
					{
						SetSelectedPage(page);
					}
					page->GetContainerComposition()->SetVisible(page==selectedPage);
					return true;
				}
				else
				{
					return false;
				}
			}

			bool GuiTab::RemovePage(GuiTabPage* page)
			{
				if(page->GetOwnerTab()==this && page->DeassociateTab(this))
				{
					vint index=tabPages.IndexOf(page);
					styleController->RemoveTab(index);
					GetContainerComposition()->RemoveChild(page->GetContainerComposition());
					tabPages.RemoveAt(index);
					if(tabPages.Count()==0)
					{
						SetSelectedPage(0);
						return 0;
					}
					else if(selectedPage==page)
					{
						SetSelectedPage(tabPages[0]);
					}
					return true;
				}
				else
				{
					return false;
				}
			}

			bool GuiTab::MovePage(GuiTabPage* page, vint newIndex)
			{
				if(!page) return false;
				vint index=tabPages.IndexOf(page);
				if(index==-1) return false;
				tabPages.RemoveAt(index);
				tabPages.Insert(newIndex, page);
				styleController->MoveTab(index, newIndex);
				styleController->SetSelectedTab(tabPages.IndexOf(selectedPage));
				return true;
			}

			const collections::List<GuiTabPage*>& GuiTab::GetPages()
			{
				return tabPages;
			}

			GuiTabPage* GuiTab::GetSelectedPage()
			{
				return selectedPage;
			}

			bool GuiTab::SetSelectedPage(GuiTabPage* value)
			{
				if(!value)
				{
					if(tabPages.Count()==0)
					{
						selectedPage=0;
					}
				}
				else if(value->GetOwnerTab()==this)
				{
					if(selectedPage!=value)
					{
						selectedPage=value;
						for(vint i=0;i<tabPages.Count();i++)
						{
							bool selected=tabPages[i]==value;
							tabPages[i]->GetContainerComposition()->SetVisible(selected);
							if(selected)
							{
								styleController->SetSelectedTab(i);
							}
						}
						SelectedPageChanged.Execute(GetNotifyEventArguments());
					}
				}
				return selectedPage==value;
			}

/***********************************************************************
GuiScrollView::StyleController
***********************************************************************/

			void GuiScrollView::StyleController::UpdateTable()
			{
				if(horizontalScroll->GetEnabled() || horizontalAlwaysVisible)
				{
					tableComposition->SetRowOption(1, GuiCellOption::AbsoluteOption(styleProvider->GetDefaultScrollSize()));
				}
				else
				{
					tableComposition->SetRowOption(1, GuiCellOption::AbsoluteOption(0));
				}
				if(verticalScroll->GetEnabled() || verticalAlwaysVisible)
				{
					tableComposition->SetColumnOption(1, GuiCellOption::AbsoluteOption(styleProvider->GetDefaultScrollSize()));
				}
				else
				{
					tableComposition->SetColumnOption(1, GuiCellOption::AbsoluteOption(0));
				}
				tableComposition->UpdateCellBounds();
			}

			GuiScrollView::StyleController::StyleController(IStyleProvider* _styleProvider)
				:styleProvider(_styleProvider)
				,scrollView(0)
				,horizontalAlwaysVisible(true)
				,verticalAlwaysVisible(true)
			{
				horizontalScroll=new GuiScroll(styleProvider->CreateHorizontalScrollStyle());
				horizontalScroll->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				horizontalScroll->SetEnabled(false);
				verticalScroll=new GuiScroll(styleProvider->CreateVerticalScrollStyle());
				verticalScroll->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				verticalScroll->SetEnabled(false);

				boundsComposition=new GuiBoundsComposition;
				GuiGraphicsComposition* tableContainerComposition=styleProvider->InstallBackground(boundsComposition);

				tableComposition=new GuiTableComposition;
				tableContainerComposition->AddChild(tableComposition);
				tableComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				tableComposition->SetRowsAndColumns(2, 2);
				tableComposition->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
				tableComposition->SetRowOption(1, GuiCellOption::MinSizeOption());
				tableComposition->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
				tableComposition->SetColumnOption(1, GuiCellOption::MinSizeOption());
				UpdateTable();
				{
					GuiCellComposition* cell=new GuiCellComposition;
					tableComposition->AddChild(cell);
					cell->SetSite(1, 0, 1, 1);
					cell->AddChild(horizontalScroll->GetBoundsComposition());
				}
				{
					GuiCellComposition* cell=new GuiCellComposition;
					tableComposition->AddChild(cell);
					cell->SetSite(0, 1, 1, 1);
					cell->AddChild(verticalScroll->GetBoundsComposition());
				}
				
				containerCellComposition=new GuiCellComposition;
				tableComposition->AddChild(containerCellComposition);
				containerCellComposition->SetSite(0, 0, 1, 1);

				containerComposition=new GuiBoundsComposition;
				containerComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				containerCellComposition->AddChild(containerComposition);

				styleProvider->AssociateStyleController(this);
			}

			GuiScrollView::StyleController::~StyleController()
			{
			}

			void GuiScrollView::StyleController::SetScrollView(GuiScrollView* _scrollView)
			{
				scrollView=_scrollView;
			}

			void GuiScrollView::StyleController::AdjustView(Size fullSize)
			{
				Size viewSize=containerComposition->GetBounds().GetSize();
				if(fullSize.x<=viewSize.x)
				{
					horizontalScroll->SetEnabled(false);
					horizontalScroll->SetPosition(0);
				}
				else
				{
					horizontalScroll->SetEnabled(true);
					horizontalScroll->SetTotalSize(fullSize.x);
					horizontalScroll->SetPageSize(viewSize.x);
				}
				if(fullSize.y<=viewSize.y)
				{
					verticalScroll->SetEnabled(false);
					verticalScroll->SetPosition(0);
				}
				else
				{
					verticalScroll->SetEnabled(true);
					verticalScroll->SetTotalSize(fullSize.y);
					verticalScroll->SetPageSize(viewSize.y);
				}
				UpdateTable();
			}

			GuiScrollView::IStyleProvider* GuiScrollView::StyleController::GetStyleProvider()
			{
				return styleProvider.Obj();
			}

			GuiScroll* GuiScrollView::StyleController::GetHorizontalScroll()
			{
				return horizontalScroll;
			}

			GuiScroll* GuiScrollView::StyleController::GetVerticalScroll()
			{
				return verticalScroll;
			}

			compositions::GuiTableComposition* GuiScrollView::StyleController::GetInternalTableComposition()
			{
				return tableComposition;
			}

			compositions::GuiBoundsComposition* GuiScrollView::StyleController::GetInternalContainerComposition()
			{
				return containerComposition;
			}

			bool GuiScrollView::StyleController::GetHorizontalAlwaysVisible()
			{
				return horizontalAlwaysVisible;
			}

			void GuiScrollView::StyleController::SetHorizontalAlwaysVisible(bool value)
			{
				if(horizontalAlwaysVisible!=value)
				{
					horizontalAlwaysVisible=value;
					scrollView->CalculateView();
				}
			}

			bool GuiScrollView::StyleController::GetVerticalAlwaysVisible()
			{
				return verticalAlwaysVisible;
			}

			void GuiScrollView::StyleController::SetVerticalAlwaysVisible(bool value)
			{
				if(verticalAlwaysVisible!=value)
				{
					verticalAlwaysVisible=value;
					scrollView->CalculateView();
				}
			}

			compositions::GuiBoundsComposition* GuiScrollView::StyleController::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* GuiScrollView::StyleController::GetContainerComposition()
			{
				return containerComposition;
			}

			void GuiScrollView::StyleController::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
				styleProvider->SetFocusableComposition(value);
			}

			void GuiScrollView::StyleController::SetText(const WString& value)
			{
				styleProvider->SetText(value);
			}

			void GuiScrollView::StyleController::SetFont(const FontProperties& value)
			{
				styleProvider->SetFont(value);
			}

			void GuiScrollView::StyleController::SetVisuallyEnabled(bool value)
			{
				styleProvider->SetVisuallyEnabled(value);
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

			void GuiScrollView::Initialize()
			{
				styleController=dynamic_cast<StyleController*>(GetStyleController());
				styleController->SetScrollView(this);

				styleController->GetInternalContainerComposition()->BoundsChanged.AttachMethod(this, &GuiScrollView::OnContainerBoundsChanged);
				styleController->GetHorizontalScroll()->PositionChanged.AttachMethod(this, &GuiScrollView::OnHorizontalScroll);
				styleController->GetVerticalScroll()->PositionChanged.AttachMethod(this, &GuiScrollView::OnVerticalScroll);
				styleController->GetBoundsComposition()->GetEventReceiver()->horizontalWheel.AttachMethod(this, &GuiScrollView::OnHorizontalWheel);
				styleController->GetBoundsComposition()->GetEventReceiver()->verticalWheel.AttachMethod(this, &GuiScrollView::OnVerticalWheel);
			}

			GuiScrollView::GuiScrollView(StyleController* _styleController)
				:GuiControl(_styleController)
				,supressScrolling(false)
			{
				Initialize();
			}

			GuiScrollView::GuiScrollView(IStyleProvider* styleProvider)
				:GuiControl(new StyleController(styleProvider))
				,supressScrolling(false)
			{
				Initialize();
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
GuiScrollContainer::StyleController
***********************************************************************/

			GuiScrollContainer::StyleController::StyleController(GuiScrollView::IStyleProvider* styleProvider)
				:GuiScrollView::StyleController(styleProvider)
				,extendToFullWidth(false)
			{
				controlContainerComposition=new GuiBoundsComposition;
				controlContainerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				GetInternalContainerComposition()->AddChild(controlContainerComposition);
			}

			GuiScrollContainer::StyleController::~StyleController()
			{
			}

			compositions::GuiGraphicsComposition* GuiScrollContainer::StyleController::GetContainerComposition()
			{
				return controlContainerComposition;
			}

			void GuiScrollContainer::StyleController::MoveContainer(Point leftTop)
			{
				controlContainerComposition->SetBounds(Rect(leftTop, Size(0, 0)));
			}

			bool GuiScrollContainer::StyleController::GetExtendToFullWidth()
			{
				return extendToFullWidth;
			}

			void GuiScrollContainer::StyleController::SetExtendToFullWidth(bool value)
			{
				if(extendToFullWidth!=value)
				{
					extendToFullWidth=value;
					if(value)
					{
						controlContainerComposition->SetAlignmentToParent(Margin(0, -1, 0, -1));
					}
					else
					{
						controlContainerComposition->SetAlignmentToParent(Margin(-1, -1, -1, -1));
					}
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
				return styleController->GetContainerComposition()->GetBounds().GetSize();
			}

			void GuiScrollContainer::UpdateView(Rect viewBounds)
			{
				styleController->MoveContainer(Point(-viewBounds.x1, -viewBounds.y1));
			}

			GuiScrollContainer::GuiScrollContainer(GuiScrollContainer::IStyleProvider* styleProvider)
				:GuiScrollView(new StyleController(styleProvider))
				,styleController(0)
			{
				styleController=dynamic_cast<StyleController*>(GetStyleController());
				GuiBoundsComposition* composition=dynamic_cast<GuiBoundsComposition*>(styleController->GetContainerComposition());
				composition->BoundsChanged.AttachMethod(this, &GuiScrollContainer::OnControlContainerBoundsChanged);
			}

			GuiScrollContainer::~GuiScrollContainer()
			{
			}

			bool GuiScrollContainer::GetExtendToFullWidth()
			{
				return styleController->GetExtendToFullWidth();
			}

			void GuiScrollContainer::SetExtendToFullWidth(bool value)
			{
				styleController->SetExtendToFullWidth(value);
			}
		}
	}
}