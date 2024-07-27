#include "GuiGraphicsHost.h"
#include "../Controls/GuiWindowControls.h"
#include "../../GraphicsElement/GuiGraphicsResourceManager.h"

namespace vl
{
	namespace presentation
	{
		namespace compositions
		{
			using namespace collections;
			using namespace controls;
			using namespace elements;

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
				// TODO: (enumerable) foreach:indexed(alterable(reversed))
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

			void GuiGraphicsHost::SetFocusInternal(GuiGraphicsComposition* composition)
			{
				if (focusedComposition && focusedComposition->HasEventReceiver())
				{
					GuiEventArgs arguments;
					arguments.compositionSource = focusedComposition;
					arguments.eventSource = focusedComposition;
					focusedComposition->GetEventReceiver()->lostFocus.Execute(arguments);
				}
				focusedComposition = composition;
				SetCaretPoint(Point(0, 0));
				if (focusedComposition && focusedComposition->HasEventReceiver())
				{
					GuiEventArgs arguments;
					arguments.compositionSource = focusedComposition;
					arguments.eventSource = focusedComposition;
					focusedComposition->GetEventReceiver()->gotFocus.Execute(arguments);
				}
			}

			void GuiGraphicsHost::DisconnectCompositionInternal(GuiGraphicsComposition* composition)
			{
				// TODO: (enumerable) foreach
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
						mouseCaptureComposition = windowComposition->FindVisibleComposition(point, true);
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
				GuiCharEventArgs arguments(composition);
				(NativeWindowCharInfo&)arguments = info;

				while (composition)
				{
					if (composition->HasEventReceiver())
					{
						if (!arguments.eventSource)
						{
							arguments.eventSource = composition;
						}
						compositions.Add(composition);
					}
					composition = composition->GetParent();
				}

				// TODO: (enumerable) foreach:reversed
				for(vint i=compositions.Count()-1;i>=0;i--)
				{
					compositions[i]->GetEventReceiver()->previewCharInput.Execute(arguments);
					if(arguments.handled)
					{
						return;
					}
				}

				// TODO: (enumerable) foreach
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
				GuiKeyEventArgs arguments(composition);
				(NativeWindowKeyInfo&)arguments = info;

				while (composition)
				{
					if (composition->HasEventReceiver())
					{
						if (!arguments.eventSource)
						{
							arguments.eventSource = composition;
						}
						compositions.Add(composition);
					}
					composition = composition->GetParent();
				}

				// TODO: (enumerable) foreach:reversed
				for (vint i = compositions.Count() - 1; i >= 0; i--)
				{
					compositions[i]->GetEventReceiver()->previewKey.Execute(arguments);
					if (arguments.handled)
					{
						return;
					}
				}

				// TODO: (enumerable) foreach
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
						Rect parentBounds=parent->GetCachedBounds();
						Rect clientArea=parent->GetCachedClientArea();
						Rect childBounds=composition->GetCachedBounds();

						x+=childBounds.x1+(clientArea.x1-parentBounds.x1);
						y+=childBounds.y1+(clientArea.y1-parentBounds.y1);
						arguments.x=x;
						arguments.y=y;
					}
					composition=parent;
				}
			}

			void GuiGraphicsHost::OnMouseInput(const NativeWindowMouseInfo& info, bool capture, bool release, GuiMouseEvent GuiGraphicsEventReceiver::* eventReceiverEvent)
			{
				if (capture) MouseCapture(info);
				GuiGraphicsComposition* composition = 0;
				if (mouseCaptureComposition)
				{
					composition = mouseCaptureComposition;
				}
				else
				{
					auto point = hostRecord.nativeWindow->Convert(NativePoint(info.x, info.y));
					composition = windowComposition->FindVisibleComposition(point, true);
				}
				if (release) MouseUncapture(info);

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

			void GuiGraphicsHost::ResetRenderTarget()
			{
				windowComposition->UpdateRelatedHostRecord(nullptr);
			}

			void GuiGraphicsHost::CreateRenderTarget()
			{
				GetGuiGraphicsResourceManager()->RecreateRenderTarget(hostRecord.nativeWindow);
				RefreshRelatedHostRecord(hostRecord.nativeWindow);
			}

			INativeWindowListener::HitTestResult GuiGraphicsHost::HitTest(NativePoint location)
			{
				NativeRect bounds = hostRecord.nativeWindow->GetBounds();
				NativeRect clientBounds = hostRecord.nativeWindow->GetClientBoundsInScreen();
				NativePoint clientLocation(location.x + bounds.x1 - clientBounds.x1, location.y + bounds.y1 - clientBounds.y1);
				auto point = hostRecord.nativeWindow->Convert(clientLocation);
				if (auto hitComposition = windowComposition->FindVisibleComposition(point, true))
				{
					return hitComposition->GetRelatedHitTestResult();
				}
				return INativeWindowListener::NoDecision;
			}

			void GuiGraphicsHost::Moving(NativeRect& bounds, bool fixSizeOnly, bool draggingBorder)
			{
				NativeRect oldBounds = hostRecord.nativeWindow->GetBounds();
				minSize = windowComposition->GetCachedMinSize();
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
				RequestUpdateSizeFromNativeWindow();
			}

			void GuiGraphicsHost::DpiChanged(bool preparing)
			{
				if (preparing)
				{
					ResetRenderTarget();
				}
				else
				{
					CreateRenderTarget();
					needRender = true;
				}
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
				OnMouseInput(info, true, false, &GuiGraphicsEventReceiver::leftButtonDown);
			}

			void GuiGraphicsHost::LeftButtonUp(const NativeWindowMouseInfo& info)
			{
				OnMouseInput(info, false, true, &GuiGraphicsEventReceiver::leftButtonUp);
			}

			void GuiGraphicsHost::LeftButtonDoubleClick(const NativeWindowMouseInfo& info)
			{
				altActionManager->CloseAltHost();
				OnMouseInput(info, false, false, &GuiGraphicsEventReceiver::leftButtonDown);
				OnMouseInput(info, false, false, &GuiGraphicsEventReceiver::leftButtonDoubleClick);
			}

			void GuiGraphicsHost::RightButtonDown(const NativeWindowMouseInfo& info)
			{
				altActionManager->CloseAltHost();
				OnMouseInput(info, true, false, &GuiGraphicsEventReceiver::rightButtonDown);
			}

			void GuiGraphicsHost::RightButtonUp(const NativeWindowMouseInfo& info)
			{
				OnMouseInput(info, false, true, &GuiGraphicsEventReceiver::rightButtonUp);
			}

			void GuiGraphicsHost::RightButtonDoubleClick(const NativeWindowMouseInfo& info)
			{
				altActionManager->CloseAltHost();
				OnMouseInput(info, false, false, &GuiGraphicsEventReceiver::rightButtonDown);
				OnMouseInput(info, false, false, &GuiGraphicsEventReceiver::rightButtonDoubleClick);
			}

			void GuiGraphicsHost::MiddleButtonDown(const NativeWindowMouseInfo& info)
			{
				altActionManager->CloseAltHost();
				OnMouseInput(info, true, false, &GuiGraphicsEventReceiver::middleButtonDown);
			}

			void GuiGraphicsHost::MiddleButtonUp(const NativeWindowMouseInfo& info)
			{
				OnMouseInput(info, false, true, &GuiGraphicsEventReceiver::middleButtonUp);
			}

			void GuiGraphicsHost::MiddleButtonDoubleClick(const NativeWindowMouseInfo& info)
			{
				altActionManager->CloseAltHost();
				OnMouseInput(info, false, false, &GuiGraphicsEventReceiver::middleButtonDown);
				OnMouseInput(info, false, false, &GuiGraphicsEventReceiver::middleButtonDoubleClick);
			}

			void GuiGraphicsHost::HorizontalWheel(const NativeWindowMouseInfo& info)
			{
				OnMouseInput(info, false, false, &GuiGraphicsEventReceiver::horizontalWheel);
			}

			void GuiGraphicsHost::VerticalWheel(const NativeWindowMouseInfo& info)
			{
				OnMouseInput(info, false, false, &GuiGraphicsEventReceiver::verticalWheel);
			}

			void GuiGraphicsHost::MouseMoving(const NativeWindowMouseInfo& info)
			{
				CompositionList newCompositions;
				{
					auto point = hostRecord.nativeWindow->Convert(NativePoint(info.x, info.y));
					GuiGraphicsComposition* composition = windowComposition->FindVisibleComposition(point, true);
					while (composition)
					{
						newCompositions.Insert(0, composition);
						composition = composition->GetParent();
					}
				}

				vint firstDifferentIndex = mouseEnterCompositions.Count();
				// TODO: (enumerable) foreach:indexed
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

				// TODO: (enumerable) foreach:reversed Linq:Take
				for (vint i = mouseEnterCompositions.Count() - 1; i >= firstDifferentIndex; i--)
				{
					GuiGraphicsComposition* composition = mouseEnterCompositions[i];
					if (composition->HasEventReceiver())
					{
						composition->GetEventReceiver()->mouseLeave.Execute(GuiEventArgs(composition));
					}
				}

				CopyFrom(mouseEnterCompositions, newCompositions);
				// TODO: (enumerable) foreach Linq:Skip
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

				OnMouseInput(info, false, false, &GuiGraphicsEventReceiver::mouseMove);
			}

			void GuiGraphicsHost::MouseEntered()
			{
			}

			void GuiGraphicsHost::MouseLeaved()
			{
				// TODO: (enumerable) foreach:reversed
				for (vint i = mouseEnterCompositions.Count() - 1; i >= 0; i--)
				{
					GuiGraphicsComposition* composition = mouseEnterCompositions[i];
					if (composition->HasEventReceiver())
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

				auto receiver = focusedComposition;
				if (!receiver) receiver = controlHost->GetFocusableComposition();
				if (!receiver) receiver = controlHost->GetBoundsComposition();

				if (receiver && receiver->HasEventReceiver())
				{
					OnKeyInput(info, receiver, &GuiGraphicsEventReceiver::keyDown);
				}
			}

			void GuiGraphicsHost::KeyUp(const NativeWindowKeyInfo& info)
			{
				if (altActionManager->KeyUp(info)) { return; }
				if (!info.ctrl && !info.shift && info.code == VKEY::KEY_MENU && hostRecord.nativeWindow)
				{
					hostRecord.nativeWindow->SupressAlt();
				}

				auto receiver = focusedComposition;
				if (!receiver) receiver = controlHost->GetFocusableComposition();
				if (!receiver) receiver = controlHost->GetBoundsComposition();

				if(receiver && receiver->HasEventReceiver())
				{
					OnKeyInput(info, receiver, &GuiGraphicsEventReceiver::keyUp);
				}
			}

			void GuiGraphicsHost::Char(const NativeWindowCharInfo& info)
			{
				if (altActionManager->Char(info)) { return; }
				if (tabActionManager->Char(info)) { return; }

				auto receiver = focusedComposition;
				if (!receiver) receiver = controlHost->GetFocusableComposition();
				if (!receiver) receiver = controlHost->GetBoundsComposition();

				if(receiver && receiver->HasEventReceiver())
				{
					OnCharInput(info, receiver, &GuiGraphicsEventReceiver::charInput);
				}
			}

			bool GuiGraphicsHost::NeedRefresh()
			{
				return needRender;
			}

			void GuiGraphicsHost::ForceRefresh(bool handleFailure, bool& updated, bool& failureByResized, bool& failureByLostDevice)
			{
				if (hostRecord.nativeWindow && hostRecord.nativeWindow->IsVisible())
				{
					auto result = Render(true, handleFailure, updated);
					failureByResized |= result == RenderTargetFailure::ResizeWhileRendering;
					failureByLostDevice |= result == RenderTargetFailure::LostDevice;
				}
				else
				{
					updated = false;
					failureByResized = false;
					failureByLostDevice = false;
				}
			}

			void GuiGraphicsHost::GlobalTimer()
			{
				timerManager.Play();

				DateTime now = DateTime::UtcTime();
				if (now.osMilliseconds - lastCaretTime >= CaretInterval)
				{
					lastCaretTime = now.osMilliseconds;
					if (focusedComposition && focusedComposition->HasEventReceiver())
					{
						focusedComposition->GetEventReceiver()->caretNotify.Execute(GuiEventArgs(focusedComposition));
					}
				}

				if (hostRecord.nativeWindow && hostRecord.nativeWindow->IsVisible() && hostRecord.nativeWindow->IsActivelyRefreshing())
				{
					bool updated = false;
					Render(false, true, updated);
				}
			}

			elements::RenderTargetFailure GuiGraphicsHost::Render(bool forceUpdate, bool handleFailure, bool& updated)
			{
				RenderTargetFailure result = RenderTargetFailure::None;

				if (!renderingTriggeredInLastFrame && needRender)
				{
					GuiControlSignalEventArgs arguments(controlHost->boundsComposition);
					arguments.controlSignal = ControlSignal::UpdateRequested;
					controlHost->ControlSignalTrigerred.Execute(arguments);
				}
				else if (renderingTriggeredInLastFrame && !needRender)
				{
					GuiControlSignalEventArgs arguments(controlHost->boundsComposition);
					arguments.controlSignal = ControlSignal::UpdateFullfilled;
					controlHost->ControlSignalTrigerred.Execute(arguments);
				}

				updated = needRender;
				renderingTriggeredInLastFrame = needRender;

				if (!forceUpdate && !needRender)
				{
					return result;
				}
				needRender = false;

				if(hostRecord.nativeWindow && hostRecord.nativeWindow->IsVisible())
				{
					supressPaint = true;
					{
						hostRecord.renderTarget->StartRendering();
						auto nativeOffset = hostRecord.nativeWindow->GetRenderingOffset();
						auto localOffset = hostRecord.nativeWindow->Convert(nativeOffset);
						windowComposition->Render(localOffset);
						result = hostRecord.renderTarget->StopRendering();
						hostRecord.nativeWindow->RedrawContent();
					}
					supressPaint = false;

					switch (result)
					{
					case RenderTargetFailure::ResizeWhileRendering:
						if (handleFailure)
						{
							GetGuiGraphicsResourceManager()->ResizeRenderTarget(hostRecord.nativeWindow);
							needRender = true;
						}
						break;
					case RenderTargetFailure::LostDevice:
						if (handleFailure)
						{
							ResetRenderTarget();
							CreateRenderTarget();
							needRender = true;
						}
						break;
					default:
						{
							supressPaint = true;
							auto bounds = windowComposition->GetCachedBounds();
							windowComposition->Layout_UpdateMinSize();
							auto preferred = windowComposition->GetCachedMinSize();
							auto width = bounds.Width() > preferred.x ? bounds.Width() : preferred.x;
							auto height = bounds.Height() > preferred.y ? bounds.Height() : preferred.y;
							controlHost->UpdateClientSizeAfterRendering(preferred, Size(width, height));
							windowComposition->Layout_UpdateBounds({ width,height });
							supressPaint = false;
						}
					}
				}

				if (!needRender)
				{
					{
						ProcList procs;
						CopyFrom(procs, afterRenderProcs);
						afterRenderProcs.Clear();
						// TODO: (enumerable) foreach
						for (vint i = 0; i < procs.Count(); i++)
						{
							procs[i]();
						}
					}
					{
						ProcMap procs;
						CopyFrom(procs, afterRenderKeyedProcs);
						afterRenderKeyedProcs.Clear();
						// TODO: (enumerable) foreach
						for (vint i = 0; i < procs.Count(); i++)
						{
							procs.Values()[i]();
						}
					}
				}

				return result;
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
						minSize = windowComposition->GetCachedMinSize();
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

			void GuiGraphicsHost::RequestRender()
			{
				needRender = true;
			}

			void GuiGraphicsHost::RequestUpdateSizeFromNativeWindow()
			{
				NativeSize size = hostRecord.nativeWindow->GetClientSize();
				if (previousClientSize != size)
				{
					previousClientSize = size;
					windowComposition->Layout_UpdateMinSize();
					windowComposition->Layout_UpdateBounds(hostRecord.nativeWindow->Convert(size));
					minSize = windowComposition->GetCachedMinSize();
					needRender = true;
				}
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
				if(focusedComposition == composition)
				{
					return true;
				}
				SetFocusInternal(composition);
				return true;
			}

			bool GuiGraphicsHost::ClearFocus()
			{
				if (!focusedComposition) return false;
				SetFocusInternal(nullptr);
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