/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Basic

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONBASIC
#define VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONBASIC

#include "../../GacUI.h"
#include "../GuiInstanceLoader.h"
#include "../GuiInstanceSharedScript.h"

namespace vl
{
	namespace reflection
	{
		namespace description
		{

#ifndef VCZH_DEBUG_NO_REFLECTION

/***********************************************************************
Type List
***********************************************************************/

#define GUIREFLECTIONBASIC_TYPELIST(F)\
			F(presentation::Color)\
			F(presentation::Alignment)\
			F(presentation::AxisDirection)\
			F(presentation::TextPos)\
			F(presentation::GridPos)\
			F(presentation::Point)\
			F(presentation::Size)\
			F(presentation::Rect)\
			F(presentation::Margin)\
			F(presentation::FontProperties)\
			F(presentation::GlobalStringKey)\
			F(presentation::INativeImageFrame)\
			F(presentation::INativeImage)\
			F(presentation::INativeImage::FormatType)\
			F(presentation::INativeCursor)\
			F(presentation::INativeCursor::SystemCursorType)\
			F(presentation::INativeWindow)\
			F(presentation::INativeWindow::WindowSizeState)\
			F(presentation::INativeDelay)\
			F(presentation::INativeDelay::ExecuteStatus)\
			F(presentation::INativeScreen)\
			F(presentation::INativeImageService)\
			F(presentation::INativeResourceService)\
			F(presentation::INativeAsyncService)\
			F(presentation::INativeClipboardService)\
			F(presentation::INativeScreenService)\
			F(presentation::INativeInputService)\
			F(presentation::INativeDialogService::MessageBoxButtonsInput)\
			F(presentation::INativeDialogService::MessageBoxButtonsOutput)\
			F(presentation::INativeDialogService::MessageBoxDefaultButton)\
			F(presentation::INativeDialogService::MessageBoxIcons)\
			F(presentation::INativeDialogService::MessageBoxModalOptions)\
			F(presentation::INativeDialogService::ColorDialogCustomColorOptions)\
			F(presentation::INativeDialogService::FileDialogTypes)\
			F(presentation::INativeDialogService::FileDialogOptions)\
			F(presentation::INativeController)\
			F(presentation::GuiImageData)\
			F(presentation::GuiTextData)\
			F(presentation::DocumentStyleProperties)\
			F(presentation::DocumentRun)\
			F(presentation::DocumentContainerRun)\
			F(presentation::DocumentContentRun)\
			F(presentation::DocumentTextRun)\
			F(presentation::DocumentInlineObjectRun)\
			F(presentation::DocumentImageRun)\
			F(presentation::DocumentStylePropertiesRun)\
			F(presentation::DocumentStyleApplicationRun)\
			F(presentation::DocumentHyperlinkRun)\
			F(presentation::DocumentParagraphRun)\
			F(presentation::DocumentStyle)\
			F(presentation::DocumentModel)\
			F(presentation::GuiInstanceSharedScript)\
			F(presentation::GuiInstanceStyle)\
			F(presentation::GuiInstanceStyleContext)\
			F(presentation::GuiValueRepr)\
			F(presentation::GuiTextRepr)\
			F(presentation::GuiAttSetterRepr)\
			F(presentation::GuiAttSetterRepr::SetterValue)\
			F(presentation::GuiAttSetterRepr::EventValue)\
			F(presentation::GuiConstructorRepr)\
			F(presentation::GuiInstanceNamespace)\
			F(presentation::GuiInstanceParameter)\
			F(presentation::GuiInstanceProperty)\
			F(presentation::GuiInstanceState)\
			F(presentation::GuiInstanceContext)\
			F(presentation::GuiInstanceContext::NamespaceInfo)\
			F(presentation::GuiResourceNodeBase)\
			F(presentation::GuiResourceItem)\
			F(presentation::GuiResourceFolder)\
			F(presentation::GuiResource)\
			F(presentation::elements::IGuiGraphicsElement)\
			F(presentation::compositions::GuiGraphicsComposition)\
			F(presentation::compositions::GuiGraphicsComposition::MinSizeLimitation)\
			F(presentation::INativeWindowListener::HitTestResult)\
			F(presentation::compositions::GuiGraphicsSite)\
			F(presentation::compositions::GuiWindowComposition)\
			F(presentation::compositions::GuiBoundsComposition)\
			F(presentation::controls::GuiControl)\
			F(presentation::controls::GuiControl::IStyleController)\
			F(presentation::controls::GuiControl::IStyleProvider)\
			F(presentation::controls::GuiComponent)\
			F(presentation::controls::GuiControlHost)\

			GUIREFLECTIONBASIC_TYPELIST(DECL_TYPE_INFO)

/***********************************************************************
Type Declaration
***********************************************************************/

			template<>
			struct TypedValueSerializerProvider<Color>
			{
				static Color GetDefaultValue();
				static bool Serialize(const Color& input, WString& output);
				static bool Deserialize(const WString& input, Color& output);
			};

			template<>
			struct CustomTypeDescriptorSelector<Color>
			{
			public:
				typedef SerializableTypeDescriptor<TypedDefaultValueSerializer<Color>> CustomTypeDescriptorImpl;
			};

			template<>
			struct TypedValueSerializerProvider<GlobalStringKey>
			{
				static GlobalStringKey GetDefaultValue();
				static bool Serialize(const GlobalStringKey& input, WString& output);
				static bool Deserialize(const WString& input, GlobalStringKey& output);
			};

			template<>
			struct CustomTypeDescriptorSelector<GlobalStringKey>
			{
			public:
				typedef SerializableTypeDescriptor<TypedDefaultValueSerializer<GlobalStringKey>> CustomTypeDescriptorImpl;
			};

/***********************************************************************
Interface Proxy
***********************************************************************/

			namespace interface_proxy
			{
				class GuiControl_IStyleController : public ValueInterfaceRoot, public virtual GuiControl::IStyleController
				{
				public:
					GuiControl_IStyleController(Ptr<IValueInterfaceProxy> _proxy)
						:ValueInterfaceRoot(_proxy)
					{
					}

					static GuiControl::IStyleController* Create(Ptr<IValueInterfaceProxy> _proxy)
					{
						return new GuiControl_IStyleController(_proxy);
					}

					compositions::GuiBoundsComposition* GetBoundsComposition()override
					{
						return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBoundsComposition);
					}

					compositions::GuiGraphicsComposition* GetContainerComposition()override
					{
						return INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetContainerComposition);
					}

					void SetFocusableComposition(compositions::GuiGraphicsComposition* value)override
					{
						INVOKE_INTERFACE_PROXY(SetFocusableComposition, value);
					}

					void SetText(const WString& value)override
					{
						INVOKE_INTERFACE_PROXY(SetText, value);
					}

					void SetFont(const FontProperties& value)override
					{
						INVOKE_INTERFACE_PROXY(SetFont, value);
					}

					void SetVisuallyEnabled(bool value)override
					{
						INVOKE_INTERFACE_PROXY(SetVisuallyEnabled, value);
					}
				};
				
				class GuiControl_IStyleProvider : public ValueInterfaceRoot, public virtual GuiControl::IStyleProvider
				{
				public:
					GuiControl_IStyleProvider(Ptr<IValueInterfaceProxy> _proxy)
						:ValueInterfaceRoot(_proxy)
					{
					}

					static GuiControl::IStyleProvider* Create(Ptr<IValueInterfaceProxy> _proxy)
					{
						return new GuiControl_IStyleProvider(_proxy);
					}

					void AssociateStyleController(GuiControl::IStyleController* controller)override
					{
						INVOKE_INTERFACE_PROXY(AssociateStyleController, controller);
					}

					void SetFocusableComposition(compositions::GuiGraphicsComposition* value)override
					{
						INVOKE_INTERFACE_PROXY(SetFocusableComposition, value);
					}

					void SetText(const WString& value)override
					{
						INVOKE_INTERFACE_PROXY(SetText, value);
					}

					void SetFont(const FontProperties& value)override
					{
						INVOKE_INTERFACE_PROXY(SetFont, value);
					}

					void SetVisuallyEnabled(bool value)override
					{
						INVOKE_INTERFACE_PROXY(SetVisuallyEnabled, value);
					}
				};
			}

/***********************************************************************
Type Loader
***********************************************************************/

#endif

			extern bool						LoadGuiBasicTypes();
		}
	}
}

#endif