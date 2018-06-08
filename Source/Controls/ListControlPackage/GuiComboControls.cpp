#include "GuiComboControls.h"
#include "../GuiApplication.h"

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

			void GuiComboBoxBase::BeforeControlTemplateUninstalled_()
			{
				auto ct = GetControlTemplateObject(false);
				if (!ct) return;

				ct->SetCommands(nullptr);
			}

			void GuiComboBoxBase::AfterControlTemplateInstalled_(bool initialize)
			{
				GetControlTemplateObject(true)->SetCommands(commandExecutor.Obj());
			}

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
				ItemSelected.Execute(GetNotifyEventArguments());
			}

			void GuiComboBoxBase::OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				Size size=GetPreferredMenuClientSize();
				size.x=boundsComposition->GetBounds().Width();
				SetPreferredMenuClientSize(size);
			}

			GuiComboBoxBase::GuiComboBoxBase(theme::ThemeName themeName)
				:GuiMenuButton(themeName)
			{
				commandExecutor = new CommandExecutor(this);

				CreateSubMenu();
				SetCascadeAction(false);

				boundsComposition->BoundsChanged.AttachMethod(this, &GuiComboBoxBase::OnBoundsChanged);
			}

			GuiComboBoxBase::~GuiComboBoxBase()
			{
			}

/***********************************************************************
GuiComboBoxListControl
***********************************************************************/

			void GuiComboBoxListControl::BeforeControlTemplateUninstalled()
			{
				GuiComboBoxBase::BeforeControlTemplateUninstalled();
			}

			void GuiComboBoxListControl::AfterControlTemplateInstalled(bool initialize)
			{
				GuiComboBoxBase::AfterControlTemplateInstalled(initialize);
				GetControlTemplateObject(true)->SetTextVisible(!itemStyleProperty);
			}

			bool GuiComboBoxListControl::IsAltAvailable()
			{
				return true;
			}

			void GuiComboBoxListControl::OnActiveAlt()
			{
				GuiMenuButton::OnActiveAlt();
				GetSubMenu()->GetNativeWindow()->SetFocus();
				containedListControl->SetFocus();
			}

			void GuiComboBoxListControl::RemoveStyleController()
			{
				if (itemStyleController)
				{
					SafeDeleteComposition(itemStyleController);
					itemStyleController = nullptr;
				}
			}

			void GuiComboBoxListControl::InstallStyleController(vint itemIndex)
			{
				if (itemStyleProperty)
				{
					if (itemIndex != -1)
					{
						auto item = containedListControl->GetItemProvider()->GetBindingValue(itemIndex);
						if (!item.IsNull())
						{
							if (auto style = itemStyleProperty(item))
							{
								itemStyleController = style;
								itemStyleController->SetText(GetText());
								itemStyleController->SetFont(GetFont());
								itemStyleController->SetContext(GetContext());
								itemStyleController->SetVisuallyEnabled(GetVisuallyEnabled());
								itemStyleController->SetAlignmentToParent(Margin(0, 0, 0, 0));
								containerComposition->AddChild(itemStyleController);
							}
						}
					}
				}
			}

			void GuiComboBoxListControl::DisplaySelectedContent(vint itemIndex)
			{
				if(itemIndex==-1)
				{
					SetText(L"");
				}
				else
				{
					WString text = containedListControl->GetItemProvider()->GetTextValue(itemIndex);
					SetText(text);
					GetSubMenu()->Hide();
				}

				RemoveStyleController();
				InstallStyleController(itemIndex);
			}

			void GuiComboBoxListControl::AdoptSubMenuSize()
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

			void GuiComboBoxListControl::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (itemStyleController)
				{
					itemStyleController->SetText(GetText());
				}
			}

			void GuiComboBoxListControl::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (itemStyleController)
				{
					itemStyleController->SetFont(GetFont());
				}
				AdoptSubMenuSize();
			}

			void GuiComboBoxListControl::OnContextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (itemStyleController)
				{
					itemStyleController->SetContext(GetContext());
				}
				AdoptSubMenuSize();
			}

			void GuiComboBoxListControl::OnVisuallyEnabledChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (itemStyleController)
				{
					itemStyleController->SetVisuallyEnabled(GetVisuallyEnabled());
				}
			}

			void GuiComboBoxListControl::OnListControlAdoptedSizeInvalidated(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				AdoptSubMenuSize();
			}

			void GuiComboBoxListControl::OnListControlBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				auto flag = flagDisposed;
				GetApplication()->InvokeLambdaInMainThread(GetRelatedControlHost(), [=]()
				{
					if (!*flag.Obj())
					{
						AdoptSubMenuSize();
					}
				});
			}

			void GuiComboBoxListControl::OnListControlSelectionChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				DisplaySelectedContent(GetSelectedIndex());
				SelectItem();
				SelectedIndexChanged.Execute(GetNotifyEventArguments());
			}

			GuiComboBoxListControl::GuiComboBoxListControl(theme::ThemeName themeName, GuiSelectableListControl* _containedListControl)
				:GuiComboBoxBase(themeName)
				, containedListControl(_containedListControl)
			{
				TextChanged.AttachMethod(this, &GuiComboBoxListControl::OnTextChanged);
				FontChanged.AttachMethod(this, &GuiComboBoxListControl::OnFontChanged);
				ContextChanged.AttachMethod(this, &GuiComboBoxListControl::OnContextChanged);
				VisuallyEnabledChanged.AttachMethod(this, &GuiComboBoxListControl::OnVisuallyEnabledChanged);

				containedListControl->SetMultiSelect(false);
				containedListControl->AdoptedSizeInvalidated.AttachMethod(this, &GuiComboBoxListControl::OnListControlAdoptedSizeInvalidated);
				containedListControl->SelectionChanged.AttachMethod(this, &GuiComboBoxListControl::OnListControlSelectionChanged);
				boundsChangedHandler = containedListControl->GetBoundsComposition()->BoundsChanged.AttachMethod(this, &GuiComboBoxListControl::OnListControlBoundsChanged);

				auto itemProvider = containedListControl->GetItemProvider();

				SelectedIndexChanged.SetAssociatedComposition(boundsComposition);

				containedListControl->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				GetSubMenu()->GetContainerComposition()->AddChild(containedListControl->GetBoundsComposition());
				SetFont(GetFont());
			}

			GuiComboBoxListControl::~GuiComboBoxListControl()
			{
				containedListControl->GetBoundsComposition()->BoundsChanged.Detach(boundsChangedHandler);
				boundsChangedHandler = nullptr;
			}

			GuiSelectableListControl* GuiComboBoxListControl::GetContainedListControl()
			{
				return containedListControl;
			}

			GuiComboBoxListControl::ItemStyleProperty GuiComboBoxListControl::GetItemTemplate()
			{
				return itemStyleProperty;
			}

			void GuiComboBoxListControl::SetItemTemplate(ItemStyleProperty value)
			{
				RemoveStyleController();
				itemStyleProperty = value;
				GetControlTemplateObject(true)->SetTextVisible(!itemStyleProperty);
				InstallStyleController(GetSelectedIndex());
				ItemTemplateChanged.Execute(GetNotifyEventArguments());
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
					return containedListControl->GetItemProvider()->GetBindingValue(selectedIndex);
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
