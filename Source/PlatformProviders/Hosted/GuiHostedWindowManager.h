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
				friend struct WindowManager<T>;
			protected:
				WindowManager<T>*							windowManager = nullptr;

			public:
				const T										id = {};
				const bool									normal = false;
				Window<T>*									parent = nullptr;
				collections::List<Window<T>*>				children;

				NativeRect									bounds;
				bool										topMost = false;
				bool										visible = false;
				bool										enabled = true;
				bool										active = false;
				bool										renderedAsActive = false;

			protected:

				void UpdateWindowOrder()
				{
					if (windowManager->ordinaryWindowsInOrder.Remove(this) || windowManager->topMostedWindowsInOrder.Remove(this))
					{
						windowManager->needRefresh = true;
					}

					if (visible)
					{
						if (IsEventuallyTopMost())
						{
							windowManager->topMostedWindowsInOrder.Insert(0, this);
						}
						else
						{
							windowManager->ordinaryWindowsInOrder.Insert(0, this);
						}
						windowManager->needRefresh = true;
					}
				}

			public:
				Window(T _id, bool _normal) :id(_id), normal(_normal)
				{
				}

				~Window()
				{
					for (auto child : children)
					{
						child->parent = nullptr;
					}
					children.Clear();
					SetParent(nullptr);
				}

				bool IsEventuallyTopMost()
				{
					bool result = topMost;
					auto current = parent;
					while (current)
					{
						result |= current->topMost;
						current = current->parent;
					}
					return result;
				}

				void SetParent(Window<T>* value)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::Window<T>::SetParent(Window<T>*)#"
					if (parent == value) return;
					CHECK_ERROR(
						windowManager->mainWindow != this || !value,
						ERROR_MESSAGE_PREFIX L"A main window should not have a parent window."
						);
					CHECK_ERROR(
						!normal || ! value || value->normal,
						ERROR_MESSAGE_PREFIX L"Window's parent window should not be a popup menu."
						);

					auto current = value;
					while (current)
					{
						CHECK_ERROR(current != this, ERROR_MESSAGE_PREFIX L"Parent window should not be cyclic.");
						current = current->parent;
					}

					if (parent)
					{
						parent->children.Remove(this);
					}
					parent = value;
					if (parent)
					{
						parent->children.Add(this);
					}
					UpdateWindowOrder();
#undef ERROR_MESSAGE_PREFIX
				}

				Window<T>* GetParent()
				{
					return !parent && normal && this != windowManager->mainWindow
						? windowManager->mainWindow
						: parent;
				}

				void SetBounds(const NativeRect& value)
				{
					if (bounds == value) return;
					bounds = value;
				}

				void SetVisible(bool value)
				{
					if (visible == value) return;
					visible = value;
					UpdateWindowOrder();
				}

				void SetTopMost(bool value)
				{
					if (topMost == value) return;
					topMost = value;
					UpdateWindowOrder();
				}

				void SetEnabled(bool value)
				{
					if (enabled == value) return;
					CHECK_FAIL(L"Not Implemented.");
				}

				void Activate()
				{
					if (active) return;
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
				bool										needRefresh = false;

				WindowManager()
				{
				}

#pragma warning(push)
#pragma warning(disable: 4297)
				~WindowManager()
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::WindowManager<T>::RegisterWindow(Window<T>*)#"
					CHECK_ERROR(registeredWindows.Count() == 0, ERROR_MESSAGE_PREFIX L"All windows must be unregistered before deleting the window manager.");
#undef ERROR_MESSAGE_PREFIX
				}
#pragma warning(pop)

				void RegisterWindow(Window<T>* window)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::WindowManager<T>::RegisterWindow(Window<T>*)#"
					CHECK_ERROR(!window->windowManager, ERROR_MESSAGE_PREFIX L"The window has been registered.");
					CHECK_ERROR(!registeredWindows.Keys().Contains(window->id), ERROR_MESSAGE_PREFIX L"The window has a duplicated key with an existing window.");
					CHECK_ERROR(!window->visible, ERROR_MESSAGE_PREFIX L"RegisterWindow must be called right after a window is created.");

					window->windowManager = this;
					registeredWindows.Add(window->id, window);
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
					registeredWindows.Remove(window->id);
					window->windowManager = nullptr;

					topMostedWindowsInOrder.Remove(window);
					ordinaryWindowsInOrder.Remove(window);
					needRefresh = true;
#undef ERROR_MESSAGE_PREFIX
				}

				void Start(Window<T>* window)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::WindowManager<T>::Start(Window<T>*)#"
					CHECK_ERROR(!mainWindow, ERROR_MESSAGE_PREFIX L"The window manager has started.");
					CHECK_ERROR(!window->parent, ERROR_MESSAGE_PREFIX L"A main window should not have a parent window.");
					mainWindow = window;
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