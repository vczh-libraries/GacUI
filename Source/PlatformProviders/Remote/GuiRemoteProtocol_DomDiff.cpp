#include "GuiRemoteProtocol_DomDiff.h"

namespace vl::presentation::remoteprotocol
{

/***********************************************************************
GuiRemoteEventDomDiffConverter
***********************************************************************/

	GuiRemoteEventDomDiffConverter::GuiRemoteEventDomDiffConverter()
	{
	}

	GuiRemoteEventDomDiffConverter::~GuiRemoteEventDomDiffConverter()
	{
	}

	void GuiRemoteEventDomDiffConverter::OnControllerConnect()
	{
		lastDom = {};
		TBase::OnControllerConnect();
	}

/***********************************************************************
GuiRemoteProtocolDomDiffConverter
***********************************************************************/

	GuiRemoteProtocolDomDiffConverter::GuiRemoteProtocolDomDiffConverter(IGuiRemoteProtocol* _protocol)
		: TBase(_protocol)
	{
	}

	GuiRemoteProtocolDomDiffConverter::~GuiRemoteProtocolDomDiffConverter()
	{
	}

	void GuiRemoteProtocolDomDiffConverter::RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments)
	{
		renderingDomBuilder.RequestRendererBeginRendering();
		TBase::RequestRendererBeginRendering(arguments);
	}

	void GuiRemoteProtocolDomDiffConverter::RequestRendererEndRendering(vint id)
	{
		auto dom = renderingDomBuilder.RequestRendererEndRendering();
		DomIndex domIndex;
		BuildDomIndex(dom, domIndex);

		if (eventCombinator.lastDom)
		{
			RenderingDom_DiffsInOrder diffs;
			DiffDom(eventCombinator.lastDom, eventCombinator.lastDomIndex, dom, domIndex, diffs);
			targetProtocol->RequestRendererRenderDomDiff(diffs);
		}
		else
		{
			targetProtocol->RequestRendererRenderDom(dom);
		}

		eventCombinator.lastDom = dom;
		eventCombinator.lastDomIndex = std::move(domIndex);
		TBase::RequestRendererEndRendering(id);
	}

	void GuiRemoteProtocolDomDiffConverter::RequestRendererBeginBoundary(const remoteprotocol::ElementBoundary& arguments)
	{
		renderingDomBuilder.RequestRendererBeginBoundary(arguments);
	}

	void GuiRemoteProtocolDomDiffConverter::RequestRendererEndBoundary()
	{
		renderingDomBuilder.RequestRendererEndBoundary();
	}

	void GuiRemoteProtocolDomDiffConverter::RequestRendererRenderElement(const remoteprotocol::ElementRendering& arguments)
	{
		renderingDomBuilder.RequestRendererRenderElement(arguments);
	}
}