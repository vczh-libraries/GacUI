#include "GuiWindowControls.h"
#include "GuiApplication.h"
#include "../GraphicsHost/GuiGraphicsHost.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace compositions;
			using namespace collections;
			using namespace reflection::description;

/***********************************************************************
GuiControlHost
***********************************************************************/

			void GuiControlHost::OnNativeWindowChanged()
			{
			}

			void GuiControlHost::OnVisualStatusChanged()
			{
			}

			controls::GuiControlHost* GuiControlHost::GetControlHostForInstance()
			{
				return this;
			}

			GuiControl* GuiControlHost::GetTooltipOwner(Point location)
			{
				GuiGraphicsComposition* composition=this->GetBoundsComposition()->FindComposition(location, false);
				if(composition)
				{
					GuiControl* control=composition->GetRelatedControl();
					while(control)
					{
						if(control->GetTooltipControl())
						{
							return control;
						}
						control=control->GetParent();
					}
				}
				return nullptr;
			}

			void GuiControlHost::MoveIntoTooltipControl(GuiControl* tooltipControl, Point location)
			{
				if (tooltipLocation != location)
				{
					tooltipLocation = location;
					{
						GuiControl* currentOwner = GetApplication()->GetTooltipOwner();
						if (currentOwner && currentOwner != tooltipControl)
						{
							if (tooltipCloseDelay)
							{
								tooltipCloseDelay->Cancel();
								tooltipCloseDelay = 0;
							}
							GetApplication()->DelayExecuteInMainThread([=]()
							{
								currentOwner->CloseTooltip();
							}, TooltipDelayCloseTime);
						}
					}
					if (!tooltipControl)
					{
						if (tooltipOpenDelay)
						{
							tooltipOpenDelay->Cancel();
							tooltipOpenDelay = 0;
						}
					}
					else if (tooltipOpenDelay)
					{
						tooltipOpenDelay->Delay(TooltipDelayOpenTime);
					}
					else if (GetApplication()->GetTooltipOwner() != tooltipControl)
					{
						tooltipOpenDelay = GetApplication()->DelayExecuteInMainThread([this]()
						{
							GuiControl* owner = GetTooltipOwner(tooltipLocation);
							if (owner)
							{
								Point offset = owner->GetBoundsComposition()->GetGlobalBounds().LeftTop();
								Point p(tooltipLocation.x - offset.x, tooltipLocation.y - offset.y + 24);
								owner->DisplayTooltip(p);
								tooltipOpenDelay = 0;

								tooltipCloseDelay = GetApplication()->DelayExecuteInMainThread([this, owner]()
								{
									owner->CloseTooltip();
								}, TooltipDelayLifeTime);
							}
						}, TooltipDelayOpenTime);
					}
				}
			}

			void GuiControlHost::MouseMoving(const NativeWindowMouseInfo& info)
			{
				if (!info.left && !info.middle && !info.right)
				{
					GuiControl* tooltipControl = GetTooltipOwner(tooltipLocation);
					MoveIntoTooltipControl(tooltipControl, Point(host->GetNativeWindow()->Convert(NativePoint(info.x, info.y))));
				}
			}

			void GuiControlHost::MouseLeaved()
			{
				MoveIntoTooltipControl(0, Point(-1, -1));
			}

			void GuiControlHost::Moved()
			{
				OnVisualStatusChanged();
			}

			void GuiControlHost::Enabled()
			{
				GuiControl::SetEnabled(true);
				OnVisualStatusChanged();
			}

			void GuiControlHost::Disabled()
			{
				GuiControl::SetEnabled(false);
				OnVisualStatusChanged();
			}

			void GuiControlHost::GotFocus()
			{
				WindowGotFocus.Execute(GetNotifyEventArguments());
				OnVisualStatusChanged();
			}

			void GuiControlHost::LostFocus()
			{
				WindowLostFocus.Execute(GetNotifyEventArguments());
				OnVisualStatusChanged();
			}

			void GuiControlHost::Activated()
			{
				WindowActivated.Execute(GetNotifyEventArguments());
				OnVisualStatusChanged();
			}

			void GuiControlHost::Deactivated()
			{
				WindowDeactivated.Execute(GetNotifyEventArguments());
				OnVisualStatusChanged();
			}

			void GuiControlHost::Opened()
			{
				WindowOpened.Execute(GetNotifyEventArguments());
			}

			void GuiControlHost::Closing(bool& cancel)
			{
				GuiRequestEventArgs arguments(boundsComposition);
				arguments.cancel=cancel;
				WindowClosing.Execute(arguments);
				if(!arguments.handled)
				{
					cancel=arguments.cancel;
				}
			}

			void GuiControlHost::Closed()
			{
				WindowClosed.Execute(GetNotifyEventArguments());
			}

			void GuiControlHost::Destroying()
			{
				WindowDestroying.Execute(GetNotifyEventArguments());
				calledDestroyed = true;
				if (deleteWhenDestroyed)
				{
					GetApplication()->InvokeInMainThread(this, [=]()
					{
						delete this;
					});
				}
				SetNativeWindow(nullptr);
			}

			void GuiControlHost::UpdateClientSizeAfterRendering(Size clientSize)
			{
				SetClientSize(clientSize);
			}

			GuiControlHost::GuiControlHost(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
				boundsComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
				
				WindowGotFocus.SetAssociatedComposition(boundsComposition);
				WindowLostFocus.SetAssociatedComposition(boundsComposition);
				WindowActivated.SetAssociatedComposition(boundsComposition);
				WindowDeactivated.SetAssociatedComposition(boundsComposition);
				WindowOpened.SetAssociatedComposition(boundsComposition);
				WindowClosing.SetAssociatedComposition(boundsComposition);
				WindowClosed.SetAssociatedComposition(boundsComposition);
				WindowDestroying.SetAssociatedComposition(boundsComposition);

				host=new GuiGraphicsHost(this, boundsComposition);
				sharedPtrDestructorProc = 0;
			}

			GuiControlHost::~GuiControlHost()
			{
				FinalizeInstanceRecursively(this);
				OnBeforeReleaseGraphicsHost();
				delete host;
			}

			void GuiControlHost::DeleteAfterProcessingAllEvents()
			{
				auto window = host->GetNativeWindow();
				if (calledDestroyed || !window)
				{
					delete this;
				}
				else
				{
					deleteWhenDestroyed = true;
					GetCurrentController()->WindowService()->DestroyNativeWindow(window);
				}
			}

			compositions::GuiGraphicsHost* GuiControlHost::GetGraphicsHost()
			{
				return host;
			}

			compositions::GuiGraphicsComposition* GuiControlHost::GetMainComposition()
			{
				return host->GetMainComposition();
			}

			INativeWindow* GuiControlHost::GetNativeWindow()
			{
				return host->GetNativeWindow();
			}

			void GuiControlHost::SetNativeWindow(INativeWindow* window)
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->UninstallListener(this);
				}
				host->SetNativeWindow(window);
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->InstallListener(this);
				}
				OnNativeWindowChanged();
			}

			void GuiControlHost::ForceCalculateSizeImmediately()
			{
				auto size = GetClientSize();
				boundsComposition->ForceCalculateSizeImmediately();
				SetClientSize(size);
			}

			bool GuiControlHost::GetEnabled()
			{
				if(host->GetNativeWindow())
				{
					return host->GetNativeWindow()->IsEnabled();
				}
				else
				{
					return false;
				}
			}

			void GuiControlHost::SetEnabled(bool value)
			{
				if(host->GetNativeWindow())
				{
					if(value)
					{
						host->GetNativeWindow()->Enable();
					}
					else
					{
						host->GetNativeWindow()->Disable();
					}
				}
			}

			bool GuiControlHost::GetFocused()
			{
				if(host->GetNativeWindow())
				{
					return host->GetNativeWindow()->IsFocused();
				}
				else
				{
					return false;
				}
			}

			void GuiControlHost::SetFocused()
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->SetFocus();
				}
			}

			bool GuiControlHost::GetActivated()
			{
				if(host->GetNativeWindow())
				{
					return host->GetNativeWindow()->IsActivated();
				}
				else
				{
					return false;
				}
			}

			void GuiControlHost::SetActivated()
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->SetActivate();
				}
			}

			bool GuiControlHost::GetShowInTaskBar()
			{
				if(host->GetNativeWindow())
				{
					return host->GetNativeWindow()->IsAppearedInTaskBar();
				}
				else
				{
					return false;
				}
			}

			void GuiControlHost::SetShowInTaskBar(bool value)
			{
				if(host->GetNativeWindow())
				{
					if(value)
					{
						host->GetNativeWindow()->ShowInTaskBar();
					}
					else
					{
						host->GetNativeWindow()->HideInTaskBar();
					}
				}
			}

			bool GuiControlHost::GetEnabledActivate()
			{
				if(host->GetNativeWindow())
				{
					return host->GetNativeWindow()->IsEnabledActivate();
				}
				else
				{
					return false;
				}
			}

			void GuiControlHost::SetEnabledActivate(bool value)
			{
				if(host->GetNativeWindow())
				{
					if(value)
					{
						host->GetNativeWindow()->EnableActivate();
					}
					else
					{
						host->GetNativeWindow()->DisableActivate();
					}
				}
			}

			bool GuiControlHost::GetTopMost()
			{
				if(host->GetNativeWindow())
				{
					return host->GetNativeWindow()->GetTopMost();
				}
				else
				{
					return false;
				}
			}

			void GuiControlHost::SetTopMost(bool topmost)
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->SetTopMost(topmost);
				}
			}

			compositions::IGuiShortcutKeyManager* GuiControlHost::GetShortcutKeyManager()
			{
				return host->GetShortcutKeyManager();
			}

			void GuiControlHost::SetShortcutKeyManager(compositions::IGuiShortcutKeyManager* value)
			{
				host->SetShortcutKeyManager(value);
			}

			compositions::GuiGraphicsTimerManager* GuiControlHost::GetTimerManager()
			{
				return host->GetTimerManager();
			}

			Size GuiControlHost::GetClientSize()
			{
				if (auto window = host->GetNativeWindow())
				{
					return window->Convert(window->GetClientSize());
				}
				else
				{
					return Size(0, 0);
				}
			}

			void GuiControlHost::SetClientSize(Size value)
			{
				if (auto window = host->GetNativeWindow())
				{
					host->GetNativeWindow()->SetClientSize(window->Convert(value));
				}
			}

			NativePoint GuiControlHost::GetLocation()
			{
				if(auto window = host->GetNativeWindow())
				{
					return window->GetBounds().LeftTop();
				}
				else
				{
					return NativePoint();
				}
			}

			void GuiControlHost::SetLocation(NativePoint value)
			{
				if (auto window = host->GetNativeWindow())
				{
					auto bounds = window->GetBounds();
					window->SetBounds(NativeRect(value, bounds.GetSize()));
				}
			}

			void GuiControlHost::SetBounds(NativePoint location, Size size)
			{
				if (auto window = host->GetNativeWindow())
				{
					window->SetBounds(NativeRect(location, window->Convert(size)));
				}
			}

			GuiControlHost* GuiControlHost::GetRelatedControlHost()
			{
				return this;
			}

			const WString& GuiControlHost::GetText()
			{
				WString result;
				if(host->GetNativeWindow())
				{
					result=host->GetNativeWindow()->GetTitle();
				}
				if(result!=GuiControl::GetText())
				{
					GuiControl::SetText(result);
				}
				return GuiControl::GetText();
			}

			void GuiControlHost::SetText(const WString& value)
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->SetTitle(value);
					GuiControl::SetText(value);
				}
			}

			INativeScreen* GuiControlHost::GetRelatedScreen()
			{
				if(host->GetNativeWindow())
				{
					return GetCurrentController()->ScreenService()->GetScreen(host->GetNativeWindow());
				}
				else
				{
					return 0;
				}
			}

			void GuiControlHost::Show()
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->Show();
				}
			}

			void GuiControlHost::ShowDeactivated()
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->ShowDeactivated();
				}
			}

			void GuiControlHost::ShowRestored()
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->ShowRestored();
				}
			}

			void GuiControlHost::ShowMaximized()
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->ShowMaximized();
				}
			}

			void GuiControlHost::ShowMinimized()
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->ShowMinimized();
				}
			}

			void GuiControlHost::Hide()
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->Hide(false);
				}
			}

			void GuiControlHost::Close()
			{
				if (auto window = host->GetNativeWindow())
				{
					auto mainWindow = GetCurrentController()->WindowService()->GetMainWindow();
					if (mainWindow == window)
					{
						SetNativeWindow(nullptr);
						GetCurrentController()->WindowService()->DestroyNativeWindow(window);
					}
					else
					{
						window->Hide(false);
					}
				}
			}

			bool GuiControlHost::GetOpening()
			{
				INativeWindow* window=host->GetNativeWindow();
				if(window)
				{
					return window->IsVisible();
				}
				return false;
			}

/***********************************************************************
GuiWindow
***********************************************************************/

			void GuiWindow::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiWindow::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject(true);
#define FIX_WINDOW_PROPERTY(VARIABLE, NAME) \
				switch (ct->Get ## NAME ## Option()) \
				{ \
				case templates::BoolOption::AlwaysTrue: \
					VARIABLE = true; \
					break; \
				case templates::BoolOption::AlwaysFalse: \
					VARIABLE = false; \
					break; \
				default:; \
				} \

				FIX_WINDOW_PROPERTY(hasMaximizedBox, MaximizedBox)
				FIX_WINDOW_PROPERTY(hasMinimizedBox, MinimizedBox)
				FIX_WINDOW_PROPERTY(hasBorder, Border)
				FIX_WINDOW_PROPERTY(hasSizeBox, SizeBox)
				FIX_WINDOW_PROPERTY(isIconVisible, IconVisible)
				FIX_WINDOW_PROPERTY(hasTitleBar, TitleBar)

#undef FIX_WINDOW_PROPERTY
				ct->SetMaximizedBox(hasMaximizedBox);
				ct->SetMinimizedBox(hasMinimizedBox);
				ct->SetBorder(hasBorder);
				ct->SetSizeBox(hasSizeBox);
				ct->SetIconVisible(isIconVisible);
				ct->SetTitleBar(hasTitleBar);
				ct->SetMaximized(GetNativeWindow()->GetSizeState() != INativeWindow::Maximized);
				ct->SetActivated(GetActivated());

				auto window = GetNativeWindow();
				if (window)
				{
					window->SetIcon(icon);
				}
				UpdateCustomFramePadding(window, ct);

				ct->SetIcon(icon ? icon : window ? window->GetIcon() : nullptr);
				SyncNativeWindowProperties();
			}

			void GuiWindow::UpdateCustomFramePadding(INativeWindow* window, templates::GuiWindowTemplate* ct)
			{
				if (auto window = GetNativeWindow())
				{
					ct->SetCustomFramePadding(window->Convert(window->GetCustomFramePadding()));
				}
				else
				{
					ct->SetCustomFramePadding({8, 8, 8, 8});
				}
			}

			void GuiWindow::SyncNativeWindowProperties()
			{
				if (auto window = GetNativeWindow())
				{
					if (GetControlTemplateObject(true)->GetCustomFrameEnabled())
					{
						window->EnableCustomFrameMode();
						window->SetBorder(false);
					}
					else
					{
						window->DisableCustomFrameMode();
						window->SetBorder(hasBorder);
					}

					window->SetMaximizedBox(hasMaximizedBox);
					window->SetMinimizedBox(hasMinimizedBox);
					window->SetSizeBox(hasSizeBox);
					window->SetIconVisible(isIconVisible);
					window->SetTitleBar(hasTitleBar);
				}
			}

			void GuiWindow::Moved()
			{
				GuiControlHost::Moved();
				GetControlTemplateObject(true)->SetMaximized(GetNativeWindow()->GetSizeState() != INativeWindow::Maximized);
			}

			void GuiWindow::DpiChanged()
			{
				if (auto ct = GetControlTemplateObject(false))
				{
					UpdateCustomFramePadding(GetNativeWindow(), ct);
				}
			}

			void GuiWindow::OnNativeWindowChanged()
			{
				SyncNativeWindowProperties();
				GuiControlHost::OnNativeWindowChanged();
			}

			void GuiWindow::OnVisualStatusChanged()
			{
				GuiControlHost::OnVisualStatusChanged();
			}

			void GuiWindow::MouseClickedOnOtherWindow(GuiWindow* window)
			{
			}

			void GuiWindow::OnWindowActivated(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (auto ct = GetControlTemplateObject(false))
				{
					ct->SetActivated(true);
				}
			}

			void GuiWindow::OnWindowDeactivated(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (auto ct = GetControlTemplateObject(false))
				{
					ct->SetActivated(false);
				}
			}

			GuiWindow::GuiWindow(theme::ThemeName themeName)
				:GuiControlHost(themeName)
			{
				SetAltComposition(boundsComposition);
				SetAltControl(this, true);

				INativeWindow* window = GetCurrentController()->WindowService()->CreateNativeWindow();
				SetNativeWindow(window);
				GetApplication()->RegisterWindow(this);
				ClipboardUpdated.SetAssociatedComposition(boundsComposition);

				WindowActivated.AttachMethod(this, &GuiWindow::OnWindowActivated);
				WindowDeactivated.AttachMethod(this, &GuiWindow::OnWindowDeactivated);
			}

			GuiWindow::~GuiWindow()
			{
				FinalizeAggregation();
				GetApplication()->UnregisterWindow(this);
				INativeWindow* window=host->GetNativeWindow();
				if(window)
				{
					SetNativeWindow(nullptr);
					GetCurrentController()->WindowService()->DestroyNativeWindow(window);
				}
			}

			IDescriptable* GuiWindow::QueryService(const WString& identifier)
			{
				if (identifier == IGuiAltActionHost::Identifier)
				{
					return (IGuiAltActionHost*)this;
				}
				else
				{
					return GuiControlHost::QueryService(identifier);
				}
			}

			void GuiWindow::MoveToScreenCenter()
			{
				MoveToScreenCenter(GetRelatedScreen());
			}

			void GuiWindow::MoveToScreenCenter(INativeScreen* screen)
			{
				if (screen)
				{
					if (auto window = host->GetNativeWindow())
					{
						NativeRect screenBounds = screen->GetClientBounds();
						NativeSize windowSize = window->GetBounds().GetSize();
						SetLocation(
							NativePoint(
								screenBounds.Left() + (screenBounds.Width() - windowSize.x) / 2,
								screenBounds.Top() + (screenBounds.Height() - windowSize.y) / 2
							)
						);
					}
				}
			}

#define IMPL_WINDOW_PROPERTY(VARIABLE, NAME, CONDITION_BREAK) \
			bool GuiWindow::Get ## NAME() \
			{ \
				return VARIABLE; \
			} \
			void GuiWindow::Set ## NAME(bool visible) \
			{ \
				auto ct = GetControlTemplateObject(true); \
				if (ct->Get ## NAME ## Option() == templates::BoolOption::Customizable) \
				{ \
					VARIABLE = visible; \
					ct->Set ## NAME(visible); \
					auto window = GetNativeWindow(); \
					if (window) \
					{ \
						CONDITION_BREAK \
						window->Set ## NAME(visible); \
					} \
					UpdateCustomFramePadding(window, ct); \
				} \
			} \

#define IMPL_WINDOW_PROPERTY_EMPTY_CONDITION
#define IMPL_WINDOW_PROPERTY_BORDER_CONDITION if (!ct->GetCustomFrameEnabled())

			IMPL_WINDOW_PROPERTY(hasMaximizedBox, MaximizedBox, IMPL_WINDOW_PROPERTY_EMPTY_CONDITION)
			IMPL_WINDOW_PROPERTY(hasMinimizedBox, MinimizedBox, IMPL_WINDOW_PROPERTY_EMPTY_CONDITION)
			IMPL_WINDOW_PROPERTY(hasBorder, Border, IMPL_WINDOW_PROPERTY_BORDER_CONDITION)
			IMPL_WINDOW_PROPERTY(hasSizeBox, SizeBox, IMPL_WINDOW_PROPERTY_EMPTY_CONDITION)
			IMPL_WINDOW_PROPERTY(isIconVisible, IconVisible, IMPL_WINDOW_PROPERTY_EMPTY_CONDITION)
			IMPL_WINDOW_PROPERTY(hasTitleBar, TitleBar, IMPL_WINDOW_PROPERTY_EMPTY_CONDITION)

			Ptr<GuiImageData> GuiWindow::GetIcon()
			{
				return icon;
			}

			void GuiWindow::SetIcon(Ptr<GuiImageData> value)
			{
				if (icon != value)
				{
					icon = value;

					auto window = GetNativeWindow();
					if (window)
					{
						window->SetIcon(icon);
					}

					if (auto ct = GetControlTemplateObject(false))
					{
						ct->SetIcon(icon ? icon : window ? window->GetIcon() : nullptr);
					}
				}
			}

#undef IMPL_WINDOW_PROPERTY_BORDER_CONDITION
#undef IMPL_WINDOW_PROPERTY_EMPTY_CONDITION
#undef IMPL_WINDOW_PROPERTY

			void GuiWindow::ShowModal(GuiWindow* owner, const Func<void()>& callback)
			{
				owner->SetEnabled(false);
				GetNativeWindow()->SetParent(owner->GetNativeWindow());
				auto container = MakePtr<IGuiGraphicsEventHandler::Container>();
				container->handler = WindowClosed.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
				{
					GetApplication()->InvokeInMainThread(this, [=]()
					{
						WindowClosed.Detach(container->handler);
						container->handler = nullptr;
						GetNativeWindow()->SetParent(nullptr);
						callback();
						owner->SetEnabled(true);
						owner->SetActivated();
					});
				});
				Show();
			}

			void GuiWindow::ShowModalAndDelete(GuiWindow* owner, const Func<void()>& callback)
			{
				ShowModal(owner, [=]()
				{
					callback();
					DeleteAfterProcessingAllEvents();
				});
			}

			Ptr<reflection::description::IAsync> GuiWindow::ShowModalAsync(GuiWindow* owner)
			{
				auto future = IFuture::Create();
				ShowModal(owner, [promise = future->GetPromise()]()
				{
					promise->SendResult({});
				});
				return future;
			}

/***********************************************************************
GuiPopup
***********************************************************************/

			void GuiPopup::UpdateClientSizeAfterRendering(Size clientSize)
			{
				if (popupType == -1)
				{
					GuiWindow::UpdateClientSizeAfterRendering(clientSize);
				}
				else
				{
					auto window = host->GetNativeWindow();
					auto currentClientSize = window->GetClientSize();
					auto currentWindowSize = window->GetBounds().GetSize();
					auto offsetX = currentWindowSize.x - currentClientSize.x;
					auto offsetY = currentWindowSize.y - currentClientSize.y;
					auto nativeClientSize = window->Convert(clientSize);
					auto position = CalculatePopupPosition(NativeSize(nativeClientSize.x + offsetX, nativeClientSize.y + offsetY), popupType, popupInfo);
					SetBounds(position, clientSize);
				}
			}

			void GuiPopup::MouseClickedOnOtherWindow(GuiWindow* window)
			{
				Hide();
			}

			void GuiPopup::PopupOpened(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				GetApplication()->RegisterPopupOpened(this);
			}

			void GuiPopup::PopupClosed(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				popupType = -1;
				GetApplication()->RegisterPopupClosed(this);
				if(auto window = GetNativeWindow())
				{
					window->SetParent(nullptr);
				}
			}

			void GuiPopup::OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments)
			{
				if (!arguments.handled)
				{
					Hide();
				}
			}

			bool GuiPopup::IsClippedByScreen(NativeSize size, NativePoint location, INativeScreen* screen)
			{
				NativeRect screenBounds = screen->GetClientBounds();
				NativeRect windowBounds(location, size);
				return !screenBounds.Contains(windowBounds.LeftTop()) || !screenBounds.Contains(windowBounds.RightBottom());
			}

			NativePoint GuiPopup::CalculatePopupPosition(NativeSize windowSize, NativePoint location, INativeScreen* screen)
			{
				NativeRect screenBounds = screen->GetClientBounds();

				if (location.x < screenBounds.x1)
				{
					location.x = screenBounds.x1;
				}
				else if (location.x + windowSize.x > screenBounds.x2)
				{
					location.x = screenBounds.x2 - windowSize.x;
				}

				if (location.y < screenBounds.y1)
				{
					location.y = screenBounds.y1;
				}
				else if (location.y + windowSize.y > screenBounds.y2)
				{
					location.y = screenBounds.y2 - windowSize.y;
				}

				return location;
			}

			NativePoint GuiPopup::CalculatePopupPosition(NativeSize windowSize, GuiControl* control, INativeWindow* controlWindow, Rect bounds, bool preferredTopBottomSide)
			{
				NativePoint controlClientOffset = controlWindow->Convert(control->GetBoundsComposition()->GetGlobalBounds().LeftTop());
				NativePoint controlWindowOffset = controlWindow->GetClientBoundsInScreen().LeftTop();
				NativeRect targetBounds(controlWindow->Convert(bounds.LeftTop()), controlWindow->Convert(bounds.GetSize()));
				targetBounds.x1 += controlClientOffset.x + controlWindowOffset.x;
				targetBounds.x2 += controlClientOffset.x + controlWindowOffset.x;
				targetBounds.y1 += controlClientOffset.y + controlWindowOffset.y;
				targetBounds.y2 += controlClientOffset.y + controlWindowOffset.y;

				NativePoint locations[4];
				if (preferredTopBottomSide)
				{
					locations[0] = NativePoint(targetBounds.x1, targetBounds.y2);
					locations[1] = NativePoint(targetBounds.x2 - windowSize.x, targetBounds.y2);
					locations[2] = NativePoint(targetBounds.x1, targetBounds.y1 - windowSize.y);
					locations[3] = NativePoint(targetBounds.x2 - windowSize.x, targetBounds.y1 - windowSize.y);
				}
				else
				{
					locations[0] = NativePoint(targetBounds.x2, targetBounds.y1);
					locations[1] = NativePoint(targetBounds.x2, targetBounds.y2 - windowSize.y);
					locations[2] = NativePoint(targetBounds.x1 - windowSize.x, targetBounds.y1);
					locations[3] = NativePoint(targetBounds.x1 - windowSize.x, targetBounds.y2 - windowSize.y);
				}

				auto screen = GetCurrentController()->ScreenService()->GetScreen(controlWindow);
				for (vint i = 0; i < 4; i++)
				{
					if (!IsClippedByScreen(windowSize, locations[i], screen))
					{
						return CalculatePopupPosition(windowSize, locations[i], screen);
					}
				}
				return CalculatePopupPosition(windowSize, locations[0], screen);
			}

			NativePoint GuiPopup::CalculatePopupPosition(NativeSize windowSize, GuiControl* control, INativeWindow* controlWindow, Point location)
			{
				NativePoint controlClientOffset = controlWindow->Convert(control->GetBoundsComposition()->GetGlobalBounds().LeftTop());
				NativePoint controlWindowOffset = controlWindow->GetClientBoundsInScreen().LeftTop();
				NativePoint targetLocation = controlWindow->Convert(location);
				NativeCoordinate x = controlClientOffset.x + controlWindowOffset.x + targetLocation.x;
				NativeCoordinate y = controlClientOffset.y + controlWindowOffset.y + targetLocation.y;
				return CalculatePopupPosition(windowSize, NativePoint(x, y), GetCurrentController()->ScreenService()->GetScreen(controlWindow));
			}

			NativePoint GuiPopup::CalculatePopupPosition(NativeSize windowSize, GuiControl* control, INativeWindow* controlWindow, bool preferredTopBottomSide)
			{
				Rect bounds(Point(0, 0), control->GetBoundsComposition()->GetBounds().GetSize());
				return CalculatePopupPosition(windowSize, control, controlWindow, bounds, preferredTopBottomSide);
			}

			NativePoint GuiPopup::CalculatePopupPosition(NativeSize windowSize, vint popupType, const PopupInfo& popupInfo)
			{
				switch (popupType)
				{
				case 1:
					return CalculatePopupPosition(windowSize, popupInfo._1.location, popupInfo._1.screen);
				case 2:
					return CalculatePopupPosition(windowSize, popupInfo._2.control, popupInfo._2.controlWindow, popupInfo._2.bounds, popupInfo._2.preferredTopBottomSide);
				case 3:
					return CalculatePopupPosition(windowSize, popupInfo._3.control, popupInfo._3.controlWindow, popupInfo._3.location);
				case 4:
					return CalculatePopupPosition(windowSize, popupInfo._4.control, popupInfo._4.controlWindow, popupInfo._4.preferredTopBottomSide);
				default:
					CHECK_FAIL(L"vl::presentation::controls::GuiPopup::CalculatePopupPosition(Size, const PopupInfo&)#Internal error.");
				}
			}

			void GuiPopup::ShowPopupInternal()
			{
				auto window = GetNativeWindow();
				UpdateClientSizeAfterRendering(window->Convert(window->GetClientSize()));

				INativeWindow* controlWindow = nullptr;
				switch (popupType)
				{
				case 2: controlWindow = popupInfo._2.controlWindow; break;
				case 3: controlWindow = popupInfo._3.controlWindow; break;
				case 4: controlWindow = popupInfo._4.controlWindow; break;
				}

				if (controlWindow)
				{
					window->SetParent(controlWindow);
					window->SetTopMost(controlWindow->GetTopMost());
				}
				else
				{
					window->SetTopMost(true);
				}
				ShowDeactivated();
			}

			GuiPopup::GuiPopup(theme::ThemeName themeName)
				:GuiWindow(themeName)
			{
				SetMinimizedBox(false);
				SetMaximizedBox(false);
				SetSizeBox(false);
				SetTitleBar(false);
				SetShowInTaskBar(false);

				WindowOpened.AttachMethod(this, &GuiPopup::PopupOpened);
				WindowClosed.AttachMethod(this, &GuiPopup::PopupClosed);
				boundsComposition->GetEventReceiver()->keyDown.AttachMethod(this, &GuiPopup::OnKeyDown);
			}

			GuiPopup::~GuiPopup()
			{
				GetApplication()->RegisterPopupClosed(this);
			}

			void GuiPopup::ShowPopup(NativePoint location, INativeScreen* screen)
			{
				if (auto window = GetNativeWindow())
				{
					if (!screen)
					{
						SetBounds(location, GetClientSize());
						screen = GetCurrentController()->ScreenService()->GetScreen(window);
					}

					popupType = 1;
					popupInfo._1.location = location;
					popupInfo._1.screen = screen;
					ShowPopupInternal();
				}
			}
			
			void GuiPopup::ShowPopup(GuiControl* control, Rect bounds, bool preferredTopBottomSide)
			{
				if (auto window = GetNativeWindow())
				{
					if (auto controlHost = control->GetBoundsComposition()->GetRelatedControlHost())
					{
						if (auto controlWindow = controlHost->GetNativeWindow())
						{
							popupType = 2;
							popupInfo._2.control = control;
							popupInfo._2.controlWindow = controlWindow;
							popupInfo._2.bounds = bounds;
							popupInfo._2.preferredTopBottomSide = preferredTopBottomSide;
							ShowPopupInternal();
						}
					}
				}
			}

			void GuiPopup::ShowPopup(GuiControl* control, Point location)
			{
				if (auto window = GetNativeWindow())
				{
					if (auto controlHost = control->GetBoundsComposition()->GetRelatedControlHost())
					{
						if (auto controlWindow = controlHost->GetNativeWindow())
						{
							popupType = 3;
							popupInfo._3.control = control;
							popupInfo._3.controlWindow = controlWindow;
							popupInfo._3.location = location;
							ShowPopupInternal();
						}
					}
				}
			}

			void GuiPopup::ShowPopup(GuiControl* control, bool preferredTopBottomSide)
			{
				if (auto window = GetNativeWindow())
				{
					if (auto controlHost = control->GetBoundsComposition()->GetRelatedControlHost())
					{
						if (auto controlWindow = controlHost->GetNativeWindow())
						{
							popupType = 4;
							popupInfo._4.control = control;
							popupInfo._4.controlWindow = controlWindow;
							popupInfo._4.preferredTopBottomSide = preferredTopBottomSide;
							ShowPopupInternal();
						}
					}
				}
			}

/***********************************************************************
GuiPopup
***********************************************************************/

			void GuiTooltip::GlobalTimer()
			{
				SetClientSize(GetClientSize());
			}

			void GuiTooltip::TooltipOpened(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
			}

			void GuiTooltip::TooltipClosed(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				SetTemporaryContentControl(0);
			}

			GuiTooltip::GuiTooltip(theme::ThemeName themeName)
				:GuiPopup(themeName)
				,temporaryContentControl(0)
			{
				containerComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				containerComposition->SetPreferredMinSize(Size(20, 10));
				GetCurrentController()->CallbackService()->InstallListener(this);

				WindowOpened.AttachMethod(this, &GuiTooltip::TooltipOpened);
				WindowClosed.AttachMethod(this, &GuiTooltip::TooltipClosed);
			}

			GuiTooltip::~GuiTooltip()
			{
				GetCurrentController()->CallbackService()->UninstallListener(this);
			}

			vint GuiTooltip::GetPreferredContentWidth()
			{
				return containerComposition->GetPreferredMinSize().x;
			}

			void GuiTooltip::SetPreferredContentWidth(vint value)
			{
				containerComposition->SetPreferredMinSize(Size(value, 10));
			}

			GuiControl* GuiTooltip::GetTemporaryContentControl()
			{
				return temporaryContentControl;
			}

			void GuiTooltip::SetTemporaryContentControl(GuiControl* control)
			{
				if(temporaryContentControl && HasChild(temporaryContentControl))
				{
					containerComposition->RemoveChild(temporaryContentControl->GetBoundsComposition());
					temporaryContentControl=0;
				}
				temporaryContentControl=control;
				if(control)
				{
					control->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					AddChild(control);
				}
			}
		}
	}
}
