#include "TuiGraphics.h"
#include "../../GraphicsElement/GuiGraphicsDocumentRenderer.h"

namespace vl::presentation::elements
{
	template<typename TElement>
	class TuiElementRenderer : public GuiElementRendererBase<TElement, TuiElementRenderer<TElement>, TuiGraphicsRenderTarget>
	{
	public:
		void InitializeInternal() {}
		void FinalizeInternal() {}
		void RenderTargetChangedInternal(TuiGraphicsRenderTarget*, TuiGraphicsRenderTarget*) {}
		void OnElementStateChanged() override {}
		void Render(Rect bounds) override
		{
			if constexpr (std::is_same_v<TElement, GuiSolidBackgroundElement>)
			{
				this->renderTarget->Fill(bounds, this->element->GetColor());
			}
			else if constexpr (std::is_same_v<TElement, GuiSolidBorderElement>)
			{
				this->renderTarget->Border(bounds, this->element->GetColor(), TuiLineStyle::Thin, this->element->GetShape());
			}
			else
			{
				this->renderTarget->Border(bounds, this->element->GetColor(), this->element->GetLineStyle(), this->element->GetShape());
			}
		}
	};

	class TuiLabelRenderer : public GuiElementRendererBase<GuiSolidLabelElement, TuiLabelRenderer, TuiGraphicsRenderTarget>
	{
	protected:
		TuiGraphicsLayoutProvider		provider;
		Ptr<IGuiGraphicsParagraph>		paragraph;
		vint							lastWidth = -1;
		Size							naturalSize;
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
			if (!element->GetMultiline())
			{
				collections::Array<wchar_t> buffer(text.Length());
				for (vint i = 0; i < text.Length(); i++) buffer[i] = text[i] == L'\r' || text[i] == L'\n' ? L' ' : text[i];
				if (buffer.Count()) text = WString::CopyFrom(&buffer[0], buffer.Count());
			}
			paragraph = provider.CreateParagraph(text, renderTarget, nullptr);
			naturalSize = paragraph->GetSize();
			if (element->GetEllipse() && !element->GetWrapLine() && lastWidth >= 0)
			{
				text = TuiEllipsizeText(text, lastWidth);
				paragraph = provider.CreateParagraph(text, renderTarget, nullptr);
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
		TuiElementRenderer<TuiBorderElement>::Register();
		TuiElementRenderer<GuiSolidBorderElement>::Register();
		TuiElementRenderer<GuiSolidBackgroundElement>::Register();
		TuiLabelRenderer::Register();
		GuiDocumentElementRenderer::Register();
	}
}
