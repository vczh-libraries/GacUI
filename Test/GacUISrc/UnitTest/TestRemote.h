#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteController.h"

extern void SetGuiMainProxy(void(*proxy)());

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;

class NotImplementedProtocolBase : public Object, public virtual IGuiRemoteProtocol
{
public:
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					void Request ## NAME()									override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id)							override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						void Request ## NAME(const REQUEST& arguments)			override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id, const REQUEST& arguments)	override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES


	void Submit()				override {}
	void ProcessRemoteEvents()	override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
};