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

			bool GuiTabPage::IsAltAvailable()
			{
				return false;
			}

			void GuiTabPage::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (tab)
				{
					tab->styleController->SetTabText(tab->tabPages.IndexOf(this), GetText());
				}
			}

			void GuiTabPage::OnAltChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (tab)
				{
					tab->styleController->SetTabAlt(tab->tabPages.IndexOf(this), GetAlt());
				}
			}

			GuiTabPage::GuiTabPage(IStyleController* _styleController)
				:GuiCustomControl(_styleController)
			{
				TextChanged.AttachMethod(this, &GuiTabPage::OnTextChanged);
				AltChanged.AttachMethod(this, &GuiTabPage::OnAltChanged);
			}

			GuiTabPage::~GuiTabPage()
			{
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
				tab->styleController->InsertTab(index);
				tab->styleController->SetTabText(index, value->GetText());
				tab->styleController->SetTabAlt(index, value->GetAlt());

				if (!tab->selectedPage)
				{
					tab->SetSelectedPage(value);
				}

				value->tab = tab;
				tab->GetContainerComposition()->AddChild(value->GetBoundsComposition());
				value->GetContainerComposition()->SetVisible(value == tab->selectedPage);
			}

			void GuiTabPageList::BeforeRemove(vint index, GuiTabPage* const& value)
			{
				tab->styleController->RemoveTab(index);
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
			}

			GuiTab::~GuiTab()
			{
				for (vint i = 0; i < tabPages.Count(); i++)
				{
					SafeDeleteControl(tabPages[i]);
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

			const GuiTabPageList& GuiTab::GetPages()
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
					if (selectedPage != value)
					{
						selectedPage = value;
						for (vint i = 0; i < tabPages.Count(); i++)
						{
							bool selected = tabPages[i] == value;
							tabPages[i]->GetContainerComposition()->SetVisible(selected);
							if (selected)
							{
								styleController->SetSelectedTab(i);
							}
						}
						SelectedPageChanged.Execute(GetNotifyEventArguments());
					}
				}
				return selectedPage == value;
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