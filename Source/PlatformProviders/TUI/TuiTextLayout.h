#ifndef VCZH_PRESENTATION_ELEMENTS_TUITEXTLAYOUT
#define VCZH_PRESENTATION_ELEMENTS_TUITEXTLAYOUT

#include "../../GraphicsElement/GuiGraphicsResourceManager.h"

namespace vl::presentation::elements
{
	class TuiGraphicsRenderTarget;

	struct TuiTextCell
	{
		vint													start = 0;
		vint													length = 0;
		char32_t												code = 0;
		Rect													bounds;
		vint													line = 0;
		Nullable<IGuiGraphicsParagraph::InlineObjectProperties>	inlineObject;
	};

	struct TuiTextLine
	{
		vint													firstCell = 0;
		vint													lastCell = 0;
		vint													start = 0;
		vint													end = 0;
		Rect													bounds;
	};

	class TuiGraphicsParagraph : public Object, public IGuiGraphicsParagraph
	{
	protected:
		IGuiGraphicsLayoutProvider*								provider;
		TuiGraphicsRenderTarget*								renderTarget;
		IGuiGraphicsParagraphCallback*							callback;
		WString													text;
		collections::Array<TextStyle>							styles;
		collections::Array<Color>								colors;
		collections::Array<Color>								backgrounds;
		collections::Dictionary<vint, collections::Pair<vint, InlineObjectProperties>> inlineObjects;
		collections::List<TuiTextCell>							cells;
		collections::List<TuiTextLine>							lines;
		collections::Dictionary<vint, vint>						caretToCell;
		bool													dirty = true;
		bool													wrapLine = false;
		vint													maxWidth = -1;
		Alignment												alignment = Alignment::Left;
		Size													size;
		vint													caretPosition = -1;
		Color													caretColor;
		bool													caretFrontSide = true;
		bool													caretVisible = false;

		bool													ValidRange(vint start, vint length);
		void													EnsureLayout();
		vint													FindLine(vint caret, bool frontSide);
	public:
		TuiGraphicsParagraph(const WString& text, IGuiGraphicsLayoutProvider* provider, TuiGraphicsRenderTarget* renderTarget, IGuiGraphicsParagraphCallback* callback);
		IGuiGraphicsLayoutProvider*								GetProvider() override;
		IGuiGraphicsRenderTarget*								GetRenderTarget() override;
		bool													GetWrapLine() override;
		void													SetWrapLine(bool value) override;
		vint													GetMaxWidth() override;
		void													SetMaxWidth(vint value) override;
		Alignment												GetParagraphAlignment() override;
		void													SetParagraphAlignment(Alignment value) override;
		bool													SetFont(vint start, vint length, const WString& value) override;
		bool													SetSize(vint start, vint length, vint value) override;
		bool													SetStyle(vint start, vint length, TextStyle value) override;
		bool													SetColor(vint start, vint length, Color value) override;
		bool													SetBackgroundColor(vint start, vint length, Color value) override;
		Size													GetSize() override;
		bool													EnableCaret(vint caret, Color color, bool frontSide) override;
		void													DisableCaret() override;
		bool													BlinkCaret() override;
		bool													IsValidCaret(vint caret) override;
		bool													IsValidTextPos(vint textPos) override;
		bool													SetInlineObject(vint start, vint length, const InlineObjectProperties& properties) override;
		bool													ResetInlineObject(vint start, vint length) override;
		void													Render(Rect bounds) override;
		vint													GetCaret(vint comparingCaret, CaretRelativePosition position, bool& preferFrontSide) override;
		Rect													GetCaretBounds(vint caret, bool frontSide) override;
		vint													GetCaretFromPoint(Point point) override;
		Nullable<InlineObjectProperties>						GetInlineObjectFromPoint(Point point, vint& start, vint& length) override;
		vint													GetNearestCaretFromTextPos(vint textPos, bool frontSide) override;
	};

	class TuiGraphicsLayoutProvider : public Object, public IGuiGraphicsLayoutProvider
	{
	public:
		Ptr<IGuiGraphicsParagraph>								CreateParagraph(const WString& text, IGuiGraphicsRenderTarget* renderTarget, IGuiGraphicsParagraphCallback* callback) override;
	};

	extern char32_t						TuiReadScalar(const WString& text, vint start, vint& length);
	extern WString						TuiEllipsizeText(const WString& text, vint width);
	extern console::TuiTextStyle			TuiGetTextStyle(IGuiGraphicsParagraph::TextStyle style);
}

#endif
