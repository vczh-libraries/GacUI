/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Application Framework

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIAPPLICATION
#define VCZH_PRESENTATION_CONTROLS_GUIAPPLICATION

#include "GuiWindowControls.h"
#include "../GraphicsHost/GuiGraphicsHost_ShortcutKey.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
Application
***********************************************************************/

			/// <summary>Represents an GacUI application, for window management and asynchronized operation supporting. Use [M:vl.presentation.controls.GetApplication] to access the instance of this class.</summary>
			class GuiApplication : public Object, private INativeControllerListener, public Description<GuiApplication>
			{
				friend void GuiApplicationInitialize();
				friend class GuiControlHost;
				friend class GuiWindow;
				friend class GuiPopup;
				friend class Ptr<GuiApplication>;
			private:

				void											InvokeClipboardNotify(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments);
				void											ClipboardUpdated()override;
				void											GlobalShortcutKeyActivated(vint id)override;

			protected:
				using WindowMap = collections::Dictionary<INativeWindow*, GuiWindow*>;

				Locale											locale;
				GuiWindow*										mainWindow = nullptr;
				GuiWindow*										sharedTooltipOwnerWindow = nullptr;
				GuiControl*										sharedTooltipOwner = nullptr;
				GuiTooltip*										sharedTooltipControl = nullptr;
				bool											sharedTooltipHovering = false;
				bool											sharedTooltipClosing = false;
				collections::List<GuiWindow*>					windows;
				WindowMap										windowMap;
				collections::SortedList<GuiPopup*>				openingPopups;
				Ptr<compositions::GuiShortcutKeyManager>		globalShortcutKeyManager;

				GuiApplication();
				~GuiApplication();

				INativeWindow*									GetThreadContextNativeWindow(GuiControlHost* controlHost);
				void											RegisterWindow(GuiWindow* window);
				void											UnregisterWindow(GuiWindow* window);
				void											NotifyNativeWindowChanged(GuiControlHost* controlHost, INativeWindow* previousNativeWindow);
				void											RegisterPopupOpened(GuiPopup* popup);
				void											RegisterPopupClosed(GuiPopup* popup);
				void											TooltipMouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void											TooltipMouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Locale changed event.</summary>
				Event<void()>									LocaleChanged;

				/// <summary>Returns the selected locale for all windows.</summary>
				/// <returns>The selected locale.</returns>
				Locale											GetLocale();
				/// <summary>Set the locale for all windows.</summary>
				/// <param name="value">The selected locale.</param>
				void											SetLocale(Locale value);

				/// <summary>Run a <see cref="GuiWindow"/> as the main window and show it. This function can only be called once in the entry point. When the main window is closed or hiden, the Run function will finished, and the application should prepare for finalization.</summary>
				/// <param name="_mainWindow">The main window.</param>
				void											Run(GuiWindow* _mainWindow);
				/// <summary>
				/// Process minimum necessary events and execute some async tasks.
				/// </summary>
				/// <returns>Return false when the main window has been closed and all finalizing are done.</returns>
				bool											RunOneCycle();
				/// <summary>Get the main window.</summary>
				/// <returns>The main window.</returns>
				GuiWindow*										GetMainWindow();
				/// <summary>Get all created <see cref="GuiWindow"/> instances. This contains normal windows, popup windows, menus, or other types of windows that inherits from <see cref="GuiWindow"/>.</summary>
				/// <returns>All created <see cref="GuiWindow"/> instances.</returns>
				const collections::List<GuiWindow*>&			GetWindows();
				/// <summary>Get the <see cref="GuiWindow"/> instance that the mouse cursor are directly in.</summary>
				/// <returns>The <see cref="GuiWindow"/> instance that the mouse cursor are directly in.</returns>
				/// <param name="location">The mouse cursor.</param>
				GuiWindow*										GetWindow(NativePoint location);
				/// <summary>Get the <see cref="GuiWindow"/> instance that associated with the specified native window.</summary>
				/// <returns>The <see cref="GuiWindow"/> instance that associated with the specified native window.</returns>
				/// <param name="nativeWindow">The native window.</param>
				GuiWindow*										GetWindowFromNative(INativeWindow* nativeWindow);
				/// <summary>Show a tooltip.</summary>
				/// <param name="owner">The control that owns this tooltip temporary.</param>
				/// <param name="tooltip">The control as the tooltip content. This control is not owned by the tooltip. User should manually release this control if no longer needed (usually when the application exit).</param>
				/// <param name="preferredContentWidth">The preferred content width for this tooltip.</param>
				/// <param name="location">The relative location to specify the left-top position of the tooltip.</param>
				void											ShowTooltip(GuiControl* owner, GuiControl* tooltip, vint preferredContentWidth, Point location);
				/// <summary>Close the tooltip</summary>
				void											CloseTooltip();
				/// <summary>Get the tooltip owner. When the tooltip closed, it returns null.</summary>
				/// <returns>The tooltip owner.</returns>
				GuiControl*										GetTooltipOwner();
				/// <summary>Get the <see cref="compositions::IGuiShortcutKeyManager"/> attached with this control host.</summary>
				/// <returns>The shortcut key manager.</returns>
				compositions::IGuiShortcutKeyManager*			GetGlobalShortcutKeyManager();
				/// <summary>Get the file path of the current executable.</summary>
				/// <returns>The file path of the current executable.</returns>
				WString											GetExecutablePath();
				/// <summary>Get the folder of the current executable.</summary>
				/// <returns>The folder of the current executable.</returns>
				WString											GetExecutableFolder();

				/// <summary>Test is the current thread the main thread for GUI.</summary>
				/// <returns>Returns true if the current thread is the main thread for GUI.</returns>
				/// <param name="controlHost">A control host to access the corressponding main thread.</param>
				bool											IsInMainThread(GuiControlHost* controlHost);
				/// <summary>Invoke a specified function asynchronously.</summary>
				/// <param name="proc">The specified function.</param>
				void											InvokeAsync(const Func<void()>& proc);
				/// <summary>Invoke a specified function in the main thread.</summary>
				/// <param name="controlHost">A control host to access the corressponding main thread.</param>
				/// <param name="proc">The specified function.</param>
				void											InvokeInMainThread(GuiControlHost* controlHost, const Func<void()>& proc);
				/// <summary>Invoke a specified function in the main thread and wait for the function to complete or timeout.</summary>
				/// <returns>Return true if the function complete. Return false if the function has not completed during a specified period of time.</returns>
				/// <param name="controlHost">A control host to access the corressponding main thread.</param>
				/// <param name="proc">The specified function.</param>
				/// <param name="milliseconds">The specified period of time to wait. Set to -1 (default value) to wait forever until the function completed.</param>
				bool											InvokeInMainThreadAndWait(GuiControlHost* controlHost, const Func<void()>& proc, vint milliseconds=-1);
				/// <summary>Delay execute a specified function with an specified argument asynchronisly.</summary>
				/// <returns>The Delay execution controller for this task.</returns>
				/// <param name="proc">The specified function.</param>
				/// <param name="milliseconds">Time to delay.</param>
				Ptr<INativeDelay>								DelayExecute(const Func<void()>& proc, vint milliseconds);
				/// <summary>Delay execute a specified function with an specified argument in the main thread.</summary>
				/// <returns>The Delay execution controller for this task.</returns>
				/// <param name="proc">The specified function.</param>
				/// <param name="milliseconds">Time to delay.</param>
				Ptr<INativeDelay>								DelayExecuteInMainThread(const Func<void()>& proc, vint milliseconds);
				/// <summary>Run the specified function in the main thread and wait until it finishes. If the caller is in the main thread, then run the specified function directly.</summary>
				/// <param name="controlHost">A control host to access the corressponding main thread.</param>
				/// <param name="proc">The specified function.</param>
				void											RunGuiTask(GuiControlHost* controlHost, const Func<void()>& proc);

				/// <summary>Run the specified function in the main thread and wait until it finishes. If the caller is in the main thread, then run the specified function directly.</summary>
				/// <typeparam name="T">The return value of the function</typeparam>
				/// <param name="controlHost">A control host to access the corressponding main thread.</param>
				/// <param name="proc">The specified function.</param>
				/// <returns>The result of the function.</returns>
				template<typename T>
				T RunGuiValue(GuiControlHost* controlHost, const Func<T()>& proc)
				{
					T result;
					RunGuiTask(controlHost, [&result, &proc]()
					{
						result=proc();
					});
					return result;
				}
			};

/***********************************************************************
Helper Functions
***********************************************************************/

			/// <summary>Get the global <see cref="GuiApplication"/> object.</summary>
			/// <returns>The global <see cref="GuiApplication"/> object.</returns>
			extern GuiApplication*								GetApplication();
		}
	}
}

extern void GuiApplicationMain();

#define GUI_VALUE(HOST, VALUE) vl::presentation::controls::GetApplication()->RunGuiValue((HOST), vl::Func([&](){return (VALUE);}))
#define GUI_RUN(HOST, VALUE) vl::presentation::controls::GetApplication()->RunGuiTask((HOST), [&](){(VALUE);})

#endif