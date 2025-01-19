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

namespace vl::presentation::remoteprotocol
{

/***********************************************************************
GuiRemoteEventDomDiffConverter
***********************************************************************/

	class GuiRemoteEventDomDiffConverter : public GuiRemoteEventCombinator_PassingThrough
	{
	public:
		GuiRemoteEventDomDiffConverter()
		{
		}
	};

/***********************************************************************
GuiRemoteProtocolDomDiffConverter
***********************************************************************/
	
	class GuiRemoteProtocolDomDiffConverter : public GuiRemoteProtocolCombinator_PassingThrough<GuiRemoteEventDomDiffConverter>
	{
	public:
		GuiRemoteProtocolDomDiffConverter(IGuiRemoteProtocol* _protocol)
			: GuiRemoteProtocolCombinator_PassingThrough<GuiRemoteEventDomDiffConverter>(_protocol)
		{
		}
	};
}

#endif