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

	class SingleScreenProtocolFeatures :
		public UnitTestRemoteProtocol_MainWindow,
		public UnitTestRemoteProtocol_IO
	{
	public:
		SingleScreenProtocolFeatures(const UnitTestScreenConfig& _globalConfig)
			: UnitTestRemoteProtocol_MainWindow(_globalConfig)
			, UnitTestRemoteProtocol_IO(_globalConfig)
		{
		}

		void Impl_ControllerConnectionEstablished()
		{
		}

		void Impl_ControllerConnectionStopped()
		{
		}

		void Impl_RendererBeginRendering(const remoteprotocol::ElementBeginRendering& arguments)
		{
		}

		void Impl_RendererEndRendering(vint id)
		{
			ElementMeasurings arguments;
			GetEvents()->RespondRendererEndRendering(id, arguments);
		}
	};
	
	class SingleScreenProtocol 
		: public SingleScreenProtocolFeatures
		, protected virtual IGuiRemoteEventProcessor
	{
		
	public:
		List<Func<void()>>			processRemoteEvents;
		vint						nextEventIndex = 0;

		SingleScreenProtocol(const UnitTestScreenConfig& _globalConfig)
			: UnitTestRemoteProtocolBase(_globalConfig)
			, SingleScreenProtocolFeatures(_globalConfig)
		{
		}
	
		template<typename TCallback>
		void OnNextFrame(TCallback&& callback)
		{
			processRemoteEvents.Add(std::move(callback));
		}

/***********************************************************************
IGuiRemoteProtocolMessages
***********************************************************************/

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					void Request ## NAME() override { SingleScreenProtocolFeatures::Impl_ ## NAME(); }
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id) override { SingleScreenProtocolFeatures::Impl_ ## NAME(id); }
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						void Request ## NAME(const REQUEST& arguments) override { SingleScreenProtocolFeatures::Impl_ ## NAME(arguments); }
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id, const REQUEST& arguments) override { SingleScreenProtocolFeatures::Impl_ ## NAME(id, arguments); }
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

		void Submit(bool& disconnected) override
		{
		}

		void ProcessRemoteEvents() override
		{
			TEST_CASE(L"Execute frame[" + itow(nextEventIndex) + L"]")
			{
				processRemoteEvents[nextEventIndex]();
			});
			nextEventIndex++;
		}
		
		IGuiRemoteEventProcessor* GetRemoteEventProcessor() override
		{
			return this;
		}
	};
}

#endif