#include "GuiComboControls.h"
#include "../../Application/Controls/GuiApplication.h"

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

			void GuiComboBoxBase::OnCachedBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				Size size=GetPreferredMenuClientSize();
				size.x=boundsComposition->GetCachedBounds().Width();
				SetPreferredMenuClientSize(size);
			}

			GuiComboBoxBase::GuiComboBoxBase(theme::ThemeName themeName)
				:GuiMenuButton(themeName)
			{
				CreateSubMenu();
				SetCascadeAction(false);

				boundsComposition->CachedBoundsChanged.AttachMethod(this, &GuiComboBoxBase::OnCachedBoundsChanged);
			}

			GuiComboBoxBase::~GuiComboBoxBase()
			{
			}

/***********************************************************************
GuiComboButton
***********************************************************************/

			GuiComboButton::GuiComboButton(theme::ThemeName themeName, GuiControl* _dropdownControl)
				:GuiComboBoxBase(themeName)
				, dropdownControl(_dropdownControl)
			{
				dropdownControl->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				GetSubMenu()->GetContainerComposition()->AddChild(dropdownControl->GetBoundsComposition());
			}

			GuiComboButton::~GuiComboButton()
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
			}

			void GuiComboBoxListControl::BeforeControlTemplateUninstalled()
			{
				GuiComboBoxBase::BeforeControlTemplateUninstalled();
			}

			void GuiComboBoxListControl::AfterControlTemplateInstalled(bool initialize)
			{
				GuiComboBoxBase::AfterControlTemplateInstalled(initialize);
				TypedControlTemplateObject(true)->SetTextVisible(!itemStyleProperty);
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
				if (auto subMenu = GetSubMenu())
				{
					Size expectedSize(0, GetDisplayFont().size * 20);
					Size adoptedSize = containedListControl->GetAdoptedSize(expectedSize);

					Size clientSize = GetPreferredMenuClientSize();
					vint height = adoptedSize.y + subMenu->GetClientSize().y - containedListControl->GetBoundsComposition()->GetCachedBounds().Height();
					if (clientSize.y != height)
					{
						clientSize.y = height;
						SetPreferredMenuClientSize(clientSize);
					}
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
				containedListControl->EnsureItemVisible(selectedIndex);
			}

			void GuiComboBoxListControl::OnListControlAdoptedSizeInvalidated(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				TryDelayExecuteIfNotDeleted([=]() { AdoptSubMenuSize(); });
			}

			void GuiComboBoxListControl::OnListControlItemMouseDown(compositions::GuiGraphicsComposition* sender, compositions::GuiItemMouseEventArgs& arguments)
			{
				DisplaySelectedContent(containedListControl->GetSelectedItemIndex());
				GetSubMenu()->Hide();
			}

			void GuiComboBoxListControl::OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments)
			{
				if (!arguments.autoRepeatKeyDown)
				{
					switch (arguments.code)
					{
					case VKEY::KEY_RETURN:
						DisplaySelectedContent(containedListControl->GetSelectedItemIndex());
						arguments.handled = true;
					case VKEY::KEY_ESCAPE:
						GetSubMenu()->Hide();
						arguments.handled = true;
						break;
					default:
						containedListControl->SelectItemsByKey(arguments.code, arguments.ctrl, arguments.shift);
					}
				}
			}

			void GuiComboBoxListControl::OnAttached(list::IItemProvider* provider)
			{
			}

			void GuiComboBoxListControl::OnItemModified(vint start, vint count, vint newCount, bool itemReferenceUpdated)
			{
				if (count == newCount && start <= selectedIndex && selectedIndex < start + count)
				{
					DisplaySelectedContent(selectedIndex);
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
				boundsComposition->GetEventReceiver()->keyDown.AttachMethod(this, &GuiComboBoxListControl::OnKeyDown);

				auto itemProvider = containedListControl->GetItemProvider();

				SelectedIndexChanged.SetAssociatedComposition(boundsComposition);

				containedListControl->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				GetSubMenu()->GetContainerComposition()->AddChild(containedListControl->GetBoundsComposition());
				SetFont(GetFont());
			}

			GuiComboBoxListControl::~GuiComboBoxListControl()
			{
				containedListControl->GetItemProvider()->DetachCallback(this);
				containedListControl->GetBoundsComposition()->CachedBoundsChanged.Detach(boundsChangedHandler);
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
				TypedControlTemplateObject(true)->SetTextVisible(!itemStyleProperty);
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

			list::IItemProvider* GuiComboBoxListControl::GetItemProvider()
			{
				return containedListControl->GetItemProvider();
			}
		}
	}
}
