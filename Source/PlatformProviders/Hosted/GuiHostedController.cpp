#include "GuiHostedController.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;

/***********************************************************************
GuiHostedController
***********************************************************************/

		NativePoint GuiHostedController::GetPointInClientSpace(NativePoint location)
		{
			auto windowBounds = nativeWindow->GetBounds();
			auto clientBounds = nativeWindow->GetClientBoundsInScreen();
			location.x.value += windowBounds.x1.value - clientBounds.x1.value;
			location.y.value += windowBounds.y1.value - clientBounds.y1.value;
			return location;
		}

		GuiHostedWindow* GuiHostedController::HitTestInClientSpace(NativePoint location)
		{
			auto window = wmManager->HitTest(location);
			return window ? window->id : nullptr;
		}

		void GuiHostedController::UpdateHoveringWindow(Nullable<NativePoint> location)
		{
			if (location)
			{
				hoveringLocation = location.Value();
			}
			hoveringWindow = HitTestInClientSpace(hoveringLocation);
		}

		void GuiHostedController::UpdateEnteringWindow(GuiHostedWindow* window)
		{
			if (enteringWindow != window)
			{
				if (enteringWindow)
				{
					for (auto listener : enteringWindow->listeners)
					{
						listener->MouseLeaved();
					}
				}
				enteringWindow = window;
				if (enteringWindow)
				{
					for (auto listener : enteringWindow->listeners)
					{
						listener->MouseEntered();
					}
				}
			}
		}

/***********************************************************************
GuiHostedController::WindowManager<GuiHostedWindow*>
***********************************************************************/

		void GuiHostedController::OnOpened(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			if (!mainWindow || window != &mainWindow->wmWindow)
			{
				for (auto listener : window->id->listeners)
				{
					listener->Opened();
				}
			}
		}

		void GuiHostedController::OnClosed(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->Closed();
			}
		}

		void GuiHostedController::OnEnabled(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->Enabled();
			}
		}

		void GuiHostedController::OnDisabled(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->Disabled();
			}
		}

		void GuiHostedController::OnGotFocus(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			window->id->BecomeFocusedWindow();
			for (auto listener : window->id->listeners)
			{
				listener->GotFocus();
			}
		}

		void GuiHostedController::OnLostFocus(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->LostFocus();
			}
		}

		void GuiHostedController::OnActivated(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->RenderingAsActivated();
			}
		}

		void GuiHostedController::OnDeactivated(hosted_window_manager::Window<GuiHostedWindow*>* window)
		{
			for (auto listener : window->id->listeners)
			{
				listener->RenderingAsDeactivated();
			}
		}

/***********************************************************************
GuiHostedController::INativeWindowListener
***********************************************************************/

		INativeWindowListener::HitTestResult GuiHostedController::HitTest(NativePoint location)
		{
			if (mainWindow && mainWindow->IsEnabled())
			{
				auto point = GetPointInClientSpace(location);
				auto window = HitTestInClientSpace(point);
				if (window == mainWindow)
				{
					return PerformHitTest(From(mainWindow->listeners), point);
				}
			}
			return INativeWindowListener::HitTestResult::NoDecision;
		}

		void GuiHostedController::Moving(NativeRect& bounds, bool fixSizeOnly, bool draggingBorder)
		{
			if (mainWindow)
			{
				auto windowBounds = nativeWindow->GetBounds();
				auto clientBounds = nativeWindow->GetClientBoundsInScreen();
				auto w = clientBounds.Width().value - windowBounds.Width().value;
				auto h = clientBounds.Height().value - windowBounds.Height().value;

				NativeRect mainBounds;
				mainBounds.x2 = bounds.Width().value - w;
				mainBounds.y2 = bounds.Height().value - h;

				for (auto listener : mainWindow->listeners)
				{
					listener->Moving(mainBounds, fixSizeOnly, draggingBorder);
				}

				bounds.x1.value += mainBounds.x1.value;
				bounds.y1.value += mainBounds.y1.value;
				bounds.x2.value = bounds.x1.value + mainBounds.Width().value + w;
				bounds.y2.value = bounds.y1.value + mainBounds.Height().value + h;
			}
		}

		void GuiHostedController::Moved()
		{
			if (mainWindow)
			{
				auto size = mainWindow->GetBounds().GetSize();
				auto clientSize = nativeWindow->GetClientSize();
				if (size != clientSize)
				{
					mainWindow->SetBounds({ {},clientSize });
				}

				for (auto listener : mainWindow->listeners)
				{
					listener->Moved();
				}
			}
		}

		void GuiHostedController::DpiChanged(bool preparing)
		{
			if (!preparing)
			{
				hostedResourceManager->nativeManager->RecreateRenderTarget(nativeWindow);
				wmManager->needRefresh = true;
			}

			for (auto hostedWindow : createdWindows)
			{
				for (auto listener : hostedWindow->listeners)
				{
					listener->DpiChanged(preparing);
				}
			}
		}

		void GuiHostedController::GotFocus()
		{
			if (lastFocusedWindow)
			{
				lastFocusedWindow->wmWindow.Activate();
				lastFocusedWindow = nullptr;
			}
			else if (mainWindow)
			{
				mainWindow->wmWindow.Activate();
			}
		}

		void GuiHostedController::Opened()
		{
			if (mainWindow)
			{
				for (auto listener : mainWindow->wmWindow.id->listeners)
				{
					listener->Opened();
				}
			}
		}

		void GuiHostedController::LostFocus()
		{
			lastFocusedWindow = wmManager->activeWindow ? wmManager->activeWindow->id : nullptr;
			while (wmManager->activeWindow)
			{
				wmManager->activeWindow->Deactivate();
			}
		}

		void GuiHostedController::BeforeClosing(bool& cancel)
		{
			if (mainWindow)
			{
				for (auto listener : mainWindow->listeners)
				{
					listener->BeforeClosing(cancel);
					if (cancel) return;
				}
			}
		}

		void GuiHostedController::AfterClosing()
		{
			if (mainWindow)
			{
				for (auto listener : mainWindow->listeners)
				{
					listener->AfterClosing();
				}
			}
		}

		void GuiHostedController::Paint()
		{
		}

/***********************************************************************
GuiHostedController::INativeWindowListener (GetSelectedWindow)
***********************************************************************/

		GuiHostedWindow* GuiHostedController::GetSelectedWindow_MouseDown(const NativeWindowMouseInfo& info)
		{
			if (!capturingWindow)
			{
				SortedList<GuiHostedWindow*> survivedPopups;
				auto current = hoveringWindow;
				while (current)
				{
					if (current->IsVisible() && current->GetWindowMode() != INativeWindow::Normal)
					{
						survivedPopups.Add(current);
					}
					current = current->wmWindow.parent ? current->wmWindow.parent->id : nullptr;
				}

				List<GuiHostedWindow*> closingPopups;
				CopyFrom(
					closingPopups,
					From(wmManager->ordinaryWindowsInOrder)
						.Concat(wmManager->topMostedWindowsInOrder)
						.Select([](auto window) { return window->id; })
						.Where([&](auto window) { return window->GetWindowMode() != INativeWindow::Normal && !survivedPopups.Contains(window); })
					);
				for (auto popupWindow : closingPopups)
				{
					popupWindow->Hide(false);
				}
			}

			auto selectedWindow = capturingWindow ? capturingWindow : hoveringWindow;
			return selectedWindow;
		}

		GuiHostedWindow* GuiHostedController::GetSelectedWindow_MouseMoving(const NativeWindowMouseInfo& info)
		{
			UpdateHoveringWindow({ { info.x,info.y } });
			auto selectedWindow = capturingWindow ? capturingWindow : hoveringWindow;
			UpdateEnteringWindow(selectedWindow);
			return selectedWindow;
		}

		GuiHostedWindow* GuiHostedController::GetSelectedWindow_Other(const NativeWindowMouseInfo& info)
		{
			auto selectedWindow = capturingWindow ? capturingWindow : hoveringWindow;
			return selectedWindow;
		}

/***********************************************************************
GuiHostedController::INativeWindowListener (PreAction)
***********************************************************************/

		void GuiHostedController::PreAction_LeftButtonDown(const NativeWindowMouseInfo& info)
		{
			PreAction_MouseDown(info);

			if (!capturingWindow && !wmWindow && hoveringWindow && hoveringWindow != mainWindow && hoveringWindow->IsEnabled())
			{
				auto x = info.x.value - hoveringWindow->wmWindow.bounds.x1.value;
				auto y = info.y.value - hoveringWindow->wmWindow.bounds.y1.value;
				auto hitTestResult = PerformHitTest(From(hoveringWindow->listeners), { {x},{y} });

				switch (hitTestResult)
				{
#define HANDLE_HIT_TEST_RESULT(NAME)\
				case INativeWindowListener::NAME:				\
					wmOperation = WindowManagerOperation::NAME;	\
					break;										\

				HANDLE_HIT_TEST_RESULT(Title)
				HANDLE_HIT_TEST_RESULT(BorderLeft)
				HANDLE_HIT_TEST_RESULT(BorderRight)
				HANDLE_HIT_TEST_RESULT(BorderTop)
				HANDLE_HIT_TEST_RESULT(BorderBottom)
				HANDLE_HIT_TEST_RESULT(BorderLeftTop)
				HANDLE_HIT_TEST_RESULT(BorderRightBottom)
				HANDLE_HIT_TEST_RESULT(BorderRightTop)
				HANDLE_HIT_TEST_RESULT(BorderLeftBottom)
				default:;

#undef HANDLE_HIT_TEST_RESULT
				}

				switch (wmOperation)
				{
				case WindowManagerOperation::None:
					return;
				case WindowManagerOperation::Title:
					if (!hoveringWindow->GetTitleBar())
					{
						wmOperation = WindowManagerOperation::None;
						return;
					}
					break;
				default:
					if (!hoveringWindow->GetSizeBox())
					{
						wmOperation = WindowManagerOperation::None;
						return;
					}
				}

				wmWindow = hoveringWindow;
				nativeWindow->RequireCapture();

				switch (wmOperation)
				{
				case WindowManagerOperation::Title:
				case WindowManagerOperation::BorderLeft:
				case WindowManagerOperation::BorderLeftTop:
				case WindowManagerOperation::BorderLeftBottom:
					wmRelative.x.value = x;
					break;
				case WindowManagerOperation::BorderTop:
				case WindowManagerOperation::BorderBottom:
					wmRelative.x.value = wmWindow->wmWindow.bounds.Width().value / 2;
					break;
				case WindowManagerOperation::BorderRight:
				case WindowManagerOperation::BorderRightTop:
				case WindowManagerOperation::BorderRightBottom:
					wmRelative.x.value = wmWindow->wmWindow.bounds.Width().value - x;
					break;
				default:;
				}

				switch (wmOperation)
				{
				case WindowManagerOperation::Title:
				case WindowManagerOperation::BorderTop:
				case WindowManagerOperation::BorderLeftTop:
				case WindowManagerOperation::BorderRightTop:
					wmRelative.y.value = y;
					break;
				case WindowManagerOperation::BorderLeft:
				case WindowManagerOperation::BorderRight:
					wmRelative.y.value = wmWindow->wmWindow.bounds.Height().value / 2;
					break;
				case WindowManagerOperation::BorderBottom:
				case WindowManagerOperation::BorderLeftBottom:
				case WindowManagerOperation::BorderRightBottom:
					wmRelative.y.value = wmWindow->wmWindow.bounds.Height().value - y;
					break;
				default:;
				}
			}
		}

		void GuiHostedController::PreAction_MouseDown(const NativeWindowMouseInfo& info)
		{
			if (!capturingWindow && !wmWindow && hoveringWindow && hoveringWindow->IsEnabled() && hoveringWindow->IsEnabledActivate())
			{
				hoveringWindow->SetActivate();
			}
		}

		void GuiHostedController::PreAction_MouseMoving(const NativeWindowMouseInfo& info)
		{
			if (!capturingWindow && !wmWindow && hoveringWindow && hoveringWindow != mainWindow && hoveringWindow->IsEnabled() && hoveringWindow->GetSizeBox())
			{
				auto x = info.x.value - hoveringWindow->wmWindow.bounds.x1.value;
				auto y = info.y.value - hoveringWindow->wmWindow.bounds.y1.value;
				auto hitTestResult = PerformHitTest(From(hoveringWindow->listeners), { {x},{y} });
				auto cursor = GetCursorFromHitTest(hitTestResult, ResourceService());
				if (cursor == nullptr)
				{
					cursor = hoveringWindow->GetWindowCursor();
				}
				nativeWindow->SetWindowCursor(cursor);
			}

			if (wmWindow)
			{
				auto oldBounds = wmWindow->wmWindow.bounds;
				auto newBounds = oldBounds;
				vint mouseX = info.x.value;
				vint mouseY = info.y.value;
				vint displayX = mainWindow->wmWindow.bounds.Width().value;
				vint displayY = mainWindow->wmWindow.bounds.Height().value;

				if (mouseX < 0)
				{
					mouseX = 0;
				}
				else if (mouseX >= displayX)
				{
					mouseX = displayX - 1;
				}

				if (mouseY < 0)
				{
					mouseY = 0;
				}
				else if (mouseY >= displayY)
				{
					mouseY = displayY - 1;
				}

				if (wmOperation == WindowManagerOperation::Title)
				{
					newBounds = {
						{
							{mouseX - wmRelative.x.value},
							{mouseY - wmRelative.y.value}
						},
						oldBounds.GetSize()
					};
				}
				else
				{
					switch (wmOperation)
					{
					case WindowManagerOperation::BorderLeft:
					case WindowManagerOperation::BorderLeftTop:
					case WindowManagerOperation::BorderLeftBottom:
						newBounds.x1.value = mouseX - wmRelative.x.value;
						break;
					case WindowManagerOperation::BorderRight:
					case WindowManagerOperation::BorderRightTop:
					case WindowManagerOperation::BorderRightBottom:
						newBounds.x2.value = mouseX + wmRelative.x.value;
						break;
					default:;
					}

					switch (wmOperation)
					{
					case WindowManagerOperation::BorderTop:
					case WindowManagerOperation::BorderLeftTop:
					case WindowManagerOperation::BorderRightTop:
						newBounds.y1.value = mouseY - wmRelative.y.value;
						break;
					case WindowManagerOperation::BorderBottom:
					case WindowManagerOperation::BorderLeftBottom:
					case WindowManagerOperation::BorderRightBottom:
						newBounds.y2.value = mouseY + wmRelative.y.value;
						break;
					default:;
					}
				}

				for (auto listener : wmWindow->listeners)
				{
					listener->Moving(newBounds, false, wmOperation != WindowManagerOperation::Title);
				}

				wmWindow->wmWindow.SetBounds(newBounds);

				for (auto listener : wmWindow->listeners)
				{
					listener->Moved();
				}
			}
		}

		void GuiHostedController::PreAction_Other(const NativeWindowMouseInfo& info)
		{
		}

/***********************************************************************
GuiHostedController::INativeWindowListener (PostAction)
***********************************************************************/

		void GuiHostedController::PostAction_LeftButtonUp(GuiHostedWindow* selectedWindow, const NativeWindowMouseInfo& info)
		{
			if (!capturingWindow && !wmWindow && selectedWindow && selectedWindow != mainWindow && selectedWindow->IsEnabled())
			{
				auto x = info.x.value - hoveringWindow->wmWindow.bounds.x1.value;
				auto y = info.y.value - hoveringWindow->wmWindow.bounds.y1.value;
				auto hitTestResult = PerformHitTest(From(hoveringWindow->listeners), { {x},{y} });
				if (hitTestResult == INativeWindowListener::ButtonClose)
				{
					hoveringWindow->Hide(true);
				}
			}

			if (wmWindow)
			{
				wmWindow = nullptr;
				wmOperation = WindowManagerOperation::None;
				nativeWindow->ReleaseCapture();
			}
		}

		void GuiHostedController::PostAction_Other(GuiHostedWindow* selectedWindow, const NativeWindowMouseInfo& info)
		{
		}

/***********************************************************************
GuiHostedController::INativeWindowListener (Template)
***********************************************************************/


		template<
			void (GuiHostedController::* PreAction)(const NativeWindowMouseInfo&),
			GuiHostedWindow*(GuiHostedController::* GetSelectedWindow)(const NativeWindowMouseInfo&),
			void (GuiHostedController::* PostAction)(GuiHostedWindow*, const NativeWindowMouseInfo&),
			void (INativeWindowListener::* Callback)(const NativeWindowMouseInfo&)
		>
		void GuiHostedController::HandleMouseCallback(const NativeWindowMouseInfo& info)
		{
			(this->*PreAction)(info);
			auto postActionWindow = hoveringWindow;
			if (!wmWindow)
			{
				if (auto selectedWindow = (this->*GetSelectedWindow)(info))
				{
					postActionWindow = selectedWindow;
					if (!selectedWindow->IsEnabled()) return;
					auto adjustedInfo = info;
					adjustedInfo.x.value -= selectedWindow->wmWindow.bounds.x1.value;
					adjustedInfo.y.value -= selectedWindow->wmWindow.bounds.y1.value;
					for (auto listener : selectedWindow->listeners)
					{
						(listener->*Callback)(adjustedInfo);
					}
				}
			}
			(this->*PostAction)(postActionWindow, info);
		}

		template<
			typename TInfo,
			void (INativeWindowListener::* Callback)(const TInfo&)
		>
		void GuiHostedController::HandleKeyboardCallback(const TInfo& info)
		{
			if (wmManager->activeWindow && !wmWindow)
			{
				auto hostedWindow = wmManager->activeWindow->id;
				for (auto listener : hostedWindow->listeners)
				{
					(listener->*Callback)(info);
				}
			}
		}

/***********************************************************************
GuiHostedController::INativeWindowListener (IO Event Handling)
***********************************************************************/

#define IMPLEMENT_MOUSE_CALLBACK(NAME, PREACTION, POLICY, POSTACTION)					\
		void GuiHostedController::NAME(const NativeWindowMouseInfo& info)				\
		{																				\
			HandleMouseCallback<														\
				&GuiHostedController::PreAction_##PREACTION,							\
				&GuiHostedController::GetSelectedWindow_##POLICY,						\
				&GuiHostedController::PostAction_##POSTACTION,							\
				&INativeWindowListener::NAME											\
			>(info);																	\
		}																				\

		IMPLEMENT_MOUSE_CALLBACK(LeftButtonDown,			LeftButtonDown,	MouseDown,		Other			)
		IMPLEMENT_MOUSE_CALLBACK(LeftButtonUp,				Other,			Other,			LeftButtonUp	)
		IMPLEMENT_MOUSE_CALLBACK(LeftButtonDoubleClick,		Other,			Other,			Other			)
		IMPLEMENT_MOUSE_CALLBACK(RightButtonDown,			MouseDown,		MouseDown,		Other			)
		IMPLEMENT_MOUSE_CALLBACK(RightButtonUp,				Other,			Other,			Other			)
		IMPLEMENT_MOUSE_CALLBACK(RightButtonDoubleClick,	Other,			Other,			Other			)
		IMPLEMENT_MOUSE_CALLBACK(MiddleButtonDown,			MouseDown,		MouseDown,		Other			)
		IMPLEMENT_MOUSE_CALLBACK(MiddleButtonUp,			Other,			Other,			Other			)
		IMPLEMENT_MOUSE_CALLBACK(MiddleButtonDoubleClick,	Other,			Other,			Other			)
		IMPLEMENT_MOUSE_CALLBACK(HorizontalWheel,			Other,			Other,			Other			)
		IMPLEMENT_MOUSE_CALLBACK(VerticalWheel,				Other,			Other,			Other			)
		IMPLEMENT_MOUSE_CALLBACK(MouseMoving,				MouseMoving,	MouseMoving,	Other			)

#undef IMPLEMENT_MOUSE_CALLBACK

		void GuiHostedController::MouseEntered()
		{
		}

		void GuiHostedController::MouseLeaved()
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiHostedController::MouseLeaved()#"
			UpdateEnteringWindow(nullptr);
#undef ERROR_MESSAGE_PREFIX
		}

#define IMPLEMENT_KEY_CALLBACK(NAME, TYPE)										\
		void GuiHostedController::NAME(const NativeWindow##TYPE##Info& info)	\
		{																		\
			HandleKeyboardCallback<												\
				NativeWindow##TYPE##Info,										\
				&INativeWindowListener::NAME									\
			>(info);															\
		}																		\

		IMPLEMENT_KEY_CALLBACK(KeyDown, Key)
		IMPLEMENT_KEY_CALLBACK(KeyUp, Key)
		IMPLEMENT_KEY_CALLBACK(Char, Char)

#undef IMPLEMENT_KEY_CALLBACK

/***********************************************************************
GuiHostedController::INativeControllerListener
***********************************************************************/

		void GuiHostedController::GlobalTimer()
		{
			callbackService.InvokeGlobalTimer();

			if (hostedResourceManager && nativeWindow && nativeWindow->IsVisible())
			{
				auto renderTarget = hostedResourceManager->nativeManager->GetRenderTarget(nativeWindow);
				if (renderTarget->IsInHostedRendering())
				{
					return;
				}

				for (auto visibleWindow : From(wmManager->ordinaryWindowsInOrder).Concat(wmManager->topMostedWindowsInOrder))
				{
					for (auto listener : visibleWindow->id->listeners)
					{
						if (listener->NeedRefresh())
						{
							wmManager->needRefresh = true;
							goto NEED_REFRESH;
						}
					}
				}

				if (!wmManager->needRefresh && !windowsUpdatedInLastFrame)
				{
					return;
				}

			NEED_REFRESH:
				wmManager->needRefresh = false;
				windowsUpdatedInLastFrame = false;

				while (true)
				{
					renderTarget->StartHostedRendering();
					bool failureByResized = false;
					bool failureByLostDevice = false;

					auto forceRefreshWindows = [&](List<hosted_window_manager::Window<GuiHostedWindow*>*>& windows)
					{
						// TODO: (enumerable) foreach:reversed
						for (vint i = windows.Count() - 1; i >= 0; i--)
						{
							auto hostedWindow = windows[i]->id;
							for (auto listener : hostedWindow->listeners)
							{
								bool updated = false;
								listener->ForceRefresh(false, updated, failureByResized, failureByLostDevice);
								windowsUpdatedInLastFrame |= updated;
								if (failureByResized || failureByLostDevice)
								{
									return false;
								}
							}
						}
						return true;
					};

					if (!forceRefreshWindows(wmManager->ordinaryWindowsInOrder)) goto STOP_RENDERING;
					if (!forceRefreshWindows(wmManager->topMostedWindowsInOrder)) goto STOP_RENDERING;

				STOP_RENDERING:
					switch (renderTarget->StopHostedRendering())
					{
					case elements::RenderTargetFailure::LostDevice:
						failureByLostDevice = true;
						break;
					case elements::RenderTargetFailure::ResizeWhileRendering:
						failureByResized = true;
						break;
					default:;
					}

					if (failureByLostDevice)
					{
						hostedResourceManager->nativeManager->RecreateRenderTarget(nativeWindow);
						wmManager->needRefresh = true;
					}
					else if (failureByResized)
					{
						hostedResourceManager->nativeManager->ResizeRenderTarget(nativeWindow);
						wmManager->needRefresh = true;
					}
					else
					{
						nativeWindow->RedrawContent();
						break;
					}
				}
			}
		}

		void GuiHostedController::ClipboardUpdated()
		{
			callbackService.InvokeClipboardUpdated();
		}

		void GuiHostedController::GlobalShortcutKeyActivated(vint id)
		{
			callbackService.InvokeGlobalShortcutKeyActivated(id);
		}

		void GuiHostedController::NativeWindowDestroying(INativeWindow* window)
		{
			if (nativeWindow == window)
			{
				DestroyHostedWindowsAfterRunning();
				nativeWindow->UninstallListener(this);
				nativeWindow = nullptr;
			}
		}

/***********************************************************************
GuiHostedController::INativeAsyncService
***********************************************************************/

		bool GuiHostedController::IsInMainThread(INativeWindow* window)
		{
			return nativeController->AsyncService()->IsInMainThread(nativeWindow);
		}

		void GuiHostedController::InvokeAsync(const Func<void()>& proc)
		{
			return nativeController->AsyncService()->InvokeAsync(proc);
		}

		void GuiHostedController::InvokeInMainThread(INativeWindow* window, const Func<void()>& proc)
		{
			return nativeController->AsyncService()->InvokeInMainThread(nativeWindow, proc);
		}

		bool GuiHostedController::InvokeInMainThreadAndWait(INativeWindow* window, const Func<void()>& proc, vint milliseconds)
		{
			return nativeController->AsyncService()->InvokeInMainThreadAndWait(nativeWindow, proc, milliseconds);
		}

		Ptr<INativeDelay> GuiHostedController::DelayExecute(const Func<void()>& proc, vint milliseconds)
		{
			return nativeController->AsyncService()->DelayExecute(proc, milliseconds);
		}

		Ptr<INativeDelay> GuiHostedController::DelayExecuteInMainThread(const Func<void()>& proc, vint milliseconds)
		{
			return nativeController->AsyncService()->DelayExecuteInMainThread(proc, milliseconds);
		}

/***********************************************************************
GuiHostedController::INativeScreenService
***********************************************************************/

		vint GuiHostedController::GetScreenCount()
		{
			return 1;
		}

		INativeScreen* GuiHostedController::GetScreen(vint index)
		{
			CHECK_ERROR(index == 0, L"vl::presentation::GuiHostedController::GetScreen(vint)#Index out of range.");
			return this;
		}

		INativeScreen* GuiHostedController::GetScreen(INativeWindow* window)
		{
			return this;
		}

/***********************************************************************
GuiHostedController::INativeScreen
***********************************************************************/

		NativeRect GuiHostedController::GetBounds()
		{
			if (nativeWindow->IsCustomFrameModeEnabled())
			{
				return { {},nativeWindow->GetBounds().GetSize() };
			}
			else
			{
				return { {},nativeWindow->GetClientSize() };
			}
		}

		NativeRect GuiHostedController::GetClientBounds()
		{
			return { {},nativeWindow->GetClientSize() };
		}

		WString GuiHostedController::GetName()
		{
			return WString::Unmanaged(L"GacUI Virtual Screen");
		}

		bool GuiHostedController::IsPrimary()
		{
			return true;
		}

		double GuiHostedController::GetScalingX()
		{
			return nativeController->ScreenService()->GetScreen(nativeWindow)->GetScalingX();
		}

		double GuiHostedController::GetScalingY()
		{
			return nativeController->ScreenService()->GetScreen(nativeWindow)->GetScalingY();
		}

/***********************************************************************
GuiHostedController::INativeWindowService
***********************************************************************/

		const NativeWindowFrameConfig& GuiHostedController::GetMainWindowFrameConfig()
		{
			return nativeController->WindowService()->GetMainWindowFrameConfig();
		}

		const NativeWindowFrameConfig& GuiHostedController::GetNonMainWindowFrameConfig()
		{
			static const NativeWindowFrameConfig config = {
				.MaximizedBoxOption = BoolOption::AlwaysFalse,
				.MinimizedBoxOption = BoolOption::AlwaysFalse,
				.CustomFrameEnabled = BoolOption::AlwaysTrue,
			};
			return config;
		}

		INativeWindow* GuiHostedController::CreateNativeWindow(INativeWindow::WindowMode windowMode)
		{
			auto hostedWindow = Ptr(new GuiHostedWindow(this, windowMode));
			createdWindows.Add(hostedWindow);
			wmManager->RegisterWindow(&hostedWindow->wmWindow);

			callbackService.InvokeNativeWindowCreated(hostedWindow.Obj());
			if (mainWindow)
			{
				hostedWindow->BecomeNonMainWindow();
			}
			return hostedWindow.Obj();
		}

		void GuiHostedController::DestroyNativeWindow(INativeWindow* window)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiHostedController::DestroyNativeWindow(INativeWindow*)#"
			auto hostedWindow = dynamic_cast<GuiHostedWindow*>(window);
			CHECK_ERROR(hostedWindow, ERROR_MESSAGE_PREFIX L"The window is not created by GuiHostedController.");
			vint index = createdWindows.IndexOf(hostedWindow);
			CHECK_ERROR(index != -1, ERROR_MESSAGE_PREFIX L"The window has been destroyed.");

			if (hostedWindow == enteringWindow) enteringWindow = nullptr;
			if (hostedWindow == hoveringWindow) hoveringWindow = nullptr;
			if (hostedWindow == lastFocusedWindow) enteringWindow = nullptr;
			if (hostedWindow == capturingWindow)
			{
				capturingWindow->ReleaseCapture();
			}
			if (hostedWindow == wmWindow)
			{
				wmOperation = WindowManagerOperation::None;
				wmWindow = nullptr;
				nativeWindow->ReleaseCapture();
			}

			for (auto listener : hostedWindow->listeners)
			{
				listener->Destroying();
			}

			callbackService.InvokeNativeWindowDestroying(hostedWindow);
			wmManager->UnregisterWindow(&hostedWindow->wmWindow);
			createdWindows.RemoveAt(index);

			UpdateHoveringWindow({});
#undef ERROR_MESSAGE_PREFIX
		}

		INativeWindow* GuiHostedController::GetMainWindow()
		{
			return mainWindow;
		}

		INativeWindow* GuiHostedController::GetWindow(NativePoint location)
		{
			auto wmWindow = wmManager->HitTest(location);
			return wmWindow ? wmWindow->id : nullptr;
		}

		void GuiHostedController::SettingHostedWindowsBeforeRunning()
		{
			if (nativeWindow)
			{
				for (auto window : createdWindows)
				{
					if (window == mainWindow)
					{
						window->BecomeMainWindow();
					}
					else
					{
						window->BecomeNonMainWindow();
					}
				}

				if (auto screen = nativeController->ScreenService()->GetScreen(nativeWindow))
				{
					auto screenBounds = screen->GetClientBounds();
					auto windowSize = nativeWindow->GetBounds().GetSize();
					nativeWindow->SetBounds({
						{
							screenBounds.Left() + (screenBounds.Width() - windowSize.x) / 2,
							screenBounds.Top() + (screenBounds.Height() - windowSize.y) / 2
						},
						windowSize
						});
				}

				wmManager->Start(&mainWindow->wmWindow);
			}
		}

		void GuiHostedController::DestroyHostedWindowsAfterRunning()
		{
			if (nativeWindow)
			{
				if (wmManager->mainWindow)
				{
					wmManager->Stop();
				}

				// TODO: (enumerable) foreach:indexed(alterable(reversed))
				for (vint i = createdWindows.Count() - 1; i >= 0; i--)
				{
					auto hostedWindow = createdWindows[i];
					if (hostedWindow != mainWindow)
					{
						DestroyNativeWindow(hostedWindow.Obj());
					}
				}

				if (mainWindow)
				{
					DestroyNativeWindow(mainWindow);
					mainWindow = nullptr;
				}
			}
		}

		void GuiHostedController::Run(INativeWindow* window)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiHostedController::Run(INativeWindow*)#"
			CHECK_ERROR(!mainWindow, ERROR_MESSAGE_PREFIX L"This function has been called.");
			auto hostedWindow = dynamic_cast<GuiHostedWindow*>(window);
			CHECK_ERROR(hostedWindow, ERROR_MESSAGE_PREFIX L"The window is not created by GuiHostedController.");
			mainWindow = hostedWindow;

			SettingHostedWindowsBeforeRunning();
			wmManager->needRefresh = true;
			nativeController->WindowService()->Run(nativeWindow);
			CHECK_ERROR((nativeWindow == nullptr) == (mainWindow == nullptr), ERROR_MESSAGE_PREFIX L"Hosted windows should have been destroyed if the native windows is destroyed.");
			DestroyHostedWindowsAfterRunning();
#undef ERROR_MESSAGE_PREFIX
		}

		bool GuiHostedController::RunOneCycle()
		{
			return nativeController->WindowService()->RunOneCycle();
		}

/***********************************************************************
GuiHostedController::IGuiHostedApplication
***********************************************************************/

		INativeWindow* GuiHostedController::GetNativeWindowHost()
		{
			return nativeWindow;
		}

/***********************************************************************
GuiHostedController
***********************************************************************/

		GuiHostedController::GuiHostedController(INativeController* _nativeController)
			: nativeController(_nativeController)
		{
			wmManager = this;
			nativeController->CallbackService()->InstallListener(this);
		}

		GuiHostedController::~GuiHostedController()
		{
		}

		IGuiHostedApplication* GuiHostedController::GetHostedApplication()
		{
			return this;
		}

		void GuiHostedController::Initialize()
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiHostedController()::Initialize()#"
			CHECK_ERROR(!nativeWindow, ERROR_MESSAGE_PREFIX L"Initialize() has been called");
			CHECK_ERROR(!nativeWindowDestroyed, ERROR_MESSAGE_PREFIX L"Finalize() has been called.");

			nativeController->CallbackService()->InstallListener(this);
			nativeWindow = nativeController->WindowService()->CreateNativeWindow(INativeWindow::WindowMode::Normal);
			nativeWindow->InstallListener(this);
#undef ERROR_MESSAGE_PREFIX
		}

		void GuiHostedController::Finalize()
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::GuiHostedController()::Finalize()#"
			CHECK_ERROR(!nativeWindowDestroyed, ERROR_MESSAGE_PREFIX L"Finalize() has been called.");

			if (nativeWindow)
			{
				nativeController->WindowService()->DestroyNativeWindow(nativeWindow);
			}
			nativeController->CallbackService()->UninstallListener(this);
			nativeWindowDestroyed = true;

#undef ERROR_MESSAGE_PREFIX
		}

		void GuiHostedController::RequestRefresh()
		{
			wmManager->needRefresh = true;
		}

/***********************************************************************
GuiHostedController::INativeController
***********************************************************************/

		INativeCallbackService* GuiHostedController::CallbackService()
		{
			return &callbackService;
		}

		INativeResourceService* GuiHostedController::ResourceService()
		{
			return nativeController->ResourceService();
		}

		INativeAsyncService* GuiHostedController::AsyncService()
		{
			return this;
		}

		INativeClipboardService* GuiHostedController::ClipboardService()
		{
			return nativeController->ClipboardService();
		}

		INativeImageService* GuiHostedController::ImageService()
		{
			return nativeController->ImageService();
		}

		INativeInputService* GuiHostedController::InputService()
		{
			return nativeController->InputService();
		}

		INativeDialogService* GuiHostedController::DialogService()
		{
			// Use FakeDialogServiceBase
			return nullptr;
		}

		WString GuiHostedController::GetExecutablePath()
		{
			return nativeController->GetExecutablePath();
		}
		
		INativeScreenService* GuiHostedController::ScreenService()
		{
			return this;
		}

		INativeWindowService* GuiHostedController::WindowService()
		{
			return this;
		}
	}
}