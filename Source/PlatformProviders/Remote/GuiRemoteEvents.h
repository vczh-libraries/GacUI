/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  GuiRemoteEvent

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEEVENT
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_GUIREMOTEEVENT

#include "../../Utilities/SharedServices/GuiSharedCallbackService.h"
#include "../../Utilities/SharedServices/GuiSharedAsyncService.h"
#include "GuiRemoteProtocol.h"

namespace vl::presentation
{
	class GuiRemoteController;

/***********************************************************************
GuiRemoteEvents
***********************************************************************/

	class GuiRemoteEvents : public Object, public virtual IGuiRemoteProtocolEvents
	{
		friend class GuiRemoteController;
	protected:
		GuiRemoteController*						remote;

#define MESSAGE_NOREQ_RES(NAME, RESPONSE)\
		collections::Dictionary<vint, RESPONSE>		response ## NAME;

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE) MESSAGE_NOREQ_RES(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGE_RESPONDS(MESSAGE_NOREQ_RES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_NOREQ_RES

	public:
		GuiRemoteEvents(GuiRemoteController* _remote);
		~GuiRemoteEvents();

		// =============================================================
		// IGuiRemoteProtocolEvents
		// =============================================================

		// message

#define MESSAGE_NOREQ_RES(NAME, RESPONSE)\
		void Respond ## NAME(vint id, const RESPONSE& arguments) override;\
		const RESPONSE& Retrive ## NAME(vint id);\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE) MESSAGE_NOREQ_RES(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGE_RESPONDS(MESSAGE_NOREQ_RES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_NOREQ_RES

		void	ClearResponses();

		// controlling

		void	OnConnect() override;
		void	OnDisconnect() override;
		void	OnExit() override;

		// system

		void	OnScreenUpdated(const remoteprotocol::ScreenConfig& arguments) override;
	};
}

#endif