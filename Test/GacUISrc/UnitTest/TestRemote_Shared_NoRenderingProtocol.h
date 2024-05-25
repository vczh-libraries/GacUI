#ifndef GACUISRC_REMOTE_NORENDERINGPROTOCOL
#define GACUISRC_REMOTE_NORENDERINGPROTOCOL

#include "../../../Source/UnitTestUtilities/GuiUnitTestUtilities.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;

namespace remote_protocol_tests
{
	using namespace vl::presentation::unittest;

	class NotImplementedProtocolBase : public UnitTestRemoteProtocolBase
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
	
		NotImplementedProtocolBase(UnitTestScreenConfig _globalConfig)
			: UnitTestRemoteProtocolBase(_globalConfig)
		{
		}
		
		void Submit() override
		{
		}
		
		void ProcessRemoteEvents() override
		{
			CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!");
		}
	};

	using SingleScreenProtocolFeatures = Mixin<
		NotImplementedProtocolBase,
		UnitTestRemoteProtocol_MainWindow,
		UnitTestRemoteProtocol_IO
	>::Type;
	
	class SingleScreenProtocol : public SingleScreenProtocolFeatures
	{
	public:
		List<Func<void()>>			processRemoteEvents;
		vint						nextEventIndex = 0;

		SingleScreenProtocol(UnitTestScreenConfig _globalConfig)
			: SingleScreenProtocolFeatures(_globalConfig)
		{
		}
	
		template<typename TCallback>
		void OnNextFrame(TCallback&& callback)
		{
			processRemoteEvents.Add(std::move(callback));
		}
	
		void ProcessRemoteEvents() override
		{
			TEST_CASE(L"Execute frame[" + itow(nextEventIndex) + L"]")
			{
				processRemoteEvents[nextEventIndex]();
			});
			nextEventIndex++;
		}

		void RequestControllerConnectionEstablished() override
		{
		}
	
		void RequestControllerConnectionStopped() override
		{
		}

		void RequestRendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments) override
		{
		}

		void RequestRendererEndRendering(vint id) override
		{
			ElementMeasurings arguments;
			GetEvents()->RespondRendererEndRendering(id, arguments);
		}
	};
}

#endif