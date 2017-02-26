/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI Reflection: Basic

Interfaces:
***********************************************************************/

#ifdef VCZH_DEBUG_NO_REFLECTION
static_assert(false, "Don't use GacUIReflection.(h|cpp) if VCZH_DEBUG_NO_REFLECTION is defined.");
#endif

#ifndef VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONBASIC
#define VCZH_PRESENTATION_REFLECTION_GUIREFLECTIONBASIC

#include "../../GacUIReflectionHelper.h"

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
			F(presentation::DocumentFontSize)\
			F(presentation::DocumentStyleProperties)\
			F(presentation::DocumentRun)\
			F(presentation::DocumentContainerRun)\
			F(presentation::DocumentContentRun)\
			F(presentation::DocumentTextRun)\
			F(presentation::DocumentInlineObjectRun)\
			F(presentation::DocumentImageRun)\
			F(presentation::DocumentEmbeddedObjectRun)\
			F(presentation::DocumentStylePropertiesRun)\
			F(presentation::DocumentStyleApplicationRun)\
			F(presentation::DocumentHyperlinkRun)\
			F(presentation::DocumentParagraphRun)\
			F(presentation::DocumentStyle)\
			F(presentation::DocumentModel)\
			F(presentation::GuiResourceNodeBase)\
			F(presentation::GuiResourceLocation)\
			F(presentation::GuiResourceTextPos)\
			F(presentation::GuiResourceError)\
			F(presentation::GuiResourceItem)\
			F(presentation::GuiResourceFolder)\
			F(presentation::GuiResource)\
			F(presentation::GuiResourcePathResolver)\
			F(presentation::GuiResourceUsage)\
			F(presentation::IGuiResourceManager)\
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
Interface Proxy
***********************************************************************/

			BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR(presentation::controls::GuiControl::IStyleController)
				presentation::compositions::GuiBoundsComposition* GetBoundsComposition()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetBoundsComposition);
				}

				presentation::compositions::GuiGraphicsComposition* GetContainerComposition()override
				{
					INVOKEGET_INTERFACE_PROXY_NOPARAMS(GetContainerComposition);
				}

				void SetFocusableComposition(presentation::compositions::GuiGraphicsComposition* value)override
				{
					INVOKE_INTERFACE_PROXY(SetFocusableComposition, value);
				}

				void SetText(const WString& value)override
				{
					INVOKE_INTERFACE_PROXY(SetText, value);
				}

				void SetFont(const presentation::FontProperties& value)override
				{
					INVOKE_INTERFACE_PROXY(SetFont, value);
				}

				void SetVisuallyEnabled(bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetVisuallyEnabled, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiControl::IStyleController)
				
			BEGIN_INTERFACE_PROXY_NOPARENT_RAWPTR(presentation::controls::GuiControl::IStyleProvider)
				void AssociateStyleController(presentation::controls::GuiControl::IStyleController* controller)override
				{
					INVOKE_INTERFACE_PROXY(AssociateStyleController, controller);
				}

				void SetFocusableComposition(presentation::compositions::GuiGraphicsComposition* value)override
				{
					INVOKE_INTERFACE_PROXY(SetFocusableComposition, value);
				}

				void SetText(const WString& value)override
				{
					INVOKE_INTERFACE_PROXY(SetText, value);
				}

				void SetFont(const presentation::FontProperties& value)override
				{
					INVOKE_INTERFACE_PROXY(SetFont, value);
				}

				void SetVisuallyEnabled(bool value)override
				{
					INVOKE_INTERFACE_PROXY(SetVisuallyEnabled, value);
				}
			END_INTERFACE_PROXY(presentation::controls::GuiControl::IStyleProvider)

/***********************************************************************
Type Loader
***********************************************************************/

#endif

			extern bool						LoadGuiBasicTypes();
		}
	}
}

#endif