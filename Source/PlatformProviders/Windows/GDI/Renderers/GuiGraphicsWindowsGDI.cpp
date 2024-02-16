#include "GuiGraphicsWindowsGDI.h"
#include "GuiGraphicsRenderersWindowsGDI.h"
#include "GuiGraphicsLayoutProviderWindowsGDI.h"
#include "..\..\WinNativeWindow.h"
#include "..\..\..\Hosted\GuiHostedController.h"

namespace vl
{
	namespace presentation
	{
		namespace elements
		{

/***********************************************************************
GuiGDIElement
***********************************************************************/

			GuiGDIElement::GuiGDIElement()
			{
			}
		}

		namespace elements_windows_gdi
		{
			using namespace windows;
			using namespace elements;
			using namespace collections;

/***********************************************************************
WindowsGDIRenderTarget
***********************************************************************/

			class WindowsGDIRenderTarget : public IWindowsGDIRenderTarget
			{
			protected:
				INativeWindow*				window = nullptr;
				WinDC*						dc = nullptr;
				List<Rect>					clippers;
				vint						clipperCoverWholeTargetCounter = 0;
				bool						hostedRendering = false;
				bool						rendering = false;

				void ApplyClipper(Rect validArea, bool clipperExists)
				{
					if (clipperExists)
					{
						dc->ClipRegion(Ptr(new WinRegion(validArea.Left(), validArea.Top(), validArea.Right(), validArea.Bottom(), true)));
					}
					else
					{
						dc->RemoveClip();
					}
				}

				void StartRenderingOnNativeWindow() override
				{
					dc = GetWindowsGDIObjectProvider()->GetNativeWindowDC(window);
				}

				RenderTargetFailure StopRenderingOnNativeWindow() override
				{
					dc = nullptr;
					return RenderTargetFailure::None;
				}

				Size GetCanvasSize() override
				{
					return window->Convert(window->GetClientSize());
				}

				void AfterPushedClipper(Rect clipper, Rect validArea, reflection::DescriptableObject* generator) override
				{
					ApplyClipper(validArea, true);
				}

				void AfterPushedClipperAndBecameInvalid(Rect clipper, reflection::DescriptableObject* generator) override
				{
				}

				void AfterPoppedClipperAndBecameValid(Rect validArea, bool clipperExists, reflection::DescriptableObject* generator) override
				{
					ApplyClipper(validArea, clipperExists);
				}

				void AfterPoppedClipper(Rect validArea, bool clipperExists, reflection::DescriptableObject* generator) override
				{
					ApplyClipper(validArea, clipperExists);
				}
			public:
				WindowsGDIRenderTarget(INativeWindow* _window)
					: window(_window)
				{
				}

				WinDC* GetDC()override
				{
					return dc ? dc : GetWindowsGDIObjectProvider()->GetNativeWindowDC(window);
				}
			};

/***********************************************************************
CachedResourceAllocator
***********************************************************************/

			class CachedPenAllocator : public GuiCachedResourceAllocatorBase<CachedPenAllocator, Color, Ptr<WinPen>>
			{
			public:
				Ptr<WinPen> CreateInternal(Color color)
				{
					return Ptr(new WinPen(PS_SOLID, 1, RGB(color.r, color.g, color.b)));
				}
			};

			class CachedBrushAllocator : public GuiCachedResourceAllocatorBase<CachedBrushAllocator, Color, Ptr<WinBrush>>
			{
			public:
				Ptr<WinBrush> CreateInternal(Color color)
				{
					return color.a == 0 ? Ptr(new WinBrush) : Ptr(new WinBrush(RGB(color.r, color.g, color.b)));
				}
			};

			class CachedFontAllocator : public GuiCachedResourceAllocatorBase<CachedFontAllocator, FontProperties, Ptr<WinFont>>
			{
			public:
				static Ptr<WinFont> CreateGdiFont(const FontProperties& value)
				{
					vint size = value.size < 0 ? value.size : -value.size;
					return Ptr(new WinFont(value.fontFamily, size, 0, 0, 0, (value.bold ? FW_BOLD : FW_NORMAL), value.italic, value.underline, value.strikeline, value.antialias));
				}

				Ptr<WinFont> CreateInternal(const FontProperties& value)
				{
					return CreateGdiFont(value);
				}
			};

			class CachedCharMeasurerAllocator : public GuiCachedResourceAllocatorBase<CachedCharMeasurerAllocator, FontProperties, Ptr<text::CharMeasurer>>
			{
			protected:
				class GdiCharMeasurer : public text::CharMeasurer
				{
				protected:
					Ptr<WinFont>			font;
					vint					size;

					Size MeasureInternal(text::UnicodeCodePoint codePoint, IGuiGraphicsRenderTarget* renderTarget)
					{
						if (renderTarget)
						{
							WindowsGDIRenderTarget* gdiRenderTarget = dynamic_cast<WindowsGDIRenderTarget*>(renderTarget);
							WinDC* dc = gdiRenderTarget->GetDC();
							dc->SetFont(font);

							vint count = text::UTF16SPFirst(codePoint.characters[0]) && text::UTF16SPSecond(codePoint.characters[1]) ? 2 : 1;
							SIZE size = dc->MeasureBuffer(codePoint.characters, count, -1);
							return Size(size.cx, size.cy);
						}
						else
						{
							return Size(0, 0);
						}
					}

					vint MeasureWidthInternal(text::UnicodeCodePoint codePoint, IGuiGraphicsRenderTarget* renderTarget)
					{
						return MeasureInternal(codePoint, renderTarget).x;
					}

					vint GetRowHeightInternal(IGuiGraphicsRenderTarget* renderTarget)
					{
						if (renderTarget)
						{
							return MeasureInternal({ L' ' }, renderTarget).y;
						}
						else
						{
							return size;
						}
					}
				public:
					GdiCharMeasurer(Ptr<WinFont> _font, vint _size)
						: text::CharMeasurer(_size)
						, size(_size)
						, font(_font)
					{
					}
				};
			public:
				Ptr<text::CharMeasurer> CreateInternal(const FontProperties& value)
				{
					return Ptr(new GdiCharMeasurer(CachedFontAllocator::CreateGdiFont(value), value.size));
				}
			};

/***********************************************************************
WindowsGDIResourceManager
***********************************************************************/

			class WindowsGDIImageFrameCache : public Object, public INativeImageFrameCache
			{
			protected:
				IWindowsGDIResourceManager* resourceManager = nullptr;
				INativeImageFrame* cachedFrame = nullptr;
				Ptr<WinBitmap>						bitmap;
				Ptr<WinBitmap>						disabledBitmap;
			public:
				WindowsGDIImageFrameCache(IWindowsGDIResourceManager* _resourceManager)
					:resourceManager(_resourceManager)
				{
				}

				~WindowsGDIImageFrameCache()
				{
				}

				void OnAttach(INativeImageFrame* frame)override
				{
					cachedFrame = frame;
					Size size = frame->GetSize();
					bitmap = Ptr(new WinBitmap(size.x, size.y, WinBitmap::vbb32Bits, true));

					IWICBitmap* wicBitmap = GetWindowsGDIObjectProvider()->GetWICBitmap(frame);
					WICRect rect;
					rect.X = 0;
					rect.Y = 0;
					rect.Width = (INT)size.x;
					rect.Height = (INT)size.y;
					wicBitmap->CopyPixels(&rect, (UINT)bitmap->GetLineBytes(), (UINT)(bitmap->GetLineBytes() * size.y), (BYTE*)bitmap->GetScanLines()[0]);

					bitmap->BuildAlphaChannel(false);
				}

				void OnDetach(INativeImageFrame* frame)override
				{
					resourceManager->DestroyBitmapCache(cachedFrame);
				}

				INativeImageFrame* GetFrame()
				{
					return cachedFrame;
				}

				Ptr<WinBitmap> GetBitmap(bool enabled)
				{
					if (enabled)
					{
						return bitmap;
					}
					else
					{
						if (!disabledBitmap)
						{
							vint w = bitmap->GetWidth();
							vint h = bitmap->GetHeight();
							disabledBitmap = Ptr(new WinBitmap(w, h, WinBitmap::vbb32Bits, true));
							for (vint y = 0; y < h; y++)
							{
								BYTE* read = bitmap->GetScanLines()[y];
								BYTE* write = disabledBitmap->GetScanLines()[y];
								for (vint x = 0; x < w; x++)
								{
									BYTE g = (read[0] + read[1] + read[2]) / 6 + read[3] / 2;
									write[0] = g;
									write[1] = g;
									write[2] = g;
									write[3] = read[3];
									read += 4;
									write += 4;
								}
							}
							disabledBitmap->BuildAlphaChannel(false);
						}
						return disabledBitmap;
					}
				}
			};

			class WindowsGDIResourceManager : public GuiGraphicsResourceManager, public IWindowsGDIResourceManager, public INativeControllerListener
			{
				typedef SortedList<Ptr<WindowsGDIImageFrameCache>> ImageCacheList;
			protected:
				SortedList<Ptr<WindowsGDIRenderTarget>>		renderTargets;
				Ptr<WindowsGDILayoutProvider>				layoutProvider;
				Ptr<WinPen>									focusRectanglePen;
				CachedPenAllocator							pens;
				CachedBrushAllocator						brushes;
				CachedFontAllocator							fonts;
				CachedCharMeasurerAllocator					charMeasurers;
				ImageCacheList								imageCaches;
			public:
				WindowsGDIResourceManager()
				{
					layoutProvider = Ptr(new WindowsGDILayoutProvider);
				}

				IGuiGraphicsRenderTarget* GetRenderTarget(INativeWindow* window)override
				{
					return GetWindowsGDIObjectProvider()->GetBindedRenderTarget(window);
				}

				void RecreateRenderTarget(INativeWindow* window)override
				{
				}

				void ResizeRenderTarget(INativeWindow* window)override
				{
				}

				IGuiGraphicsLayoutProvider* GetLayoutProvider()override
				{
					return layoutProvider.Obj();
				}

				void NativeWindowCreated(INativeWindow* window)override
				{
					auto renderTarget = Ptr(new WindowsGDIRenderTarget(window));
					renderTargets.Add(renderTarget);
					GetWindowsGDIObjectProvider()->SetBindedRenderTarget(window, renderTarget.Obj());
				}

				void NativeWindowDestroying(INativeWindow* window)override
				{
					WindowsGDIRenderTarget* renderTarget = dynamic_cast<WindowsGDIRenderTarget*>(GetWindowsGDIObjectProvider()->GetBindedRenderTarget(window));
					GetWindowsGDIObjectProvider()->SetBindedRenderTarget(window, 0);
					renderTargets.Remove(renderTarget);
				}

				Ptr<windows::WinPen> GetFocusRectanglePen()override
				{
					if (!focusRectanglePen)
					{
						DWORD styleArray[] = { 1,1 };
						focusRectanglePen = Ptr(new WinPen(PS_USERSTYLE, PS_ENDCAP_FLAT, PS_JOIN_BEVEL, 1, RGB(255, 255, 255), (DWORD)(sizeof(styleArray) / sizeof(*styleArray)), styleArray));
					}
					return focusRectanglePen;
				}

				Ptr<windows::WinPen> CreateGdiPen(Color color)override
				{
					return pens.Create(color);
				}

				void DestroyGdiPen(Color color)override
				{
					pens.Destroy(color);
				}

				Ptr<windows::WinBrush> CreateGdiBrush(Color color)override
				{
					return brushes.Create(color);
				}

				void DestroyGdiBrush(Color color)override
				{
					brushes.Destroy(color);
				}

				Ptr<windows::WinFont> CreateGdiFont(const FontProperties& fontProperties)override
				{
					return fonts.Create(fontProperties);
				}

				void DestroyGdiFont(const FontProperties& fontProperties)override
				{
					fonts.Destroy(fontProperties);
				}

				Ptr<elements::text::CharMeasurer> CreateCharMeasurer(const FontProperties& fontProperties)override
				{
					return charMeasurers.Create(fontProperties);
				}

				void DestroyCharMeasurer(const FontProperties& fontProperties)override
				{
					charMeasurers.Destroy(fontProperties);
				}

				Ptr<windows::WinBitmap> GetBitmap(INativeImageFrame* frame, bool enabled)override
				{
					Ptr<INativeImageFrameCache> cache = frame->GetCache(this);
					if (cache)
					{
						return cache.Cast<WindowsGDIImageFrameCache>()->GetBitmap(enabled);
					}
					else
					{
						auto gdiCache = Ptr(new WindowsGDIImageFrameCache(this));
						if (frame->SetCache(this, gdiCache))
						{
							return gdiCache->GetBitmap(enabled);
						}
						else
						{
							return 0;
						}
					}
				}

				void DestroyBitmapCache(INativeImageFrame* frame)override
				{
					WindowsGDIImageFrameCache* cache = frame->GetCache(this).Cast<WindowsGDIImageFrameCache>().Obj();
					imageCaches.Remove(cache);
				}
			};
		}

		namespace elements_windows_gdi
		{
			IWindowsGDIResourceManager* windowsGDIResourceManager = nullptr;

			IWindowsGDIResourceManager* GetWindowsGDIResourceManager()
			{
				return windowsGDIResourceManager;
			}

			void SetWindowsGDIResourceManager(IWindowsGDIResourceManager* resourceManager)
			{
				windowsGDIResourceManager = resourceManager;
			}

/***********************************************************************
OS Supporting
***********************************************************************/

			IWindowsGDIObjectProvider* windowsGDIObjectProvider = nullptr;

			IWindowsGDIObjectProvider* GetWindowsGDIObjectProvider()
			{
				return windowsGDIObjectProvider;
			}

			void SetWindowsGDIObjectProvider(IWindowsGDIObjectProvider* provider)
			{
				windowsGDIObjectProvider = provider;
			}
		}
	}
}

/***********************************************************************
NativeMain
***********************************************************************/

using namespace vl::presentation;
using namespace vl::presentation::elements;

extern void GuiApplicationMain();

void RendererMainGDI(GuiHostedController* hostedController)
{
	elements_windows_gdi::WindowsGDIResourceManager resourceManager;
	elements_windows_gdi::SetWindowsGDIResourceManager(&resourceManager);
	windows::GetWindowsNativeController()->CallbackService()->InstallListener(&resourceManager);

	auto hostedResourceManager = hostedController ? new GuiHostedGraphicsResourceManager(hostedController, &resourceManager) : nullptr;
	SetGuiGraphicsResourceManager(
		hostedResourceManager
		? hostedResourceManager
		: static_cast<IGuiGraphicsResourceManager*>(&resourceManager)
		);

	{
		elements_windows_gdi::GuiFocusRectangleElementRenderer::Register();
		elements_windows_gdi::GuiSolidBorderElementRenderer::Register();
		elements_windows_gdi::Gui3DBorderElementRenderer::Register();
		elements_windows_gdi::Gui3DSplitterElementRenderer::Register();
		elements_windows_gdi::GuiSolidBackgroundElementRenderer::Register();
		elements_windows_gdi::GuiGradientBackgroundElementRenderer::Register();
		elements_windows_gdi::GuiInnerShadowElementRenderer::Register();
		elements_windows_gdi::GuiSolidLabelElementRenderer::Register();
		elements_windows_gdi::GuiImageFrameElementRenderer::Register();
		elements_windows_gdi::GuiPolygonElementRenderer::Register();
		elements_windows_gdi::GuiColorizedTextElementRenderer::Register();
		elements_windows_gdi::GuiGDIElementRenderer::Register();
		elements::GuiDocumentElement::GuiDocumentElementRenderer::Register();

		if (hostedController) hostedController->Initialize();
		GuiApplicationMain();
		if (hostedController) hostedController->Finalize();
	}

	SetGuiGraphicsResourceManager(nullptr);
	if (hostedResourceManager) delete hostedResourceManager;

	windows::GetWindowsNativeController()->CallbackService()->UninstallListener(&resourceManager);
	elements_windows_gdi::SetWindowsGDIResourceManager(nullptr);
}