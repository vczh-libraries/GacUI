/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Hosted Window

Interfaces:
  Window<T>
  WindowManager<T>

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIHOSTEDWINDOWMANAGER
#define VCZH_PRESENTATION_GUIHOSTEDWINDOWMANAGER

#include "../../GuiTypes.h"

namespace vl
{
	namespace presentation
	{
		namespace hosted_window_manager
		{
			template<typename T>
			struct WindowManager;

/***********************************************************************
Window
***********************************************************************/

			template<typename T>
			struct Window
			{
				const T										id = {};
				const bool									normal = false;
				WindowManager<T>*							windowManager = nullptr;
				Window<T>*									parent = nullptr;
				collections::List<Window<T>*>				children;

				NativeRect									bounds;
				bool										topMost = false;
				bool										visible = false;
				bool										enabled = true;
				bool										active = false;
				bool										renderedAsActive = false;

				Window(T _id, bool _normal) :id(_id), normal(_normal) {}

				void SetParent(Window<T>* value)
				{
					CHECK_FAIL(L"Not Implemented.");
				}

				Window<T>* GetParent()
				{
					return !parent && normal && this != windowManager->mainWindow
						? windowManager->mainWindow
						: parent;
				}

				void SetBounds(const NativeRect& value)
				{
					bounds = value;
				}

				void SetVisible(bool value)
				{
					visible = value;
				}

				void SetTopMost(bool value)
				{
					CHECK_FAIL(L"Not Implemented.");
				}

				void SetEnabled(bool value)
				{
					CHECK_FAIL(L"Not Implemented.");
				}

				void Activate()
				{
					if (!windowManager->mainWindow) return;
					if (!visible) return;
					if (!enabled) return;
					if (windowManager->activeWindow != this)
					{
						if (windowManager->activeWindow)
						{
							windowManager->activeWindow->active = false;
						}
						windowManager->activeWindow = this;
						active = true;
					}
					renderedAsActive = true;
					// TODO: change renderedAsActive of other windows
					// TODO: bring the window to front
				}

				void Inactive()
				{
					if (!windowManager->mainWindow) return;
					active = false;
					if (windowManager->activeWindow == this)
					{
						windowManager->activeWindow = nullptr;
					}
				}

				void Show()
				{
					SetVisible(true);
					Activate();
				}
			};

/***********************************************************************
WindowManager
***********************************************************************/

			template<typename T>
			struct WindowManager
			{
				collections::Dictionary<T, Window<T>*>		registeredWindows;
				collections::List<Window<T>*>				topMostedWindowsInOrder;
				collections::List<Window<T>*>				ordinaryWindowsInOrder;

				Window<T>*									mainWindow = nullptr;
				Window<T>*									activeWindow = nullptr;

				void RegisterWindow(Window<T>* window)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::WindowManager<T>::RegisterWindow(Window<T>*)#"
					CHECK_ERROR(!window->windowManager, ERROR_MESSAGE_PREFIX L"The window has been registered.");
					CHECK_ERROR(!registeredWindows.Keys().Contains(window->id), ERROR_MESSAGE_PREFIX L"The window has a duplicated key with an existing window.");
					window->windowManager = this;
					registeredWindows.Add(window->id, window);
					ordinaryWindowsInOrder.Insert(0, window);
#undef ERROR_MESSAGE_PREFIX
				}

				void UnregisterWindow(Window<T>* window)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::WindowManager<T>::UnregisterWindow(Window<T>*)#"
					CHECK_ERROR(window->windowManager == this, ERROR_MESSAGE_PREFIX L"The window has not been registered.");
					CHECK_ERROR(window != mainWindow, ERROR_MESSAGE_PREFIX L"The main window cannot be unregistered before stopping the window manager.");
					if (activeWindow == window)
					{
						window->Inactive();
					}
					window->windowManager = nullptr;
					topMostedWindowsInOrder.Remove(window);
					ordinaryWindowsInOrder.Remove(window);
					registeredWindows.Remove(window->id);
#undef ERROR_MESSAGE_PREFIX
				}

				void Start(Window<T>* window)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::WindowManager<T>::Start(Window<T>*)#"
					CHECK_ERROR(!mainWindow, ERROR_MESSAGE_PREFIX L"The window manager has started.");
					mainWindow = window;
					// TODO: ensure parent window correctness
#undef ERROR_MESSAGE_PREFIX
				}

				void Stop()
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::WindowManager<T>::Stop(Window<T>*)#"
					CHECK_ERROR(mainWindow, ERROR_MESSAGE_PREFIX L"The window manager has stopped.");
					mainWindow = nullptr;
#undef ERROR_MESSAGE_PREFIX
				}

				Window<T>* HitTest(NativePoint position)
				{
					for (auto window : topMostedWindowsInOrder)
					{
						if (!window->visible) continue;
						if (window->bounds.Contains(position)) return window;
					}

					for (auto window : ordinaryWindowsInOrder)
					{
						if (!window->visible) continue;
						if (window->bounds.Contains(position)) return window;
					}

					return nullptr;
				}
			};
		}
	}
}

#endif