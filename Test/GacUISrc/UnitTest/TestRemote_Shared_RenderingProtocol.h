#ifndef GACUISRC_REMOTE_RENDERINGPROTOCOL
#define GACUISRC_REMOTE_RENDERINGPROTOCOL

#include "TestRemote_Shared_NoRenderingProtocol.h"

namespace remote_protocol_tests
{
	class SingleScreenRenderingProtocol : public SingleScreenProtocol
	{
	public:
		collections::List<WString>& eventLogs;

		SingleScreenRenderingProtocol(SingleScreenConfig _globalConfig, collections::List<WString>& _eventLogs)
			: SingleScreenProtocol(_globalConfig)
			, eventLogs(_eventLogs)
		{
		}

		void RequestRendererBeginRendering(vint id) override
		{
			eventLogs.Add(WString::Unmanaged(L"BeginRendering()"));
			ElementMeasurings arguments;
			events->RespondRendererBeginRendering(id, arguments);
		}

		void RequestRendererEndRendering(vint id) override
		{
			eventLogs.Add(WString::Unmanaged(L"EndRendering()"));
			ElementMeasurings arguments;
			events->RespondRendererEndRendering(id, arguments);
		}
	};
}

#endif