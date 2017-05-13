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

			void GuiButton::OnParentLineChanged()
			{
				GuiControl::OnParentLineChanged();
				if(GetRelatedControlHost()==0)
				{
					mousePressing=false;
					mouseHoving=false;
					UpdateControlState();
				}
			}

			void GuiButton::OnActiveAlt()
			{
				Clicked.Execute(GetNotifyEventArguments());
			}

			void GuiButton::UpdateControlState()
			{
				auto newControlState = ButtonState::Normal;
				if (mousePressing)
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
					styleController->Transfer(controlState);
				}
			}

			void GuiButton::OnLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(arguments.eventSource==boundsComposition)
				{
					mousePressing=true;
					boundsComposition->GetRelatedGraphicsHost()->SetFocus(boundsComposition);
					UpdateControlState();
					if(!clickOnMouseUp && arguments.eventSource->GetAssociatedControl()==this)
					{
						Clicked.Execute(GetNotifyEventArguments());
					}
				}
			}

			void GuiButton::OnLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
			{
				if(arguments.eventSource==boundsComposition)
				{
					mousePressing=false;
					UpdateControlState();
				}
				if(GetVisuallyEnabled())
				{
					if(mouseHoving && clickOnMouseUp)
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
						Clicked.Execute(GetNotifyEventArguments());
					}
				}
			}

			void GuiButton::OnMouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(arguments.eventSource==boundsComposition)
				{
					mouseHoving=true;
					UpdateControlState();
				}
			}

			void GuiButton::OnMouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(arguments.eventSource==boundsComposition)
				{
					mouseHoving=false;
					UpdateControlState();
				}
			}

			GuiButton::GuiButton(IStyleController* _styleController)
				:GuiControl(_styleController)
				,styleController(_styleController)
				,clickOnMouseUp(true)
				,mousePressing(false)
				,mouseHoving(false)
				,controlState(ButtonState::Normal)
			{
				Clicked.SetAssociatedComposition(boundsComposition);
				styleController->Transfer(ButtonState::Normal);
				SetFocusableComposition(boundsComposition);

				GetEventReceiver()->leftButtonDown.AttachMethod(this, &GuiButton::OnLeftButtonDown);
				GetEventReceiver()->leftButtonUp.AttachMethod(this, &GuiButton::OnLeftButtonUp);
				GetEventReceiver()->mouseEnter.AttachMethod(this, &GuiButton::OnMouseEnter);
				GetEventReceiver()->mouseLeave.AttachMethod(this, &GuiButton::OnMouseLeave);
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

/***********************************************************************
GuiSelectableButton::GroupController
***********************************************************************/

			GuiSelectableButton::GroupController::GroupController()
			{
			}

			GuiSelectableButton::GroupController::~GroupController()
			{
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

			void GuiSelectableButton::OnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				if(autoSelection)
				{
					SetSelected(!GetSelected());
				}
			}

			GuiSelectableButton::GuiSelectableButton(IStyleController* _styleController)
				:GuiButton(_styleController)
				,styleController(_styleController)
				,groupController(0)
				,autoSelection(true)
				,isSelected(false)
			{
				GroupControllerChanged.SetAssociatedComposition(boundsComposition);
				AutoSelectionChanged.SetAssociatedComposition(boundsComposition);
				SelectedChanged.SetAssociatedComposition(boundsComposition);

				Clicked.AttachMethod(this, &GuiSelectableButton::OnClicked);
				styleController->SetSelected(isSelected);
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
				if(isSelected!=value)
				{
					isSelected=value;
					styleController->SetSelected(isSelected);
					if(groupController)
					{
						groupController->OnSelectedChanged(this);
					}
					SelectedChanged.Execute(GetNotifyEventArguments());
				}
			}
		}
	}
}