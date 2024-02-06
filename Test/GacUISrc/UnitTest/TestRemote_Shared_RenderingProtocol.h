#ifndef GACUISRC_REMOTE_RENDERINGPROTOCOL
#define GACUISRC_REMOTE_RENDERINGPROTOCOL

#include "TestRemote_Shared_NoRenderingProtocol.h"

namespace remote_protocol_tests
{
	class SingleScreenRenderingProtocol : public SingleScreenProtocol
	{
	public:
		SingleScreenRenderingProtocol(SingleScreenConfig _globalConfig)
			: SingleScreenProtocol(_globalConfig)
		{
		}

		void RequestRendererBeginRendering(vint id) override
		{
			ElementMeasurings arguments;
			events->RespondRendererBeginRendering(id, arguments);
		}

		void RequestRendererEndRendering(vint id) override
		{
			ElementMeasurings arguments;
			events->RespondRendererEndRendering(id, arguments);
		}
	};
}

#endif