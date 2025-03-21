#include "GuiRemoteRendererSingle.h"

namespace vl::presentation::remote_renderer
{
	void GuiRemoteRendererSingle::RequestRendererCreated(const Ptr<collections::List<remoteprotocol::RendererCreation>>& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererDestroyed(const Ptr<collections::List<vint>>& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererEndRendering(vint id)
	{
		CHECK_FAIL(L"Not Implemented");
	}

	void GuiRemoteRendererSingle::RequestRendererBeginBoundary(const remoteprotocol::ElementBoundary& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererEndBoundary()
	{
	}

	void GuiRemoteRendererSingle::RequestRendererRenderElement(const remoteprotocol::ElementRendering& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SolidBorder(const remoteprotocol::ElementDesc_SolidBorder& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SinkBorder(const remoteprotocol::ElementDesc_SinkBorder& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SinkSplitter(const remoteprotocol::ElementDesc_SinkSplitter& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_SolidBackground(const remoteprotocol::ElementDesc_SolidBackground& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_GradientBackground(const remoteprotocol::ElementDesc_GradientBackground& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_InnerShadow(const remoteprotocol::ElementDesc_InnerShadow& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_Polygon(const remoteprotocol::ElementDesc_Polygon& arguments)
	{
	}
	
	void GuiRemoteRendererSingle::RequestImageCreated(vint id, const remoteprotocol::ImageCreation& arguments)
	{
		CHECK_FAIL(L"Not Implemented");
	}

	void GuiRemoteRendererSingle::RequestImageDestroyed(const vint& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererUpdateElement_ImageFrame(const remoteprotocol::ElementDesc_ImageFrame& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererRenderDom(const Ptr<remoteprotocol::RenderingDom>& arguments)
	{
	}

	void GuiRemoteRendererSingle::RequestRendererRenderDomDiff(const remoteprotocol::RenderingDom_DiffsInOrder& arguments)
	{
	}
}