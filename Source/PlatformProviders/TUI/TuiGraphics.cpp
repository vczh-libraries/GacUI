#include "TuiGraphics.h"

namespace vl::presentation::elements
{
	using namespace console;

	TuiColor TuiBlend(Color color, TuiColor background)
	{
		return {
			(vuint8_t)((color.r * color.a + background.r * (255 - color.a)) / 255),
			(vuint8_t)((color.g * color.a + background.g * (255 - color.a)) / 255),
			(vuint8_t)((color.b * color.a + background.b * (255 - color.a)) / 255)
		};
	}

/***********************************************************************
TuiGraphicsRenderTarget
***********************************************************************/

	TuiGraphicsRenderTarget::TuiGraphicsRenderTarget(INativeWindow* value)
		: window(value)
	{
	}

	bool TuiGraphicsRenderTarget::CanDraw()
	{
		return TUI::IsInUse() && !TUI::IsStopRequested() && window->IsVisible();
	}

	Rect TuiGraphicsRenderTarget::GetVisibleClipper()
	{
		return GetClipper().Intersect(Rect(Point(), GetCanvasSize()));
	}

	void TuiGraphicsRenderTarget::StartRenderingOnNativeWindow()
	{
		if (CanDraw()) TUI::Clear({}, 0, 0, TUI::GetBufferWidth() - 1, TUI::GetBufferHeight() - 1);
	}

	RenderTargetFailure TuiGraphicsRenderTarget::StopRenderingOnNativeWindow()
	{
		if (CanDraw()) TUI::RenderBuffer();
		return RenderTargetFailure::None;
	}

	Size TuiGraphicsRenderTarget::GetCanvasSize()
	{
		return {TUI::GetBufferWidth(), TUI::GetBufferHeight()};
	}

	void TuiGraphicsRenderTarget::AfterPushedClipper(Rect clipper, Rect validArea, reflection::DescriptableObject* generator)
	{
	}

	void TuiGraphicsRenderTarget::AfterPushedClipperAndBecameInvalid(Rect clipper, reflection::DescriptableObject* generator)
	{
	}

	void TuiGraphicsRenderTarget::AfterPoppedClipperAndBecameValid(Rect validArea, bool clipperExists, reflection::DescriptableObject* generator)
	{
	}

	void TuiGraphicsRenderTarget::AfterPoppedClipper(Rect validArea, bool clipperExists, reflection::DescriptableObject* generator)
	{
	}

	void TuiGraphicsRenderTarget::Fill(Rect bounds, Color color)
	{
		if (!CanDraw() || color.a == 0) return;
		auto area = bounds.Intersect(GetVisibleClipper());
		for (vint y = area.y1; y < area.y2; y++)
		{
			for (vint x = area.x1; x < area.x2; x++)
			{
				auto background = TUI::GetBuffer()[y * TUI::GetBufferWidth() + x].backgroundColor;
				TUI::Clear(TuiBlend(color, background), x, y, x, y);
			}
		}
	}

	void TuiGraphicsRenderTarget::Border(Rect bounds, Color color, TuiLineStyle style, ElementShape shape)
	{
		if (!CanDraw() || color.a == 0 || bounds.Width() <= 0 || bounds.Height() <= 0) return;
		if (bounds.Width() == 1 && bounds.Height() == 1) return;
		auto area = bounds.Intersect(GetVisibleClipper());
		if (area.Width() <= 0 || area.Height() <= 0) return;
		auto width = TUI::GetBufferWidth();
		auto height = TUI::GetBufferHeight();
		borderBuffer.Resize(width * height);
		for (vint i = 0; i < borderBuffer.Count(); i++) borderBuffer[i] = TUI::GetBuffer()[i];
		auto glyph = style == TuiLineStyle::Thin ? TuiMergeableGlyph::ThinLine
			: style == TuiLineStyle::Thick ? TuiMergeableGlyph::ThickLine : TuiMergeableGlyph::DoubleLine;
		TuiLineOptions line{glyph, {color.r, color.g, color.b}};
		if (bounds.Width() == 1)
		{
			TUI::DrawLineV(&borderBuffer[0], width, height, line, bounds.x1, bounds.y1, bounds.y2 - 1);
		}
		else if (bounds.Height() == 1)
		{
			TUI::DrawLineH(&borderBuffer[0], width, height, line, bounds.x1, bounds.x2 - 1, bounds.y1);
		}
		else
		{
			TuiRectOptions rectangle{glyph, line.foregroundColor};
			if (style == TuiLineStyle::Thin && shape.shapeType != ElementShapeType::Rectangle && bounds.Width() > 2 && bounds.Height() > 2)
			{
				rectangle.corner = TuiRectCorner::Round;
			}
			TUI::DrawRect(&borderBuffer[0], width, height, rectangle, bounds.x1, bounds.y1, bounds.x2 - 1, bounds.y2 - 1);
		}
		for (vint y = area.y1; y < area.y2; y++)
		{
			for (vint x = area.x1; x < area.x2; x++)
			{
				if (x != bounds.x1 && x != bounds.x2 - 1 && y != bounds.y1 && y != bounds.y2 - 1) continue;
				auto index = y * width + x;
				auto pixel = borderBuffer[index];
				pixel.foregroundColor = TuiBlend(color, TUI::GetBuffer()[index].foregroundColor);
				TUI::Clear(pixel.backgroundColor, x, y, x, y);
				TUI::GetBuffer()[index] = pixel;
			}
		}
	}

	void TuiGraphicsRenderTarget::Print(Point location, char32_t code, Color foreground, Color background, TuiTextStyle style)
	{
		if (!CanDraw()) return;
		auto width = TUI::MeasureChar(code);
		auto clipper = GetVisibleClipper();
		if (width == 0 || !clipper.Contains(location) || location.x + width > clipper.x2) return;
		auto pixel = TUI::GetBuffer()[location.y * TUI::GetBufferWidth() + location.x];
		TuiPrintOptions options;
		options.foregroundColor = TuiBlend(foreground, pixel.foregroundColor);
		options.backgroundColor = TuiBlend(background, pixel.backgroundColor);
		options.style = style;
		if (foreground.a != 0)
		{
			TUI::PrintChar(options, code, location.x, location.y);
		}
		else if (background.a != 0)
		{
			Fill(Rect(location, Size(width, 1)), background);
		}
	}

	void TuiGraphicsRenderTarget::Caret(Point location, Color color)
	{
		if (!CanDraw() || !GetVisibleClipper().Contains(location)) return;
		auto index = location.y * TUI::GetBufferWidth() + location.x;
		auto pixel = TUI::GetBuffer()[index];
		if (pixel.glyph == TuiPixelGlyph::WideCharContinuation)
		{
			location.x--;
			pixel = TUI::GetBuffer()[--index];
		}
		auto code = pixel.GetChar32();
		Print(location, code ? code : U' ', Color(pixel.backgroundColor.r, pixel.backgroundColor.g, pixel.backgroundColor.b), color,
			pixel.glyph == TuiPixelGlyph::Char ? pixel.character.style : TuiTextStyle());
	}

/***********************************************************************
TuiGraphicsResourceManager
***********************************************************************/

	IGuiGraphicsRenderTarget* TuiGraphicsResourceManager::GetRenderTarget(INativeWindow* window)
	{
		return renderTarget.Obj();
	}

	void TuiGraphicsResourceManager::RecreateRenderTarget(INativeWindow* window)
	{
	}

	void TuiGraphicsResourceManager::ResizeRenderTarget(INativeWindow* window)
	{
	}

	IGuiGraphicsLayoutProvider* TuiGraphicsResourceManager::GetLayoutProvider()
	{
		return &layoutProvider;
	}

	Ptr<IGuiGraphicsElement> TuiGraphicsResourceManager::CreateRawElement()
	{
		CHECK_FAIL(L"Raw graphics are not supported by the terminal renderer.");
	}

	void TuiGraphicsResourceManager::NativeWindowCreated(INativeWindow* window)
	{
		renderTarget = Ptr(new TuiGraphicsRenderTarget(window));
	}

	void TuiGraphicsResourceManager::NativeWindowDestroying(INativeWindow* window)
	{
		renderTarget = nullptr;
	}
}
