/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIBASICCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIBASICCONTROLS

#include "../GraphicsElement/GuiGraphicsHost.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
Basic Construction
***********************************************************************/

			/// <summary>
			/// The base class of all controls.
			/// When the control is destroyed, it automatically destroys sub controls, and the bounds composition from the style controller.
			/// If you want to manually destroy a control, you should first remove it from its parent.
			/// The only way to remove a control from a parent control, is to remove the bounds composition from its parent composition. The same to inserting a control.
			/// </summary>
			class GuiControl : public Object, protected compositions::IGuiAltAction, public Description<GuiControl>
			{
				friend class compositions::GuiGraphicsComposition;
				typedef collections::List<GuiControl*>		ControlList;
			public:
				/// <summary>
				/// Represents a style for a control. A style is something like a skin, but contains some default action handlers.
				/// </summary>
				class IStyleController : public virtual IDescriptable, public Description<IStyleController>
				{
				public:
					/// <summary>Get the bounds composition. A bounds composition represents all visible contents of a control.</summary>
					/// <returns>The bounds composition.</returns>
					virtual compositions::GuiBoundsComposition*		GetBoundsComposition()=0;
					/// <summary>Get the container composition. A container composition is where to place all bounds compositions for child controls.</summary>
					/// <returns>The container composition.</returns>
					virtual compositions::GuiGraphicsComposition*	GetContainerComposition()=0;
					/// <summary>Set the focusable composition. A focusable composition is the composition to be focused when the control is focused.</summary>
					/// <param name="value">The focusable composition.</param>
					virtual void									SetFocusableComposition(compositions::GuiGraphicsComposition* value)=0;
					/// <summary>Set the text to display on the control.</summary>
					/// <param name="value">The text to display.</param>
					virtual void									SetText(const WString& value)=0;
					/// <summary>Set the font to render the text.</summary>
					/// <param name="value">The font to render the text.</param>
					virtual void									SetFont(const FontProperties& value)=0;
					/// <summary>Set the enableing state to affect the rendering of the control.</summary>
					/// <param name="value">The enableing state.</param>
					virtual void									SetVisuallyEnabled(bool value)=0;
				};

				/// <summary>
				/// An empty style for <see cref="GuiControl"/>.
				/// </summary>
				class EmptyStyleController : public Object, public virtual IStyleController, public Description<EmptyStyleController>
				{
				protected:
					compositions::GuiBoundsComposition*				boundsComposition;
				public:
					EmptyStyleController();
					~EmptyStyleController();

					compositions::GuiBoundsComposition*				GetBoundsComposition()override;
					compositions::GuiGraphicsComposition*			GetContainerComposition()override;
					void											SetFocusableComposition(compositions::GuiGraphicsComposition* value)override;
					void											SetText(const WString& value)override;
					void											SetFont(const FontProperties& value)override;
					void											SetVisuallyEnabled(bool value)override;
				};

				/// <summary>
				/// A style provider is a callback interface for some control that already provides a style controller, but the controller need callbacks to create sub compositions or handle actions.
				/// </summary>
				class IStyleProvider : public virtual IDescriptable, public Description<IStyleProvider>
				{
				public:
					/// <summary>Called when a style provider is associated with a style controller.</summary>
					/// <param name="controller">The style controller that is associated.</param>
					virtual void								AssociateStyleController(IStyleController* controller)=0;
					/// <summary>Set the focusable composition. A focusable composition is the composition to be focused when the control is focused.</summary>
					/// <param name="value">The focusable composition.</param>
					virtual void								SetFocusableComposition(compositions::GuiGraphicsComposition* value)=0;
					/// <summary>Set the text to display on the control.</summary>
					/// <param name="value">The text to display.</param>
					virtual void								SetText(const WString& value)=0;
					/// <summary>Set the font to render the text.</summary>
					/// <param name="value">The font to render the text.</param>
					virtual void								SetFont(const FontProperties& value)=0;
					/// <summary>Set the enableing state to affect the rendering of the control.</summary>
					/// <param name="value">The enableing state.</param>
					virtual void								SetVisuallyEnabled(bool value)=0;
				};
			protected:
				Ptr<IStyleController>							styleController;
				compositions::GuiBoundsComposition*				boundsComposition;
				compositions::GuiGraphicsComposition*			focusableComposition;
				compositions::GuiGraphicsEventReceiver*			eventReceiver;

				bool									isEnabled;
				bool									isVisuallyEnabled;
				bool									isVisible;
				WString									alt;
				WString									text;
				FontProperties							font;
				compositions::IGuiAltActionHost*		activatingAltHost;

				GuiControl*								parent;
				ControlList								children;
				description::Value						tag;
				GuiControl*								tooltipControl;
				vint									tooltipWidth;

				virtual void							OnChildInserted(GuiControl* control);
				virtual void							OnChildRemoved(GuiControl* control);
				virtual void							OnParentChanged(GuiControl* oldParent, GuiControl* newParent);
				virtual void							OnParentLineChanged();
				virtual void							OnRenderTargetChanged(elements::IGuiGraphicsRenderTarget* renderTarget);
				virtual void							OnBeforeReleaseGraphicsHost();
				virtual void							UpdateVisuallyEnabled();
				void									SetFocusableComposition(compositions::GuiGraphicsComposition* value);

				bool									IsAltEnabled()override;
				bool									IsAltAvailable()override;
				compositions::GuiGraphicsComposition*	GetAltComposition()override;
				compositions::IGuiAltActionHost*		GetActivatingAltHost()override;
				void									OnActiveAlt()override;

				static bool								SharedPtrDestructorProc(DescriptableObject* obj, bool forceDisposing);

			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiControl(IStyleController* _styleController);
				~GuiControl();

				/// <summary>Visible event. This event will be raised when the visibility state of the control is changed.</summary>
				compositions::GuiNotifyEvent			VisibleChanged;
				/// <summary>Enabled event. This event will be raised when the enabling state of the control is changed.</summary>
				compositions::GuiNotifyEvent			EnabledChanged;
				/// <summary>
				/// Enabled event. This event will be raised when the visually enabling state of the control is changed. A visually enabling is combined by the enabling state and the parent's visually enabling state.
				/// A control is rendered as disabled, not only when the control itself is disabled, but also when the parent control is rendered as disabled.
				/// </summary>
				compositions::GuiNotifyEvent			VisuallyEnabledChanged;
				/// <summary>Alt changed event. This event will be raised when the associated Alt-combined shortcut key of the control is changed.</summary>
				compositions::GuiNotifyEvent			AltChanged;
				/// <summary>Text changed event. This event will be raised when the text of the control is changed.</summary>
				compositions::GuiNotifyEvent			TextChanged;
				/// <summary>Font changed event. This event will be raised when the font of the control is changed.</summary>
				compositions::GuiNotifyEvent			FontChanged;

				/// <summary>A function to create the argument for notify events that raised by itself.</summary>
				/// <returns>The created argument.</returns>
				compositions::GuiEventArgs				GetNotifyEventArguments();
				/// <summary>Get the associated style controller.</summary>
				/// <returns>The associated style controller.</returns>
				IStyleController*						GetStyleController();
				/// <summary>Get the bounds composition for the control. The value is from <see cref="IStyleController::GetBoundsComposition"/>.</summary>
				/// <returns>The bounds composition.</returns>
				compositions::GuiBoundsComposition*		GetBoundsComposition();
				/// <summary>Get the container composition for the control. The value is from <see cref="IStyleController::GetContainerComposition"/>.</summary>
				/// <returns>The container composition.</returns>
				compositions::GuiGraphicsComposition*	GetContainerComposition();
				/// <summary>Get the focusable composition for the control. A focusable composition is the composition to be focused when the control is focused.</summary>
				/// <returns>The focusable composition.</returns>
				compositions::GuiGraphicsComposition*	GetFocusableComposition();
				/// <summary>Get the event receiver from the bounds composition.</summary>
				/// <returns>The event receiver.</returns>
				compositions::GuiGraphicsEventReceiver*	GetEventReceiver();
				/// <summary>Get the parent control.</summary>
				/// <returns>The parent control.</returns>
				GuiControl*								GetParent();
				/// <summary>Get the number of child controls.</summary>
				/// <returns>The number of child controls.</returns>
				vint									GetChildrenCount();
				/// <summary>Get the child control using a specified index.</summary>
				/// <returns>The child control.</returns>
				/// <param name="index">The specified index.</param>
				GuiControl*								GetChild(vint index);
				/// <summary>Put another control in the container composition of this control.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="control">The control to put in this control.</param>
				bool									AddChild(GuiControl* control);
				/// <summary>Test if a control owned by this control.</summary>
				/// <returns>Returns true if the control is owned by this control.</returns>
				/// <param name="control">The control to test.</param>
				bool									HasChild(GuiControl* control);
				
				/// <summary>Get the <see cref="GuiControlHost"/> that contains this control.</summary>
				/// <returns>The <see cref="GuiControlHost"/> that contains this control.</returns>
				virtual GuiControlHost*					GetRelatedControlHost();
				/// <summary>Test if this control is rendered as enabled.</summary>
				/// <returns>Returns true if this control is rendered as enabled.</returns>
				virtual bool							GetVisuallyEnabled();
				/// <summary>Test if this control is enabled.</summary>
				/// <returns>Returns true if this control is enabled.</returns>
				virtual bool							GetEnabled();
				/// <summary>Make the control enabled or disabled.</summary>
				/// <param name="value">Set to true to make the control enabled.</param>
				virtual void							SetEnabled(bool value);
				/// <summary>Test if this visible or invisible.</summary>
				/// <returns>Returns true if this control is visible.</returns>
				virtual bool							GetVisible();
				/// <summary>Make the control visible or invisible.</summary>
				/// <param name="value">Set to true to make the visible enabled.</param>
				virtual void							SetVisible(bool value);
				/// <summary>Get the Alt-combined shortcut key associated with this control.</summary>
				/// <returns>The Alt-combined shortcut key associated with this control.</returns>
				virtual const WString&					GetAlt();
				/// <summary>Associate a Alt-combined shortcut key with this control.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="value">The Alt-combined shortcut key to associate. Only zero, sigle or multiple upper case letters are legal.</param>
				virtual bool							SetAlt(const WString& value);
				/// <summary>Make the control as the parent of multiple Alt-combined shortcut key activatable controls.</summary>
				/// <param name="host">The alt action host object.</param>
				void									SetActivatingAltHost(compositions::IGuiAltActionHost* host);
				/// <summary>Get the text to display on the control.</summary>
				/// <returns>The text to display on the control.</returns>
				virtual const WString&					GetText();
				/// <summary>Set the text to display on the control.</summary>
				/// <param name="value">The text to display on the control.</param>
				virtual void							SetText(const WString& value);
				/// <summary>Get the font to render the text.</summary>
				/// <returns>The font to render the text.</returns>
				virtual const FontProperties&			GetFont();
				/// <summary>Set the font to render the text.</summary>
				/// <param name="value">The font to render the text.</param>
				virtual void							SetFont(const FontProperties& value);
				/// <summary>Focus this control.</summary>
				virtual void							SetFocus();

				/// <summary>Get the tag object of the control.</summary>
				/// <returns>The tag object of the control.</returns>
				description::Value						GetTag();
				/// <summary>Set the tag object of the control.</summary>
				/// <param name="value">The tag object of the control.</param>
				void									SetTag(const description::Value& value);
				/// <summary>Get the tooltip control of the control.</summary>
				/// <returns>The tooltip control of the control.</returns>
				GuiControl*								GetTooltipControl();
				/// <summary>Set the tooltip control of the control. The tooltip control will be released when this control is released. If you set a new tooltip control to replace the old one, the old one will not be owned by this control anymore, therefore user should release the old tooltip control manually.</summary>
				/// <returns>The old tooltip control.</returns>
				/// <param name="value">The tooltip control of the control.</param>
				GuiControl*								SetTooltipControl(GuiControl* value);
				/// <summary>Get the tooltip width of the control.</summary>
				/// <returns>The tooltip width of the control.</returns>
				vint									GetTooltipWidth();
				/// <summary>Set the tooltip width of the control.</summary>
				/// <param name="value">The tooltip width of the control.</param>
				void									SetTooltipWidth(vint value);
				/// <summary>Display the tooltip.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="location">The relative location to specify the left-top position of the tooltip.</param>
				bool									DisplayTooltip(Point location);
				/// <summary>Close the tooltip that owned by this control.</summary>
				void									CloseTooltip();

				/// <summary>Query a service using an identifier. If you want to get a service of type IXXX, use IXXX::Identifier as the identifier.</summary>
				/// <returns>The requested service. If the control doesn't support this service, it will be null.</returns>
				/// <param name="identifier">The identifier.</param>
				virtual IDescriptable*					QueryService(const WString& identifier);

				template<typename T>
				T* QueryTypedService()
				{
					return dynamic_cast<T*>(QueryService(T::Identifier));
				}
			};

			class GuiInstanceRootObject;

			/// <summary>
			/// Represnets a component.
			/// </summary>
			class GuiComponent : public Object, public Description<GuiComponent>
			{
			public:
				GuiComponent();
				~GuiComponent();

				virtual void							Attach(GuiInstanceRootObject* rootObject);
				virtual void							Detach(GuiInstanceRootObject* rootObject);
			};
			
			/// <summary>Represnets a root GUI object.</summary>
			class GuiInstanceRootObject abstract : public Description<GuiInstanceRootObject>
			{
				typedef collections::List<Ptr<description::IValueSubscription>>		SubscriptionList;
			protected:
				collections::SortedList<GuiComponent*>			components;
				SubscriptionList								subscriptions;

				void											ClearSubscriptions();
				void											ClearComponents();
				void											FinalizeInstance();
			public:
				GuiInstanceRootObject();
				~GuiInstanceRootObject();

				/// <summary>Add a subscription. When this control host is disposing, all attached subscriptions will be deleted.</summary>
				/// <returns>Returns null if this operation failed.</returns>
				/// <param name="subscription">The subscription to test.</param>
				Ptr<description::IValueSubscription>			AddSubscription(Ptr<description::IValueSubscription> subscription);
				/// <summary>Remove a subscription.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="subscription">The subscription to test.</param>
				bool											RemoveSubscription(Ptr<description::IValueSubscription> subscription);
				/// <summary>Test does the window contain the subscription.</summary>
				/// <returns>Returns true if the window contains the subscription.</returns>
				/// <param name="subscription">The subscription to test.</param>
				bool											ContainsSubscription(Ptr<description::IValueSubscription> subscription);

				/// <summary>Add a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="component">The component to add.</param>
				bool											AddComponent(GuiComponent* component);

				/// <summary>Add a control host as a component. When this control host is disposing, all attached components will be deleted.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="component">The controlHost to add.</param>
				bool											AddControlHostComponent(GuiControlHost* controlHost);
				/// <summary>Remove a component.</summary>
				/// <returns>Returns true if this operation succeeded.</returns>
				/// <param name="component">The component to remove.</param>
				bool											RemoveComponent(GuiComponent* component);
				/// <summary>Test does the window contain the component.</summary>
				/// <returns>Returns true if the window contains the component.</returns>
				/// <param name="component">The component to test.</param>
				bool											ContainsComponent(GuiComponent* component);
			};

			/// <summary>Represnets a user customizable control.</summary>
			class GuiCustomControl : public GuiControl, public GuiInstanceRootObject, public Description<GuiCustomControl>
			{
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiCustomControl(IStyleController* _styleController);
				~GuiCustomControl();
			};

			template<typename T>
			class GuiObjectComponent : public GuiComponent
			{
			public:
				Ptr<T>				object;

				GuiObjectComponent()
				{
				}

				GuiObjectComponent(Ptr<T> _object)
					:object(_object)
				{
				}
			};

/***********************************************************************
Label
***********************************************************************/

			/// <summary>A control to display a text.</summary>
			class GuiLabel : public GuiControl, public Description<GuiLabel>
			{
			public:
				/// <summary>Style controller interface for <see cref="GuiLabel"/>.</summary>
				class IStyleController : virtual public GuiControl::IStyleController, public Description<IStyleController>
				{
				public:
					/// <summary>Get the default text color.</summary>
					/// <returns>The default text color.</returns>
					virtual Color						GetDefaultTextColor()=0;
					/// <summary>Called when the text color changed.</summary>
					/// <param name="value">The new text color.</param>
					virtual void						SetTextColor(Color value)=0;
				};
			protected:
				Color									textColor;
				IStyleController*						styleController;
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiLabel(IStyleController* _styleController);
				~GuiLabel();
				
				/// <summary>Get the text color.</summary>
				/// <returns>The text color.</returns>
				Color									GetTextColor();
				/// <summary>Set the text color.</summary>
				/// <param name="value">The text color.</param>
				void									SetTextColor(Color value);
			};

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
					virtual void						Transfer(ControlState value)=0;
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
					virtual void						SetSelected(bool value)=0;
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
					virtual void						OnSelectedChanged(GuiSelectableButton* button)=0;
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

/***********************************************************************
Scrolls
***********************************************************************/

			/// <summary>A scroll control, which represents a one dimension sub range of a whole range.</summary>
			class GuiScroll : public GuiControl, public Description<GuiScroll>
			{
			public:
				/// <summary>A command executor for the style controller to change the control state.</summary>
				class ICommandExecutor : public virtual IDescriptable, public Description<ICommandExecutor>
				{
				public:
					/// <summary>Do small decrement.</summary>
					virtual void						SmallDecrease()=0;
					/// <summary>Do small increment.</summary>
					virtual void						SmallIncrease()=0;
					/// <summary>Do big decrement.</summary>
					virtual void						BigDecrease()=0;
					/// <summary>Do big increment.</summary>
					virtual void						BigIncrease()=0;

					/// <summary>Change to total size of the scroll.</summary>
					/// <param name="value">The total size.</param>
					virtual void						SetTotalSize(vint value)=0;
					/// <summary>Change to page size of the scroll.</summary>
					/// <param name="value">The page size.</param>
					virtual void						SetPageSize(vint value)=0;
					/// <summary>Change to position of the scroll.</summary>
					/// <param name="value">The position.</param>
					virtual void						SetPosition(vint value)=0;
				};
				
				/// <summary>Style controller interface for <see cref="GuiScroll"/>.</summary>
				class IStyleController : public virtual GuiControl::IStyleController, public Description<IStyleController>
				{
				public:
					/// <summary>Called when the command executor is changed.</summary>
					/// <param name="value">The command executor.</param>
					virtual void						SetCommandExecutor(ICommandExecutor* value)=0;
					/// <summary>Called when the total size is changed.</summary>
					/// <param name="value">The total size.</param>
					virtual void						SetTotalSize(vint value)=0;
					/// <summary>Called when the page size is changed.</summary>
					/// <param name="value">The page size.</param>
					virtual void						SetPageSize(vint value)=0;
					/// <summary>Called when the position is changed.</summary>
					/// <param name="value">The position.</param>
					virtual void						SetPosition(vint value)=0;
				};
			protected:
				class CommandExecutor : public Object, public ICommandExecutor
				{
				protected:
					GuiScroll*							scroll;
				public:
					CommandExecutor(GuiScroll* _scroll);
					~CommandExecutor();

					void								SmallDecrease()override;
					void								SmallIncrease()override;
					void								BigDecrease()override;
					void								BigIncrease()override;

					void								SetTotalSize(vint value)override;
					void								SetPageSize(vint value)override;
					void								SetPosition(vint value)override;
				};

				IStyleController*						styleController;
				Ptr<CommandExecutor>					commandExecutor;
				vint									totalSize;
				vint									pageSize;
				vint									position;
				vint									smallMove;
				vint									bigMove;
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="_styleController">The style controller.</param>
				GuiScroll(IStyleController* _styleController);
				~GuiScroll();
				
				/// <summary>Total size changed event.</summary>
				compositions::GuiNotifyEvent			TotalSizeChanged;
				/// <summary>Page size changed event.</summary>
				compositions::GuiNotifyEvent			PageSizeChanged;
				/// <summary>Position changed event.</summary>
				compositions::GuiNotifyEvent			PositionChanged;
				/// <summary>Small move changed event.</summary>
				compositions::GuiNotifyEvent			SmallMoveChanged;
				/// <summary>Big move changed event.</summary>
				compositions::GuiNotifyEvent			BigMoveChanged;
				
				/// <summary>Get the total size.</summary>
				/// <returns>The total size.</returns>
				virtual vint							GetTotalSize();
				/// <summary>Set the total size.</summary>
				/// <param name="value">The total size.</param>
				virtual void							SetTotalSize(vint value);
				/// <summary>Get the page size.</summary>
				/// <returns>The page size.</returns>
				virtual vint							GetPageSize();
				/// <summary>Set the page size.</summary>
				/// <param name="value">The page size.</param>
				virtual void							SetPageSize(vint value);
				/// <summary>Get the position.</summary>
				/// <returns>The position.</returns>
				virtual vint							GetPosition();
				/// <summary>Set the position.</summary>
				/// <param name="value">The position.</param>
				virtual void							SetPosition(vint value);
				/// <summary>Get the small move.</summary>
				/// <returns>The small move.</returns>
				virtual vint							GetSmallMove();
				/// <summary>Set the small move.</summary>
				/// <param name="value">The small move.</param>
				virtual void							SetSmallMove(vint value);
				/// <summary>Get the big move.</summary>
				/// <returns>The big move.</returns>
				virtual vint							GetBigMove();
				/// <summary>Set the big move.</summary>
				/// <param name="value">The big move.</param>
				virtual void							SetBigMove(vint value);
				
				/// <summary>Get the minimum possible position.</summary>
				/// <returns>The minimum possible position.</returns>
				vint									GetMinPosition();
				/// <summary>Get the maximum possible position.</summary>
				/// <returns>The maximum possible position.</returns>
				vint									GetMaxPosition();
			};

/***********************************************************************
Dialogs
***********************************************************************/

			/// <summary>Base class for dialogs.</summary>
			class GuiDialogBase abstract : public GuiComponent, public Description<GuiDialogBase>
			{
			protected:
				GuiInstanceRootObject*								rootObject = nullptr;

				GuiWindow*											GetHostWindow();
			public:
				GuiDialogBase();
				~GuiDialogBase();

				void												Attach(GuiInstanceRootObject* _rootObject);
				void												Detach(GuiInstanceRootObject* _rootObject);
			};
			
			/// <summary>Message dialog.</summary>
			class GuiMessageDialog : public GuiDialogBase, public Description<GuiMessageDialog>
			{
			protected:
				INativeDialogService::MessageBoxButtonsInput		input = INativeDialogService::DisplayOK;
				INativeDialogService::MessageBoxDefaultButton		defaultButton = INativeDialogService::DefaultFirst;
				INativeDialogService::MessageBoxIcons				icon = INativeDialogService::IconNone;
				INativeDialogService::MessageBoxModalOptions		modalOption = INativeDialogService::ModalWindow;
				WString												text;
				WString												title;

			public:
				/// <summary>Create a message dialog.</summary>
				GuiMessageDialog();
				~GuiMessageDialog();

				/// <summary>Get the button combination that appear on the dialog.</summary>
				/// <returns>The button combination.</returns>
				INativeDialogService::MessageBoxButtonsInput		GetInput();
				/// <summary>Set the button combination that appear on the dialog.</summary>
				/// <param name="value">The button combination.</param>
				void												SetInput(INativeDialogService::MessageBoxButtonsInput value);
				
				/// <summary>Get the default button for the selected button combination.</summary>
				/// <returns>The default button.</returns>
				INativeDialogService::MessageBoxDefaultButton		GetDefaultButton();
				/// <summary>Set the default button for the selected button combination.</summary>
				/// <param name="value">The default button.</param>
				void												SetDefaultButton(INativeDialogService::MessageBoxDefaultButton value);

				/// <summary>Get the icon that appears on the dialog.</summary>
				/// <returns>The icon.</returns>
				INativeDialogService::MessageBoxIcons				GetIcon();
				/// <summary>Set the icon that appears on the dialog.</summary>
				/// <param name="value">The icon.</param>
				void												SetIcon(INativeDialogService::MessageBoxIcons value);

				/// <summary>Get the way that how this dialog disable windows of the current process.</summary>
				/// <returns>The way that how this dialog disable windows of the current process.</returns>
				INativeDialogService::MessageBoxModalOptions		GetModalOption();
				/// <summary>Set the way that how this dialog disable windows of the current process.</summary>
				/// <param name="value">The way that how this dialog disable windows of the current process.</param>
				void												SetModalOption(INativeDialogService::MessageBoxModalOptions value);

				/// <summary>Get the text for the dialog.</summary>
				/// <returns>The text.</returns>
				const WString&										GetText();
				/// <summary>Set the text for the dialog.</summary>
				/// <param name="value">The text.</param>
				void												SetText(const WString& value);

				/// <summary>Get the title for the dialog.</summary>
				/// <returns>The title.</returns>
				const WString&										GetTitle();
				/// <summary>Set the title for the dialog. If the title is empty, the dialog will use the title of the window that host this dialog.</summary>
				/// <param name="value">The title.</param>
				void												SetTitle(const WString& value);
				
				/// <summary>Show the dialog.</summary>
				/// <returns>Returns the clicked button.</returns>
				INativeDialogService::MessageBoxButtonsOutput		ShowDialog();
			};
			
			/// <summary>Color dialog.</summary>
			class GuiColorDialog : public GuiDialogBase, public Description<GuiColorDialog>
			{
			protected:
				bool												enabledCustomColor = true;
				bool												openedCustomColor = false;
				Color												selectedColor;
				bool												showSelection = true;
				collections::List<Color>							customColors;

			public:
				/// <summary>Create a color dialog.</summary>
				GuiColorDialog();
				~GuiColorDialog();

				/// <summary>Selected color changed event.</summary>
				compositions::GuiNotifyEvent						SelectedColorChanged;
				
				/// <summary>Get if the custom color panel is enabled for the dialog.</summary>
				/// <returns>Returns true if the color panel is enabled for the dialog.</returns>
				bool												GetEnabledCustomColor();
				/// <summary>Set if custom color panel is enabled for the dialog.</summary>
				/// <param name="value">Set to true to enable the custom color panel for the dialog.</param>
				void												SetEnabledCustomColor(bool value);
				
				/// <summary>Get if the custom color panel is opened by default when it is enabled.</summary>
				/// <returns>Returns true if the custom color panel is opened by default.</returns>
				bool												GetOpenedCustomColor();
				/// <summary>Set if the custom color panel is opened by default when it is enabled.</summary>
				/// <param name="value">Set to true to open custom color panel by default if it is enabled.</param>
				void												SetOpenedCustomColor(bool value);
				
				/// <summary>Get the selected color.</summary>
				/// <returns>The selected color.</returns>
				Color												GetSelectedColor();
				/// <summary>Set the selected color.</summary>
				/// <param name="value">The selected color.</param>
				void												SetSelectedColor(Color value);
				
				/// <summary>Get the list to access 16 selected custom colors on the palette. Colors in the list is guaranteed to have exactly 16 items after the dialog is closed.</summary>
				/// <returns>The list to access custom colors on the palette.</returns>
				collections::List<Color>&							GetCustomColors();
				
				/// <summary>Show the dialog.</summary>
				/// <returns>Returns true if the "OK" button is clicked.</returns>
				bool												ShowDialog();
			};
			
			/// <summary>Font dialog.</summary>
			class GuiFontDialog : public GuiDialogBase, public Description<GuiFontDialog>
			{
			protected:
				FontProperties										selectedFont;
				Color												selectedColor;
				bool												showSelection = true;
				bool												showEffect = true;
				bool												forceFontExist = true;

			public:
				/// <summary>Create a font dialog.</summary>
				GuiFontDialog();
				~GuiFontDialog();

				/// <summary>Selected font changed event.</summary>
				compositions::GuiNotifyEvent						SelectedFontChanged;
				/// <summary>Selected color changed event.</summary>
				compositions::GuiNotifyEvent						SelectedColorChanged;
				
				/// <summary>Get the selected font.</summary>
				/// <returns>The selected font.</returns>
				const FontProperties&								GetSelectedFont();
				/// <summary>Set the selected font.</summary>
				/// <param name="value">The selected font.</param>
				void												SetSelectedFont(const FontProperties& value);
				
				/// <summary>Get the selected color.</summary>
				/// <returns>The selected color.</returns>
				Color												GetSelectedColor();
				/// <summary>Set the selected color.</summary>
				/// <param name="value">The selected color.</param>
				void												SetSelectedColor(Color value);
				
				/// <summary>Get if the selected font is alreadt selected on the dialog when it is opened.</summary>
				/// <returns>Returns true if the selected font is already selected on the dialog when it is opened.</returns>
				bool												GetShowSelection();
				/// <summary>Set if the selected font is alreadt selected on the dialog when it is opened.</summary>
				/// <param name="value">Set to true to select the selected font when the dialog is opened.</param>
				void												SetShowSelection(bool value);
				
				/// <summary>Get if the font preview is enabled.</summary>
				/// <returns>Returns true if the font preview is enabled.</returns>
				bool												GetShowEffect();
				/// <summary>Set if the font preview is enabled.</summary>
				/// <param name="value">Set to true to enable the font preview.</param>
				void												SetShowEffect(bool value);
				
				/// <summary>Get if the dialog only accepts an existing font.</summary>
				/// <returns>Returns true if the dialog only accepts an existing font.</returns>
				bool												GetForceFontExist();
				/// <summary>Set if the dialog only accepts an existing font.</summary>
				/// <param name="value">Set to true to let the dialog only accept an existing font.</param>
				void												SetForceFontExist(bool value);
				
				/// <summary>Show the dialog.</summary>
				/// <returns>Returns true if the "OK" button is clicked.</returns>
				bool												ShowDialog();
			};
			
			/// <summary>Base class for file dialogs.</summary>
			class GuiFileDialogBase abstract : public GuiDialogBase, public Description<GuiFileDialogBase>
			{
			protected:
				WString												filter = L"All Files (*.*)|*.*";
				vint												filterIndex = 0;
				bool												enabledPreview = false;
				WString												title;
				WString												fileName;
				WString												directory;
				WString												defaultExtension;
				INativeDialogService::FileDialogOptions				options;

			public:
				GuiFileDialogBase();
				~GuiFileDialogBase();

				/// <summary>File name changed event.</summary>
				compositions::GuiNotifyEvent						FileNameChanged;
				/// <summary>Filter index changed event.</summary>
				compositions::GuiNotifyEvent						FilterIndexChanged;
				
				/// <summary>Get the filter.</summary>
				/// <returns>The filter.</returns>
				const WString&										GetFilter();
				/// <summary>Set the filter. The filter is formed by pairs of filter name and wildcard concatenated by "|", like "Text Files (*.txt)|*.txt|All Files (*.*)|*.*".</summary>
				/// <param name="value">The filter.</param>
				void												SetFilter(const WString& value);
				
				/// <summary>Get the filter index.</summary>
				/// <returns>The filter index.</returns>
				vint												GetFilterIndex();
				/// <summary>Set the filter index.</summary>
				/// <param name="value">The filter index.</param>
				void												SetFilterIndex(vint value);
				
				/// <summary>Get if the file preview is enabled.</summary>
				/// <returns>Returns true if the file preview is enabled.</returns>
				bool												GetEnabledPreview();
				/// <summary>Set if the file preview is enabled.</summary>
				/// <param name="value">Set to true to enable the file preview.</param>
				void												SetEnabledPreview(bool value);
				
				/// <summary>Get the title.</summary>
				/// <returns>The title.</returns>
				WString												GetTitle();
				/// <summary>Set the title.</summary>
				/// <param name="value">The title.</param>
				void												SetTitle(const WString& value);
				
				/// <summary>Get the selected file name.</summary>
				/// <returns>The selected file name.</returns>
				WString												GetFileName();
				/// <summary>Set the selected file name.</summary>
				/// <param name="value">The selected file name.</param>
				void												SetFileName(const WString& value);
				
				/// <summary>Get the default folder.</summary>
				/// <returns>The default folder.</returns>
				WString												GetDirectory();
				/// <summary>Set the default folder.</summary>
				/// <param name="value">The default folder.</param>
				void												SetDirectory(const WString& value);
				
				/// <summary>Get the default file extension.</summary>
				/// <returns>The default file extension.</returns>
				WString												GetDefaultExtension();
				/// <summary>Set the default file extension like "txt". If the user does not specify a file extension, the default file extension will be appended using "." after the file name.</summary>
				/// <param name="value">The default file extension.</param>
				void												SetDefaultExtension(const WString& value);
				
				/// <summary>Get the dialog options.</summary>
				/// <returns>The dialog options.</returns>
				INativeDialogService::FileDialogOptions				GetOptions();
				/// <summary>Set the dialog options.</summary>
				/// <param name="value">The dialog options.</param>
				void												SetOptions(INativeDialogService::FileDialogOptions value);
			};
			
			/// <summary>Open file dialog.</summary>
			class GuiOpenFileDialog : public GuiFileDialogBase, public Description<GuiOpenFileDialog>
			{
			protected:
				collections::List<WString>							fileNames;

			public:
				/// <summary>Create a open file dialog.</summary>
				GuiOpenFileDialog();
				~GuiOpenFileDialog();
				
				/// <summary>Get the list to access multiple selected file names.</summary>
				/// <returns>The list to access multiple selected file names.</returns>
				collections::List<WString>&							GetFileNames();
				
				/// <summary>Show the dialog.</summary>
				/// <returns>Returns true if the "Open" button is clicked.</returns>
				bool												ShowDialog();
			};
			
			/// <summary>Save file dialog.</summary>
			class GuiSaveFileDialog : public GuiFileDialogBase, public Description<GuiSaveFileDialog>
			{
			public:
				/// <summary>Create a save file dialog.</summary>
				GuiSaveFileDialog();
				~GuiSaveFileDialog();

				/// <summary>Show the dialog.</summary>
				/// <returns>Returns true if the "Save" button is clicked.</returns>
				bool												ShowDialog();
			};
			
			namespace list
			{
/***********************************************************************
List interface common implementation
***********************************************************************/

				template<typename T, typename K=typename KeyType<T>::Type>
				class ItemsBase : public Object, public virtual collections::IEnumerable<T>
				{
				protected:
					collections::List<T, K>					items;

					virtual void NotifyUpdateInternal(vint start, vint count, vint newCount)
					{
					}

					virtual bool QueryInsert(vint index, const T& value)
					{
						return true;
					}

					virtual void BeforeInsert(vint index, const T& value)
					{
					}

					virtual void AfterInsert(vint index, const T& value)
					{
					}

					virtual bool QueryRemove(vint index, const T& value)
					{
						return true;
					}

					virtual void BeforeRemove(vint index, const T& value)
					{
					}

					virtual void AfterRemove(vint index, vint count)
					{
					}
					
				public:
					ItemsBase()
					{
					}

					~ItemsBase()
					{
					}

					collections::IEnumerator<T>* CreateEnumerator()const
					{
						return items.CreateEnumerator();
					}

					bool NotifyUpdate(vint start, vint count=1)
					{
						if(start<0 || start>=items.Count() || count<=0 || start+count>items.Count())
						{
							return false;
						}
						else
						{
							NotifyUpdateInternal(start, count, count);
							return true;
						}
					}

					bool Contains(const K& item)const
					{
						return items.Contains(item);
					}

					vint Count()const
					{
						return items.Count();
					}

					vint Count()
					{
						return items.Count();
					}

					const T& Get(vint index)const
					{
						return items.Get(index);
					}

					const T& operator[](vint index)const
					{
						return items.Get(index);
					}

					vint IndexOf(const K& item)const
					{
						return items.IndexOf(item);
					}

					vint Add(const T& item)
					{
						return Insert(items.Count(), item);
					}

					bool Remove(const K& item)
					{
						vint index=items.IndexOf(item);
						if(index==-1) return false;
						return RemoveAt(index);
					}

					bool RemoveAt(vint index)
					{
						if (0 <= index && index < items.Count() && QueryRemove(index, items[index]))
						{
							BeforeRemove(index, items[index]);
							T item = items[index];
							items.RemoveAt(index);
							AfterRemove(index, 1);
							NotifyUpdateInternal(index, 1, 0);
							return true;
						}
						return false;
					}

					bool RemoveRange(vint index, vint count)
					{
						if(count<=0) return false;
						if (0 <= index && index<items.Count() && index + count <= items.Count())
						{
							for (vint i = 0; i < count; i++)
							{
								if (!QueryRemove(index + 1, items[index + i])) return false;
							}
							for (vint i = 0; i < count; i++)
							{
								BeforeRemove(index + i, items[index + i]);
							}
							items.RemoveRange(index, count);
							AfterRemove(index, count);
							NotifyUpdateInternal(index, count, 0);
							return true;
						}
						return false;
					}

					bool Clear()
					{
						vint count = items.Count();
						for (vint i = 0; i < count; i++)
						{
							if (!QueryRemove(i, items[i])) return false;
						}
						for (vint i = 0; i < count; i++)
						{
							BeforeRemove(i, items[i]);
						}
						items.Clear();
						AfterRemove(0, count);
						NotifyUpdateInternal(0, count, 0);
						return true;
					}

					vint Insert(vint index, const T& item)
					{
						if (0 <= index && index <= items.Count() && QueryInsert(index, item))
						{
							BeforeInsert(index, item);
							items.Insert(index, item);
							AfterInsert(index, item);
							NotifyUpdateInternal(index, 0, 1);
							return index;
						}
						else
						{
							return -1;
						}
					}

					bool Set(vint index, const T& item)
					{
						if (0 <= index && index < items.Count())
						{
							if (QueryRemove(index, items[index]) && QueryInsert(index, item))
							{
								BeforeRemove(index, items[index]);
								items.RemoveAt(index);
								AfterRemove(index, 1);

								BeforeInsert(index, item);
								items.Insert(index, item);
								AfterInsert(index, item);

								NotifyUpdateInternal(index, 1, 1);
								return true;
							}
						}
						return false;
					}
				};

				template<typename T>
				class ObservableList : public ItemsBase<T>
				{
				protected:
					Ptr<description::IValueObservableList>		observableList;

					void NotifyUpdateInternal(vint start, vint count, vint newCount)override
					{
						if (observableList)
						{
							observableList->ItemChanged(start, count, newCount);
						}
					}
				public:

					Ptr<description::IValueObservableList> GetWrapper()
					{
						if (!observableList)
						{
							observableList = new description::ValueObservableListWrapper<ObservableList<T>*>(this);
						}
						return observableList;
					}
				};
			}
		}
	}

	namespace collections
	{
		namespace randomaccess_internal
		{
			template<typename T>
			struct RandomAccessable<presentation::controls::list::ItemsBase<T>>
			{
				static const bool							CanRead = true;
				static const bool							CanResize = false;
			};
		}
	}
}

#endif