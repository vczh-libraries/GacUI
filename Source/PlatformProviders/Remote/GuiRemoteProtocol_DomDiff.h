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
		GuiRemoteEventDomDiffConverter();
		~GuiRemoteEventDomDiffConverter();

		void							OnControllerConnect() override;
	};

/***********************************************************************
GuiRemoteProtocolDomDiffConverter
***********************************************************************/
	
	class GuiRemoteProtocolDomDiffConverter : public GuiRemoteProtocolCombinator_PassingThrough<GuiRemoteEventDomDiffConverter>
	{
		using TBase = GuiRemoteProtocolCombinator_PassingThrough<GuiRemoteEventDomDiffConverter>;
	protected:
		RenderingDomBuilder				renderingDomBuilder;

	public:
		GuiRemoteProtocolDomDiffConverter(IGuiRemoteProtocol* _protocol);
		~GuiRemoteProtocolDomDiffConverter();

		void							RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments) override;
		void							RequestRendererEndRendering(vint id) override;
		void							RequestRendererBeginBoundary(const remoteprotocol::ElementBoundary& arguments) override;
		void							RequestRendererEndBoundary() override;
		void							RequestRendererRenderElement(const remoteprotocol::ElementRendering& arguments) override;
	};
}

#endif