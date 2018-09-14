/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Native Window::Direct2D Provider for Windows Implementation::Renderer

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSWINDOWSDIRECT2D
#define VCZH_PRESENTATION_ELEMENTS_GUIGRAPHICSWINDOWSDIRECT2D

#include "..\..\GraphicsElement\GuiGraphicsTextElement.h"
#include "..\..\GraphicsComposition\IncludeForward.h"
#include <d2d1_1.h>
#include <dwrite_1.h>
#include <d2d1effects.h>
#include <wincodec.h>

namespace vl
{
	namespace presentation
	{
		namespace elements
		{
			
/***********************************************************************
Raw API Rendering Element
***********************************************************************/

			class GuiDirect2DElement;
			
			/// <summary>Direct2D Rendering event arguments.</summary>
			struct GuiDirect2DElementEventArgs : compositions::GuiEventArgs
			{
			public:
				/// <summary>The element that raised this event.</summary>
				GuiDirect2DElement*				element;
				/// <summary>Direct2D render target object.</summary>
				ID2D1RenderTarget*				rt;
				/// <summary>DirectWrite factory object.</summary>
				IDWriteFactory*					factoryDWrite;
				/// <summary>Direct2D factory object.</summary>
				ID2D1Factory*					factoryD2D;
				/// <summary>The range for rendering.</summary>
				Rect							bounds;

				GuiDirect2DElementEventArgs(GuiDirect2DElement* _element, ID2D1RenderTarget* _rt, IDWriteFactory* _fdw, ID2D1Factory* _fd2d, Rect _bounds)
					:element(_element)
					,rt(_rt)
					,factoryDWrite(_fdw)
					,factoryD2D(_fd2d)
					,bounds(_bounds)
				{
				}
			};

			/// <summary>
			/// Defines an element for customized rendering using Direct2D.
			/// </summary>
			class GuiDirect2DElement : public GuiElementBase<GuiDirect2DElement>
			{
				DEFINE_GUI_GRAPHICS_ELEMENT(GuiDirect2DElement, L"Direct2DElement")
			protected:
				GuiDirect2DElement();
			public:
				/// <summary>Render target changed (before) event. Resources that binded to the render target can be released at this moment.</summary>
				compositions::GuiGraphicsEvent<GuiDirect2DElementEventArgs>		BeforeRenderTargetChanged;
				/// <summary>Render target changed (after) event. Resources that binded to the render target can be recreated at this moment.</summary>
				compositions::GuiGraphicsEvent<GuiDirect2DElementEventArgs>		AfterRenderTargetChanged;
				/// <summary>Rendering event.</summary>
				compositions::GuiGraphicsEvent<GuiDirect2DElementEventArgs>		Rendering;
			};
		}

		namespace elements_windows_d2d
		{

/***********************************************************************
Functionality
***********************************************************************/

			class IWindowsDirect2DRenderTarget : public elements::IGuiGraphicsRenderTarget
			{
			public:
				virtual ID2D1RenderTarget*					GetDirect2DRenderTarget()=0;
				virtual ComPtr<ID2D1Bitmap>					GetBitmap(INativeImageFrame* frame, bool enabled)=0;
				virtual void								DestroyBitmapCache(INativeImageFrame* frame)=0;
				virtual void								SetTextAntialias(bool antialias, bool verticalAntialias)=0;

				virtual ID2D1Effect*						GetFocusRectangleEffect() = 0;
				virtual ID2D1SolidColorBrush*				CreateDirect2DBrush(Color color)=0;
				virtual void								DestroyDirect2DBrush(Color color)=0;
				virtual ID2D1LinearGradientBrush*			CreateDirect2DLinearBrush(Color c1, Color c2)=0;
				virtual void								DestroyDirect2DLinearBrush(Color c1, Color c2)=0;
				virtual ID2D1RadialGradientBrush*			CreateDirect2DRadialBrush(Color c1, Color c2) = 0;
				virtual void								DestroyDirect2DRadialBrush(Color c1, Color c2) = 0;
			};

			class Direct2DTextFormatPackage
			{
			public:
				ComPtr<IDWriteTextFormat>		textFormat;
				DWRITE_TRIMMING					trimming;
				ComPtr<IDWriteInlineObject>		ellipseInlineObject;
			};

			class IWindowsDirect2DResourceManager : public Interface
			{
			public:
				virtual Direct2DTextFormatPackage*			CreateDirect2DTextFormat(const FontProperties& fontProperties)=0;
				virtual void								DestroyDirect2DTextFormat(const FontProperties& fontProperties)=0;
				virtual Ptr<elements::text::CharMeasurer>	CreateDirect2DCharMeasurer(const FontProperties& fontProperties)=0;
				virtual void								DestroyDirect2DCharMeasurer(const FontProperties& fontProperties)=0;
			};

			extern IWindowsDirect2DResourceManager*			GetWindowsDirect2DResourceManager();
			extern D2D1::ColorF								GetD2DColor(Color color);

/***********************************************************************
OS Supporting
***********************************************************************/

			class IWindowsDirect2DObjectProvider : public Interface
			{
			public:
				virtual void								RecreateRenderTarget(INativeWindow* window) = 0;
				virtual void								ResizeRenderTarget(INativeWindow* window) = 0;
				virtual ID2D1RenderTarget*					GetNativeWindowDirect2DRenderTarget(INativeWindow* window) = 0;
				virtual void								StartRendering(INativeWindow* window) = 0;
				virtual elements::RenderTargetFailure		StopRenderingAndPresent(INativeWindow* window) = 0;

				virtual ID2D1Factory*						GetDirect2DFactory() = 0;
				virtual IDWriteFactory*						GetDirectWriteFactory() = 0;
				virtual IWindowsDirect2DRenderTarget*		GetBindedRenderTarget(INativeWindow* window) = 0;
				virtual void								SetBindedRenderTarget(INativeWindow* window, IWindowsDirect2DRenderTarget* renderTarget) = 0;
				virtual IWICImagingFactory*					GetWICImagingFactory() = 0;
				virtual IWICBitmap*							GetWICBitmap(INativeImageFrame* frame) = 0;
			};

			extern IWindowsDirect2DObjectProvider*			GetWindowsDirect2DObjectProvider();
			extern void										SetWindowsDirect2DObjectProvider(IWindowsDirect2DObjectProvider* provider);
		}
	}
}

extern void RendererMainDirect2D();

#endif