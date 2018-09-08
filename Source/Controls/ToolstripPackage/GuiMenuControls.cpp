#include "GuiMenuControls.h"
#include "../Templates/GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace compositions;

/***********************************************************************
IGuiMenuService
***********************************************************************/

			const wchar_t* const IGuiMenuService::Identifier = L"vl::presentation::controls::IGuiMenuService";
			const wchar_t* const IGuiMenuDropdownProvider::Identifier = L"vl::presentation::controls::IGuiMenuDropdownProvider";

			IGuiMenuService::IGuiMenuService()
				:openingMenu(0)
			{
			}

			void IGuiMenuService::MenuItemExecuted()
			{
				if(openingMenu)
				{
					openingMenu->Hide();
				}
				if(GetParentMenuService())
				{
					GetParentMenuService()->MenuItemExecuted();
				}
			}

			GuiMenu* IGuiMenuService::GetOpeningMenu()
			{
				return openingMenu;
			}

			void IGuiMenuService::MenuOpened(GuiMenu* menu)
			{
				if(openingMenu!=menu && openingMenu)
				{
					openingMenu->Hide();
				}
				openingMenu=menu;
			}

			void IGuiMenuService::MenuClosed(GuiMenu* menu)
			{
				if(openingMenu==menu)
				{
					openingMenu=0;
				}
			}

/***********************************************************************
GuiMenu
***********************************************************************/

			void GuiMenu::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiMenu::AfterControlTemplateInstalled_(bool initialize)
			{
			}

			IGuiMenuService* GuiMenu::GetParentMenuService()
			{
				return parentMenuService;
			}

			IGuiMenuService::Direction GuiMenu::GetPreferredDirection()
			{
				return IGuiMenuService::Vertical;
			}

			bool GuiMenu::IsActiveState()
			{
				return true;
			}

			bool GuiMenu::IsSubMenuActivatedByMouseDown()
			{
				return false;
			}

			void GuiMenu::MenuItemExecuted()
			{
				IGuiMenuService::MenuItemExecuted();
				Hide();
			}

			void GuiMenu::OnWindowOpened(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(parentMenuService)
				{
					parentMenuService->MenuOpened(this);
				}
			}

			void GuiMenu::OnDeactivatedAltHost()
			{
				if(hideOnDeactivateAltHost)
				{
					Hide();
				}
				GuiPopup::OnDeactivatedAltHost();
			}

			void GuiMenu::MouseClickedOnOtherWindow(GuiWindow* window)
			{
				GuiMenu* targetMenu=dynamic_cast<GuiMenu*>(window);
				if(!targetMenu)
				{
					Hide();
				}
			}

			void GuiMenu::OnWindowClosed(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(parentMenuService)
				{
					parentMenuService->MenuClosed(this);
					GuiMenu* openingSubMenu=GetOpeningMenu();
					if(openingSubMenu)
					{
						openingSubMenu->Hide();
					}
				}
			}

			GuiMenu::GuiMenu(theme::ThemeName themeName, GuiControl* _owner)
				:GuiPopup(themeName)
				, owner(_owner)
				, parentMenuService(0)
			{
				GetNativeWindow()->SetAlwaysPassFocusToParent(true);
				UpdateMenuService();
				WindowOpened.AttachMethod(this, &GuiMenu::OnWindowOpened);
				WindowClosed.AttachMethod(this, &GuiMenu::OnWindowClosed);
			}

			GuiMenu::~GuiMenu()
			{
			}

			void GuiMenu::UpdateMenuService()
			{
				if(owner)
				{
					parentMenuService=owner->QueryTypedService<IGuiMenuService>();
				}
			}

			IDescriptable* GuiMenu::QueryService(const WString& identifier)
			{
				if(identifier==IGuiMenuService::Identifier)
				{
					return (IGuiMenuService*)this;
				}
				else
				{
					return GuiPopup::QueryService(identifier);
				}
			}
			
			bool GuiMenu::GetHideOnDeactivateAltHost()
			{
				return hideOnDeactivateAltHost;
			}

			void GuiMenu::SetHideOnDeactivateAltHost(bool value)
			{
				hideOnDeactivateAltHost = value;
			}

/***********************************************************************
GuiMenuBar
***********************************************************************/

			IGuiMenuService* GuiMenuBar::GetParentMenuService()
			{
				return 0;
			}

			IGuiMenuService::Direction GuiMenuBar::GetPreferredDirection()
			{
				return IGuiMenuService::Horizontal;
			}

			bool GuiMenuBar::IsActiveState()
			{
				return GetOpeningMenu()!=0;
			}

			bool GuiMenuBar::IsSubMenuActivatedByMouseDown()
			{
				return true;
			}

			GuiMenuBar::GuiMenuBar(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
			}

			GuiMenuBar::~GuiMenuBar()
			{
			}

			IDescriptable* GuiMenuBar::QueryService(const WString& identifier)
			{
				if(identifier==IGuiMenuService::Identifier)
				{
					return (IGuiMenuService*)this;
				}
				else
				{
					return GuiControl::QueryService(identifier);
				}
			}

/***********************************************************************
GuiMenuButton
***********************************************************************/

			void GuiMenuButton::BeforeControlTemplateUninstalled_()
			{
				auto host = GetSubMenuHost();
				host->Clicked.Detach(hostClickedHandler);
				host->GetBoundsComposition()->GetEventReceiver()->mouseEnter.Detach(hostMouseEnterHandler);

				hostClickedHandler = nullptr;
				hostMouseEnterHandler = nullptr;
			}

			void GuiMenuButton::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
				auto host = GetSubMenuHost();

				ct->SetSubMenuOpening(GetSubMenuOpening());
				ct->SetLargeImage(largeImage);
				ct->SetImage(image);
				ct->SetShortcutText(shortcutText);
				ct->SetSubMenuExisting(subMenu != nullptr);

				hostClickedHandler = host->Clicked.AttachMethod(this, &GuiMenuButton::OnClicked);
				hostMouseEnterHandler = host->GetBoundsComposition()->GetEventReceiver()->mouseEnter.AttachMethod(this, &GuiMenuButton::OnMouseEnter);
			}

			GuiButton* GuiMenuButton::GetSubMenuHost()
			{
				GuiButton* button = GetControlTemplateObject(true)->GetSubMenuHost();
				return button ? button : this;
			}

			bool GuiMenuButton::OpenSubMenuInternal()
			{
				if (!GetSubMenuOpening())
				{
					if (ownerMenuService)
					{
						GuiMenu* openingSiblingMenu = ownerMenuService->GetOpeningMenu();
						if (openingSiblingMenu)
						{
							openingSiblingMenu->Hide();
						}
					}

					BeforeSubMenuOpening.Execute(GetNotifyEventArguments());
					if (subMenu)
					{
						subMenu->SetClientSize(preferredMenuClientSize);
						IGuiMenuService::Direction direction = GetSubMenuDirection();
						subMenu->ShowPopup(GetSubMenuHost(), direction == IGuiMenuService::Horizontal);
						AfterSubMenuOpening.Execute(GetNotifyEventArguments());
						return true;
					}
				}
				return false;
			}

			void GuiMenuButton::OnParentLineChanged()
			{
				GuiButton::OnParentLineChanged();
				ownerMenuService=QueryTypedService<IGuiMenuService>();
				if(ownerMenuService)
				{
					SetClickOnMouseUp(!ownerMenuService->IsSubMenuActivatedByMouseDown());
				}
				if(subMenu)
				{
					subMenu->UpdateMenuService();
				}
			}

			compositions::IGuiAltActionHost* GuiMenuButton::GetActivatingAltHost()
			{
				if (subMenu)
				{
					return subMenu->QueryTypedService<IGuiAltActionHost>();
				}
				else
				{
					return GuiSelectableButton::GetActivatingAltHost();
				}
			}

			void GuiMenuButton::OnSubMenuWindowOpened(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				SubMenuOpeningChanged.Execute(GetNotifyEventArguments());
				GetControlTemplateObject(true)->SetSubMenuOpening(true);
			}

			void GuiMenuButton::OnSubMenuWindowClosed(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				SubMenuOpeningChanged.Execute(GetNotifyEventArguments());
				GetControlTemplateObject(true)->SetSubMenuOpening(false);
			}

			void GuiMenuButton::OnMouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					if(cascadeAction && ownerMenuService && ownerMenuService->IsActiveState())
					{
						OpenSubMenuInternal();
					}
				}
			}

			void GuiMenuButton::OnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(GetVisuallyEnabled())
				{
					if(!OpenSubMenuInternal() && ownerMenuService)
					{
						ownerMenuService->MenuItemExecuted();
					}
				}
			}

			IGuiMenuService::Direction GuiMenuButton::GetSubMenuDirection()
			{
				return ownerMenuService?ownerMenuService->GetPreferredDirection():IGuiMenuService::Horizontal;
			}

			GuiMenu* GuiMenuButton::ProvideDropdownMenu()
			{
				return GetSubMenu();
			}

			GuiMenuButton::GuiMenuButton(theme::ThemeName themeName)
				:GuiSelectableButton(themeName)
				,subMenu(0)
				,ownedSubMenu(false)
				,ownerMenuService(0)
				,cascadeAction(true)
			{
				SetAutoSelection(false);
				BeforeSubMenuOpening.SetAssociatedComposition(boundsComposition);
				SubMenuOpeningChanged.SetAssociatedComposition(boundsComposition);
				LargeImageChanged.SetAssociatedComposition(boundsComposition);
				ImageChanged.SetAssociatedComposition(boundsComposition);
				ShortcutTextChanged.SetAssociatedComposition(boundsComposition);
			}

			GuiMenuButton::~GuiMenuButton()
			{
				if(subMenu && ownedSubMenu)
				{
					delete subMenu;
				}
			}

			Ptr<GuiImageData> GuiMenuButton::GetLargeImage()
			{
				return largeImage;
			}

			void GuiMenuButton::SetLargeImage(Ptr<GuiImageData> value)
			{
				if (largeImage != value)
				{
					largeImage = value;
					GetControlTemplateObject(true)->SetLargeImage(largeImage);
					LargeImageChanged.Execute(GetNotifyEventArguments());
				}
			}

			Ptr<GuiImageData> GuiMenuButton::GetImage()
			{
				return image;
			}

			void GuiMenuButton::SetImage(Ptr<GuiImageData> value)
			{
				if (image != value)
				{
					image = value;
					GetControlTemplateObject(true)->SetImage(image);
					ImageChanged.Execute(GetNotifyEventArguments());
				}
			}

			const WString& GuiMenuButton::GetShortcutText()
			{
				return shortcutText;
			}

			void GuiMenuButton::SetShortcutText(const WString& value)
			{
				if (shortcutText != value)
				{
					shortcutText = value;
					GetControlTemplateObject(true)->SetShortcutText(shortcutText);
					ShortcutTextChanged.Execute(GetNotifyEventArguments());
				}
			}

			bool GuiMenuButton::IsSubMenuExists()
			{
				return subMenu!=0;
			}

			GuiMenu* GuiMenuButton::GetSubMenu()
			{
				return subMenu;
			}

			GuiMenu* GuiMenuButton::CreateSubMenu(TemplateProperty<templates::GuiMenuTemplate> subMenuTemplate)
			{
				if (!subMenu)
				{
					GuiMenu* newSubMenu = new GuiMenu(theme::ThemeName::Menu, this);
					newSubMenu->SetControlTemplate(subMenuTemplate ? subMenuTemplate : GetControlTemplateObject(true)->GetSubMenuTemplate());
					SetSubMenu(newSubMenu, true);
				}
				return subMenu;
			}

			void GuiMenuButton::SetSubMenu(GuiMenu* value, bool owned)
			{
				if(subMenu)
				{
					if(ownedSubMenu)
					{
						delete subMenu;
					}
				}
				subMenu=value;
				ownedSubMenu=owned;
				if(subMenu)
				{
					subMenu->WindowOpened.AttachMethod(this, &GuiMenuButton::OnSubMenuWindowOpened);
					subMenu->WindowClosed.AttachMethod(this, &GuiMenuButton::OnSubMenuWindowClosed);
				}
				GetControlTemplateObject(true)->SetSubMenuExisting(subMenu != nullptr);
			}

			void GuiMenuButton::DestroySubMenu()
			{
				if(subMenu)
				{
					if(ownedSubMenu)
					{
						delete subMenu;
					}
					subMenu=0;
					ownedSubMenu=false;
					GetControlTemplateObject(true)->SetSubMenuExisting(false);
				}
			}

			bool GuiMenuButton::GetOwnedSubMenu()
			{
				return subMenu && ownedSubMenu;
			}

			bool GuiMenuButton::GetSubMenuOpening()
			{
				if(subMenu)
				{
					return subMenu->GetOpening();
				}
				else
				{
					return false;
				}
			}

			void GuiMenuButton::SetSubMenuOpening(bool value)
			{
				if (subMenu && subMenu->GetOpening() != value)
				{
					if (value)
					{
						OpenSubMenuInternal();
					}
					else
					{
						subMenu->Close();
					}
				}
			}

			Size GuiMenuButton::GetPreferredMenuClientSize()
			{
				return preferredMenuClientSize;
			}

			void GuiMenuButton::SetPreferredMenuClientSize(Size value)
			{
				preferredMenuClientSize=value;
			}

			bool GuiMenuButton::GetCascadeAction()
			{
				return cascadeAction;
			}

			void GuiMenuButton::SetCascadeAction(bool value)
			{
				cascadeAction=value;
			}

			IDescriptable* GuiMenuButton::QueryService(const WString& identifier)
			{
				if (identifier == IGuiMenuDropdownProvider::Identifier)
				{
					return (IGuiMenuDropdownProvider*)this;
				}
				else
				{
					return GuiSelectableButton::QueryService(identifier);
				}
			}
		}
	}
}