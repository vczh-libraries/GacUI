#include "GuiGraphicsHost.h"
#include "../Controls/GuiWindowControls.h"
#include "../Controls/Styles/GuiThemeStyleFactory.h"

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
GuiGraphicsAnimationManager
***********************************************************************/

			GuiGraphicsAnimationManager::GuiGraphicsAnimationManager()
			{
			}

			GuiGraphicsAnimationManager::~GuiGraphicsAnimationManager()
			{
			}

			void GuiGraphicsAnimationManager::AddAnimation(Ptr<IGuiGraphicsAnimation> animation)
			{
				playingAnimations.Add(animation);
			}

			bool GuiGraphicsAnimationManager::HasAnimation()
			{
				return playingAnimations.Count()>0;
			}

			void GuiGraphicsAnimationManager::Play()
			{
				for(vint i=playingAnimations.Count()-1;i>=0;i--)
				{
					Ptr<IGuiGraphicsAnimation> animation=playingAnimations[i];
					vint totalLength=animation->GetTotalLength();
					vint currentPosition=animation->GetCurrentPosition();
					animation->Play(currentPosition, totalLength);
					if(currentPosition>=totalLength)
					{
						playingAnimations.RemoveAt(i);
						animation->Stop();
					}
				}
			}

/***********************************************************************
IGuiAltAction
***********************************************************************/

			const wchar_t* const IGuiAltAction::Identifier = L"vl::presentation::compositions::IGuiAltAction";
			const wchar_t* const IGuiAltActionContainer::Identifier = L"vl::presentation::compositions::IGuiAltAction";
			const wchar_t* const IGuiAltActionHost::Identifier = L"vl::presentation::compositions::IGuiAltAction";

			bool IGuiAltAction::IsLegalAlt(const WString& alt)
			{
				for (vint i = 0; i < alt.Length(); i++)
				{
					if (alt[i] < L'A' || L'Z' < alt[i])
					{
						return false;
					}
				}
				return true;
			}

			void IGuiAltActionHost::CollectAltActionsFromControl(controls::GuiControl* control, collections::Group<WString, IGuiAltAction*>& actions)
			{
				List<GuiControl*> controls;
				controls.Add(control);
				vint current = 0;

				while (current < controls.Count())
				{
					GuiControl* control = controls[current++];

					if (auto container = control->QueryTypedService<IGuiAltActionContainer>())
					{
						vint count = container->GetAltActionCount();
						for (vint i = 0; i < count; i++)
						{
							auto action = container->GetAltAction(i);
							actions.Add(action->GetAlt(), action);
						}
						continue;
					}
					else if (auto action = control->QueryTypedService<IGuiAltAction>())
					{
						if (action->IsAltAvailable())
						{
							if (action->IsAltEnabled())
							{
								actions.Add(action->GetAlt(), action);
								continue;
							}
						}
					}

					vint count = control->GetChildrenCount();
					for (vint i = 0; i < count; i++)
					{
						controls.Add(control->GetChild(i));
					}
				}
			}

/***********************************************************************
GuiGraphicsHost
***********************************************************************/

			void GuiGraphicsHost::EnterAltHost(IGuiAltActionHost* host)
			{
				ClearAltHost();

				Group<WString, IGuiAltAction*> actions;
				host->CollectAltActions(actions);
				if (actions.Count() == 0)
				{
					CloseAltHost();
					return;
				}

				host->OnActivatedAltHost(currentAltHost);
				currentAltHost = host;
				CreateAltTitles(actions);
			}

			void GuiGraphicsHost::LeaveAltHost()
			{
				if (currentAltHost)
				{
					ClearAltHost();
					auto previousHost = currentAltHost->GetPreviousAltHost();
					currentAltHost->OnDeactivatedAltHost();
					currentAltHost = previousHost;

					if (currentAltHost)
					{
						Group<WString, IGuiAltAction*> actions;
						currentAltHost->CollectAltActions(actions);
						CreateAltTitles(actions);
					}
				}
			}

			bool GuiGraphicsHost::EnterAltKey(wchar_t key)
			{
				currentAltPrefix += key;
				vint index = currentActiveAltActions.Keys().IndexOf(currentAltPrefix);
				if (index == -1)
				{
					if (FilterTitles() == 0)
					{
						currentAltPrefix = currentAltPrefix.Left(currentAltPrefix.Length() - 1);
						FilterTitles();
					}
				}
				else
				{
					auto action = currentActiveAltActions.Values()[index];
					if (action->GetActivatingAltHost())
					{
						EnterAltHost(action->GetActivatingAltHost());
					}
					else
					{
						CloseAltHost();
					}
					action->OnActiveAlt();
					return true;
				}
				return false;
			}

			void GuiGraphicsHost::LeaveAltKey()
			{
				if (currentAltPrefix.Length() >= 1)
				{
					currentAltPrefix = currentAltPrefix.Left(currentAltPrefix.Length() - 1);
				}
				FilterTitles();
			}

			void GuiGraphicsHost::CreateAltTitles(const collections::Group<WString, IGuiAltAction*>& actions)
			{
				if (currentAltHost)
				{
					vint count = actions.Count();
					for (vint i = 0; i < count; i++)
					{
						WString key = actions.Keys()[i];
						const auto& values = actions.GetByIndex(i);
						vint numberLength = 0;
						if (values.Count() == 1 && key.Length() > 0)
						{
							numberLength = 0;
						}
						else if (values.Count() <= 10)
						{
							numberLength = 1;
						}
						else if (values.Count() <= 100)
						{
							numberLength = 2;
						}
						else if (values.Count() <= 1000)
						{
							numberLength = 3;
						}
						else
						{
							continue;
						}

						FOREACH_INDEXER(IGuiAltAction*, action, index, values)
						{
							WString key = actions.Keys()[i];
							if (numberLength > 0)
							{
								WString number = itow(index);
								while (number.Length() < numberLength)
								{
									number = L"0" + number;
								}
								key += number;
							}
							currentActiveAltActions.Add(key, action);
						}
					}

					count = currentActiveAltActions.Count();
					auto window = dynamic_cast<GuiWindow*>(currentAltHost->GetAltComposition()->GetRelatedControlHost());
					auto windowStyle = dynamic_cast<GuiWindow::IStyleController*>(window->GetStyleController());
					for (vint i = 0; i < count; i++)
					{
						auto key = currentActiveAltActions.Keys()[i];
						auto composition = currentActiveAltActions.Values()[i]->GetAltComposition();

						auto labelStyle = windowStyle->CreateShortcutKeyStyle();
						if (!labelStyle)labelStyle = GetCurrentTheme()->CreateShortcutKeyStyle();
						auto label = new GuiLabel(labelStyle);
						label->SetText(key);
						composition->AddChild(label->GetBoundsComposition());
						currentActiveAltTitles.Add(key, label);
					}

					FilterTitles();
				}
			}

			vint GuiGraphicsHost::FilterTitles()
			{
				vint count = currentActiveAltTitles.Count();
				vint visibles = 0;
				for (vint i = 0; i < count; i++)
				{
					auto key = currentActiveAltTitles.Keys()[i];
					auto value = currentActiveAltTitles.Values()[i];
					if (key.Length() >= currentAltPrefix.Length() && key.Left(currentAltPrefix.Length()) == currentAltPrefix)
					{
						value->SetVisible(true);
						if (currentAltPrefix.Length() <= key.Length())
						{
							value->SetText(
								key
								.Insert(currentAltPrefix.Length(), L"[")
								.Insert(currentAltPrefix.Length() + 2, L"]")
								);
						}
						else
						{
							value->SetText(key);
						}
						visibles++;
					}
					else
					{
						value->SetVisible(false);
					}
				}
				return visibles;
			}

			void GuiGraphicsHost::ClearAltHost()
			{
				FOREACH(GuiControl*, title, currentActiveAltTitles.Values())
				{
					SafeDeleteControl(title);
				}
				currentActiveAltActions.Clear();
				currentActiveAltTitles.Clear();
				currentAltPrefix = L"";
			}

			void GuiGraphicsHost::CloseAltHost()
			{
				ClearAltHost();
				while (currentAltHost)
				{
					currentAltHost->OnDeactivatedAltHost();
					currentAltHost = currentAltHost->GetPreviousAltHost();
				}
			}

			void GuiGraphicsHost::DisconnectCompositionInternal(GuiGraphicsComposition* composition)
			{
				for(vint i=0;i<composition->Children().Count();i++)
				{
					DisconnectCompositionInternal(composition->Children().Get(i));
				}
				if(mouseCaptureComposition==composition)
				{
					if(nativeWindow)
					{
						nativeWindow->ReleaseCapture();
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
				if(nativeWindow && (info.left || info.middle || info.right))
				{
					if(!nativeWindow->IsCapturing() && !info.nonClient)
					{
						nativeWindow->RequireCapture();
						mouseCaptureComposition=windowComposition->FindComposition(Point(info.x, info.y));
					}
				}
			}

			void GuiGraphicsHost::MouseUncapture(const NativeWindowMouseInfo& info)
			{
				if(nativeWindow && !(info.left || info.middle || info.right))
				{
					nativeWindow->ReleaseCapture();
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
				while(composition)
				{
					if(composition->HasEventReceiver())
					{
						compositions.Add(composition);
					}
					composition=composition->GetParent();
				}

				GuiKeyEventArgs arguments(composition);
				(NativeWindowKeyInfo&)arguments=info;

				for(vint i=compositions.Count()-1;i>=0;i--)
				{
					compositions[i]->GetEventReceiver()->previewKey.Execute(arguments);
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
				GuiGraphicsComposition* composition=0;
				if(mouseCaptureComposition)
				{
					composition=mouseCaptureComposition;
				}
				else
				{
					composition=windowComposition->FindComposition(Point(info.x, info.y));
				}
				if(composition)
				{
					Rect bounds=composition->GetGlobalBounds();
					GuiMouseEventArgs arguments;
					(NativeWindowMouseInfo&)arguments=info;
					arguments.x-=bounds.x1;
					arguments.y-=bounds.y1;
					RaiseMouseEvent(arguments, composition, eventReceiverEvent);
				}
			}

			INativeWindowListener::HitTestResult GuiGraphicsHost::HitTest(Point location)
			{
				Rect bounds=nativeWindow->GetBounds();
				Rect clientBounds=nativeWindow->GetClientBoundsInScreen();
				Point clientLocation(location.x+bounds.x1-clientBounds.x1, location.y+bounds.y1-clientBounds.y1);
				GuiGraphicsComposition* hitComposition=windowComposition->FindComposition(clientLocation);
				while(hitComposition)
				{
					INativeWindowListener::HitTestResult result=hitComposition->GetAssociatedHitTestResult();
					if(result==INativeWindowListener::NoDecision)
					{
						hitComposition=hitComposition->GetParent();
					}
					else
					{
						return result;
					}
				}
				return INativeWindowListener::NoDecision;
			}

			void GuiGraphicsHost::Moving(Rect& bounds, bool fixSizeOnly)
			{
				Rect oldBounds=nativeWindow->GetBounds();
				minSize=windowComposition->GetPreferredBounds().GetSize();
				Size minWindowSize=minSize+(oldBounds.GetSize()-nativeWindow->GetClientSize());
				if(bounds.Width()<minWindowSize.x)
				{
					if(fixSizeOnly)
					{
						if(bounds.Width()<minWindowSize.x)
						{
							bounds.x2=bounds.x1+minWindowSize.x;
						}
					}
					else if(oldBounds.x1!=bounds.x1)
					{
						bounds.x1=oldBounds.x2-minWindowSize.x;
					}
					else if(oldBounds.x2!=bounds.x2)
					{
						bounds.x2=oldBounds.x1+minWindowSize.x;
					}
				}
				if(bounds.Height()<minWindowSize.y)
				{
					if(fixSizeOnly)
					{
						if(bounds.Height()<minWindowSize.y)
						{
							bounds.y2=bounds.y1+minWindowSize.y;
						}
					}
					else if(oldBounds.y1!=bounds.y1)
					{
						bounds.y1=oldBounds.y2-minWindowSize.y;
					}
					else if(oldBounds.y2!=bounds.y2)
					{
						bounds.y2=oldBounds.y1+minWindowSize.y;
					}
				}
			}

			void GuiGraphicsHost::Moved()
			{
				Size size=nativeWindow->GetClientSize();
				if(previousClientSize!=size)
				{
					previousClientSize=size;
					minSize=windowComposition->GetPreferredBounds().GetSize();
					Render();
				}
			}

			void GuiGraphicsHost::LeftButtonDown(const NativeWindowMouseInfo& info)
			{
				CloseAltHost();
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
				CloseAltHost();
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
				CloseAltHost();
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
					GuiGraphicsComposition* composition=windowComposition->FindComposition(Point(info.x, info.y));
					while(composition)
					{
						newCompositions.Insert(0, composition);
						composition=composition->GetParent();
					}
				}

				vint firstDifferentIndex=mouseEnterCompositions.Count();
				for(vint i=0;i<mouseEnterCompositions.Count();i++)
				{
					if(i==newCompositions.Count())
					{
						firstDifferentIndex=newCompositions.Count();
						break;
					}
					if(mouseEnterCompositions[i]!=newCompositions[i])
					{
						firstDifferentIndex=i;
						break;
					}
				}

				for(vint i=mouseEnterCompositions.Count()-1;i>=firstDifferentIndex;i--)
				{
					GuiGraphicsComposition* composition=mouseEnterCompositions[i];
					if(composition->HasEventReceiver())
					{
						composition->GetEventReceiver()->mouseLeave.Execute(GuiEventArgs(composition));
					}
				}

				CopyFrom(mouseEnterCompositions, newCompositions);
				for(vint i=firstDifferentIndex;i<mouseEnterCompositions.Count();i++)
				{
					GuiGraphicsComposition* composition=mouseEnterCompositions[i];
					if(composition->HasEventReceiver())
					{
						composition->GetEventReceiver()->mouseEnter.Execute(GuiEventArgs(composition));
					}
				}

				INativeCursor* cursor=0;
				if(newCompositions.Count()>0)
				{
					cursor=newCompositions[newCompositions.Count()-1]->GetRelatedCursor();
				}
				if(cursor)
				{
					nativeWindow->SetWindowCursor(cursor);
				}
				else
				{
					nativeWindow->SetWindowCursor(GetCurrentController()->ResourceService()->GetDefaultSystemCursor());
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
				if (!info.ctrl && !info.shift && currentAltHost)
				{
					if (info.code == VKEY_ESCAPE)
					{
						LeaveAltHost();
						return;
					}
					else if (info.code == VKEY_BACK)
					{
						LeaveAltKey();
					}
					else if (VKEY_NUMPAD0 <= info.code && info.code <= VKEY_NUMPAD9)
					{
						if (EnterAltKey((wchar_t)(L'0' + (info.code - VKEY_NUMPAD0))))
						{
							supressAltKey = info.code;
							return;
						}
					}
					else if (('0' <= info.code && info.code <= '9') || ('A' <= info.code && info.code <= 'Z'))
					{
						if (EnterAltKey((wchar_t)info.code))
						{
							supressAltKey = info.code;
							return;
						}
					}
				}

				if (currentAltHost)
				{
					return;
				}
				
				if(shortcutKeyManager && shortcutKeyManager->Execute(info))
				{
					return;
				}
				if(focusedComposition && focusedComposition->HasEventReceiver())
				{
					OnKeyInput(info, focusedComposition, &GuiGraphicsEventReceiver::keyDown);
				}
			}

			void GuiGraphicsHost::KeyUp(const NativeWindowKeyInfo& info)
			{
				if (!info.ctrl && !info.shift && info.code == supressAltKey)
				{
					supressAltKey = 0;
					return;
				}

				if(focusedComposition && focusedComposition->HasEventReceiver())
				{
					OnKeyInput(info, focusedComposition, &GuiGraphicsEventReceiver::keyUp);
				}
			}

			void GuiGraphicsHost::SysKeyDown(const NativeWindowKeyInfo& info)
			{
				if (!info.ctrl && !info.shift && info.code == VKEY_MENU && !currentAltHost)
				{
					if (auto window = dynamic_cast<GuiWindow*>(windowComposition->Children()[0]->GetRelatedControlHost()))
					{
						if (auto altHost = window->QueryTypedService<IGuiAltActionHost>())
						{
							if (!altHost->GetPreviousAltHost())
							{
								EnterAltHost(altHost);
							}
						}
					}
				}

				if (currentAltHost)
				{
					return;
				}

				if(focusedComposition && focusedComposition->HasEventReceiver())
				{
					OnKeyInput(info, focusedComposition, &GuiGraphicsEventReceiver::systemKeyDown);
				}
			}

			void GuiGraphicsHost::SysKeyUp(const NativeWindowKeyInfo& info)
			{
				if (!info.ctrl && !info.shift && info.code == VKEY_MENU && nativeWindow)
				{
					if (nativeWindow)
					{
						nativeWindow->SupressAlt();
					}
				}

				if(focusedComposition && focusedComposition->HasEventReceiver())
				{
					OnKeyInput(info, focusedComposition, &GuiGraphicsEventReceiver::systemKeyUp);
				}
			}

			void GuiGraphicsHost::Char(const NativeWindowCharInfo& info)
			{
				if (!currentAltHost && !supressAltKey)
				{
					if(focusedComposition && focusedComposition->HasEventReceiver())
					{
						OnCharInput(info, focusedComposition, &GuiGraphicsEventReceiver::charInput);
					}
				}
			}

			void GuiGraphicsHost::GlobalTimer()
			{
				if(animationManager.HasAnimation())
				{
					animationManager.Play();
				}

				DateTime now=DateTime::UtcTime();
				if(now.totalMilliseconds-lastCaretTime>=CaretInterval)
				{
					lastCaretTime=now.totalMilliseconds;
					if(focusedComposition && focusedComposition->HasEventReceiver())
					{
						focusedComposition->GetEventReceiver()->caretNotify.Execute(GuiEventArgs(focusedComposition));
					}
				}
				
				Render();
			}

			GuiGraphicsHost::GuiGraphicsHost()
				:nativeWindow(0)
				,shortcutKeyManager(0)
				,windowComposition(0)
				,focusedComposition(0)
				,mouseCaptureComposition(0)
				,lastCaretTime(0)
				,currentAltHost(0)
				,supressAltKey(0)
			{
				windowComposition=new GuiWindowComposition;
				windowComposition->SetAssociatedHost(this);
				windowComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			}

			GuiGraphicsHost::~GuiGraphicsHost()
			{
				if(shortcutKeyManager)
				{
					delete shortcutKeyManager;
					shortcutKeyManager=0;
				}
				delete windowComposition;
			}

			INativeWindow* GuiGraphicsHost::GetNativeWindow()
			{
				return nativeWindow;
			}

			void GuiGraphicsHost::SetNativeWindow(INativeWindow* _nativeWindow)
			{
				if(nativeWindow!=_nativeWindow)
				{
					if(nativeWindow)
					{
						GetCurrentController()->CallbackService()->UninstallListener(this);
						nativeWindow->UninstallListener(this);
					}
					nativeWindow=_nativeWindow;
					windowComposition->SetAttachedWindow(nativeWindow);
					if(nativeWindow)
					{
						nativeWindow->InstallListener(this);
						GetCurrentController()->CallbackService()->InstallListener(this);
						previousClientSize=nativeWindow->GetClientSize();
						minSize=windowComposition->GetPreferredBounds().GetSize();
						nativeWindow->SetCaretPoint(caretPoint);
					}
				}
			}

			GuiGraphicsComposition* GuiGraphicsHost::GetMainComposition()
			{
				return windowComposition;
			}

			void GuiGraphicsHost::Render()
			{
				if(nativeWindow && nativeWindow->IsVisible())
				{
					windowComposition->GetRenderTarget()->StartRendering();
					windowComposition->Render(Size());
					bool success = windowComposition->GetRenderTarget()->StopRendering();
					nativeWindow->RedrawContent();

					if (!success)
					{
						windowComposition->SetAttachedWindow(0);
						GetGuiGraphicsResourceManager()->RecreateRenderTarget(nativeWindow);
						windowComposition->SetAttachedWindow(nativeWindow);
					}
				}
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
				if(referenceComposition)
				{
					Rect bounds=referenceComposition->GetGlobalBounds();
					value.x+=bounds.x1;
					value.y+=bounds.y1;
				}
				caretPoint=value;
				if(nativeWindow)
				{
					nativeWindow->SetCaretPoint(caretPoint);
				}
			}

			GuiGraphicsAnimationManager* GuiGraphicsHost::GetAnimationManager()
			{
				return &animationManager;
			}

			void GuiGraphicsHost::DisconnectComposition(GuiGraphicsComposition* composition)
			{
				DisconnectCompositionInternal(composition);
			}

/***********************************************************************
GuiTimeBasedAnimation
***********************************************************************/

			GuiTimeBasedAnimation::GuiTimeBasedAnimation(vint totalMilliseconds)
				:startTime(0)
				,length(totalMilliseconds)
			{
				Restart();
			}

			GuiTimeBasedAnimation::~GuiTimeBasedAnimation()
			{
			}

			void GuiTimeBasedAnimation::Restart(vint totalMilliseconds)
			{
				startTime=DateTime::LocalTime().totalMilliseconds;
				if(totalMilliseconds>-1)
				{
					length=totalMilliseconds;
				}
			}

			vint GuiTimeBasedAnimation::GetTotalLength()
			{
				return length;
			}

			vint GuiTimeBasedAnimation::GetCurrentPosition()
			{
				return (vint)(DateTime::LocalTime().totalMilliseconds-startTime);
			}

/***********************************************************************
GuiShortcutKeyItem
***********************************************************************/

			GuiShortcutKeyItem::GuiShortcutKeyItem(GuiShortcutKeyManager* _shortcutKeyManager, bool _ctrl, bool _shift, bool _alt, vint _key)
				:shortcutKeyManager(_shortcutKeyManager)
				,ctrl(_ctrl)
				,shift(_shift)
				,alt(_alt)
				,key(_key)
			{
			}

			GuiShortcutKeyItem::~GuiShortcutKeyItem()
			{
			}

			IGuiShortcutKeyManager* GuiShortcutKeyItem::GetManager()
			{
				return shortcutKeyManager;
			}

			WString GuiShortcutKeyItem::GetName()
			{
				WString name;
				if(ctrl) name+=L"Ctrl+";
				if(shift) name+=L"Shift+";
				if(alt) name+=L"Alt+";
				name+=GetCurrentController()->InputService()->GetKeyName(key);
				return name;
			}

			bool GuiShortcutKeyItem::CanActivate(const NativeWindowKeyInfo& info)
			{
				return
					info.ctrl==ctrl &&
					info.shift==shift &&
					info.alt==alt &&
					info.code==key;
			}

			bool GuiShortcutKeyItem::CanActivate(bool _ctrl, bool _shift, bool _alt, vint _key)
			{
				return
					_ctrl==ctrl &&
					_shift==shift &&
					_alt==alt &&
					_key==key;
			}

/***********************************************************************
GuiShortcutKeyManager
***********************************************************************/

			GuiShortcutKeyManager::GuiShortcutKeyManager()
			{
			}

			GuiShortcutKeyManager::~GuiShortcutKeyManager()
			{
			}

			vint GuiShortcutKeyManager::GetItemCount()
			{
				return shortcutKeyItems.Count();
			}

			IGuiShortcutKeyItem* GuiShortcutKeyManager::GetItem(vint index)
			{
				return shortcutKeyItems[index].Obj();
			}

			bool GuiShortcutKeyManager::Execute(const NativeWindowKeyInfo& info)
			{
				bool executed=false;
				FOREACH(Ptr<GuiShortcutKeyItem>, item, shortcutKeyItems)
				{
					if(item->CanActivate(info))
					{
						GuiEventArgs arguments;
						item->Executed.Execute(arguments);
						executed=true;
					}
				}
				return executed;
			}

			IGuiShortcutKeyItem* GuiShortcutKeyManager::CreateShortcut(bool ctrl, bool shift, bool alt, vint key)
			{
				FOREACH(Ptr<GuiShortcutKeyItem>, item, shortcutKeyItems)
				{
					if(item->CanActivate(ctrl, shift, alt, key))
					{
						return item.Obj();
					}
				}
				Ptr<GuiShortcutKeyItem> item=new GuiShortcutKeyItem(this, ctrl, shift, alt, key);
				shortcutKeyItems.Add(item);
				return item.Obj();
			}

			bool GuiShortcutKeyManager::DestroyShortcut(bool ctrl, bool shift, bool alt, vint key)
			{
				FOREACH(Ptr<GuiShortcutKeyItem>, item, shortcutKeyItems)
				{
					if(item->CanActivate(ctrl, shift, alt, key))
					{
						shortcutKeyItems.Remove(item.Obj());
						return true;
					}
				}
				return false;
			}

			IGuiShortcutKeyItem* GuiShortcutKeyManager::TryGetShortcut(bool ctrl, bool shift, bool alt, vint key)
			{
				FOREACH(Ptr<GuiShortcutKeyItem>, item, shortcutKeyItems)
				{
					if(item->CanActivate(ctrl, shift, alt, key))
					{
						return item.Obj();
					}
				}
				return 0;
			}
		}
	}
}