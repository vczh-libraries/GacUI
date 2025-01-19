/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocol

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_DOMDIFF
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEPROTOCOL_DOMDIFF

#include "GuiRemoteProtocol_Shared.h"
#include "Protocol/FrameOperations/GuiRemoteProtocolSchema_FrameOperations.h"

namespace vl::presentation::remoteprotocol
{

/***********************************************************************
GuiRemoteEventDomDiffConverter
***********************************************************************/

	class GuiRemoteProtocolDomDiffConverter;

	class GuiRemoteEventDomDiffConverter : public GuiRemoteEventCombinator_PassingThrough
	{
		friend class GuiRemoteProtocolDomDiffConverter;
		using TBase = GuiRemoteEventCombinator_PassingThrough;
	protected:
		Ptr<RenderingDom>				lastDom;
		DomIndex						lastDomIndex;

	public:
		GuiRemoteEventDomDiffConverter()
		{
		}

		void OnControllerConnect() override
		{
			lastDom = {};
			TBase::OnControllerConnect();
		}
	};

/***********************************************************************
GuiRemoteProtocolDomDiffConverter
***********************************************************************/
	
	class GuiRemoteProtocolDomDiffConverter : public GuiRemoteProtocolCombinator_PassingThrough<GuiRemoteEventDomDiffConverter>
	{
	protected:
		RenderingDomBuilder				renderingDomBuilder;

	public:
		GuiRemoteProtocolDomDiffConverter(IGuiRemoteProtocol* _protocol)
			: GuiRemoteProtocolCombinator_PassingThrough<GuiRemoteEventDomDiffConverter>(_protocol)
		{
		}

		void RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments) override
		{
			renderingDomBuilder.RequestRendererBeginRendering();
		}

		void RequestRendererEndRendering(vint id) override
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
		}

		void RequestRendererBeginBoundary(const remoteprotocol::ElementBoundary& arguments) override
		{
			renderingDomBuilder.RequestRendererBeginBoundary(arguments);
		}

		void RequestRendererEndBoundary() override
		{
			renderingDomBuilder.RequestRendererEndBoundary();
		}

		void RequestRendererRenderElement(const remoteprotocol::ElementRendering& arguments) override
		{
			renderingDomBuilder.RequestRendererRenderElement(arguments);
		}
	};
}

#endif