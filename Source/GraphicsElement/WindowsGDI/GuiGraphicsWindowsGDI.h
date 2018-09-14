/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::GDI Provider for Windows Implementation::Renderer

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSWINDOWSGDI
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSWINDOWSGDI

#include "..\GuiGraphicsElement.h"
#include "..\GuiGraphicsTextElement.h"
#include "..\..\GraphicsComposition\IncludeForward.h"
#include "..\..\NativeWindow\Windows\GDI\WinGDI.h"
#include <wincodec.h>
#include <MLang.h>

namespace vl
{
	namespace presentation
	{
		namespace elements
		{
			
/***********************************************************************
Raw API Rendering Element
***********************************************************************/

			class GuiGDIElement;

			/// <summary>GDI Rendering event arguments.</summary>
			struct GuiGDIElementEventArgs : compositions::GuiEventArgs
			{
			public:
				/// <summary>The element that raised this event.</summary>
				GuiGDIElement*				element;
				/// <summary>The device context to draw. The HDC handle can be retrived using dc->GetHandle(), but it is recommended that using the WinDC class directly.</summary>
				windows::WinDC*				dc;
				/// <summary>The range for rendering.</summary>
				Rect						bounds;

				GuiGDIElementEventArgs(GuiGDIElement* _element, windows::WinDC* _dc, Rect _bounds)
					:element(_element)
					,dc(_dc)
					,bounds(_bounds)
				{
				}
			};

			/// <summary>
			/// Defines an element for customized rendering using GDI.
			/// </summary>
			class GuiGDIElement : public GuiElementBase<GuiGDIElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiGDIElement, L"GDIElement")
			protected:
				GuiGDIElement();
			public:
				/// <summary>Rendering event.</summary>
				compositions::GuiGraphicsEvent<GuiGDIElementEventArgs>		Rendering;
			};
		}

		namespace elements_windows_gdi
		{

/***********************************************************************
Functionality
***********************************************************************/

			class IWindowsGDIRenderTarget : public elements::IGuiGraphicsRenderTarget
			{
			public:
				virtual windows::WinDC*					GetDC()=0;
			};

			class IWindowsGDIResourceManager : public Interface
			{
			public:
				virtual Ptr<windows::WinPen>				GetFocusRectanglePen()=0;
				virtual Ptr<windows::WinPen>				CreateGdiPen(Color color)=0;
				virtual void								DestroyGdiPen(Color color)=0;
				virtual Ptr<windows::WinBrush>				CreateGdiBrush(Color color)=0;
				virtual void								DestroyGdiBrush(Color color)=0;
				virtual Ptr<windows::WinFont>				CreateGdiFont(const FontProperties& fontProperties)=0;
				virtual void								DestroyGdiFont(const FontProperties& fontProperties)=0;
				virtual Ptr<elements::text::CharMeasurer>	CreateCharMeasurer(const FontProperties& fontProperties)=0;
				virtual void								DestroyCharMeasurer(const FontProperties& fontProperties)=0;

				virtual Ptr<windows::WinBitmap>				GetBitmap(INativeImageFrame* frame, bool enabled)=0;
				virtual void								DestroyBitmapCache(INativeImageFrame* frame)=0;
			};

			extern IWindowsGDIResourceManager*				GetWindowsGDIResourceManager();

/***********************************************************************
OS Supporting
***********************************************************************/

			class IWindowsGDIObjectProvider : public Interface
			{
			public:
				virtual windows::WinDC*						GetNativeWindowDC(INativeWindow* window)=0;
				virtual IWindowsGDIRenderTarget*			GetBindedRenderTarget(INativeWindow* window)=0;
				virtual void								SetBindedRenderTarget(INativeWindow* window, IWindowsGDIRenderTarget* renderTarget)=0;
				virtual IWICImagingFactory*					GetWICImagingFactory()=0;
				virtual IWICBitmap*							GetWICBitmap(INativeImageFrame* frame)=0;
				virtual IMLangFontLink2*					GetMLangFontLink()=0;
			};

			extern IWindowsGDIObjectProvider*				GetWindowsGDIObjectProvider();
			extern void										SetWindowsGDIObjectProvider(IWindowsGDIObjectProvider* provider);
		}
	}
}

extern void RendererMainGDI();

#endif