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
#define MESSAGE_NOREQ_NORES(NAME)					void Request ## NAME()									override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_NOREQ_RES(NAME, RESPONSE)			void Request ## NAME(vint id)							override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_REQ_NORES(NAME, REQUEST)			void Request ## NAME(const REQUEST& arguments)			override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)	void Request ## NAME(vint id, const REQUEST& arguments)	override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
	GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_NOREQ_NORES, MESSAGE_NOREQ_RES, MESSAGE_REQ_NORES, MESSAGE_REQ_RES)
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES
};