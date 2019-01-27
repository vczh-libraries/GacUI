/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIWINDOWCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIWINDOWCONTROLS

#include "GuiBasicControls.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			class IGuiShortcutKeyManager;
			class GuiGraphicsTimerManager;
		}

		namespace controls
		{

/***********************************************************************
Control Host
***********************************************************************/

			/// <summary>
			/// Represents a control that host by a <see cref="INativeWindow"/>.
			/// </summary>
			class GuiControlHost : public GuiControl, public GuiInstanceRootObject, protected INativeWindowListener, public Description<GuiControlHost>
			{
				friend class compositions::GuiGraphicsHost;
			protected:
				compositions::GuiGraphicsHost*					host;

				virtual void									OnNativeWindowChanged();
				virtual void									OnVisualStatusChanged();
			protected:
				static const vint								TooltipDelayOpenTime = 500;
				static const vint								TooltipDelayCloseTime = 500;
				static const vint								TooltipDelayLifeTime = 5000;

				Ptr<INativeDelay>								tooltipOpenDelay;
				Ptr<INativeDelay>								tooltipCloseDelay;
				Point											tooltipLocation;

				bool											calledDestroyed = false;
				bool											deleteWhenDestroyed = false;

				controls::GuiControlHost*						GetControlHostForInstance()override;
				GuiControl*										GetTooltipOwner(Point location);
				void											MoveIntoTooltipControl(GuiControl* tooltipControl, Point location);
				void											MouseMoving(const NativeWindowMouseInfo& info)override;
				void											MouseLeaved()override;
				void											Moved()override;
				void											Enabled()override;
				void											Disabled()override;
				void											GotFocus()override;
				void											LostFocus()override;
				void											Activated()override;
				void											Deactivated()override;
				void											Opened()override;
				void											Closing(bool& cancel)override;
				void											Closed()override;
				void											Destroying()override;

				virtual void									UpdateClientSizeAfterRendering(Size clientSize);
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiControlHost(theme::ThemeName themeName);
				~GuiControlHost();
				
				/// <summary>Window got focus event.</summary>
				compositions::GuiNotifyEvent					WindowGotFocus;
				/// <summary>Window lost focus event.</summary>
				compositions::GuiNotifyEvent					WindowLostFocus;
				/// <summary>Window activated event.</summary>
				compositions::GuiNotifyEvent					WindowActivated;
				/// <summary>Window deactivated event.</summary>
				compositions::GuiNotifyEvent					WindowDeactivated;
				/// <summary>Window opened event.</summary>
				compositions::GuiNotifyEvent					WindowOpened;
				/// <summary>Window closing event.</summary>
				compositions::GuiRequestEvent					WindowClosing;
				/// <summary>Window closed event.</summary>
				compositions::GuiNotifyEvent					WindowClosed;
				/// <summary>Window destroying event.</summary>
				compositions::GuiNotifyEvent					WindowDestroying;

				/// <summary>Delete this control host after processing all events.</summary>
				void											DeleteAfterProcessingAllEvents();

				/// <summary>Get the internal <see cref="compositions::GuiGraphicsHost"/> object to host the window content.</summary>
				/// <returns>The internal <see cref="compositions::GuiGraphicsHost"/> object to host the window content.</returns>
				compositions::GuiGraphicsHost*					GetGraphicsHost();
				/// <summary>Get the main composition to host the window content.</summary>
				/// <returns>The main composition to host the window content.</returns>
				compositions::GuiGraphicsComposition*			GetMainComposition();
				/// <summary>Get the internal <see cref="INativeWindow"/> object to host the content.</summary>
				/// <returns>The the internal <see cref="INativeWindow"/> object to host the content.</returns>
				INativeWindow*									GetNativeWindow();
				/// <summary>Set the internal <see cref="INativeWindow"/> object to host the content.</summary>
				/// <param name="window">The the internal <see cref="INativeWindow"/> object to host the content.</param>
				void											SetNativeWindow(INativeWindow* window);
				/// <summary>Force to calculate layout and size immediately</summary>
				void											ForceCalculateSizeImmediately();
				
				/// <summary>Test is the window enabled.</summary>
				/// <returns>Returns true if the window is enabled.</returns>
				bool											GetEnabled()override;
				/// <summary>Enable or disable the window.</summary>
				/// <param name="value">Set to true to enable the window.</param>
				void											SetEnabled(bool value)override;
				/// <summary>Test is the window focused.</summary>
				/// <returns>Returns true if the window is focused.</returns>
				bool											GetFocused()override;
				/// <summary>Focus the window.</summary>
				void											SetFocused();
				/// <summary>Test is the window activated.</summary>
				/// <returns>Returns true if the window is activated.</returns>
				bool											GetActivated();
				/// <summary>Activate the window.</summary>
				void											SetActivated();
				/// <summary>Test is the window icon shown in the task bar.</summary>
				/// <returns>Returns true if the window is icon shown in the task bar.</returns>
				bool											GetShowInTaskBar();
				/// <summary>Show or hide the window icon in the task bar.</summary>
				/// <param name="value">Set to true to show the window icon in the task bar.</param>
				void											SetShowInTaskBar(bool value);
				/// <summary>Test is the window allowed to be activated.</summary>
				/// <returns>Returns true if the window is allowed to be activated.</returns>
				bool											GetEnabledActivate();
				/// <summary>Allow or forbid the window to be activated.</summary>
				/// <param name="value">Set to true to allow the window to be activated.</param>
				void											SetEnabledActivate(bool value);
				/// <summary>
				/// Test is the window always on top of the desktop.
				/// </summary>
				/// <returns>Returns true if the window is always on top of the desktop.</returns>
				bool											GetTopMost();
				/// <summary>
				/// Make the window always or never on top of the desktop.
				/// </summary>
				/// <param name="topmost">True to make the window always  on top of the desktop.</param>
				void											SetTopMost(bool topmost);

				/// <summary>Get the <see cref="compositions::IGuiShortcutKeyManager"/> attached with this control host.</summary>
				/// <returns>The shortcut key manager.</returns>
				compositions::IGuiShortcutKeyManager*			GetShortcutKeyManager();
				/// <summary>Attach or detach the <see cref="compositions::IGuiShortcutKeyManager"/> associated with this control host. When this control host is disposing, the associated shortcut key manager will be deleted if exists.</summary>
				/// <param name="value">The shortcut key manager. Set to null to detach the previous shortcut key manager from this control host.</param>
				void											SetShortcutKeyManager(compositions::IGuiShortcutKeyManager* value);
				/// <summary>Get the timer manager.</summary>
				/// <returns>The timer manager.</returns>
				compositions::GuiGraphicsTimerManager*			GetTimerManager();

				/// <summary>Get the client size of the window.</summary>
				/// <returns>The client size of the window.</returns>
				Size											GetClientSize();
				/// <summary>Set the client size of the window.</summary>
				/// <param name="value">The client size of the window.</param>
				void											SetClientSize(Size value);
				/// <summary>Get the location of the window in screen space.</summary>
				/// <returns>The location of the window.</returns>
				NativePoint										GetLocation();
				/// <summary>Set the location of the window in screen space.</summary>
				/// <param name="value">The location of the window.</param>
				void											SetLocation(NativePoint value);
				/// <summary>Set the location in screen space and the client size of the window.</summary>
				/// <param name="location">The location of the window.</param>
				/// <param name="size">The client size of the window.</param>
				void											SetBounds(NativePoint location, Size size);

				GuiControlHost*									GetRelatedControlHost()override;
				const WString&									GetText()override;
				void											SetText(const WString& value)override;

				/// <summary>Get the screen that contains the window.</summary>
				/// <returns>The screen that contains the window.</returns>
				INativeScreen*									GetRelatedScreen();
				/// <summary>
				/// Show the window.
				/// </summary>
				void											Show();
				/// <summary>
				/// Show the window without activation.
				/// </summary>
				void											ShowDeactivated();
				/// <summary>
				/// Restore the window.
				/// </summary>
				void											ShowRestored();
				/// <summary>
				/// Maximize the window.
				/// </summary>
				void											ShowMaximized();
				/// <summary>
				/// Minimize the window.
				/// </summary>
				void											ShowMinimized();
				/// <summary>
				/// Hide the window.
				/// </summary>
				void											Hide();
				/// <summary>
				/// Close the window and destroy the internal <see cref="INativeWindow"/> object.
				/// </summary>
				void											Close();
				/// <summary>Test is the window opened.</summary>
				/// <returns>Returns true if the window is opened.</returns>
				bool											GetOpening();
			};

/***********************************************************************
Window
***********************************************************************/

			/// <summary>
			/// Represents a normal window.
			/// </summary>
			class GuiWindow : public GuiControlHost, protected compositions::GuiAltActionHostBase, public AggregatableDescription<GuiWindow>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(WindowTemplate, GuiControlHost)
				friend class GuiApplication;
			protected:
				compositions::IGuiAltActionHost*		previousAltHost = nullptr;
				bool									hasMaximizedBox = true;
				bool									hasMinimizedBox = true;
				bool									hasBorder = true;
				bool									hasSizeBox = true;
				bool									isIconVisible = true;
				bool									hasTitleBar = true;
				Ptr<GuiImageData>						icon;
				
				void									UpdateCustomFramePadding(INativeWindow* window, templates::GuiWindowTemplate* ct);
				void									SyncNativeWindowProperties();
				void									Moved()override;
				void									DpiChanged()override;
				void									OnNativeWindowChanged()override;
				void									OnVisualStatusChanged()override;
				virtual void							MouseClickedOnOtherWindow(GuiWindow* window);
				
				void									OnWindowActivated(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									OnWindowDeactivated(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiWindow(theme::ThemeName themeName);
				~GuiWindow();

				IDescriptable*							QueryService(const WString& identifier)override;

				/// <summary>Clipboard updated event.</summary>
				compositions::GuiNotifyEvent			ClipboardUpdated;

				/// <summary>Move the window to the center of the screen. If multiple screens exist, the window move to the screen that contains the biggest part of the window.</summary>
				void									MoveToScreenCenter();
				/// <summary>Move the window to the center of the specified screen.</summary>
				/// <param name="screen">The screen.</param>
				void									MoveToScreenCenter(INativeScreen* screen);
				
				/// <summary>
				/// Test is the maximize box visible.
				/// </summary>
				/// <returns>Returns true if the maximize box is visible.</returns>
				bool									GetMaximizedBox();
				/// <summary>
				/// Make the maximize box visible or invisible.
				/// </summary>
				/// <param name="visible">True to make the maximize box visible.</param>
				void									SetMaximizedBox(bool visible);
				/// <summary>
				/// Test is the minimize box visible.
				/// </summary>
				/// <returns>Returns true if the minimize box is visible.</returns>
				bool									GetMinimizedBox();
				/// <summary>
				/// Make the minimize box visible or invisible.
				/// </summary>
				/// <param name="visible">True to make the minimize box visible.</param>
				void									SetMinimizedBox(bool visible);
				/// <summary>
				/// Test is the border visible.
				/// </summary>
				/// <returns>Returns true if the border is visible.</returns>
				bool									GetBorder();
				/// <summary>
				/// Make the border visible or invisible.
				/// </summary>
				/// <param name="visible">True to make the border visible.</param>
				void									SetBorder(bool visible);
				/// <summary>
				/// Test is the size box visible.
				/// </summary>
				/// <returns>Returns true if the size box is visible.</returns>
				bool									GetSizeBox();
				/// <summary>
				/// Make the size box visible or invisible.
				/// </summary>
				/// <param name="visible">True to make the size box visible.</param>
				void									SetSizeBox(bool visible);
				/// <summary>
				/// Test is the icon visible.
				/// </summary>
				/// <returns>Returns true if the icon is visible.</returns>
				bool									GetIconVisible();
				/// <summary>
				/// Make the icon visible or invisible.
				/// </summary>
				/// <param name="visible">True to make the icon visible.</param>
				void									SetIconVisible(bool visible);
				/// <summary>
				/// Get the icon which replaces the default one.
				/// </summary>
				/// <returns>Returns the icon that replaces the default one.</returns>
				Ptr<GuiImageData>						GetIcon();
				/// <summary>
				/// Set the icon that replaces the default one.
				/// </summary>
				/// <param name="value">The icon that replaces the default one.</param>
				void									SetIcon(Ptr<GuiImageData> value);
				/// <summary>
				/// Test is the title bar visible.
				/// </summary>
				/// <returns>Returns true if the title bar is visible.</returns>
				bool									GetTitleBar();
				/// <summary>
				/// Make the title bar visible or invisible.
				/// </summary>
				/// <param name="visible">True to make the title bar visible.</param>
				void									SetTitleBar(bool visible);
				/// <summary>
				/// Show a model window, get a callback when the window is closed.
				/// </summary>
				/// <param name="owner">The window to disable as a parent window.</param>
				/// <param name="callback">The callback to call after the window is closed.</param>
				void									ShowModal(GuiWindow* owner, const Func<void()>& callback);
				/// <summary>
				/// Show a model window, get a callback when the window is closed, and then delete itself.
				/// </summary>
				/// <param name="owner">The window to disable as a parent window.</param>
				/// <param name="callback">The callback to call after the window is closed.</param>
				void									ShowModalAndDelete(GuiWindow* owner, const Func<void()>& callback);
				/// <summary>
				/// Show a model window as an async operation, which ends when the window is closed.
				/// </summary>
				/// <returns>Returns true if the size box is visible.</returns>
				/// <param name="owner">The window to disable as a parent window.</param>
				Ptr<reflection::description::IAsync>	ShowModalAsync(GuiWindow* owner);
			};
			
			/// <summary>
			/// Represents a popup window. When the mouse click on other window or the desktop, the popup window will be closed automatically.
			/// </summary>
			class GuiPopup : public GuiWindow, public Description<GuiPopup>
			{
			protected:
				union PopupInfo
				{
					struct _s1 { NativePoint location; INativeScreen* screen; };
					struct _s2 { GuiControl* control; INativeWindow* controlWindow; Rect bounds; bool preferredTopBottomSide; };
					struct _s3 { GuiControl* control; INativeWindow* controlWindow; Point location; };
					struct _s4 { GuiControl* control; INativeWindow* controlWindow; bool preferredTopBottomSide; };

					_s1 _1;
					_s2 _2;
					_s3 _3;
					_s4 _4;

					PopupInfo() {}
				};
			protected:
				vint									popupType = -1;
				PopupInfo								popupInfo;

				void									UpdateClientSizeAfterRendering(Size clientSize)override;
				void									MouseClickedOnOtherWindow(GuiWindow* window)override;
				void									PopupOpened(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									PopupClosed(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments);

				static bool								IsClippedByScreen(NativeSize size, NativePoint location, INativeScreen* screen);
				static NativePoint						CalculatePopupPosition(NativeSize windowSize, NativePoint location, INativeScreen* screen);
				static NativePoint						CalculatePopupPosition(NativeSize windowSize, GuiControl* control, INativeWindow* controlWindow, Rect bounds, bool preferredTopBottomSide);
				static NativePoint						CalculatePopupPosition(NativeSize windowSize, GuiControl* control, INativeWindow* controlWindow, Point location);
				static NativePoint						CalculatePopupPosition(NativeSize windowSize, GuiControl* control, INativeWindow* controlWindow, bool preferredTopBottomSide);
				static NativePoint						CalculatePopupPosition(NativeSize windowSize, vint popupType, const PopupInfo& popupInfo);

				void									ShowPopupInternal();
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiPopup(theme::ThemeName themeName);
				~GuiPopup();

				/// <summary>Test will the whole popup window be in the screen if the popup's left-top position is set to a specified value.</summary>
				/// <returns>Returns true if the whole popup window will be in the screen.</returns>
				/// <param name="location">The specified left-top position.</param>
				bool									IsClippedByScreen(Point location);
				/// <summary>Show the popup window with the left-top position set to a specified value. The position of the popup window will be adjusted to make it totally inside the screen if possible.</summary>
				/// <param name="location">The specified left-top position.</param>
				/// <param name="screen">The expected screen. If you don't want to specify any screen, don't set this parameter.</param>
				void									ShowPopup(NativePoint location, INativeScreen* screen = 0);
				/// <summary>Show the popup window with the bounds set to a specified control-relative value. The position of the popup window will be adjusted to make it totally inside the screen if possible.</summary>
				/// <param name="control">The control that owns this popup temporary. And the location is relative to this control.</param>
				/// <param name="bounds">The specified bounds.</param>
				/// <param name="preferredTopBottomSide">Set to true if the popup window is expected to be opened at the top or bottom side of that bounds.</param>
				void									ShowPopup(GuiControl* control, Rect bounds, bool preferredTopBottomSide);
				/// <summary>Show the popup window with the left-top position set to a specified control-relative value. The position of the popup window will be adjusted to make it totally inside the screen if possible.</summary>
				/// <param name="control">The control that owns this popup temporary. And the location is relative to this control.</param>
				/// <param name="location">The specified left-top position.</param>
				void									ShowPopup(GuiControl* control, Point location);
				/// <summary>Show the popup window aligned with a specified control. The position of the popup window will be adjusted to make it totally inside the screen if possible.</summary>
				/// <param name="control">The control that owns this popup temporary.</param>
				/// <param name="preferredTopBottomSide">Set to true if the popup window is expected to be opened at the top or bottom side of that control.</param>
				void									ShowPopup(GuiControl* control, bool preferredTopBottomSide);
			};

			/// <summary>Represents a tooltip window.</summary>
			class GuiTooltip : public GuiPopup, private INativeControllerListener, public Description<GuiTooltip>
			{
			protected:
				GuiControl*								temporaryContentControl;

				void									GlobalTimer()override;
				void									TooltipOpened(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									TooltipClosed(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiTooltip(theme::ThemeName themeName);
				~GuiTooltip();

				/// <summary>Get the preferred content width.</summary>
				/// <returns>The preferred content width.</returns>
				vint									GetPreferredContentWidth();
				/// <summary>Set the preferred content width.</summary>
				/// <param name="value">The preferred content width.</param>
				void									SetPreferredContentWidth(vint value);

				/// <summary>Get the temporary content control.</summary>
				/// <returns>The temporary content control.</returns>
				GuiControl*								GetTemporaryContentControl();
				/// <summary>Set the temporary content control.</summary>
				/// <param name="control">The temporary content control.</param>
				void									SetTemporaryContentControl(GuiControl* control);
			};
		}
	}
}

#endif
