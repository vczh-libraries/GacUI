#include "GuiWindowControls.h"
#include "GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace compositions;
			using namespace collections;

/***********************************************************************
GuiControlHost
***********************************************************************/

			void GuiControlHost::OnNativeWindowChanged()
			{
			}

			void GuiControlHost::OnVisualStatusChanged()
			{
			}

			GuiControl* GuiControlHost::GetTooltipOwner(Point location)
			{
				GuiGraphicsComposition* composition=this->GetBoundsComposition()->FindComposition(location);
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
				return 0;
			}

			void GuiControlHost::MoveIntoTooltipControl(GuiControl* tooltipControl, Point location)
			{
				if(tooltipLocation!=location)
				{
					tooltipLocation=location;
					{
						GuiControl* currentOwner=GetApplication()->GetTooltipOwner();
						if(currentOwner && currentOwner!=tooltipControl)
						{
							if(tooltipCloseDelay)
							{
								tooltipCloseDelay->Cancel();
								tooltipCloseDelay=0;
							}
							GetApplication()->DelayExecuteInMainThread([=]()
							{
								currentOwner->CloseTooltip();
							}, TooltipDelayCloseTime);
						}
					}
					if(!tooltipControl)
					{
						if(tooltipOpenDelay)
						{
							tooltipOpenDelay->Cancel();
							tooltipOpenDelay=0;
						}
					}
					else if(tooltipOpenDelay)
					{
						tooltipOpenDelay->Delay(TooltipDelayOpenTime);
					}
					else if(GetApplication()->GetTooltipOwner()!=tooltipControl)
					{
						tooltipOpenDelay=GetApplication()->DelayExecuteInMainThread([this]()
						{
							GuiControl* owner=GetTooltipOwner(tooltipLocation);
							if(owner)
							{
								Point offset=owner->GetBoundsComposition()->GetGlobalBounds().LeftTop();
								Point p(tooltipLocation.x-offset.x, tooltipLocation.y-offset.y+24);
								owner->DisplayTooltip(p);
								tooltipOpenDelay=0;

								/*
								When you use VS2010 to compiler this code,
								you will see there is an error here.
								This is due to VS2010's bug about processing [this] capture.
								I don't want to do workaround in my code, but I can tell you how to do that:

								Use a variable to save the value of "this", and capture [theThisValue, owner] instead of [this, owner].
								*/
								tooltipCloseDelay=GetApplication()->DelayExecuteInMainThread([this, owner]()
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
				if(!info.left && !info.middle && !info.right)
				{
					GuiControl* tooltipControl=GetTooltipOwner(tooltipLocation);
					MoveIntoTooltipControl(tooltipControl, Point(info.x, info.y));
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
				GuiRequestEventArgs arguments(GetStyleController()->GetBoundsComposition());
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
				SetNativeWindow(0);
			}

			GuiControlHost::GuiControlHost(GuiControl::IStyleController* _styleController)
				:GuiControl(_styleController)
			{
				GetStyleController()->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				
				WindowGotFocus.SetAssociatedComposition(GetStyleController()->GetBoundsComposition());
				WindowLostFocus.SetAssociatedComposition(GetStyleController()->GetBoundsComposition());
				WindowActivated.SetAssociatedComposition(GetStyleController()->GetBoundsComposition());
				WindowDeactivated.SetAssociatedComposition(GetStyleController()->GetBoundsComposition());
				WindowOpened.SetAssociatedComposition(GetStyleController()->GetBoundsComposition());
				WindowClosing.SetAssociatedComposition(GetStyleController()->GetBoundsComposition());
				WindowClosed.SetAssociatedComposition(GetStyleController()->GetBoundsComposition());
				WindowDestroying.SetAssociatedComposition(GetStyleController()->GetBoundsComposition());

				host=new GuiGraphicsHost;
				host->GetMainComposition()->AddChild(GetStyleController()->GetBoundsComposition());
				sharedPtrDestructorProc=0;
			}

			GuiControlHost::~GuiControlHost()
			{
				OnBeforeReleaseGraphicsHost();
				FinalizeInstance();
				styleController=0;
				delete host;
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
				boundsComposition->ForceCalculateSizeImmediately();
				SetBounds(GetBounds());
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

			compositions::GuiGraphicsAnimationManager* GuiControlHost::GetAnimationManager()
			{
				return host->GetAnimationManager();
			}

			Size GuiControlHost::GetClientSize()
			{
				if(host->GetNativeWindow())
				{
					return host->GetNativeWindow()->GetClientSize();
				}
				else
				{
					return Size(0, 0);
				}
			}

			void GuiControlHost::SetClientSize(Size value)
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->SetClientSize(value);
				}
			}

			Rect GuiControlHost::GetBounds()
			{
				if(host->GetNativeWindow())
				{
					return host->GetNativeWindow()->GetBounds();
				}
				else
				{
					return Rect();
				}
			}

			void GuiControlHost::SetBounds(Rect value)
			{
				if(host->GetNativeWindow())
				{
					host->GetNativeWindow()->SetBounds(value);
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
					host->GetNativeWindow()->Hide();
				}
			}

			void GuiControlHost::Close()
			{
				INativeWindow* window=host->GetNativeWindow();
				if(window)
				{
					if(GetCurrentController()->WindowService()->GetMainWindow()!=window)
					{
						window->Hide();
					}
					else
					{
						SetNativeWindow(0);
						GetCurrentController()->WindowService()->DestroyNativeWindow(window);
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
GuiWindow::DefaultBehaviorStyleController
***********************************************************************/

			GuiWindow::DefaultBehaviorStyleController::DefaultBehaviorStyleController()
				:window(0)
			{
			}

			GuiWindow::DefaultBehaviorStyleController::~DefaultBehaviorStyleController()
			{
			}

			void GuiWindow::DefaultBehaviorStyleController::AttachWindow(GuiWindow* _window)
			{
				window=_window;
			}

			void GuiWindow::DefaultBehaviorStyleController::InitializeNativeWindowProperties()
			{
			}

			void GuiWindow::DefaultBehaviorStyleController::SetSizeState(INativeWindow::WindowSizeState value)
			{
			}

			bool GuiWindow::DefaultBehaviorStyleController::GetMaximizedBox()
			{
				if(window->GetNativeWindow())
				{
					return window->GetNativeWindow()->GetMaximizedBox();
				}
				else
				{
					return false;
				}
			}

			void GuiWindow::DefaultBehaviorStyleController::SetMaximizedBox(bool visible)
			{
				if(window->GetNativeWindow())
				{
					window->GetNativeWindow()->SetMaximizedBox(visible);
				}
			}

			bool GuiWindow::DefaultBehaviorStyleController::GetMinimizedBox()
			{
				if(window->GetNativeWindow())
				{
					return window->GetNativeWindow()->GetMinimizedBox();
				}
				else
				{
					return false;
				}
			}

			void GuiWindow::DefaultBehaviorStyleController::SetMinimizedBox(bool visible)
			{
				if(window->GetNativeWindow())
				{
					window->GetNativeWindow()->SetMinimizedBox(visible);
				}
			}

			bool GuiWindow::DefaultBehaviorStyleController::GetBorder()
			{
				if(window->GetNativeWindow())
				{
					return window->GetNativeWindow()->GetBorder();
				}
				else
				{
					return false;
				}
			}

			void GuiWindow::DefaultBehaviorStyleController::SetBorder(bool visible)
			{
				if(window->GetNativeWindow())
				{
					window->GetNativeWindow()->SetBorder(visible);
				}
			}

			bool GuiWindow::DefaultBehaviorStyleController::GetSizeBox()
			{
				if(window->GetNativeWindow())
				{
					return window->GetNativeWindow()->GetSizeBox();
				}
				else
				{
					return false;
				}
			}

			void GuiWindow::DefaultBehaviorStyleController::SetSizeBox(bool visible)
			{
				if(window->GetNativeWindow())
				{
					window->GetNativeWindow()->SetSizeBox(visible);
				}
			}

			bool GuiWindow::DefaultBehaviorStyleController::GetIconVisible()
			{
				if(window->GetNativeWindow())
				{
					return window->GetNativeWindow()->GetIconVisible();
				}
				else
				{
					return false;
				}
			}

			void GuiWindow::DefaultBehaviorStyleController::SetIconVisible(bool visible)
			{
				if(window->GetNativeWindow())
				{
					window->GetNativeWindow()->SetIconVisible(visible);
				}
			}

			bool GuiWindow::DefaultBehaviorStyleController::GetTitleBar()
			{
				if(window->GetNativeWindow())
				{
					return window->GetNativeWindow()->GetTitleBar();
				}
				else
				{
					return false;
				}
			}

			void GuiWindow::DefaultBehaviorStyleController::SetTitleBar(bool visible)
			{
				if(window->GetNativeWindow())
				{
					window->GetNativeWindow()->SetTitleBar(visible);
				}
			}

			GuiWindow::IStyleController* GuiWindow::DefaultBehaviorStyleController::CreateTooltipStyle()
			{
				return 0;
			}

			GuiLabel::IStyleController* GuiWindow::DefaultBehaviorStyleController::CreateShortcutKeyStyle()
			{
				return 0;
			}

/***********************************************************************
GuiWindow
***********************************************************************/

			void GuiWindow::Moved()
			{
				GuiControlHost::Moved();
				styleController->SetSizeState(GetNativeWindow()->GetSizeState());
			}

			void GuiWindow::OnNativeWindowChanged()
			{
				styleController->InitializeNativeWindowProperties();
				GuiControlHost::OnNativeWindowChanged();
			}

			void GuiWindow::OnVisualStatusChanged()
			{
				GuiControlHost::OnVisualStatusChanged();
			}

			void GuiWindow::MouseClickedOnOtherWindow(GuiWindow* window)
			{
			}

			compositions::GuiGraphicsComposition* GuiWindow::GetAltComposition()
			{
				return boundsComposition;
			}

			compositions::IGuiAltActionHost* GuiWindow::GetPreviousAltHost()
			{
				return previousAltHost;
			}

			void GuiWindow::OnActivatedAltHost(IGuiAltActionHost* previousHost)
			{
				previousAltHost = previousHost;
			}

			void GuiWindow::OnDeactivatedAltHost()
			{
				previousAltHost = 0;
			}

			void GuiWindow::CollectAltActions(collections::Group<WString, IGuiAltAction*>& actions)
			{
				IGuiAltActionHost::CollectAltActionsFromControl(this, actions);
			}

			GuiWindow::GuiWindow(IStyleController* _styleController)
				:GuiControlHost(_styleController)
				,styleController(_styleController)
				,previousAltHost(0)
			{
				INativeWindow* window=GetCurrentController()->WindowService()->CreateNativeWindow();
				styleController->AttachWindow(this);
				SetNativeWindow(window);
				GetApplication()->RegisterWindow(this);
				ClipboardUpdated.SetAssociatedComposition(GetBoundsComposition());
			}

			GuiWindow::~GuiWindow()
			{
				GetApplication()->UnregisterWindow(this);
				INativeWindow* window=host->GetNativeWindow();
				if(window)
				{
					SetNativeWindow(0);
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
				INativeScreen* screen=GetRelatedScreen();
				if(screen)
				{
					Rect screenBounds=screen->GetClientBounds();
					Rect windowBounds=GetBounds();
					SetBounds(
						Rect(
							Point(
								screenBounds.Left()+(screenBounds.Width()-windowBounds.Width())/2,
								screenBounds.Top()+(screenBounds.Height()-windowBounds.Height())/2
								),
							windowBounds.GetSize()
							)
						);
				}
			}

			bool GuiWindow::GetMaximizedBox()
			{
				return styleController->GetMaximizedBox();
			}

			void GuiWindow::SetMaximizedBox(bool visible)
			{
				styleController->SetMaximizedBox(visible);
			}

			bool GuiWindow::GetMinimizedBox()
			{
				return styleController->GetMinimizedBox();
			}

			void GuiWindow::SetMinimizedBox(bool visible)
			{
				styleController->SetMinimizedBox(visible);
			}

			bool GuiWindow::GetBorder()
			{
				return styleController->GetBorder();
			}

			void GuiWindow::SetBorder(bool visible)
			{
				styleController->SetBorder(visible);
			}

			bool GuiWindow::GetSizeBox()
			{
				return styleController->GetSizeBox();
			}

			void GuiWindow::SetSizeBox(bool visible)
			{
				styleController->SetSizeBox(visible);
			}

			bool GuiWindow::GetIconVisible()
			{
				return styleController->GetIconVisible();
			}

			void GuiWindow::SetIconVisible(bool visible)
			{
				styleController->SetIconVisible(visible);
			}

			bool GuiWindow::GetTitleBar()
			{
				return styleController->GetTitleBar();
			}

			void GuiWindow::SetTitleBar(bool visible)
			{
				styleController->SetTitleBar(visible);
			}

			void GuiWindow::ShowModal(GuiWindow* owner, const Func<void()>& callback)
			{
				owner->SetEnabled(false);
				GetNativeWindow()->SetParent(owner->GetNativeWindow());
				auto container = CreateEventHandlerContainer<GuiEventArgs>();
				container->handler = WindowClosed.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
				{
					GetApplication()->InvokeInMainThread([=]()
					{
						WindowClosed.Detach(container->handler);
						GetNativeWindow()->SetParent(0);
						callback();
						owner->SetEnabled(true);
						owner->SetActivated();
					});
				});
				Show();
			}

			void GuiWindow::ShowModalAndDelete(GuiWindow* owner, const Func<void()>& callback)
			{
				owner->SetEnabled(false);
				GetNativeWindow()->SetParent(owner->GetNativeWindow());
				WindowClosed.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
				{
					GetApplication()->InvokeInMainThread([=]()
					{
						GetNativeWindow()->SetParent(0);
						callback();
						owner->SetEnabled(true);
						owner->SetActivated();
						delete this;
					});
				});
				Show();
			}

/***********************************************************************
GuiPopup
***********************************************************************/

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
				GetApplication()->RegisterPopupClosed(this);
				INativeWindow* window=GetNativeWindow();
				if(window)
				{
					window->SetParent(0);
				}
			}

			GuiPopup::GuiPopup(IStyleController* _styleController)
				:GuiWindow(_styleController)
			{
				SetMinimizedBox(false);
				SetMaximizedBox(false);
				SetSizeBox(false);
				SetTitleBar(false);
				SetShowInTaskBar(false);
				GetBoundsComposition()->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

				WindowOpened.AttachMethod(this, &GuiPopup::PopupOpened);
				WindowClosed.AttachMethod(this, &GuiPopup::PopupClosed);
			}

			GuiPopup::~GuiPopup()
			{
				GetApplication()->RegisterPopupClosed(this);
			}

			bool GuiPopup::IsClippedByScreen(Point location)
			{
				SetBounds(Rect(location, GetBounds().GetSize()));
				INativeWindow* window=GetNativeWindow();
				if(window)
				{
					INativeScreen* screen=GetCurrentController()->ScreenService()->GetScreen(window);
					if(screen)
					{
						Rect screenBounds=screen->GetClientBounds();
						Rect windowBounds=window->GetBounds();
						return !screenBounds.Contains(windowBounds.LeftTop()) || !screenBounds.Contains(windowBounds.RightBottom());
					}
				}
				return true;
			}

			void GuiPopup::ShowPopup(Point location, INativeScreen* screen)
			{
				INativeWindow* window=GetNativeWindow();
				if(window)
				{
					if (!screen)
					{
						SetBounds(Rect(location, GetBounds().GetSize()));
						screen = GetCurrentController()->ScreenService()->GetScreen(window);
					}

					if(screen)
					{
						Rect screenBounds=screen->GetClientBounds();
						Size size=window->GetBounds().GetSize();

						if(location.x<screenBounds.x1)
						{
							location.x=screenBounds.x1;
						}
						else if(location.x+size.x>screenBounds.x2)
						{
							location.x=screenBounds.x2-size.x;
						}

						if(location.y<screenBounds.y1)
						{
							location.y=screenBounds.y1;
						}
						else if(location.y+size.y>screenBounds.y2)
						{
							location.y=screenBounds.y2-size.y;
						}
					}
					SetBounds(Rect(location, GetBounds().GetSize()));
					bool value=GetNativeWindow()->IsEnabledActivate();
					ShowDeactivated();
				}
			}
			
			void GuiPopup::ShowPopup(GuiControl* control, Rect bounds, bool preferredTopBottomSide)
			{
				INativeWindow* window=GetNativeWindow();
				if(window)
				{
					Point locations[4];
					Size size=window->GetBounds().GetSize();

					GuiControlHost* controlHost=control->GetBoundsComposition()->GetRelatedControlHost();
					if(controlHost)
					{
						INativeWindow* controlWindow=controlHost->GetNativeWindow();
						if(controlWindow)
						{
							Point controlClientOffset=control->GetBoundsComposition()->GetGlobalBounds().LeftTop();
							Point controlWindowOffset=controlWindow->GetClientBoundsInScreen().LeftTop();
							bounds.x1+=controlClientOffset.x+controlWindowOffset.x;
							bounds.x2+=controlClientOffset.x+controlWindowOffset.x;
							bounds.y1+=controlClientOffset.y+controlWindowOffset.y;
							bounds.y2+=controlClientOffset.y+controlWindowOffset.y;

							if(preferredTopBottomSide)
							{
								locations[0]=Point(bounds.x1, bounds.y2);
								locations[1]=Point(bounds.x2-size.x, bounds.y2);
								locations[2]=Point(bounds.x1, bounds.y1-size.y);
								locations[3]=Point(bounds.x2-size.x, bounds.y1-size.y);
							}
							else
							{
								locations[0]=Point(bounds.x2, bounds.y1);
								locations[1]=Point(bounds.x2, bounds.y2-size.y);
								locations[2]=Point(bounds.x1-size.x, bounds.y1);
								locations[3]=Point(bounds.x1-size.x, bounds.y2-size.y);
							}

							window->SetParent(controlWindow);
							for(vint i=0;i<4;i++)
							{
								if(!IsClippedByScreen(locations[i]))
								{
									ShowPopup(locations[i]);
									return;
								}
							}
							ShowPopup(locations[0], GetCurrentController()->ScreenService()->GetScreen(controlWindow));
						}
					}
				}
			}

			void GuiPopup::ShowPopup(GuiControl* control, Point location)
			{
				INativeWindow* window=GetNativeWindow();
				if(window)
				{
					Point locations[4];
					Size size=window->GetBounds().GetSize();
					Rect controlBounds=control->GetBoundsComposition()->GetGlobalBounds();

					GuiControlHost* controlHost=control->GetBoundsComposition()->GetRelatedControlHost();
					if(controlHost)
					{
						INativeWindow* controlWindow=controlHost->GetNativeWindow();
						if(controlWindow)
						{
							Point controlClientOffset=controlWindow->GetClientBoundsInScreen().LeftTop();
							vint x=controlBounds.x1+controlClientOffset.x+location.x;
							vint y=controlBounds.y1+controlClientOffset.y+location.y;
							window->SetParent(controlWindow);
							ShowPopup(Point(x, y), GetCurrentController()->ScreenService()->GetScreen(controlWindow));
						}
					}
				}
			}

			void GuiPopup::ShowPopup(GuiControl* control, bool preferredTopBottomSide)
			{
				INativeWindow* window=GetNativeWindow();
				if(window)
				{
					Size size=window->GetBounds().GetSize();
					Rect bounds(Point(0, 0), control->GetBoundsComposition()->GetBounds().GetSize());
					ShowPopup(control, bounds, preferredTopBottomSide);
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

			GuiTooltip::GuiTooltip(IStyleController* _styleController)
				:GuiPopup(_styleController)
				,temporaryContentControl(0)
			{
				GetContainerComposition()->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				GetContainerComposition()->SetPreferredMinSize(Size(20, 10));
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
				return GetContainerComposition()->GetPreferredMinSize().x;
			}

			void GuiTooltip::SetPreferredContentWidth(vint value)
			{
				GetContainerComposition()->SetPreferredMinSize(Size(value, 10));
			}

			GuiControl* GuiTooltip::GetTemporaryContentControl()
			{
				return temporaryContentControl;
			}

			void GuiTooltip::SetTemporaryContentControl(GuiControl* control)
			{
				if(temporaryContentControl && HasChild(temporaryContentControl))
				{
					GetContainerComposition()->RemoveChild(temporaryContentControl->GetBoundsComposition());
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