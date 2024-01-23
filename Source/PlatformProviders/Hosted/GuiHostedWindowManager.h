/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Hosted Window

Interfaces:
  Window<T>
  WindowManager<T>

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIHOSTEDCONTROLLER_GUIHOSTEDWINDOWMANAGER
#define VCZH_PRESENTATION_GUIHOSTEDCONTROLLER_GUIHOSTEDWINDOWMANAGER

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
				Window<T>*									parent = nullptr;
				collections::List<Window<T>*>				children;

				NativeRect									bounds;
				bool										topMost = false;
				bool										visible = false;
				bool										enabled = true;
				bool										active = false;
				bool										renderedAsActive = false;

			protected:

				Window<T>* GetVisibleParent()
				{
					auto visibleParent = parent;
					while (visibleParent && !visibleParent->visible)
					{
						visibleParent = visibleParent->parent;
					}
					return visibleParent;
				}

				template<typename TWindows>
				void CollectVisibleSubTreeInSamePriority(TWindows& windows, bool inTopMostLevel)
				{
					if (visible)
					{
						windows.Add(this);
					}
					for (auto child : children)
					{
						if (inTopMostLevel || !child->topMost)
						{
							child->CollectVisibleSubTreeInSamePriority(windows, inTopMostLevel);
						}
					}
				}

				void EnsureChildrenMovedInFrontOf(bool eventuallyTopMost, Window<T>* baseline)
				{
					auto&& orderedWindows = eventuallyTopMost ? windowManager->topMostedWindowsInOrder : windowManager->ordinaryWindowsInOrder;
					vint order = -1;
					if (baseline) order = orderedWindows.IndexOf(baseline);
					if (order == -1) order = orderedWindows.Count();

					for (auto child : children)
					{
						if (eventuallyTopMost || !child->topMost)
						{
							if (child->visible)
							{
								vint childOrder = orderedWindows.IndexOf(child);
								if (childOrder == -1 || childOrder > order)
								{
									windowManager->ordinaryWindowsInOrder.Remove(child);
									windowManager->topMostedWindowsInOrder.Remove(child);
									orderedWindows.Insert(order, child);
								}
							}
							child->EnsureChildrenMovedInFrontOf(eventuallyTopMost || child->topMost, (child->visible ? child : baseline));
						}
					}
				}

				void EnsureMovedInFrontOf(collections::List<Window<T>*>& windowsInOrder, Window<T>* baseline, bool wasEventuallyTopMost, bool eventuallyTopMost)
				{
					vint maxOrder = -1;
					vint order = windowsInOrder.IndexOf(this);

					if (wasEventuallyTopMost && order == -1)
					{
						maxOrder = 0;
					}
					else if (baseline)
					{
						maxOrder = windowsInOrder.IndexOf(baseline);
					}
					else if (order == -1)
					{
						maxOrder = windowsInOrder.Count();
					}
					else
					{
						maxOrder = windowsInOrder.Count() - 1;
					}

					if (order == -1)
					{
						windowsInOrder.Insert(maxOrder, this);
						windowManager->needRefresh = true;
					}
					else if (order > maxOrder)
					{
						windowsInOrder.RemoveAt(order);
						windowsInOrder.Insert(maxOrder, this);
						windowManager->needRefresh = true;
					}
				}

				void FixWindowInOrder(bool wasEventuallyTopMost, bool isEventuallyTopMost)
				{
					if (!visible)
					{
						if (windowManager->ordinaryWindowsInOrder.Remove(this) || windowManager->topMostedWindowsInOrder.Remove(this))
						{
							windowManager->needRefresh = true;
						}
					}

					auto visibleParent = GetVisibleParent();

					if (visible)
					{

						if (isEventuallyTopMost)
						{
							if (windowManager->ordinaryWindowsInOrder.Remove(this))
							{
								windowManager->needRefresh = true;
							}

							if (visibleParent && !visibleParent->IsEventuallyTopMost())
							{
								visibleParent = nullptr;
							}
							EnsureMovedInFrontOf(windowManager->topMostedWindowsInOrder, visibleParent, wasEventuallyTopMost, true);
						}
						else
						{
							if (windowManager->topMostedWindowsInOrder.Remove(this))
							{
								windowManager->needRefresh = true;
							}
							EnsureMovedInFrontOf(windowManager->ordinaryWindowsInOrder, visibleParent, wasEventuallyTopMost, false);
						}
					}

					EnsureChildrenMovedInFrontOf(isEventuallyTopMost, (visible ? this : visibleParent));
				}

				void FixRenderedAsActive()
				{
					if (enabled && visible)
					{
						auto current = windowManager->activeWindow;
						while (current && current != this)
						{
							current = current->parent;
						}
						if (current == this && !renderedAsActive)
						{
							renderedAsActive = true;
							windowManager->OnActivated(this);
						}
					}
					else if (active)
					{
						Deactivate();
					}
					else if (renderedAsActive)
					{
						renderedAsActive = false;
						windowManager->OnDeactivated(this);
					}
				}

			public:
				Window(T _id)
					: id(_id)
				{
				}

				~Window()
				{
				}

				bool IsEventuallyTopMost()
				{
					bool result = visible && topMost;
					auto current = parent;
					while (current && !result)
					{
						result |= current->visible && current->topMost;
						current = current->parent;
					}
					return result;
				}

#define ENSURE_WINDOW_MANAGER CHECK_ERROR(windowManager, ERROR_MESSAGE_PREFIX L"This operation can only be called between window manager's RegisterWindow and Stop.")

				void SetParent(Window<T>* value)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::Window<T>::SetParent(Window<T>*)#"
					ENSURE_WINDOW_MANAGER;

					if (parent == value) return;
					CHECK_ERROR(
						!windowManager || windowManager->mainWindow != this || !value,
						ERROR_MESSAGE_PREFIX L"A main window should not have a parent window."
						);

					if (!value)
					{
						value = windowManager->mainWindow;
					}

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
					bool isEventuallyTopMost = IsEventuallyTopMost();
					FixWindowInOrder(isEventuallyTopMost, isEventuallyTopMost);
#undef ERROR_MESSAGE_PREFIX
				}

				void SetBounds(const NativeRect& value)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::Window<T>::SetBounds(const NativeRect&)#"
					ENSURE_WINDOW_MANAGER;

					if (bounds == value) return;
					bounds = value;
					windowManager->needRefresh = true;
#undef ERROR_MESSAGE_PREFIX
				}

				void SetVisible(bool value)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::Window<T>::SetVisible(bool)#"
					ENSURE_WINDOW_MANAGER;

					if (visible == value) return;

					bool parentEventuallyTopMost = parent ? parent->IsEventuallyTopMost() : false;
					visible = value;
					FixRenderedAsActive();

					FixWindowInOrder(
						parentEventuallyTopMost || (!visible && topMost),
						parentEventuallyTopMost || (visible && topMost)
						);

					if (visible) windowManager->OnOpened(this); else windowManager->OnClosed(this);
#undef ERROR_MESSAGE_PREFIX
				}

				void SetTopMost(bool value)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::Window<T>::SetTopMost(bool)#"
					ENSURE_WINDOW_MANAGER;

					if (topMost == value) return;
					bool parentEventuallyTopMost = parent ? parent->IsEventuallyTopMost() : false;
					bool wasEventuallyTopMost = parentEventuallyTopMost || (visible && topMost);
					topMost = value;
					bool isEventuallyTopMost = parentEventuallyTopMost || (visible && topMost);
					FixWindowInOrder(wasEventuallyTopMost, isEventuallyTopMost);
#undef ERROR_MESSAGE_PREFIX
				}

				void SetEnabled(bool value)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::Window<T>::SetEnabled(bool)#"
					ENSURE_WINDOW_MANAGER;

					if (enabled == value) return;
					enabled = value;
					FixRenderedAsActive();

					windowManager->needRefresh = true;
					if (enabled) windowManager->OnEnabled(this); else windowManager->OnDisabled(this);
#undef ERROR_MESSAGE_PREFIX
				}

				void BringToFront()
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::Window<T>::BringToFront()#"
					ENSURE_WINDOW_MANAGER;

					bool eventuallyTopMost = IsEventuallyTopMost();
					auto&& orderedWindows = eventuallyTopMost ? windowManager->topMostedWindowsInOrder : windowManager->ordinaryWindowsInOrder;

					if (orderedWindows.Contains(this))
					{
						collections::SortedList<Window<T>*> windows;
						CollectVisibleSubTreeInSamePriority(windows, eventuallyTopMost);

						collections::List<Window<T>*> selected, remainings;
						for (auto window : orderedWindows)
						{
							if (windows.Contains(window))
							{
								selected.Add(window);
							}
							else
							{
								remainings.Add(window);
							}
						}

						if (collections::CompareEnumerable(selected, From(orderedWindows).Take(selected.Count())) != 0)
						{
							collections::CopyFrom(orderedWindows, selected);
							collections::CopyFrom(orderedWindows, remainings, true);
							windowManager->needRefresh = true;
						}
					}
					else
					{
						collections::List<Window<T>*> windows, remainings;
						CollectVisibleSubTreeInSamePriority(windows, eventuallyTopMost);

						CopyFrom(remainings, orderedWindows);
						orderedWindows.Clear();
						// TODO: (enumerable) foreach:reversed
						for (vint i = windows.Count() - 1; i >= 0; i--)
						{
							orderedWindows.Add(windows[i]);
						}
						CopyFrom(orderedWindows, remainings, true);
						windowManager->needRefresh = true;
					}
#undef ERROR_MESSAGE_PREFIX
				}

				void Activate()
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::Window<T>::Activate()#"
					ENSURE_WINDOW_MANAGER;

					if (active)
					{
						BringToFront();
						return;
					}
					if (!windowManager->mainWindow) return;
					if (!visible) return;
					if (!enabled) return;

					auto previous = windowManager->activeWindow;
					if (previous != this)
					{
						if (previous)
						{
							previous->active = false;
							windowManager->OnLostFocus(previous);
						}
						windowManager->activeWindow = this;
						active = true;
						windowManager->OnGotFocus(this);
					}

					vint previousCount = 0;
					vint thisCount = 0;
					Window<T>* commonParent = nullptr;
					{
						auto current = previous;
						while (current)
						{
							previousCount++;
							current = current->parent;
						}
					}
					{
						auto current = this;
						while (current)
						{
							thisCount++;
							current = current->parent;
						}
					}
					{
						auto previousStep = previous;
						auto thisStep = this;
						if (previousCount < thisCount)
						{
							while (thisCount-- != previousCount)
							{
								thisStep = thisStep->parent;
							}
						}
						else if (previousCount > thisCount)
						{
							while (previousCount-- != thisCount)
							{
								previousStep = previousStep->parent;
							}
						}

						while (previousStep && thisStep && previousStep != thisStep)
						{
							previousStep = previousStep->parent;
							thisStep = thisStep->parent;
						}
						commonParent = thisStep;
					}
					{
						auto current = previous;
						while (current != commonParent)
						{
							if (current->renderedAsActive)
							{
								current->renderedAsActive = false;
								windowManager->OnDeactivated(current);
							}
							current = current->parent;
						}
					}
					{
						auto current = this;
						while (current != commonParent)
						{
							if (current->enabled && !current->renderedAsActive)
							{
								current->renderedAsActive = true;
								windowManager->OnActivated(current);
							}
							current = current->parent;
						}
					}

					if (commonParent != previous || commonParent != this)
					{
						windowManager->needRefresh = true;
					}

					BringToFront();
#undef ERROR_MESSAGE_PREFIX
				}

				void Deactivate()
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::Window<T>::Deactivate()#"
					ENSURE_WINDOW_MANAGER;

					if (!windowManager->mainWindow) return;
					if (!active) return;
					active = false;
					renderedAsActive = false;
					windowManager->OnLostFocus(this);
					windowManager->OnDeactivated(this);

					if (windowManager->activeWindow == this)
					{
						auto current = parent;
						while (current && !current->enabled)
						{
							current = current->parent;
						}
						windowManager->activeWindow = current;
						windowManager->needRefresh = true;
						if (current)
						{
							current->active = true;
							windowManager->OnGotFocus(current);
						}
					}
#undef ERROR_MESSAGE_PREFIX
				}

				void Show()
				{
					SetVisible(true);
					Activate();
				}
#undef ENSURE_WINDOW_MANAGER
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

				virtual void OnOpened(Window<T>* window) = 0;
				virtual void OnClosed(Window<T>* window) = 0;
				virtual void OnEnabled(Window<T>* window) = 0;
				virtual void OnDisabled(Window<T>* window) = 0;
				virtual void OnGotFocus(Window<T>* window) = 0;
				virtual void OnLostFocus(Window<T>* window) = 0;
				virtual void OnActivated(Window<T>* window) = 0;
				virtual void OnDeactivated(Window<T>* window) = 0;

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
					if (mainWindow)
					{
						window->SetVisible(false);

						auto parent = window->parent;
						for (auto child : window->children)
						{
							child->parent = parent;
						}

						if (parent)
						{
							CopyFrom(parent->children, window->children, true);
							parent->children.Remove(window);
						}
						window->parent = nullptr;
						window->children.Clear();
					}

					registeredWindows.Remove(window->id);
					window->windowManager = nullptr;
#undef ERROR_MESSAGE_PREFIX
				}

				void Start(Window<T>* window)
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::WindowManager<T>::Start(Window<T>*)#"
					CHECK_ERROR(!mainWindow, ERROR_MESSAGE_PREFIX L"The window manager has started.");
					CHECK_ERROR(!window->parent, ERROR_MESSAGE_PREFIX L"A main window should not have a parent window.");

					mainWindow = window;
					for (auto normalWindow : registeredWindows.Values())
					{
						if (!normalWindow->parent && normalWindow != mainWindow)
						{
							normalWindow->parent = mainWindow;
							mainWindow->children.Add(normalWindow);
						}
					}
#undef ERROR_MESSAGE_PREFIX
				}

				void Stop()
				{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::hosted_window_manager::WindowManager<T>::Stop(Window<T>*)#"
					CHECK_ERROR(mainWindow, ERROR_MESSAGE_PREFIX L"The window manager has stopped.");
					mainWindow = nullptr;
					activeWindow = nullptr;
					ordinaryWindowsInOrder.Clear();
					topMostedWindowsInOrder.Clear();

					for (auto window : registeredWindows.Values())
					{
						window->parent = nullptr;
						window->children.Clear();

						if (window->active)
						{
							window->active = false;
							OnLostFocus(window);
						}

						if (window->renderedAsActive)
						{
							window->renderedAsActive = false;
							OnDeactivated(window);
						}

						if (window->visible)
						{
							window->visible = false;
							OnClosed(window);
						}
					}
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