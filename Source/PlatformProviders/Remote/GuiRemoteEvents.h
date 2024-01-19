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
		vint										id = 0;

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

		// messages

#define MESSAGE_NOREQ_NORES(NAME)					void Request ## NAME();
#define MESSAGE_NOREQ_RES(NAME, RESPONSE)			vint Request ## NAME();
#define MESSAGE_REQ_NORES(NAME, REQUEST)			void Request ## NAME(const REQUEST& arguments);
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)	vint Request ## NAME(const REQUEST& arguments);
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_NOREQ_NORES, MESSAGE_NOREQ_RES, MESSAGE_REQ_NORES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

#define MESSAGE_NOREQ_RES(NAME, RESPONSE)\
		void Respond ## NAME(vint id, const RESPONSE& arguments) override;\
		const RESPONSE& Retrieve ## NAME(vint id);\

#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE) MESSAGE_NOREQ_RES(NAME, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGE_RESPONDS(MESSAGE_NOREQ_RES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_NOREQ_RES

		void	ClearResponses();

		// events

		void	OnControllerConnect() override;
		void	OnControllerDisconnect() override;
		void	OnControllerExit() override;
		void	OnControllerScreenUpdated(const remoteprotocol::ScreenConfig& arguments) override;

		void	OnWindowBoundsUpdated(const remoteprotocol::WindowSizingConfig& arguments) override;
	};
}

#endif