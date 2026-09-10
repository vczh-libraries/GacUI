#ifndef VCZH_PRESENTATION_ELEMENTS_TUIGRAPHICS
#define VCZH_PRESENTATION_ELEMENTS_TUIGRAPHICS

#include "../../GraphicsElement/TuiGraphicsElement.h"
#include "TuiTextLayout.h"

namespace vl::presentation::elements
{
	class TuiGraphicsRenderTarget : public GuiGraphicsRenderTarget
	{
	protected:
		INativeWindow*							window;
		Rect									GetVisibleClipper();
		void									StartRenderingOnNativeWindow() override;
		RenderTargetFailure						StopRenderingOnNativeWindow() override;
		Size									GetCanvasSize() override;
		void									AfterPushedClipper(Rect clipper, Rect validArea, reflection::DescriptableObject* generator) override;
		void									AfterPushedClipperAndBecameInvalid(Rect clipper, reflection::DescriptableObject* generator) override;
		void									AfterPoppedClipperAndBecameValid(Rect validArea, bool clipperExists, reflection::DescriptableObject* generator) override;
		void									AfterPoppedClipper(Rect validArea, bool clipperExists, reflection::DescriptableObject* generator) override;
	public:
		TuiGraphicsRenderTarget(INativeWindow* window);
		bool									CanDraw();
		void									Fill(Rect bounds, Color color);
		void									Border(Rect bounds, Color color, TuiLineStyle style, ElementShape shape);
		void									Print(Point location, char32_t code, Color foreground, Color background, console::TuiTextStyle style);
		void									Caret(Point location, Color color);
	};

	class TuiGraphicsResourceManager : public GuiGraphicsResourceManager, public INativeControllerListener
	{
	protected:
		Ptr<TuiGraphicsRenderTarget>			renderTarget;
		TuiGraphicsLayoutProvider				layoutProvider;
	public:
		TuiGraphicsResourceManager(const TuiConfiguration& configuration = {});
		IGuiGraphicsRenderTarget*				GetRenderTarget(INativeWindow* window) override;
		void									RecreateRenderTarget(INativeWindow* window) override;
		void									ResizeRenderTarget(INativeWindow* window) override;
		IGuiGraphicsLayoutProvider*				GetLayoutProvider() override;
		Ptr<IGuiGraphicsElement>				CreateRawElement() override;
		void									NativeWindowCreated(INativeWindow* window) override;
		void									NativeWindowDestroying(INativeWindow* window) override;
	};

	extern console::TuiColor					TuiBlend(Color color, console::TuiColor background);
	extern void									RegisterTuiRenderers();
}

#endif
