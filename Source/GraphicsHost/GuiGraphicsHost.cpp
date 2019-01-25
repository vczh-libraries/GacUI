#include "GuiGraphicsHost.h"
#include "../Controls/GuiWindowControls.h"
#include "../Controls/Templates/GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;
			using namespace controls;
			using namespace elements;
			using namespace theme;

/***********************************************************************
GuiGraphicsTimerManager
***********************************************************************/

			GuiGraphicsTimerManager::GuiGraphicsTimerManager()
			{
			}

			GuiGraphicsTimerManager::~GuiGraphicsTimerManager()
			{
			}

			void GuiGraphicsTimerManager::AddCallback(Ptr<IGuiGraphicsTimerCallback> callback)
			{
				callbacks.Add(callback);
			}

			void GuiGraphicsTimerManager::Play()
			{
				for (vint i = callbacks.Count() - 1; i >= 0; i--)
				{
					auto callback = callbacks[i];
					if (!callback->Play())
					{
						callbacks.RemoveAt(i);
					}
				}
			}

/***********************************************************************
GuiGraphicsHost
***********************************************************************/

			void GuiGraphicsHost::RefreshRelatedHostRecord(INativeWindow* nativeWindow)
			{
				hostRecord.nativeWindow = nativeWindow;
				hostRecord.renderTarget = nativeWindow ? GetGuiGraphicsResourceManager()->GetRenderTarget(nativeWindow) : nullptr;
				windowComposition->UpdateRelatedHostRecord(&hostRecord);
			}

			void GuiGraphicsHost::DisconnectCompositionInternal(GuiGraphicsComposition* composition)
			{
				for(vint i=0;i<composition->Children().Count();i++)
				{
					DisconnectCompositionInternal(composition->Children().Get(i));
				}
				if(mouseCaptureComposition==composition)
				{
					if(hostRecord.nativeWindow)
					{
						hostRecord.nativeWindow->ReleaseCapture();
					}
					mouseCaptureComposition=0;
				}
				if(focusedComposition==composition)
				{
					focusedComposition=0;
				}
				mouseEnterCompositions.Remove(composition);
			}

			void GuiGraphicsHost::MouseCapture(const NativeWindowMouseInfo& info)
			{
				if (hostRecord.nativeWindow && (info.left || info.middle || info.right))
				{
					if (!hostRecord.nativeWindow->IsCapturing() && !info.nonClient)
					{
						hostRecord.nativeWindow->RequireCapture();
						auto point = hostRecord.nativeWindow->Convert(NativePoint(info.x, info.y));
						mouseCaptureComposition = windowComposition->FindComposition(point, true);
					}
				}
			}

			void GuiGraphicsHost::MouseUncapture(const NativeWindowMouseInfo& info)
			{
				if(hostRecord.nativeWindow && !(info.left || info.middle || info.right))
				{
					hostRecord.nativeWindow->ReleaseCapture();
					mouseCaptureComposition=0;
				}
			}

			void GuiGraphicsHost::OnCharInput(const NativeWindowCharInfo& info, GuiGraphicsComposition* composition, GuiCharEvent GuiGraphicsEventReceiver::* eventReceiverEvent)
			{
				List<GuiGraphicsComposition*> compositions;
				while(composition)
				{
					if(composition->HasEventReceiver())
					{
						compositions.Add(composition);
					}
					composition=composition->GetParent();
				}

				GuiCharEventArgs arguments(composition);
				(NativeWindowCharInfo&)arguments=info;

				for(vint i=compositions.Count()-1;i>=0;i--)
				{
					compositions[i]->GetEventReceiver()->previewCharInput.Execute(arguments);
					if(arguments.handled)
					{
						return;
					}
				}

				for(vint i=0;i<compositions.Count();i++)
				{
					(compositions[i]->GetEventReceiver()->*eventReceiverEvent).Execute(arguments);
					if(arguments.handled)
					{
						return;
					}
				}
			}

			void GuiGraphicsHost::OnKeyInput(const NativeWindowKeyInfo& info, GuiGraphicsComposition* composition, GuiKeyEvent GuiGraphicsEventReceiver::* eventReceiverEvent)
			{
				List<GuiGraphicsComposition*> compositions;
				{
					auto current = composition;
					while (current)
					{
						if (current->HasEventReceiver())
						{
							compositions.Add(current);
						}
						current = current->GetParent();
					}
				}

				GuiKeyEventArgs arguments(composition);
				(NativeWindowKeyInfo&)arguments = info;

				for (vint i = compositions.Count() - 1; i >= 0; i--)
				{
					compositions[i]->GetEventReceiver()->previewKey.Execute(arguments);
					if (arguments.handled)
					{
						return;
					}
				}

				for (vint i = 0; i < compositions.Count(); i++)
				{
					(compositions[i]->GetEventReceiver()->*eventReceiverEvent).Execute(arguments);
					if (arguments.handled)
					{
						return;
					}
				}
			}

			void GuiGraphicsHost::RaiseMouseEvent(GuiMouseEventArgs& arguments, GuiGraphicsComposition* composition, GuiMouseEvent GuiGraphicsEventReceiver::* eventReceiverEvent)
			{
				arguments.compositionSource=composition;
				arguments.eventSource=0;
				vint x=arguments.x;
				vint y=arguments.y;

				while(composition)
				{
					if(composition->HasEventReceiver())
					{
						if(!arguments.eventSource)
						{
							arguments.eventSource=composition;
						}
						GuiGraphicsEventReceiver* eventReceiver=composition->GetEventReceiver();
						(eventReceiver->*eventReceiverEvent).Execute(arguments);
						if(arguments.handled)
						{
							break;
						}
					}

					GuiGraphicsComposition* parent=composition->GetParent();
					if(parent)
					{
						Rect parentBounds=parent->GetBounds();
						Rect clientArea=parent->GetClientArea();
						Rect childBounds=composition->GetBounds();

						x+=childBounds.x1+(clientArea.x1-parentBounds.x1);
						y+=childBounds.y1+(clientArea.y1-parentBounds.y1);
						arguments.x=x;
						arguments.y=y;
					}
					composition=parent;
				}
			}

			void GuiGraphicsHost::OnMouseInput(const NativeWindowMouseInfo& info, GuiMouseEvent GuiGraphicsEventReceiver::* eventReceiverEvent)
			{
				GuiGraphicsComposition* composition = 0;
				if (mouseCaptureComposition)
				{
					composition = mouseCaptureComposition;
				}
				else
				{
					auto point = hostRecord.nativeWindow->Convert(NativePoint(info.x, info.y));
					composition = windowComposition->FindComposition(point, true);
				}
				if (composition)
				{
					Rect bounds = composition->GetGlobalBounds();
					Point point = hostRecord.nativeWindow->Convert(NativePoint(info.x, info.y));
					GuiMouseEventArgs arguments;
					arguments.ctrl = info.ctrl;
					arguments.shift = info.shift;
					arguments.left = info.left;
					arguments.middle = info.middle;
					arguments.right = info.right;
					arguments.wheel = info.wheel;
					arguments.nonClient = info.nonClient;
					arguments.x = point.x - bounds.x1;
					arguments.y = point.y - bounds.y1;
					RaiseMouseEvent(arguments, composition, eventReceiverEvent);
				}
			}

			void GuiGraphicsHost::RecreateRenderTarget()
			{
				windowComposition->UpdateRelatedHostRecord(nullptr);
				GetGuiGraphicsResourceManager()->RecreateRenderTarget(hostRecord.nativeWindow);
				RefreshRelatedHostRecord(hostRecord.nativeWindow);
			}

			INativeWindowListener::HitTestResult GuiGraphicsHost::HitTest(NativePoint location)
			{
				NativeRect bounds = hostRecord.nativeWindow->GetBounds();
				NativeRect clientBounds = hostRecord.nativeWindow->GetClientBoundsInScreen();
				NativePoint clientLocation(location.x + bounds.x1 - clientBounds.x1, location.y + bounds.y1 - clientBounds.y1);
				auto point = hostRecord.nativeWindow->Convert(clientLocation);
				GuiGraphicsComposition* hitComposition = windowComposition->FindComposition(point, false);
				while (hitComposition)
				{
					INativeWindowListener::HitTestResult result = hitComposition->GetAssociatedHitTestResult();
					if (result == INativeWindowListener::NoDecision)
					{
						hitComposition = hitComposition->GetParent();
					}
					else
					{
						return result;
					}
				}
				return INativeWindowListener::NoDecision;
			}

			void GuiGraphicsHost::Moving(NativeRect& bounds, bool fixSizeOnly)
			{
				NativeRect oldBounds = hostRecord.nativeWindow->GetBounds();
				minSize = windowComposition->GetPreferredBounds().GetSize();
				NativeSize minWindowSize = hostRecord.nativeWindow->Convert(minSize) + (oldBounds.GetSize() - hostRecord.nativeWindow->GetClientSize());
				if (bounds.Width() < minWindowSize.x)
				{
					if (fixSizeOnly)
					{
						if (bounds.Width() < minWindowSize.x)
						{
							bounds.x2 = bounds.x1 + minWindowSize.x;
						}
					}
					else if (oldBounds.x1 != bounds.x1)
					{
						bounds.x1 = oldBounds.x2 - minWindowSize.x;
					}
					else if (oldBounds.x2 != bounds.x2)
					{
						bounds.x2 = oldBounds.x1 + minWindowSize.x;
					}
				}
				if (bounds.Height() < minWindowSize.y)
				{
					if (fixSizeOnly)
					{
						if (bounds.Height() < minWindowSize.y)
						{
							bounds.y2 = bounds.y1 + minWindowSize.y;
						}
					}
					else if (oldBounds.y1 != bounds.y1)
					{
						bounds.y1 = oldBounds.y2 - minWindowSize.y;
					}
					else if (oldBounds.y2 != bounds.y2)
					{
						bounds.y2 = oldBounds.y1 + minWindowSize.y;
					}
				}
			}

			void GuiGraphicsHost::Moved()
			{
				NativeSize size = hostRecord.nativeWindow->GetClientSize();
				if (previousClientSize != size)
				{
					previousClientSize = size;
					minSize = windowComposition->GetPreferredBounds().GetSize();
					needRender = true;
				}
			}

			void GuiGraphicsHost::DpiChanged()
			{
				RecreateRenderTarget();
				needRender = true;
			}

			void GuiGraphicsHost::Paint()
			{
				if (!supressPaint)
				{
					needRender = true;
				}
			}

			void GuiGraphicsHost::LeftButtonDown(const NativeWindowMouseInfo& info)
			{
				altActionManager->CloseAltHost();
				MouseCapture(info);
				OnMouseInput(info, &GuiGraphicsEventReceiver::leftButtonDown);
			}

			void GuiGraphicsHost::LeftButtonUp(const NativeWindowMouseInfo& info)
			{
				OnMouseInput(info, &GuiGraphicsEventReceiver::leftButtonUp);
				MouseUncapture(info);
			}

			void GuiGraphicsHost::LeftButtonDoubleClick(const NativeWindowMouseInfo& info)
			{
				LeftButtonDown(info);
				OnMouseInput(info, &GuiGraphicsEventReceiver::leftButtonDoubleClick);
			}

			void GuiGraphicsHost::RightButtonDown(const NativeWindowMouseInfo& info)
			{
				altActionManager->CloseAltHost();
				MouseCapture(info);
				OnMouseInput(info, &GuiGraphicsEventReceiver::rightButtonDown);
			}

			void GuiGraphicsHost::RightButtonUp(const NativeWindowMouseInfo& info)
			{
				OnMouseInput(info, &GuiGraphicsEventReceiver::rightButtonUp);
				MouseUncapture(info);
			}

			void GuiGraphicsHost::RightButtonDoubleClick(const NativeWindowMouseInfo& info)
			{
				RightButtonDown(info);
				OnMouseInput(info, &GuiGraphicsEventReceiver::rightButtonDoubleClick);
			}

			void GuiGraphicsHost::MiddleButtonDown(const NativeWindowMouseInfo& info)
			{
				altActionManager->CloseAltHost();
				MouseCapture(info);
				OnMouseInput(info, &GuiGraphicsEventReceiver::middleButtonDown);
			}

			void GuiGraphicsHost::MiddleButtonUp(const NativeWindowMouseInfo& info)
			{
				OnMouseInput(info, &GuiGraphicsEventReceiver::middleButtonUp);
				MouseUncapture(info);
			}

			void GuiGraphicsHost::MiddleButtonDoubleClick(const NativeWindowMouseInfo& info)
			{
				MiddleButtonDown(info);
				OnMouseInput(info, &GuiGraphicsEventReceiver::middleButtonDoubleClick);
			}

			void GuiGraphicsHost::HorizontalWheel(const NativeWindowMouseInfo& info)
			{
				OnMouseInput(info, &GuiGraphicsEventReceiver::horizontalWheel);
			}

			void GuiGraphicsHost::VerticalWheel(const NativeWindowMouseInfo& info)
			{
				OnMouseInput(info, &GuiGraphicsEventReceiver::verticalWheel);
			}

			void GuiGraphicsHost::MouseMoving(const NativeWindowMouseInfo& info)
			{
				CompositionList newCompositions;
				{
					auto point = hostRecord.nativeWindow->Convert(NativePoint(info.x, info.y));
					GuiGraphicsComposition* composition = windowComposition->FindComposition(point, true);
					while (composition)
					{
						newCompositions.Insert(0, composition);
						composition = composition->GetParent();
					}
				}

				vint firstDifferentIndex = mouseEnterCompositions.Count();
				for (vint i = 0; i < mouseEnterCompositions.Count(); i++)
				{
					if (i == newCompositions.Count())
					{
						firstDifferentIndex = newCompositions.Count();
						break;
					}
					if (mouseEnterCompositions[i] != newCompositions[i])
					{
						firstDifferentIndex = i;
						break;
					}
				}

				for (vint i = mouseEnterCompositions.Count() - 1; i >= firstDifferentIndex; i--)
				{
					GuiGraphicsComposition* composition = mouseEnterCompositions[i];
					if (composition->HasEventReceiver())
					{
						composition->GetEventReceiver()->mouseLeave.Execute(GuiEventArgs(composition));
					}
				}

				CopyFrom(mouseEnterCompositions, newCompositions);
				for (vint i = firstDifferentIndex; i < mouseEnterCompositions.Count(); i++)
				{
					GuiGraphicsComposition* composition = mouseEnterCompositions[i];
					if (composition->HasEventReceiver())
					{
						composition->GetEventReceiver()->mouseEnter.Execute(GuiEventArgs(composition));
					}
				}

				INativeCursor* cursor = 0;
				if (newCompositions.Count() > 0)
				{
					cursor = newCompositions[newCompositions.Count() - 1]->GetRelatedCursor();
				}
				if (cursor)
				{
					hostRecord.nativeWindow->SetWindowCursor(cursor);
				}
				else
				{
					hostRecord.nativeWindow->SetWindowCursor(GetCurrentController()->ResourceService()->GetDefaultSystemCursor());
				}

				OnMouseInput(info, &GuiGraphicsEventReceiver::mouseMove);
			}

			void GuiGraphicsHost::MouseEntered()
			{
			}

			void GuiGraphicsHost::MouseLeaved()
			{
				for(vint i=mouseEnterCompositions.Count()-1;i>=0;i--)
				{
					GuiGraphicsComposition* composition=mouseEnterCompositions[i];
					if(composition->HasEventReceiver())
					{
						composition->GetEventReceiver()->mouseLeave.Execute(GuiEventArgs(composition));
					}
				}
				mouseEnterCompositions.Clear();
			}

			void GuiGraphicsHost::KeyDown(const NativeWindowKeyInfo& info)
			{
				if (altActionManager->KeyDown(info)) { return; }
				if (tabActionManager->KeyDown(info, focusedComposition)) { return; }
				if(shortcutKeyManager && shortcutKeyManager->Execute(info)) { return; }

				if (focusedComposition && focusedComposition->HasEventReceiver())
				{
					OnKeyInput(info, focusedComposition, &GuiGraphicsEventReceiver::keyDown);
				}
			}

			void GuiGraphicsHost::KeyUp(const NativeWindowKeyInfo& info)
			{
				if (altActionManager->KeyUp(info)) { return; }

				if(focusedComposition && focusedComposition->HasEventReceiver())
				{
					OnKeyInput(info, focusedComposition, &GuiGraphicsEventReceiver::keyUp);
				}
			}

			void GuiGraphicsHost::SysKeyDown(const NativeWindowKeyInfo& info)
			{
				if (altActionManager->SysKeyDown(info)) { return; }

				if(focusedComposition && focusedComposition->HasEventReceiver())
				{
					OnKeyInput(info, focusedComposition, &GuiGraphicsEventReceiver::systemKeyDown);
				}
			}

			void GuiGraphicsHost::SysKeyUp(const NativeWindowKeyInfo& info)
			{
				if (altActionManager->SysKeyUp(info)) { return; }

				if (!info.ctrl && !info.shift && info.code == VKEY::_MENU && hostRecord.nativeWindow)
				{
					if (hostRecord.nativeWindow)
					{
						hostRecord.nativeWindow->SupressAlt();
					}
				}

				if (focusedComposition && focusedComposition->HasEventReceiver())
				{
					OnKeyInput(info, focusedComposition, &GuiGraphicsEventReceiver::systemKeyUp);
				}
			}

			void GuiGraphicsHost::Char(const NativeWindowCharInfo& info)
			{
				if (altActionManager->Char(info)) { return; }
				if (tabActionManager->Char(info)) { return; }

				if(focusedComposition && focusedComposition->HasEventReceiver())
				{
					OnCharInput(info, focusedComposition, &GuiGraphicsEventReceiver::charInput);
				}
			}

			void GuiGraphicsHost::GlobalTimer()
			{
				timerManager.Play();

				DateTime now=DateTime::UtcTime();
				if(now.totalMilliseconds-lastCaretTime>=CaretInterval)
				{
					lastCaretTime=now.totalMilliseconds;
					if(focusedComposition && focusedComposition->HasEventReceiver())
					{
						focusedComposition->GetEventReceiver()->caretNotify.Execute(GuiEventArgs(focusedComposition));
					}
				}
				
				Render(false);
			}

			GuiGraphicsHost::GuiGraphicsHost(controls::GuiControlHost* _controlHost, GuiGraphicsComposition* boundsComposition)
				:controlHost(_controlHost)
			{
				altActionManager = new GuiAltActionManager(controlHost);
				tabActionManager = new GuiTabActionManager(controlHost);
				hostRecord.host = this;
				windowComposition=new GuiWindowComposition;
				windowComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				windowComposition->AddChild(boundsComposition);
				RefreshRelatedHostRecord(nullptr);
			}

			GuiGraphicsHost::~GuiGraphicsHost()
			{
				windowComposition->RemoveChild(windowComposition->Children()[0]);
				NotifyFinalizeInstance(windowComposition);

				delete altActionManager;
				delete tabActionManager;
				if (shortcutKeyManager)
				{
					delete shortcutKeyManager;
					shortcutKeyManager = nullptr;
				}

				delete windowComposition;
			}

			INativeWindow* GuiGraphicsHost::GetNativeWindow()
			{
				return hostRecord.nativeWindow;
			}

			void GuiGraphicsHost::SetNativeWindow(INativeWindow* _nativeWindow)
			{
				if (hostRecord.nativeWindow != _nativeWindow)
				{
					if (hostRecord.nativeWindow)
					{
						GetCurrentController()->CallbackService()->UninstallListener(this);
						hostRecord.nativeWindow->UninstallListener(this);
					}

					if (_nativeWindow)
					{
						_nativeWindow->InstallListener(this);
						GetCurrentController()->CallbackService()->InstallListener(this);
						previousClientSize = _nativeWindow->GetClientSize();
						minSize = windowComposition->GetPreferredBounds().GetSize();
						_nativeWindow->SetCaretPoint(_nativeWindow->Convert(caretPoint));
						needRender = true;
					}

					RefreshRelatedHostRecord(_nativeWindow);
				}
			}

			GuiGraphicsComposition* GuiGraphicsHost::GetMainComposition()
			{
				return windowComposition;
			}

			void GuiGraphicsHost::Render(bool forceUpdate)
			{
				if (!forceUpdate && !needRender)
				{
					return;
				}
				needRender = false;

				if(hostRecord.nativeWindow && hostRecord.nativeWindow->IsVisible())
				{
					supressPaint = true;
					hostRecord.renderTarget->StartRendering();
					windowComposition->Render(Size());
					{
						auto bounds = windowComposition->GetBounds();
						auto preferred = windowComposition->GetPreferredBounds();
						auto width = bounds.Width() > preferred.Width() ? bounds.Width() : preferred.Width();
						auto height = bounds.Height() > preferred.Height() ? bounds.Height() : preferred.Height();
						if (width != bounds.Width() || height != bounds.Height())
						{
							controlHost->UpdateClientSizeAfterRendering(Size(width, height));
						}
					}
					auto result = hostRecord.renderTarget->StopRendering();
					hostRecord.nativeWindow->RedrawContent();
					supressPaint = false;

					switch (result)
					{
					case RenderTargetFailure::ResizeWhileRendering:
						{
							GetGuiGraphicsResourceManager()->ResizeRenderTarget(hostRecord.nativeWindow);
							needRender = true;
						}
						break;
					case RenderTargetFailure::LostDevice:
						{
							RecreateRenderTarget();
							needRender = true;
						}
						break;
					default:;
					}
				}

				if (!needRender)
				{
					{
						ProcList procs;
						CopyFrom(procs, afterRenderProcs);
						afterRenderProcs.Clear();
						for (vint i = 0; i < procs.Count(); i++)
						{
							procs[i]();
						}
					}
					{
						ProcMap procs;
						CopyFrom(procs, afterRenderKeyedProcs);
						afterRenderKeyedProcs.Clear();
						for (vint i = 0; i < procs.Count(); i++)
						{
							procs.Values()[i]();
						}
					}
				}
			}

			void GuiGraphicsHost::RequestRender()
			{
				needRender = true;
			}

			void GuiGraphicsHost::InvokeAfterRendering(const Func<void()>& proc, ProcKey key)
			{
				if (key.key == nullptr)
				{
					afterRenderProcs.Add(proc);
				}
				else
				{
					afterRenderKeyedProcs.Set(key, proc);
				}
			}

			void GuiGraphicsHost::InvalidateTabOrderCache()
			{
				tabActionManager->InvalidateTabOrderCache();
			}

			IGuiShortcutKeyManager* GuiGraphicsHost::GetShortcutKeyManager()
			{
				return shortcutKeyManager;
			}

			void GuiGraphicsHost::SetShortcutKeyManager(IGuiShortcutKeyManager* value)
			{
				shortcutKeyManager=value;
			}

			bool GuiGraphicsHost::SetFocus(GuiGraphicsComposition* composition)
			{
				if(!composition || composition->GetRelatedGraphicsHost()!=this)
				{
					return false;
				}
				if(focusedComposition && focusedComposition->HasEventReceiver())
				{
					GuiEventArgs arguments;
					arguments.compositionSource=focusedComposition;
					arguments.eventSource=focusedComposition;
					focusedComposition->GetEventReceiver()->lostFocus.Execute(arguments);
				}
				focusedComposition=composition;
				SetCaretPoint(Point(0, 0));
				if(focusedComposition && focusedComposition->HasEventReceiver())
				{
					GuiEventArgs arguments;
					arguments.compositionSource=focusedComposition;
					arguments.eventSource=focusedComposition;
					focusedComposition->GetEventReceiver()->gotFocus.Execute(arguments);
				}
				return true;
			}

			GuiGraphicsComposition* GuiGraphicsHost::GetFocusedComposition()
			{
				return focusedComposition;
			}

			Point GuiGraphicsHost::GetCaretPoint()
			{
				return caretPoint;
			}

			void GuiGraphicsHost::SetCaretPoint(Point value, GuiGraphicsComposition* referenceComposition)
			{
				if (referenceComposition)
				{
					Rect bounds = referenceComposition->GetGlobalBounds();
					value.x += bounds.x1;
					value.y += bounds.y1;
				}
				caretPoint = value;
				if (hostRecord.nativeWindow)
				{
					hostRecord.nativeWindow->SetCaretPoint(hostRecord.nativeWindow->Convert(caretPoint));
				}
			}

			GuiGraphicsTimerManager* GuiGraphicsHost::GetTimerManager()
			{
				return &timerManager;
			}

			void GuiGraphicsHost::DisconnectComposition(GuiGraphicsComposition* composition)
			{
				DisconnectCompositionInternal(composition);
			}
		}
	}
}