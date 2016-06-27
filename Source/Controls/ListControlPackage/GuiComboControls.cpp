#include "GuiComboControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiComboBoxBase::CommandExecutor
***********************************************************************/

			GuiComboBoxBase::CommandExecutor::CommandExecutor(GuiComboBoxBase* _combo)
				:combo(_combo)
			{
			}

			GuiComboBoxBase::CommandExecutor::~CommandExecutor()
			{
			}

			void GuiComboBoxBase::CommandExecutor::SelectItem()
			{
				combo->SelectItem();
			}

/***********************************************************************
GuiComboBoxBase
***********************************************************************/

			bool GuiComboBoxBase::IsAltAvailable()
			{
				return false;
			}

			IGuiMenuService::Direction GuiComboBoxBase::GetSubMenuDirection()
			{
				return IGuiMenuService::Horizontal;
			}

			void GuiComboBoxBase::SelectItem()
			{
				styleController->OnItemSelected();
				ItemSelected.Execute(GetNotifyEventArguments());
			}

			void GuiComboBoxBase::OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				Size size=GetPreferredMenuClientSize();
				size.x=GetBoundsComposition()->GetBounds().Width();
				SetPreferredMenuClientSize(size);
			}

			GuiComboBoxBase::GuiComboBoxBase(IStyleController* _styleController)
				:GuiMenuButton(_styleController)
			{
				commandExecutor=new CommandExecutor(this);
				styleController=dynamic_cast<IStyleController*>(GetStyleController());
				styleController->SetCommandExecutor(commandExecutor.Obj());

				CreateSubMenu();
				SetCascadeAction(false);

				GetBoundsComposition()->BoundsChanged.AttachMethod(this, &GuiComboBoxBase::OnBoundsChanged);
			}

			GuiComboBoxBase::~GuiComboBoxBase()
			{
			}

/***********************************************************************
GuiComboBoxListControl
***********************************************************************/

			void GuiComboBoxListControl::DisplaySelectedContent(vint itemIndex)
			{
				if(primaryTextView)
				{
					if(itemIndex==-1)
					{
						SetText(L"");
					}
					else if(primaryTextView->ContainsPrimaryText(itemIndex))
					{
						WString text=primaryTextView->GetPrimaryTextViewText(itemIndex);
						SetText(text);
						GetSubMenu()->Hide();
					}
				}
			}

			void GuiComboBoxListControl::OnListControlAdoptedSizeInvalidated(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				Size expectedSize(0, GetFont().size * 20);
				Size adoptedSize = containedListControl->GetAdoptedSize(expectedSize);

				Size clientSize = GetPreferredMenuClientSize();
				clientSize.y = adoptedSize.y + GetSubMenu()->GetClientSize().y - containedListControl->GetBoundsComposition()->GetBounds().Height();
				SetPreferredMenuClientSize(clientSize);

				if (GetSubMenuOpening())
				{
					GetSubMenu()->SetClientSize(clientSize);
				}
			}

			void GuiComboBoxListControl::OnListControlSelectionChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				DisplaySelectedContent(GetSelectedIndex());
				SelectItem();
				SelectedIndexChanged.Execute(GetNotifyEventArguments());
			}

			GuiComboBoxListControl::GuiComboBoxListControl(IStyleController* _styleController, GuiSelectableListControl* _containedListControl)
				:GuiComboBoxBase(_styleController)
				,containedListControl(_containedListControl)
			{
				_styleController->SetTextVisible(true);
				containedListControl->SetMultiSelect(false);
				containedListControl->AdoptedSizeInvalidated.AttachMethod(this, &GuiComboBoxListControl::OnListControlSelectionChanged);
				containedListControl->SelectionChanged.AttachMethod(this, &GuiComboBoxListControl::OnListControlSelectionChanged);
				primaryTextView=dynamic_cast<GuiListControl::IItemPrimaryTextView*>(containedListControl->GetItemProvider()->RequestView(GuiListControl::IItemPrimaryTextView::Identifier));

				SelectedIndexChanged.SetAssociatedComposition(GetBoundsComposition());

				containedListControl->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				GetSubMenu()->GetContainerComposition()->AddChild(containedListControl->GetBoundsComposition());
				SetFont(GetFont());
			}

			GuiComboBoxListControl::~GuiComboBoxListControl()
			{
				if(primaryTextView)
				{
					containedListControl->GetItemProvider()->ReleaseView(primaryTextView);
				}
			}

			void GuiComboBoxListControl::SetFont(const FontProperties& value)
			{
				GuiComboBoxBase::SetFont(value);
				OnListControlAdoptedSizeInvalidated(nullptr, GetNotifyEventArguments());
			}

			GuiSelectableListControl* GuiComboBoxListControl::GetContainedListControl()
			{
				return containedListControl;
			}

			vint GuiComboBoxListControl::GetSelectedIndex()
			{
				if(containedListControl->GetSelectedItems().Count()==1)
				{
					return containedListControl->GetSelectedItems()[0];
				}
				else
				{
					return -1;
				}
			}

			void GuiComboBoxListControl::SetSelectedIndex(vint value)
			{
				containedListControl->SetSelected(value, true);
			}

			description::Value GuiComboBoxListControl::GetSelectedItem()
			{
				auto selectedIndex = GetSelectedIndex();
				if (selectedIndex != -1)
				{
					auto view = containedListControl->GetItemProvider()->RequestView(GuiListControl::IItemBindingView::Identifier);
					if (auto bindingView = dynamic_cast<GuiListControl::IItemBindingView*>(view))
					{
						return bindingView->GetBindingValue(selectedIndex);
					}
				}
				return description::Value();
			}

			GuiListControl::IItemProvider* GuiComboBoxListControl::GetItemProvider()
			{
				return containedListControl->GetItemProvider();
			}
		}
	}
}