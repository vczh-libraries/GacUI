#include "GuiComboControls.h"
#include "../GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiComboBoxBase
***********************************************************************/

			void GuiComboBoxBase::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiComboBoxBase::AfterControlTemplateInstalled_(bool initialize)
			{
			}

			IGuiMenuService::Direction GuiComboBoxBase::GetSubMenuDirection()
			{
				return IGuiMenuService::Horizontal;
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

			void GuiComboBoxListControl::UpdateDisplayFont()
			{
				GuiControl::UpdateDisplayFont();
				if (itemStyleController)
				{
					itemStyleController->SetFont(GetDisplayFont());
				}
				AdoptSubMenuSize();
			}

			void GuiComboBoxListControl::BeforeControlTemplateUninstalled()
			{
				GuiComboBoxBase::BeforeControlTemplateUninstalled();
			}

			void GuiComboBoxListControl::AfterControlTemplateInstalled(bool initialize)
			{
				GuiComboBoxBase::AfterControlTemplateInstalled(initialize);
				GetControlTemplateObject(true)->SetTextVisible(!itemStyleProperty);
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
								itemStyleController->SetFont(GetDisplayFont());
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
				if (itemIndex == -1)
				{
					SetText(L"");
				}
				else
				{
					WString text = containedListControl->GetItemProvider()->GetTextValue(itemIndex);
					SetText(text);
				}

				RemoveStyleController();
				InstallStyleController(itemIndex);
				if (selectedIndex != itemIndex)
				{
					selectedIndex = itemIndex;
					SelectedIndexChanged.Execute(GetNotifyEventArguments());
				}
			}

			void GuiComboBoxListControl::AdoptSubMenuSize()
			{
				Size expectedSize(0, GetDisplayFont().size * 20);
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

			void GuiComboBoxListControl::OnAfterSubMenuOpening(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				containedListControl->SelectItemsByClick(selectedIndex, false, false, true);
				GetSubMenu()->GetNativeWindow()->SetFocus();
				containedListControl->SetFocus();
			}

			void GuiComboBoxListControl::OnListControlAdoptedSizeInvalidated(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				AdoptSubMenuSize();
			}

			void GuiComboBoxListControl::OnListControlBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				auto flag = GetDisposedFlag();
				GetApplication()->InvokeLambdaInMainThread(GetRelatedControlHost(), [=]()
				{
					if (!flag->IsDisposed())
					{
						AdoptSubMenuSize();
					}
				});
			}

			void GuiComboBoxListControl::OnListControlItemMouseDown(compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments)
			{
				DisplaySelectedContent(containedListControl->GetSelectedItemIndex());
				GetSubMenu()->Hide();
			}

			void GuiComboBoxListControl::OnListControlKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments)
			{
				if (!arguments.autoRepeatKeyDown)
				{
					switch (arguments.code)
					{
					case VKEY::_RETURN:
						DisplaySelectedContent(containedListControl->GetSelectedItemIndex());
						arguments.handled = true;
					case VKEY::_ESCAPE:
						GetSubMenu()->Hide();
						arguments.handled = true;
						break;
					default:;
					}
				}
			}

			void GuiComboBoxListControl::OnAttached(GuiListControl::IItemProvider* provider)
			{
			}

			void GuiComboBoxListControl::OnItemModified(vint start, vint count, vint newCount)
			{
				if (count == newCount)
				{
					if (start <= selectedIndex && selectedIndex < start + count)
					{
						DisplaySelectedContent(selectedIndex);
					}
				}
				else
				{
					DisplaySelectedContent(-1);
				}
			}

			GuiComboBoxListControl::GuiComboBoxListControl(theme::ThemeName themeName, GuiSelectableListControl* _containedListControl)
				:GuiComboBoxBase(themeName)
				, containedListControl(_containedListControl)
			{
				TextChanged.AttachMethod(this, &GuiComboBoxListControl::OnTextChanged);
				ContextChanged.AttachMethod(this, &GuiComboBoxListControl::OnContextChanged);
				VisuallyEnabledChanged.AttachMethod(this, &GuiComboBoxListControl::OnVisuallyEnabledChanged);
				AfterSubMenuOpening.AttachMethod(this, &GuiComboBoxListControl::OnAfterSubMenuOpening);

				containedListControl->GetItemProvider()->AttachCallback(this);
				containedListControl->SetMultiSelect(false);
				containedListControl->AdoptedSizeInvalidated.AttachMethod(this, &GuiComboBoxListControl::OnListControlAdoptedSizeInvalidated);
				containedListControl->ItemLeftButtonDown.AttachMethod(this, &GuiComboBoxListControl::OnListControlItemMouseDown);
				containedListControl->ItemRightButtonDown.AttachMethod(this, &GuiComboBoxListControl::OnListControlItemMouseDown);
				containedListControl->GetFocusableComposition()->GetEventReceiver()->keyDown.AttachMethod(this, &GuiComboBoxListControl::OnListControlKeyDown);
				boundsChangedHandler = containedListControl->GetBoundsComposition()->BoundsChanged.AttachMethod(this, &GuiComboBoxListControl::OnListControlBoundsChanged);

				auto itemProvider = containedListControl->GetItemProvider();

				SelectedIndexChanged.SetAssociatedComposition(boundsComposition);

				containedListControl->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				GetSubMenu()->GetContainerComposition()->AddChild(containedListControl->GetBoundsComposition());
				SetFont(GetFont());
			}

			GuiComboBoxListControl::~GuiComboBoxListControl()
			{
				containedListControl->GetItemProvider()->DetachCallback(this);
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
				InstallStyleController(selectedIndex);
				ItemTemplateChanged.Execute(GetNotifyEventArguments());
			}

			vint GuiComboBoxListControl::GetSelectedIndex()
			{
				return selectedIndex;
			}

			void GuiComboBoxListControl::SetSelectedIndex(vint value)
			{
				if (selectedIndex != value)
				{
					if (0 <= value && value < containedListControl->GetItemProvider()->Count())
					{
						DisplaySelectedContent(value);
					}
				}
				GetSubMenu()->Hide();
			}

			description::Value GuiComboBoxListControl::GetSelectedItem()
			{
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
