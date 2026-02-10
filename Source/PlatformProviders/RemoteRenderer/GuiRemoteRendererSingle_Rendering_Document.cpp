#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	using namespace collections;
	using namespace elements;
	using namespace remoteprotocol;

/***********************************************************************
* Rendering (Elements -- Document)
***********************************************************************/

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_DocumentParagraph(vint id, const remoteprotocol::ElementDesc_DocumentParagraph& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetCaret(vint id, const remoteprotocol::GetCaretRequest& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetCaretBounds(vint id, const remoteprotocol::GetCaretBoundsRequest& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetInlineObjectFromPoint(vint id, const remoteprotocol::GetInlineObjectFromPointRequest& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_GetNearestCaretFromTextPos(vint id, const remoteprotocol::GetCaretBoundsRequest& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_IsValidCaret(vint id, const remoteprotocol::IsValidCaretRequest& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_OpenCaret(const remoteprotocol::OpenCaretRequest& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}

	void GuiRemoteRendererSingle::RequestDocumentParagraph_CloseCaret(const vint& arguments)
	{
		CHECK_FAIL(L"Not implemented.");
	}
}