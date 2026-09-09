#include "TuiGraphics.h"
#include "../../GraphicsElement/GuiGraphicsDocumentRenderer.h"
#include <algorithm>

namespace vl::presentation::elements
{
	class TuiBorderRenderer : public GuiElementRendererBase<TuiBorderElement, TuiBorderRenderer, TuiGraphicsRenderTarget>
	{
	public:
		void InitializeInternal() {}
		void FinalizeInternal() {}
		void RenderTargetChangedInternal(TuiGraphicsRenderTarget*, TuiGraphicsRenderTarget*) {}
		void OnElementStateChanged() override {}
		void Render(Rect bounds) override
		{
			renderTarget->Border(bounds, element->GetColor(), element->GetLineStyle(), element->GetShape());
		}
	};

	class TuiSolidBorderRenderer : public GuiElementRendererBase<GuiSolidBorderElement, TuiSolidBorderRenderer, TuiGraphicsRenderTarget>
	{
	public:
		void InitializeInternal() {}
		void FinalizeInternal() {}
		void RenderTargetChangedInternal(TuiGraphicsRenderTarget*, TuiGraphicsRenderTarget*) {}
		void OnElementStateChanged() override {}
		void Render(Rect bounds) override
		{
			renderTarget->Border(bounds, element->GetColor(), TuiLineStyle::Thin, element->GetShape());
		}
	};

	class TuiSolidBackgroundRenderer : public GuiElementRendererBase<GuiSolidBackgroundElement, TuiSolidBackgroundRenderer, TuiGraphicsRenderTarget>
	{
	public:
		void InitializeInternal() {}
		void FinalizeInternal() {}
		void RenderTargetChangedInternal(TuiGraphicsRenderTarget*, TuiGraphicsRenderTarget*) {}
		void OnElementStateChanged() override {}
		void Render(Rect bounds) override
		{
			renderTarget->Fill(bounds, element->GetColor());
		}
	};

	class TuiLabelRenderer : public GuiElementRendererBase<GuiSolidLabelElement, TuiLabelRenderer, TuiGraphicsRenderTarget>
	{
	protected:
		Ptr<IGuiGraphicsParagraph>		paragraph;
		WString							sourceText;
		WString							displayText;
		vint							lastWidth = -1;
		Size							naturalSize = Size(0, 1);
	public:
		void InitializeInternal()
		{
			OnElementStateChanged();
		}

		void FinalizeInternal()
		{
		}

		void RenderTargetChangedInternal(TuiGraphicsRenderTarget*, TuiGraphicsRenderTarget*)
		{
			OnElementStateChanged();
		}

		void OnElementStateChanged() override
		{
			auto text = element->GetText();
			auto provider = static_cast<TuiGraphicsLayoutProvider*>(GetGuiGraphicsResourceManager()->GetLayoutProvider());
			if (!element->GetMultiline())
			{
				collections::Array<wchar_t> buffer(text.Length());
				for (vint i = 0; i < text.Length(); i++) buffer[i] = text[i] == L'\r' || text[i] == L'\n' ? L' ' : text[i];
				if (buffer.Count()) text = WString::CopyFrom(&buffer[0], buffer.Count());
			}
			if (sourceText != text)
			{
				sourceText = text;
				TuiGraphicsLayoutProvider naturalProvider(provider->GetConfiguration());
				naturalSize = naturalProvider.CreateParagraph(sourceText, nullptr, nullptr)->GetSize();
			}
			if (element->GetEllipse() && !element->GetWrapLine() && lastWidth >= 0)
			{
				text = TuiEllipsizeText(text, lastWidth, provider->GetConfiguration().tabInterval);
			}
			if (!paragraph || displayText != text || paragraph->GetRenderTarget() != renderTarget)
			{
				displayText = text;
				paragraph = provider->CreateParagraph(text, renderTarget, nullptr);
			}
			paragraph->SetColor(0, text.Length(), element->GetColor());
			auto font = element->GetFont();
			auto style = (IGuiGraphicsParagraph::TextStyle)((font.bold ? 1 : 0) | (font.italic ? 2 : 0) | (font.underline ? 4 : 0) | (font.strikeline ? 8 : 0));
			paragraph->SetStyle(0, text.Length(), style);
			paragraph->SetWrapLine(element->GetWrapLine());
			paragraph->SetParagraphAlignment(element->GetHorizontalAlignment());
			paragraph->SetMaxWidth(lastWidth);
			UpdateMinSize();
		}

		void UpdateMinSize()
		{
			minSize = naturalSize;
			if (element->GetWrapLine())
			{
				minSize.x = 0;
				minSize.y = element->GetWrapLineHeightCalculation() ? paragraph->GetSize().y : 0;
			}
			else if (element->GetEllipse())
			{
				minSize.x = 0;
			}
		}

		void Render(Rect bounds) override
		{
			if (lastWidth != bounds.Width())
			{
				lastWidth = bounds.Width();
				OnElementStateChanged();
			}
			auto size = paragraph->GetSize();
			auto offset = std::max((vint)0, bounds.Height() - size.y);
			if (element->GetVerticalAlignment() == Alignment::Center) offset /= 2;
			else if (element->GetVerticalAlignment() != Alignment::Bottom) offset = 0;
			auto location = bounds;
			location.y1 += offset;
			renderTarget->PushClipper(bounds, element);
			paragraph->Render(location);
			renderTarget->PopClipper(element);
		}
	};

	void RegisterTuiRenderers()
	{
		TuiBorderRenderer::Register();
		TuiSolidBorderRenderer::Register();
		TuiSolidBackgroundRenderer::Register();
		TuiLabelRenderer::Register();
		GuiDocumentElementRenderer::Register();
	}
}
