/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIBUTTONCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIBUTTONCONTROLS

#include "GuiBasicControls.h"

namespace vl
{
	namespace presentation
	{

		namespace controls
		{

/***********************************************************************
Buttons
***********************************************************************/

			/// <summary>A control with 3 phases state transffering when mouse click happens.</summary>
			class GuiButton : public GuiControl, public Description<GuiButton>
			{
			public:
				/// <summary>The visual state.</summary>
				enum ControlState
				{
					/// <summary>Normal state.</summary>
					Normal,
					/// <summary>Active state.</summary>
					Active,
					/// <summary>Pressed state.</summary>
					Pressed,
				};

				/// <summary>Style controller interface for <see cref="GuiButton"/>.</summary>
				class IStyleController : virtual public GuiControl::IStyleController, public Description<IStyleController>
				{
				public:
					/// <summary>Called when the control state changed.</summary>
					/// <param name="value">The new control state.</param>
					virtual void						Transfer(ControlState value) = 0;
				};
			protected:
				IStyleController*						styleController;
				bool									clickOnMouseUp;
				bool									mousePressing;
				bool									mouseHoving;
				ControlState							controlState;

				void									OnParentLineChanged()override;
				void									OnActiveAlt()override;
				void									UpdateControlState();
				void									OnLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
				void									OnLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments);
				void									OnMouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void									OnMouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiButton(IStyleController* _styleController);
				~GuiButton();

				/// <summary>Mouse click event.</summary>
				compositions::GuiNotifyEvent			Clicked;

				/// <summary>Test is the <see cref="Clicked"/> event raised when left mouse button up.</summary>
				/// <returns>Returns true if this event is raised when left mouse button up</returns>
				bool									GetClickOnMouseUp();
				/// <summary>Set is the <see cref="Clicked"/> event raised when left mouse button up or not.</summary>
				/// <param name="value">Set to true to make this event raised when left mouse button up</param>
				void									SetClickOnMouseUp(bool value);
			};

			/// <summary>A <see cref="GuiButton"/> with a selection state.</summary>
			class GuiSelectableButton : public GuiButton, public Description<GuiSelectableButton>
			{
			public:
				/// <summary>Style controller interface for <see cref="GuiSelectableButton"/>.</summary>
				class IStyleController : public virtual GuiButton::IStyleController, public Description<IStyleController>
				{
				public:
					/// <summary>Called when the selection state changed.</summary>
					/// <param name="value">The new control state.</param>
					virtual void						SetSelected(bool value) = 0;
				};

				/// <summary>Selection group controller. Control the selection state of all attached button.</summary>
				class GroupController : public GuiComponent, public Description<GroupController>
				{
				protected:
					collections::List<GuiSelectableButton*>	buttons;
				public:
					GroupController();
					~GroupController();

					/// <summary>Called when the group controller is attached to a <see cref="GuiSelectableButton"/>. use [M:vl.presentation.controls.GuiSelectableButton.SetGroupController] to attach or detach a group controller to or from a selectable button.</summary>
					/// <param name="button">The button to attach.</param>
					virtual void						Attach(GuiSelectableButton* button);
					/// <summary>Called when the group controller is deteched to a <see cref="GuiSelectableButton"/>. use [M:vl.presentation.controls.GuiSelectableButton.SetGroupController] to attach or detach a group controller to or from a selectable button.</summary>
					/// <param name="button">The button to detach.</param>
					virtual void						Detach(GuiSelectableButton* button);
					/// <summary>Called when the selection state of any <see cref="GuiSelectableButton"/> changed.</summary>
					/// <param name="button">The button that changed the selection state.</param>
					virtual void						OnSelectedChanged(GuiSelectableButton* button) = 0;
				};

				/// <summary>A mutex group controller, usually for radio buttons.</summary>
				class MutexGroupController : public GroupController, public Description<MutexGroupController>
				{
				protected:
					bool								suppress;
				public:
					MutexGroupController();
					~MutexGroupController();

					void								OnSelectedChanged(GuiSelectableButton* button)override;
				};

			protected:
				IStyleController*						styleController;
				GroupController*						groupController;
				bool									autoSelection;
				bool									isSelected;

				void									OnClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiSelectableButton(IStyleController* _styleController);
				~GuiSelectableButton();

				/// <summary>Group controller changed event.</summary>
				compositions::GuiNotifyEvent			GroupControllerChanged;
				/// <summary>Auto selection changed event.</summary>
				compositions::GuiNotifyEvent			AutoSelectionChanged;
				/// <summary>Selected changed event.</summary>
				compositions::GuiNotifyEvent			SelectedChanged;

				/// <summary>Get the attached group controller.</summary>
				/// <returns>The attached group controller.</returns>
				virtual GroupController*				GetGroupController();
				/// <summary>Set the attached group controller.</summary>
				/// <param name="value">The attached group controller.</param>
				virtual void							SetGroupController(GroupController* value);

				/// <summary>Get the auto selection state. True if the button is automatically selected or unselected when the button is clicked.</summary>
				/// <returns>The auto selection state.</returns>
				virtual bool							GetAutoSelection();
				/// <summary>Set the auto selection state. True if the button is automatically selected or unselected when the button is clicked.</summary>
				/// <param name="value">The auto selection state.</param>
				virtual void							SetAutoSelection(bool value);

				/// <summary>Get the selected state.</summary>
				/// <returns>The selected state.</returns>
				virtual bool							GetSelected();
				/// <summary>Set the selected state.</summary>
				/// <param name="value">The selected state.</param>
				virtual void							SetSelected(bool value);
			};
		}
	}
}

#endif
