/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIBASICCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIBASICCONTROLS

#include "../GraphicsElement/GuiGraphicsHost.h"
#include "Templates/GuiControlTemplates.h"

namespace vl
{
	namespace presentation
	{
		namespace theme
		{
			enum class ThemeName;
		}

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

			protected:
				using ControlList = collections::List<GuiControl*>;
				using ControlTemplatePropertyType = TemplateProperty<templates::GuiControlTemplate>;

			private:
				theme::ThemeName						controlThemeName;
				ControlTemplatePropertyType				controlTemplate;
				templates::GuiControlTemplate*			controlTemplateObject = nullptr;

			protected:
				compositions::GuiBoundsComposition*		boundsComposition = nullptr;
				compositions::GuiBoundsComposition*		containerComposition = nullptr;
				compositions::GuiGraphicsComposition*	focusableComposition = nullptr;
				compositions::GuiGraphicsEventReceiver*	eventReceiver = nullptr;

				bool									isEnabled = true;
				bool									isVisuallyEnabled = true;
				bool									isVisible = true;
				WString									alt;
				WString									text;
				FontProperties							font;
				compositions::IGuiAltActionHost*		activatingAltHost = nullptr;

				GuiControl*								parent = nullptr;
				ControlList								children;
				description::Value						tag;
				GuiControl*								tooltipControl = nullptr;
				vint									tooltipWidth = 0;

				virtual void							BeforeControlTemplateUninstalled();
				virtual void							AfterControlTemplateInstalled(bool initialize);
				virtual void							CheckAndStoreControlTemplate(templates::GuiControlTemplate* value);
				virtual void							EnsureControlTemplateExists();
				virtual void							RebuildControlTemplate();
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
				using ControlTemplateType = templates::GuiControlTemplate;

				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiControl(theme::ThemeName themeName);
				~GuiControl();

				/// <summary>Control template changed event. This event will be raised when the control template is changed.</summary>
				compositions::GuiNotifyEvent			ControlTemplateChanged;
				/// <summary>Render target changed event. This event will be raised when the render target of the control is changed.</summary>
				compositions::GuiNotifyEvent			RenderTargetChanged;
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
				/// <summary>Get the associated control template.</summary>
				/// <returns>The control template.</returns>
				ControlTemplatePropertyType				GetControlTemplate();
				/// <summary>Set the associated control template.</summary>
				/// <param name="value">The control template.</param>
				void									SetControlTemplate(const ControlTemplatePropertyType& value);
				/// <summary>Get the associated style controller.</summary>
				/// <returns>The associated style controller.</returns>
				templates::GuiControlTemplate*			GetControlTemplateObject();
				/// <summary>Get the bounds composition for the control. The value is from <see cref="IStyleController::GetBoundsComposition"/>.</summary>
				/// <returns>The bounds composition.</returns>
				compositions::GuiBoundsComposition*		GetBoundsComposition();
				/// <summary>Get the container composition for the control. The value is from <see cref="IStyleController::GetContainerComposition"/>.</summary>
				/// <returns>The container composition.</returns>
				compositions::GuiGraphicsComposition*	GetContainerComposition();
				/// <summary>Get the focusable composition for the control. A focusable composition is the composition to be focused when the control is focused.</summary>
				/// <returns>The focusable composition.</returns>
				compositions::GuiGraphicsComposition*	GetFocusableComposition();
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
				virtual const WString&					GetAlt()override;
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

			/// <summary>Represnets a user customizable control.</summary>
			class GuiCustomControl : public GuiControl, public GuiInstanceRootObject, public AggregatableDescription<GuiCustomControl>
			{
			protected:
				controls::GuiControlHost*				GetControlHostForInstance()override;
				void									OnParentLineChanged()override;
			public:
				/// <summary>Create a control with a specified style controller.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiCustomControl(theme::ThemeName themeName);
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

#define GUI_GENERATE_CONTROL_TEMPLATE_OBJECT_NAME_3(UNIQUE) controlTemplateObject ## UNIQUE
#define GUI_GENERATE_CONTROL_TEMPLATE_OBJECT_NAME_2(UNIQUE) GUI_GENERATE_CONTROL_TEMPLATE_OBJECT_NAME_3(UNIQUE)
#define GUI_GENERATE_CONTROL_TEMPLATE_OBJECT_NAME GUI_GENERATE_CONTROL_TEMPLATE_OBJECT_NAME_2(__LINE__)

#define GUI_SPECIFY_CONTROL_TEMPLATE_TYPE_2(TEMPLATE, BASE_TYPE, NAME) \
			public: \
				using ControlTemplateType = templates::Gui##TEMPLATE; \
			private: \
				templates::Gui##TEMPLATE* NAME = nullptr; \
				void BeforeControlTemplateUninstalled_(); \
				void AfterControlTemplateInstalled_(bool initialize); \
			protected: \
				void BeforeControlTemplateUninstalled()override \
				{\
					BeforeControlTemplateUninstalled_(); \
					BASE_TYPE::BeforeControlTemplateUninstalled(); \
				}\
				void AfterControlTemplateInstalled(bool initialize)override \
				{\
					BASE_TYPE::AfterControlTemplateInstalled(initialize); \
					AfterControlTemplateInstalled_(initialize); \
				}\
				void CheckAndStoreControlTemplate(templates::GuiControlTemplate* value)override \
				{ \
					auto ct = dynamic_cast<templates::Gui##TEMPLATE*>(value); \
					CHECK_ERROR(ct, L"The assigned control template is not vl::presentation::templates::Gui" L ## # TEMPLATE L"."); \
					NAME = ct; \
					BASE_TYPE::CheckAndStoreControlTemplate(value); \
				} \
				bool HasControlTemplateObject() \
				{ \
					return NAME != nullptr; \
				} \
			public: \
				templates::Gui##TEMPLATE* GetControlTemplateObject() \
				{ \
					EnsureControlTemplateExists(); \
					return NAME; \
				} \
			private: \

#define GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(TEMPLATE, BASE_TYPE) GUI_SPECIFY_CONTROL_TEMPLATE_TYPE_2(TEMPLATE, BASE_TYPE, GUI_GENERATE_CONTROL_TEMPLATE_OBJECT_NAME)

		}
	}
}

#endif
