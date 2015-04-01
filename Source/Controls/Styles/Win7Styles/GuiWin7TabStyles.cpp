#include "GuiWin7TabStyles.h"

namespace vl
{
	namespace presentation
	{
		namespace win7
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;
			using namespace controls;

/***********************************************************************
Win7TabPageHeaderStyle
***********************************************************************/

			void Win7TabPageHeaderStyle::TransferInternal(GuiButton::ControlState value, bool enabled, bool selected)
			{
				if(selected)
				{
					transferringAnimation->Transfer(Win7ButtonColors::TabPageHeaderSelected());
				}
				else
				{
					switch(value)
					{
					case GuiButton::Normal:
						transferringAnimation->Transfer(Win7ButtonColors::TabPageHeaderNormal());
						break;
					case GuiButton::Active:
					case GuiButton::Pressed:
						transferringAnimation->Transfer(Win7ButtonColors::TabPageHeaderActive());
						break;
					}
				}
			}

			Win7TabPageHeaderStyle::Win7TabPageHeaderStyle()
				:Win7ButtonStyleBase(true, false, Win7ButtonColors::TabPageHeaderNormal(), Alignment::Left, Alignment::Center)
			{
				transferringAnimation->SetEnableAnimation(false);
				{
					Margin margin=elements.backgroundComposition->GetAlignmentToParent();
					margin.bottom=0;
					elements.backgroundComposition->SetAlignmentToParent(margin);
				}
				{
					Margin margin=elements.gradientComposition->GetAlignmentToParent();
					margin.bottom=0;
					elements.gradientComposition->SetAlignmentToParent(margin);
				}
			}

			Win7TabPageHeaderStyle::~Win7TabPageHeaderStyle()
			{
			}

			void Win7TabPageHeaderStyle::SetFont(const FontProperties& value)
			{
				Win7ButtonStyleBase::SetFont(value);
				Margin margin=elements.textComposition->GetMargin();
				margin.left*=2;
				margin.right*=2;
				elements.textComposition->SetMargin(margin);
			}

/***********************************************************************
Win7TabStyle
***********************************************************************/

			void Win7TabStyle::OnHeaderButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(commandExecutor)
				{
					vint index=headerButtons.IndexOf(dynamic_cast<GuiSelectableButton*>(sender->GetAssociatedControl()));
					if(index!=-1)
					{
						commandExecutor->ShowTab(index);
					}
				}
			}

			void Win7TabStyle::OnTabHeaderBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				vint height=headerOverflowButton->GetBoundsComposition()->GetBounds().Height();
				headerOverflowButton->GetBoundsComposition()->SetBounds(Rect(Point(0, 0), Size(height, 0)));

				UpdateHeaderLayout();
			}

			void Win7TabStyle::OnHeaderOverflowButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				headerOverflowMenu->SetClientSize(Size(0, 0));
				headerOverflowMenu->ShowPopup(headerOverflowButton, true);
			}

			void Win7TabStyle::OnHeaderOverflowMenuButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				vint index=headerOverflowMenu->GetToolstripItems().IndexOf(sender->GetRelatedControl());
				commandExecutor->ShowTab(index);
			}

			void Win7TabStyle::UpdateHeaderOverflowButtonVisibility()
			{
				if(tabHeaderComposition->IsStackItemClipped())
				{
					boundsComposition->SetColumnOption(1, GuiCellOption::MinSizeOption());
				}
				else
				{
					boundsComposition->SetColumnOption(1, GuiCellOption::AbsoluteOption(0));
				}
				boundsComposition->ForceCalculateSizeImmediately();
			}

			void Win7TabStyle::UpdateHeaderZOrder()
			{
				vint itemCount=tabHeaderComposition->GetStackItems().Count();
				vint childCount=tabHeaderComposition->Children().Count();
				for(vint i=0;i<itemCount;i++)
				{
					GuiStackItemComposition* item=tabHeaderComposition->GetStackItems().Get(i);
					if(headerButtons[i]->GetSelected())
					{
						tabHeaderComposition->MoveChild(item, childCount-1);
						item->SetExtraMargin(Margin(2, 2, 2, 0));
					}
					else
					{
						item->SetExtraMargin(Margin(0, 0, 0, 0));
					}
				}
				if(childCount>1)
				{
					tabHeaderComposition->MoveChild(tabContentTopLineComposition, childCount-2);
				}
			}

			void Win7TabStyle::UpdateHeaderVisibilityIndex()
			{
				vint itemCount=tabHeaderComposition->GetStackItems().Count();
				vint selectedItem=-1;
				for(vint i=0;i<itemCount;i++)
				{
					if(headerButtons[i]->GetSelected())
					{
						selectedItem=i;
					}
				}

				if(selectedItem!=-1)
				{
					tabHeaderComposition->EnsureVisible(selectedItem);
				}
			}

			void Win7TabStyle::UpdateHeaderLayout()
			{
				UpdateHeaderZOrder();
				UpdateHeaderVisibilityIndex();
				UpdateHeaderOverflowButtonVisibility();
			}

			void Win7TabStyle::Initialize()
			{
				boundsComposition=new GuiTableComposition;
				boundsComposition->SetRowsAndColumns(2, 2);
				boundsComposition->SetRowOption(0, GuiCellOption::MinSizeOption());
				boundsComposition->SetRowOption(1, GuiCellOption::PercentageOption(1.0));
				boundsComposition->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
				boundsComposition->SetColumnOption(1, GuiCellOption::AbsoluteOption(0));
				{
					GuiCellComposition* cell=new GuiCellComposition;
					boundsComposition->AddChild(cell);
					cell->SetSite(0, 0, 1, 1);

					tabHeaderComposition=new GuiStackComposition;
					tabHeaderComposition->SetExtraMargin(Margin(2, 2, 2, 0));
					tabHeaderComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
					tabHeaderComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					tabHeaderComposition->BoundsChanged.AttachMethod(this, &Win7TabStyle::OnTabHeaderBoundsChanged);
					cell->AddChild(tabHeaderComposition);
				}
				{
					GuiCellComposition* cell=new GuiCellComposition;
					boundsComposition->AddChild(cell);
					cell->SetSite(0, 1, 1, 1);

					headerOverflowButton=new GuiButton(CreateMenuButtonStyleController());
					headerOverflowButton->GetContainerComposition()->AddChild(common_styles::CommonFragmentBuilder::BuildDownArrow(headerOverflowArrowElement));
					headerOverflowButton->GetBoundsComposition()->SetAlignmentToParent(Margin(-1, 0, 0, 0));
					headerOverflowButton->Clicked.AttachMethod(this, &Win7TabStyle::OnHeaderOverflowButtonClicked);
					cell->AddChild(headerOverflowButton->GetBoundsComposition());
				}
				{
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(GetBorderColor());

					tabContentTopLineComposition=new GuiBoundsComposition;
					tabContentTopLineComposition->SetOwnedElement(element);
					tabContentTopLineComposition->SetAlignmentToParent(Margin(0, -1, 0, 0));
					tabContentTopLineComposition->SetPreferredMinSize(Size(0, 1));
					tabHeaderComposition->AddChild(tabContentTopLineComposition);
				}
				{
					GuiCellComposition* cell=new GuiCellComposition;
					boundsComposition->AddChild(cell);
					cell->SetSite(1, 0, 1, 2);
					
					GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
					element->SetColor(GetBackgroundColor());

					containerComposition=new GuiBoundsComposition;
					containerComposition->SetOwnedElement(element);
					containerComposition->SetAlignmentToParent(Margin(1, 0, 1, 1));
					containerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					cell->AddChild(containerComposition);

					{
						GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
						element->SetColor(GetBorderColor());
						cell->SetOwnedElement(element);
					}
					{
						GuiSolidBackgroundElement* element=GuiSolidBackgroundElement::Create();
						element->SetColor(Win7GetSystemTabContentColor());
						containerComposition->SetOwnedElement(element);
					}
				}

				headerOverflowMenu=new GuiToolstripMenu(CreateMenuStyleController(), 0);
				headerController=new GuiSelectableButton::MutexGroupController;
			}
			
			controls::GuiSelectableButton::IStyleController* Win7TabStyle::CreateHeaderStyleController()
			{
				return new Win7TabPageHeaderStyle;
			}

			controls::GuiButton::IStyleController* Win7TabStyle::CreateMenuButtonStyleController()
			{
				return new Win7ButtonStyle;
			}

			controls::GuiToolstripMenu::IStyleController* Win7TabStyle::CreateMenuStyleController()
			{
				return new Win7MenuStyle;
			}

			controls::GuiToolstripButton::IStyleController* Win7TabStyle::CreateMenuItemStyleController()
			{
				return new Win7MenuItemButtonStyle;
			}

			Color Win7TabStyle::GetBorderColor()
			{
				return Win7ButtonColors::TabPageHeaderNormal().borderColor;
			}

			Color Win7TabStyle::GetBackgroundColor()
			{
				return Win7GetSystemTabContentColor();
			}

			Win7TabStyle::Win7TabStyle(bool initialize)
				:boundsComposition(0)
				,containerComposition(0)
				,tabHeaderComposition(0)
				,tabContentTopLineComposition(0)
				,commandExecutor(0)
				,headerOverflowArrowElement(0)
				,headerOverflowButton(0)
				,headerOverflowMenu(0)
			{
				if(initialize)
				{
					Initialize();
				}
			}

			Win7TabStyle::~Win7TabStyle()
			{
				delete headerOverflowMenu;
			}

			compositions::GuiBoundsComposition* Win7TabStyle::GetBoundsComposition()
			{
				return boundsComposition;
			}

			compositions::GuiGraphicsComposition* Win7TabStyle::GetContainerComposition()
			{
				return containerComposition;
			}

			void Win7TabStyle::SetFocusableComposition(compositions::GuiGraphicsComposition* value)
			{
			}

			void Win7TabStyle::SetText(const WString& value)
			{
			}

			void Win7TabStyle::SetFont(const FontProperties& value)
			{
				headerFont=value;
			}

			void Win7TabStyle::SetVisuallyEnabled(bool value)
			{
			}

			void Win7TabStyle::SetCommandExecutor(controls::GuiTab::ICommandExecutor* value)
			{
				commandExecutor=value;
			}

			void Win7TabStyle::InsertTab(vint index)
			{
				GuiSelectableButton* button=new GuiSelectableButton(CreateHeaderStyleController());
				button->SetAutoSelection(false);
				button->SetFont(headerFont);
				button->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				button->SetGroupController(headerController.Obj());
				button->Clicked.AttachMethod(this, &Win7TabStyle::OnHeaderButtonClicked);

				GuiStackItemComposition* item=new GuiStackItemComposition;
				item->AddChild(button->GetBoundsComposition());
				tabHeaderComposition->InsertStackItem(index, item);
				headerButtons.Insert(index, button);

				GuiToolstripButton* menuItem=new GuiToolstripButton(CreateMenuItemStyleController());
				menuItem->Clicked.AttachMethod(this, &Win7TabStyle::OnHeaderOverflowMenuButtonClicked);
				headerOverflowMenu->GetToolstripItems().Insert(index, menuItem);

				UpdateHeaderLayout();
			}

			void Win7TabStyle::SetTabText(vint index, const WString& value)
			{
				headerButtons[index]->SetText(value);
				headerOverflowMenu->GetToolstripItems().Get(index)->SetText(value);
				
				UpdateHeaderLayout();
			}

			void Win7TabStyle::RemoveTab(vint index)
			{
				GuiStackItemComposition* item=tabHeaderComposition->GetStackItems().Get(index);
				GuiSelectableButton* button=headerButtons[index];

				tabHeaderComposition->RemoveChild(item);
				item->RemoveChild(button->GetBoundsComposition());
				headerButtons.RemoveAt(index);

				headerOverflowMenu->GetToolstripItems().RemoveAt(index);
				delete item;
				delete button;
				
				UpdateHeaderLayout();
			}

			void Win7TabStyle::MoveTab(vint oldIndex, vint newIndex)
			{
				GuiStackItemComposition* item=tabHeaderComposition->GetStackItems().Get(oldIndex);
				tabHeaderComposition->RemoveChild(item);
				tabHeaderComposition->InsertStackItem(newIndex, item);

				GuiSelectableButton* button=headerButtons[oldIndex];
				headerButtons.RemoveAt(oldIndex);
				headerButtons.Insert(newIndex, button);
				
				UpdateHeaderLayout();
			}

			void Win7TabStyle::SetSelectedTab(vint index)
			{
				headerButtons[index]->SetSelected(true);
				
				UpdateHeaderLayout();
			}

			void Win7TabStyle::SetTabAlt(vint index, const WString& value, compositions::IGuiAltActionHost* host)
			{
				auto button = headerButtons[index];
				button->SetAlt(value);
				button->SetActivatingAltHost(host);
			}

			compositions::IGuiAltAction* Win7TabStyle::GetTabAltAction(vint index)
			{
				return headerButtons[index]->QueryTypedService<IGuiAltAction>();
			}
		}
	}
}