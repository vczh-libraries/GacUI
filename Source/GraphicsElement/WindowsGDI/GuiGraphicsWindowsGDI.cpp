#include "GuiGraphicsWindowsGDI.h"
#include "GuiGraphicsRenderersWindowsGDI.h"
#include "GuiGraphicsLayoutProviderWindowsGDI.h"
#include "..\..\Controls\GuiApplication.h"
#include "..\..\GraphicsElement\GuiGraphicsDocumentElement.h"

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

			class WindowsGDIRenderTarget : public Object, public IWindowsGDIRenderTarget
			{
			protected:
				INativeWindow*				window;
				WinDC*						dc;
				List<Rect>					clippers;
				vint							clipperCoverWholeTargetCounter;

				void ApplyClipper()
				{
					if(clipperCoverWholeTargetCounter==0)
					{
						if(clippers.Count()==0)
						{
							dc->RemoveClip();
						}
						else
						{
							Rect clipper=GetClipper();
							dc->ClipRegion(new WinRegion(clipper.Left(), clipper.Top(), clipper.Right(), clipper.Bottom(), true));
						}
					}
				}
			public:
				WindowsGDIRenderTarget(INativeWindow* _window)
					:window(_window)
					,dc(0)
					,clipperCoverWholeTargetCounter(0)
				{
				}

				WinDC* GetDC()override
				{
					return dc?dc:GetWindowsGDIObjectProvider()->GetNativeWindowDC(window);
				}

				void StartRendering()override
				{
					dc=GetWindowsGDIObjectProvider()->GetNativeWindowDC(window);
				}

				RenderTargetFailure StopRendering()override
				{
					dc = 0;
					return RenderTargetFailure::None;
				}

				void PushClipper(Rect clipper)override
				{
					if(clipperCoverWholeTargetCounter>0)
					{
						clipperCoverWholeTargetCounter++;
					}
					else
					{
						Rect previousClipper=GetClipper();
						Rect currentClipper;

						currentClipper.x1=(previousClipper.x1>clipper.x1?previousClipper.x1:clipper.x1);
						currentClipper.y1=(previousClipper.y1>clipper.y1?previousClipper.y1:clipper.y1);
						currentClipper.x2=(previousClipper.x2<clipper.x2?previousClipper.x2:clipper.x2);
						currentClipper.y2=(previousClipper.y2<clipper.y2?previousClipper.y2:clipper.y2);

						if(currentClipper.x1<currentClipper.x2 && currentClipper.y1<currentClipper.y2)
						{
							clippers.Add(currentClipper);
						}
						else
						{
							clipperCoverWholeTargetCounter++;
						}
					}
					ApplyClipper();
				}

				void PopClipper()override
				{
					if(clippers.Count()>0)
					{
						if(clipperCoverWholeTargetCounter>0)
						{
							clipperCoverWholeTargetCounter--;
						}
						else
						{
							clippers.RemoveAt(clippers.Count()-1);
						}
						ApplyClipper();
					}
				}

				Rect GetClipper()override
				{
					if (clippers.Count() == 0)
					{
						return Rect(Point(0, 0), window->Convert(window->GetClientSize()));
					}
					else
					{
						return clippers[clippers.Count() - 1];
					}
				}

				bool IsClipperCoverWholeTarget()override
				{
					return clipperCoverWholeTargetCounter>0;
				}
			};

/***********************************************************************
CachedResourceAllocator
***********************************************************************/

			class CachedPenAllocator
			{
				DEFINE_CACHED_RESOURCE_ALLOCATOR(Color, Ptr<WinPen>)
			public:
				Ptr<WinPen> CreateInternal(Color color)
				{
					return new WinPen(PS_SOLID, 1, RGB(color.r, color.g, color.b));
				}
			};

			class CachedBrushAllocator
			{
				DEFINE_CACHED_RESOURCE_ALLOCATOR(Color, Ptr<WinBrush>)
			public:
				Ptr<WinBrush> CreateInternal(Color color)
				{
					return color.a==0?new WinBrush:new WinBrush(RGB(color.r, color.g, color.b));
				}
			};

			class CachedFontAllocator
			{
				DEFINE_CACHED_RESOURCE_ALLOCATOR(FontProperties, Ptr<WinFont>)
			public:
				static Ptr<WinFont> CreateGdiFont(const FontProperties& value)
				{
					vint size=value.size<0?value.size:-value.size;
					return new WinFont(value.fontFamily, size, 0, 0, 0, (value.bold?FW_BOLD:FW_NORMAL), value.italic, value.underline, value.strikeline, value.antialias);
				}

				Ptr<WinFont> CreateInternal(const FontProperties& value)
				{
					return CreateGdiFont(value);
				}
			};

			class CachedCharMeasurerAllocator
			{
				DEFINE_CACHED_RESOURCE_ALLOCATOR(FontProperties, Ptr<text::CharMeasurer>)

			protected:
				class GdiCharMeasurer : public text::CharMeasurer
				{
				protected:
					Ptr<WinFont>			font;
					vint						size;

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
						if(renderTarget)
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
						:text::CharMeasurer(_size)
						,size(_size)
						,font(_font)
					{
					}
				};
			public:
				Ptr<text::CharMeasurer> CreateInternal(const FontProperties& value)
				{
					return new GdiCharMeasurer(CachedFontAllocator::CreateGdiFont(value), value.size);
				}
			};

/***********************************************************************
WindowsGDIResourceManager
***********************************************************************/

			class WindowsGDIImageFrameCache : public Object, public INativeImageFrameCache
			{
			protected:
				IWindowsGDIResourceManager*			resourceManager;
				INativeImageFrame*					cachedFrame;
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
					cachedFrame=frame;
					Size size=frame->GetSize();
					bitmap=new WinBitmap(size.x, size.y, WinBitmap::vbb32Bits, true);

					IWICBitmap* wicBitmap=GetWindowsGDIObjectProvider()->GetWICBitmap(frame);
					WICRect rect;
					rect.X=0;
					rect.Y=0;
					rect.Width=(INT)size.x;
					rect.Height=(INT)size.y;
					wicBitmap->CopyPixels(&rect, (UINT)bitmap->GetLineBytes(), (UINT)(bitmap->GetLineBytes()*size.y), (BYTE*)bitmap->GetScanLines()[0]);

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
					if(enabled)
					{
						return bitmap;
					}
					else
					{
						if(!disabledBitmap)
						{
							vint w=bitmap->GetWidth();
							vint h=bitmap->GetHeight();
							disabledBitmap=new WinBitmap(w, h, WinBitmap::vbb32Bits, true);
							for(vint y=0;y<h;y++)
							{
								BYTE* read=bitmap->GetScanLines()[y];
								BYTE* write=disabledBitmap->GetScanLines()[y];
								for(vint x=0;x<w;x++)
								{
									BYTE g=(read[0]+read[1]+read[2])/6+read[3]/2;
									write[0]=g;
									write[1]=g;
									write[2]=g;
									write[3]=read[3];
									read+=4;
									write+=4;
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
					layoutProvider=new WindowsGDILayoutProvider;
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
					WindowsGDIRenderTarget* renderTarget=new WindowsGDIRenderTarget(window);
					renderTargets.Add(renderTarget);
					GetWindowsGDIObjectProvider()->SetBindedRenderTarget(window, renderTarget);
				}

				void NativeWindowDestroying(INativeWindow* window)override
				{
					WindowsGDIRenderTarget* renderTarget=dynamic_cast<WindowsGDIRenderTarget*>(GetWindowsGDIObjectProvider()->GetBindedRenderTarget(window));
					GetWindowsGDIObjectProvider()->SetBindedRenderTarget(window, 0);
					renderTargets.Remove(renderTarget);
				}

				Ptr<windows::WinPen> GetFocusRectanglePen()override
				{
					if (!focusRectanglePen)
					{
						DWORD styleArray[] = { 1,1 };
						focusRectanglePen = new WinPen(PS_USERSTYLE, PS_ENDCAP_FLAT, PS_JOIN_BEVEL, 1, RGB(255, 255, 255), (DWORD)(sizeof(styleArray) / sizeof(*styleArray)), styleArray);
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
					Ptr<INativeImageFrameCache> cache=frame->GetCache(this);
					if(cache)
					{
						return cache.Cast<WindowsGDIImageFrameCache>()->GetBitmap(enabled);
					}
					else
					{
						WindowsGDIImageFrameCache* gdiCache=new WindowsGDIImageFrameCache(this);
						if(frame->SetCache(this, gdiCache))
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
					WindowsGDIImageFrameCache* cache=frame->GetCache(this).Cast<WindowsGDIImageFrameCache>().Obj();
					imageCaches.Remove(cache);
				}
			};
		}

		namespace elements_windows_gdi
		{
			IWindowsGDIResourceManager* windowsGDIResourceManager=0;

			IWindowsGDIResourceManager* GetWindowsGDIResourceManager()
			{
				return windowsGDIResourceManager;
			}

			void SetWindowsGDIResourceManager(IWindowsGDIResourceManager* resourceManager)
			{
				windowsGDIResourceManager=resourceManager;
			}

/***********************************************************************
OS Supporting
***********************************************************************/

			IWindowsGDIObjectProvider* windowsGDIObjectProvider=0;

			IWindowsGDIObjectProvider* GetWindowsGDIObjectProvider()
			{
				return windowsGDIObjectProvider;
			}

			void SetWindowsGDIObjectProvider(IWindowsGDIObjectProvider* provider)
			{
				windowsGDIObjectProvider=provider;
			}
		}
	}
}

/***********************************************************************
NativeMain
***********************************************************************/

using namespace vl::presentation;
using namespace vl::presentation::elements;

void RendererMainGDI()
{
	elements_windows_gdi::WindowsGDIResourceManager resourceManager;
	SetGuiGraphicsResourceManager(&resourceManager);
	elements_windows_gdi::SetWindowsGDIResourceManager(&resourceManager);
	GetCurrentController()->CallbackService()->InstallListener(&resourceManager);

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

	GuiApplicationMain();
	elements_windows_gdi::SetWindowsGDIResourceManager(0);
	SetGuiGraphicsResourceManager(0);
}