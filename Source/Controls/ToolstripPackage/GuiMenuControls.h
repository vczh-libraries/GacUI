/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIMENUCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIMENUCONTROLS

#include "../GuiButtonControls.h"
#include "../GuiWindowControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
Menu Service
***********************************************************************/

			class GuiMenu;

			/// <summary>IGuiMenuService is a required service for menu item container.</summary>
			class IGuiMenuService : public virtual IDescriptable, public Description<IGuiMenuService>
			{
			public:
				/// <summary>The identifier for this service.</summary>
				static const wchar_t* const				Identifier;

				/// <summary>Direction to decide the position for a menu with specified control.</summary>
				enum Direction
				{
					/// <summary>Aligned to the top or bottom side.</summary>
					Horizontal,
					/// <summary>Aligned to the left or right side.</summary>
					Vertical,
				};
			protected:
				GuiMenu*								openingMenu;
			public:
				IGuiMenuService();

				/// <summary>Get the parent service. This service represents the parent menu that host the menu item control that contains this menu.</summary>
				/// <returns>The parent service.</returns>
				virtual IGuiMenuService*				GetParentMenuService()=0;
				/// <summary>Get the preferred direction to open the sub menu.</summary>
				/// <returns>The preferred direction to open the sub menu.</returns>
				virtual Direction						GetPreferredDirection()=0;
				/// <summary>Test is this menu is active. When an menu is active, the sub menu is automatically opened when the corresponding menu item is opened.</summary>
				/// <returns>Returns true if this menu is active.</returns>
				virtual bool							IsActiveState()=0;
				/// <summary>Test all sub menu items are actived by mouse down.</summary>
				/// <returns>Returns true if all sub menu items are actived by mouse down.</returns>
				virtual bool							IsSubMenuActivatedByMouseDown()=0;

				/// <summary>Called when the menu item is executed.</summary>
				virtual void							MenuItemExecuted();
				/// <summary>Get the opening sub menu.</summary>
				/// <returns>The opening sub menu.</returns>
				virtual GuiMenu*						GetOpeningMenu();
				/// <summary>Called when the sub menu is opened.</summary>
				/// <param name="menu">The sub menu.</param>
				virtual void							MenuOpened(GuiMenu* menu);
				/// <summary>Called when the sub menu is closed.</summary>
				/// <param name="menu">The sub menu.</param>
				virtual void							MenuClosed(GuiMenu* menu);
			};

			/// <summary>IGuiMenuService is a required service to tell a ribbon group that this control has a dropdown to display.</summary>
			class IGuiMenuDropdownProvider : public virtual IDescriptable, public Description<IGuiMenuDropdownProvider>
			{
			public:
				/// <summary>The identifier for this service.</summary>
				static const wchar_t* const				Identifier;

				/// <summary>Get the dropdown to display.</summary>
				/// <returns>The dropdown to display. Returns null to indicate the dropdown cannot be displaied temporary.</returns>
				virtual GuiMenu*						ProvideDropdownMenu() = 0;
			};

/***********************************************************************
Menu
***********************************************************************/

			/// <summary>Popup menu.</summary>
			class GuiMenu : public GuiPopup, protected IGuiMenuService, public Description<GuiMenu>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(MenuTemplate, GuiPopup)
			private:
				IGuiMenuService*						parentMenuService;
				bool									hideOnDeactivateAltHost = true;

				IGuiMenuService*						GetParentMenuService()override;
				Direction								GetPreferredDirection()override;
				bool									IsActiveState()override;
				bool									IsSubMenuActivatedByMouseDown()override;
				void									MenuItemExecuted()override;

			protected:
				GuiControl*								owner;

				void									OnDeactivatedAltHost()override;
				void									MouseClickedOnOtherWindow(GuiWindow* window)override;
				void									OnWindowOpened(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									OnWindowClosed(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_owner">The owner menu item of the parent menu.</param>
				GuiMenu(theme::ThemeName themeName, GuiControl* _owner);
				~GuiMenu();

				/// <summary>Update the reference to the parent <see cref="IGuiMenuService"/>. This function is not required to call outside the menu or menu item control.</summary>
				void									UpdateMenuService();
				IDescriptable*							QueryService(const WString& identifier)override;

				/// <summary>Test if this menu hide after pressing ESC key to exit to the upper level of ALT shortcuts.</summary>
				/// <returns>Returns true if this menu hide after pressing ESC key to exit to the upper level of ALT shortcuts.</returns>
				bool									GetHideOnDeactivateAltHost();
				/// <summary>Set if this menu hide after pressing ESC key to exit to the upper level of ALT shortcuts.</summary>
				/// <param name="value">Set to true to make this menu hide after pressing ESC key to exit to the upper level of ALT shortcuts.</param>
				void									SetHideOnDeactivateAltHost(bool value);
			};
			
			/// <summary>Menu bar.</summary>
			class GuiMenuBar : public GuiControl, protected IGuiMenuService, public Description<GuiMenuBar>
			{
			private:
				IGuiMenuService*						GetParentMenuService()override;
				Direction								GetPreferredDirection()override;
				bool									IsActiveState()override;
				bool									IsSubMenuActivatedByMouseDown()override;

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiMenuBar(theme::ThemeName themeName);
				~GuiMenuBar();
				
				IDescriptable*							QueryService(const WString& identifier)override;
			};

/***********************************************************************
MenuButton
***********************************************************************/

			/// <summary>Menu item.</summary>
			class GuiMenuButton : public GuiSelectableButton, private IGuiMenuDropdownProvider, public Description<GuiMenuButton>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(ToolstripButtonTemplate, GuiSelectableButton)

				using IEventHandler = compositions::IGuiGraphicsEventHandler;
			protected:
				Ptr<IEventHandler>						hostClickedHandler;
				Ptr<IEventHandler>						hostMouseEnterHandler;
				Ptr<GuiImageData>						image;
				Ptr<GuiImageData>						largeImage;
				WString									shortcutText;
				GuiMenu*								subMenu;
				bool									ownedSubMenu;
				Size									preferredMenuClientSize;
				IGuiMenuService*						ownerMenuService;
				bool									cascadeAction;

				GuiButton*								GetSubMenuHost();
				bool									OpenSubMenuInternal();
				void									OnParentLineChanged()override;
				compositions::IGuiAltActionHost*		GetActivatingAltHost()override;

				void									OnSubMenuWindowOpened(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									OnSubMenuWindowClosed(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									OnMouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									OnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

				virtual IGuiMenuService::Direction		GetSubMenuDirection();

			private:
				GuiMenu*								ProvideDropdownMenu()override;

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiMenuButton(theme::ThemeName themeName);
				~GuiMenuButton();

				/// <summary>Before sub menu opening event.</summary>
				compositions::GuiNotifyEvent			BeforeSubMenuOpening;
				/// <summary>After sub menu opening event.</summary>
				compositions::GuiNotifyEvent			AfterSubMenuOpening;
				/// <summary>Sub menu opening changed event.</summary>
				compositions::GuiNotifyEvent			SubMenuOpeningChanged;
				/// <summary>Large image changed event.</summary>
				compositions::GuiNotifyEvent			LargeImageChanged;
				/// <summary>Image changed event.</summary>
				compositions::GuiNotifyEvent			ImageChanged;
				/// <summary>Shortcut text changed event.</summary>
				compositions::GuiNotifyEvent			ShortcutTextChanged;

				/// <summary>Get the large image for the menu button.</summary>
				/// <returns>The large image for the menu button.</returns>
				Ptr<GuiImageData>						GetLargeImage();
				/// <summary>Set the large image for the menu button.</summary>
				/// <param name="value">The large image for the menu button.</param>
				void									SetLargeImage(Ptr<GuiImageData> value);
				/// <summary>Get the image for the menu button.</summary>
				/// <returns>The image for the menu button.</returns>
				Ptr<GuiImageData>						GetImage();
				/// <summary>Set the image for the menu button.</summary>
				/// <param name="value">The image for the menu button.</param>
				void									SetImage(Ptr<GuiImageData> value);
				/// <summary>Get the shortcut key text for the menu button.</summary>
				/// <returns>The shortcut key text for the menu button.</returns>
				const WString&							GetShortcutText();
				/// <summary>Set the shortcut key text for the menu button.</summary>
				/// <param name="value">The shortcut key text for the menu button.</param>
				void									SetShortcutText(const WString& value);

				/// <summary>Test does the sub menu exist.</summary>
				/// <returns>Returns true if the sub menu exists.</returns>
				bool									IsSubMenuExists();
				/// <summary>Get the sub menu. If the sub menu is not created, it returns null.</summary>
				/// <returns>The sub menu.</returns>
				GuiMenu*								GetSubMenu();
				/// <summary>Create the sub menu if necessary. The created sub menu is owned by this menu button.</summary>
				/// <returns>The created sub menu.</returns>
				/// <param name="subMenuTemplate">The style controller for the sub menu. Set to null to use the default control template.</param>
				GuiMenu*								CreateSubMenu(TemplateProperty<templates::GuiMenuTemplate> subMenuTemplate = {});
				/// <summary>Associate a sub menu if there is no sub menu binded in this menu button. The associated sub menu is not owned by this menu button if the "owned" argument is set to false.</summary>
				/// <param name="value">The sub menu to associate.</param>
				/// <param name="owned">Set to true if the menu is expected to be owned.</param>
				void									SetSubMenu(GuiMenu* value, bool owned);
				/// <summary>Destroy the sub menu if necessary.</summary>
				void									DestroySubMenu();
				/// <summary>Test is the sub menu owned by this menu button. If the sub menu is owned, both deleting this menu button or calling <see cref="DestroySubMenu"/> will delete the sub menu.</summary>
				/// <returns>Returns true if the sub menu is owned by this menu button.</returns>
				bool									GetOwnedSubMenu();

				/// <summary>Test is the sub menu opened.</summary>
				/// <returns>Returns true if the sub menu is opened.</returns>
				bool									GetSubMenuOpening();
				/// <summary>Open or close the sub menu.</summary>
				/// <param name="value">Set to true to open the sub menu.</param>
				void									SetSubMenuOpening(bool value);

				/// <summary>Get the preferred client size for the sub menu.</summary>
				/// <returns>The preferred client size for the sub menu.</returns>
				Size									GetPreferredMenuClientSize();
				/// <summary>Set the preferred client size for the sub menu.</summary>
				/// <param name="value">The preferred client size for the sub menu.</param>
				void									SetPreferredMenuClientSize(Size value);

				/// <summary>Test is cascade action enabled. If the cascade action is enabled, when the mouse enter this menu button, the sub menu will be automatically opened if the parent menu is in an active state (see <see cref="IGuiMenuService::IsActiveState"/>), closing the sub menu will also close the parent menu.</summary>
				/// <returns>Returns true if cascade action is enabled.</returns>
				bool									GetCascadeAction();
				/// <summary>Enable or disable cascade action.</summary>
				/// <param name="value">Set to true to enable cascade action.</param>
				void									SetCascadeAction(bool value);

				IDescriptable*							QueryService(const WString& identifier)override;
			};
		}
	}
}

#endif