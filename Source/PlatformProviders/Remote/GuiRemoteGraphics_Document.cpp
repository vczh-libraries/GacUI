#include "GuiRemoteGraphics_Document.h"
#include "GuiRemoteGraphics.h"

namespace vl::presentation::elements
{

/***********************************************************************
GuiRemoteGraphicsParagraph
***********************************************************************/

	GuiRemoteGraphicsParagraph::GuiRemoteGraphicsParagraph(const WString& _text, GuiRemoteController* _remote, GuiRemoteGraphicsResourceManager* _resourceManager, GuiRemoteGraphicsRenderTarget* _renderTarget, IGuiGraphicsParagraphCallback* _callback)
		: text(_text)
		, remote(_remote)
		, resourceManager(_resourceManager)
		, renderTarget(_renderTarget)
		, callback(_callback)
	{
	}

	GuiRemoteGraphicsParagraph::~GuiRemoteGraphicsParagraph()
	{
	}

	IGuiGraphicsLayoutProvider* GuiRemoteGraphicsParagraph::GetProvider()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	IGuiGraphicsRenderTarget* GuiRemoteGraphicsParagraph::GetRenderTarget()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::GetWrapLine()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteGraphicsParagraph::SetWrapLine(bool value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	vint GuiRemoteGraphicsParagraph::GetMaxWidth()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteGraphicsParagraph::SetMaxWidth(vint value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	Alignment GuiRemoteGraphicsParagraph::GetParagraphAlignment()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteGraphicsParagraph::SetParagraphAlignment(Alignment value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetFont(vint start, vint length, const WString& value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetSize(vint start, vint length, vint value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetStyle(vint start, vint length, TextStyle value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetColor(vint start, vint length, Color value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetBackgroundColor(vint start, vint length, Color value)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::SetInlineObject(vint start, vint length, const InlineObjectProperties& properties)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::ResetInlineObject(vint start, vint length)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	Size GuiRemoteGraphicsParagraph::GetSize()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::OpenCaret(vint caret, Color color, bool frontSide)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::CloseCaret()
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	void GuiRemoteGraphicsParagraph::Render(Rect bounds)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	vint GuiRemoteGraphicsParagraph::GetCaret(vint comparingCaret, CaretRelativePosition position, bool& preferFrontSide)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	Rect GuiRemoteGraphicsParagraph::GetCaretBounds(vint caret, bool frontSide)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	vint GuiRemoteGraphicsParagraph::GetCaretFromPoint(Point point)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	Nullable<IGuiGraphicsParagraph::InlineObjectProperties> GuiRemoteGraphicsParagraph::GetInlineObjectFromPoint(Point point, vint& start, vint& length)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	vint GuiRemoteGraphicsParagraph::GetNearestCaretFromTextPos(vint textPos, bool frontSide)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::IsValidCaret(vint caret)
	{
		CHECK_FAIL(L"Not Implemented!");
	}

	bool GuiRemoteGraphicsParagraph::IsValidTextPos(vint textPos)
	{
		CHECK_FAIL(L"Not Implemented!");
	}
}