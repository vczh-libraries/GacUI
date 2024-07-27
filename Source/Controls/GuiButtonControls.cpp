#include "GuiButtonControls.h"

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
GuiButton
***********************************************************************/

			void GuiButton::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiButton::AfterControlTemplateInstalled_(bool initialize)
			{
				TypedControlTemplateObject(true)->SetState(controlState);
			}

			void GuiButton::OnParentLineChanged()
			{
				GuiControl::OnParentLineChanged();
				if (GetRelatedControlHost() == 0)
				{
					mousePressingDirect = false;
					mousePressingIndirect = false;
					mouseHoving = false;
					UpdateControlState();
				}
			}

			void GuiButton::OnActiveAlt()
			{
				if (autoFocus)
				{
					GuiControl::OnActiveAlt();
				}
				Clicked.Execute(GetNotifyEventArguments());
			}

			bool GuiButton::IsTabAvailable()
			{
				return autoFocus && GuiControl::IsTabAvailable();
			}

			void GuiButton::UpdateControlState()
			{
				auto newControlState = ButtonState::Normal;
				if (keyPressing)
				{
					newControlState = ButtonState::Pressed;
				}
				else if (mousePressingDirect || mousePressingIndirect)
				{
					if (mouseHoving)
					{
						newControlState = ButtonState::Pressed;
					}
					else
					{
						newControlState = ButtonState::Active;
					}
				}
				else
				{
					if (mouseHoving)
					{
						newControlState = ButtonState::Active;
					}
					else
					{
						newControlState = ButtonState::Normal;
					}
				}
				if (controlState != newControlState)
				{
					controlState = newControlState;
					TypedControlTemplateObject(true)->SetState(controlState);
				}
			}

			void GuiButton::CheckAndClick(bool skipChecking, compositions::GuiEventArgs& arguments)
			{
				if (!skipChecking)
				{
					auto eventSource = arguments.eventSource->GetAssociatedControl();
					while (eventSource && eventSource != this)
					{
						if (eventSource->GetFocusableComposition())
						{
							return;
						}
						eventSource = eventSource->GetParent();
					}
				}
				Clicked.Execute(GetNotifyEventArguments());
			}

			void GuiButton::OnLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if (arguments.eventSource == boundsComposition)
				{
					mousePressingDirect = true;
				}
				else if (!ignoreChildControlMouseEvents)
				{
					mousePressingIndirect = true;
				}

				if (mousePressingDirect || mousePressingIndirect)
				{
					if (GetVisuallyEnabled() && autoFocus)
					{
						SetFocused();
					}
					UpdateControlState();
					if (GetVisuallyEnabled() && !clickOnMouseUp)
					{
						CheckAndClick(mousePressingIndirect, arguments);
					}
				}
			}

			void GuiButton::OnLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if (mousePressingDirect || mousePressingIndirect)
				{
					bool skipChecking = mousePressingIndirect;
					mousePressingDirect = false;
					mousePressingIndirect = false;
					UpdateControlState();
					if (GetVisuallyEnabled() && mouseHoving && clickOnMouseUp)
					{
						CheckAndClick(skipChecking, arguments);
					}
				}
			}

			void GuiButton::OnMouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (arguments.eventSource == boundsComposition || !ignoreChildControlMouseEvents)
				{
					mouseHoving = true;
					UpdateControlState();
				}
			}

			void GuiButton::OnMouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (arguments.eventSource == boundsComposition || !ignoreChildControlMouseEvents)
				{
					mouseHoving = false;
					UpdateControlState();
				}
			}
			
			void GuiButton::OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments)
			{
				if (arguments.eventSource == focusableComposition && !arguments.ctrl && !arguments.shift && !arguments.alt)
				{
					switch (arguments.code)
					{
					case VKEY::KEY_RETURN:
						CheckAndClick(false, arguments);
						arguments.handled = true;
						break;
					case VKEY::KEY_SPACE:
						if (!arguments.autoRepeatKeyDown)
						{
							keyPressing = true;
							UpdateControlState();
						}
						arguments.handled = true;
						break;
					default:;
					}
				}
			}

			void GuiButton::OnKeyUp(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments)
			{
				if (arguments.eventSource == focusableComposition && !arguments.ctrl && !arguments.shift && !arguments.alt)
				{
					switch (arguments.code)
					{
					case VKEY::KEY_SPACE:
						if (keyPressing)
						{
							keyPressing = false;
							UpdateControlState();
							CheckAndClick(false, arguments);
						}
						arguments.handled = true;
						break;
					default:;
					}
				}
			}

			void GuiButton::OnLostFocus(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if (keyPressing)
				{
					keyPressing = false;
					UpdateControlState();
				}
			}

			GuiButton::GuiButton(theme::ThemeName themeName)
				:GuiControl(themeName)
			{
				Clicked.SetAssociatedComposition(boundsComposition);
				SetFocusableComposition(boundsComposition);

				boundsComposition->GetEventReceiver()->leftButtonDown.AttachMethod(this, &GuiButton::OnLeftButtonDown);
				boundsComposition->GetEventReceiver()->leftButtonUp.AttachMethod(this, &GuiButton::OnLeftButtonUp);
				boundsComposition->GetEventReceiver()->mouseEnter.AttachMethod(this, &GuiButton::OnMouseEnter);
				boundsComposition->GetEventReceiver()->mouseLeave.AttachMethod(this, &GuiButton::OnMouseLeave);
				boundsComposition->GetEventReceiver()->keyDown.AttachMethod(this, &GuiButton::OnKeyDown);
				boundsComposition->GetEventReceiver()->keyUp.AttachMethod(this, &GuiButton::OnKeyUp);
				boundsComposition->GetEventReceiver()->lostFocus.AttachMethod(this, &GuiButton::OnLostFocus);
			}

			GuiButton::~GuiButton()
			{
			}

			bool GuiButton::GetClickOnMouseUp()
			{
				return clickOnMouseUp;
			}

			void GuiButton::SetClickOnMouseUp(bool value)
			{
				clickOnMouseUp=value;
			}

			bool GuiButton::GetAutoFocus()
			{
				return autoFocus;
			}

			void GuiButton::SetAutoFocus(bool value)
			{
				autoFocus = value;
			}

			bool GuiButton::GetIgnoreChildControlMouseEvents()
			{
				return ignoreChildControlMouseEvents;
			}

			void GuiButton::SetIgnoreChildControlMouseEvents(bool value)
			{
				ignoreChildControlMouseEvents = value;
			}

/***********************************************************************
GuiSelectableButton::GroupController
***********************************************************************/

			GuiSelectableButton::GroupController::GroupController()
			{
			}

			GuiSelectableButton::GroupController::~GroupController()
			{
				// TODO: (enumerable) foreach:reversed
				for(vint i=buttons.Count()-1;i>=0;i--)
				{
					buttons[i]->SetGroupController(0);
				}
			}

			void GuiSelectableButton::GroupController::Attach(GuiSelectableButton* button)
			{
				if(!buttons.Contains(button))
				{
					buttons.Add(button);
				}
			}

			void GuiSelectableButton::GroupController::Detach(GuiSelectableButton* button)
			{
				buttons.Remove(button);
			}

/***********************************************************************
GuiSelectableButton::MutexGroupController
***********************************************************************/

			GuiSelectableButton::MutexGroupController::MutexGroupController()
				:suppress(false)
			{
			}

			GuiSelectableButton::MutexGroupController::~MutexGroupController()
			{
			}

			void GuiSelectableButton::MutexGroupController::OnSelectedChanged(GuiSelectableButton* button)
			{
				if(!suppress)
				{
					suppress=true;
					// TODO: (enumerable) foreach
					for(vint i=0;i<buttons.Count();i++)
					{
						buttons[i]->SetSelected(buttons[i]==button);
					}
					suppress=false;
				}
			}

/***********************************************************************
GuiSelectableButton
***********************************************************************/

			void GuiSelectableButton::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiSelectableButton::AfterControlTemplateInstalled_(bool initialize)
			{
				TypedControlTemplateObject(true)->SetSelected(isSelected);
			}

			void GuiSelectableButton::OnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(autoSelection)
				{
					SetSelected(!GetSelected());
				}
			}

			GuiSelectableButton::GuiSelectableButton(theme::ThemeName themeName)
				:GuiButton(themeName)
			{
				GroupControllerChanged.SetAssociatedComposition(boundsComposition);
				AutoSelectionChanged.SetAssociatedComposition(boundsComposition);
				SelectedChanged.SetAssociatedComposition(boundsComposition);

				Clicked.AttachMethod(this, &GuiSelectableButton::OnClicked);
			}
			
			GuiSelectableButton::~GuiSelectableButton()
			{
				if(groupController)
				{
					groupController->Detach(this);
				}
			}

			GuiSelectableButton::GroupController* GuiSelectableButton::GetGroupController()
			{
				return groupController;
			}

			void GuiSelectableButton::SetGroupController(GroupController* value)
			{
				if(groupController)
				{
					groupController->Detach(this);
				}
				groupController=value;
				if(groupController)
				{
					groupController->Attach(this);
				}
				GroupControllerChanged.Execute(GetNotifyEventArguments());
			}

			bool GuiSelectableButton::GetAutoSelection()
			{
				return autoSelection;
			}

			void GuiSelectableButton::SetAutoSelection(bool value)
			{
				if(autoSelection!=value)
				{
					autoSelection=value;
					AutoSelectionChanged.Execute(GetNotifyEventArguments());
				}
			}

			bool GuiSelectableButton::GetSelected()
			{
				return isSelected;
			}

			void GuiSelectableButton::SetSelected(bool value)
			{
				if (isSelected != value)
				{
					isSelected = value;
					TypedControlTemplateObject(true)->SetSelected(isSelected);
					if (groupController)
					{
						groupController->OnSelectedChanged(this);
					}
					SelectedChanged.Execute(GetNotifyEventArguments());
				}
			}
		}
	}
}