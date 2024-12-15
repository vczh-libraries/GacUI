/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_LOGGING
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_LOGGING

#include "GuiUnitTestProtocol_Rendering.h"
#include "../PlatformProviders/Remote/Protocol/FrameOperations/GuiRemoteProtocolSchema_FrameOperations.h"

namespace vl::presentation::unittest
{

/***********************************************************************
UnitTestRemoteProtocol
***********************************************************************/
	
	template<typename TProtocol>
	class UnitTestRemoteProtocol_Logging : public TProtocol
	{
		using CommandList = UnitTestRenderingCommandList;
		using CommandListRef = UnitTestRenderingCommandListRef;
		using LoggedFrameList = collections::List<remoteprotocol::RenderingFrame>;
	protected:

		bool								everRendered = false;
		vint								candidateFrameId = 0;
		CommandListRef						candidateRenderingResult;

		bool LogRenderingResult()
		{
			auto lastRenderingCommandsPair = this->TryGetLastRenderingCommandsAndReset();
			if (lastRenderingCommandsPair.value)
			{
				candidateFrameId = lastRenderingCommandsPair.key;
				candidateRenderingResult = lastRenderingCommandsPair.value;
				everRendered = true;
			}
			else if (everRendered)
			{
				if (candidateRenderingResult)
				{
					auto descs = Ptr(new collections::Dictionary<vint, remoteprotocol::ElementDescVariant>);
					CopyFrom(*descs.Obj(), this->lastElementDescs);
					auto transformed = BuildDomFromRenderingCommands(candidateRenderingResult);
					this->loggedTrace.frames->Add({
						candidateFrameId,
						{},
						this->sizingConfig,
						descs,
						transformed
						});
					candidateRenderingResult = {};
					return true;
				}
			}
			return false;
		}

	public:

		template<typename ...TArgs>
		UnitTestRemoteProtocol_Logging(TArgs&& ...args)
			: TProtocol(std::forward<TArgs&&>(args)...)
		{
		}

		const auto& GetLoggedTrace()
		{
			return this->loggedTrace;
		}

		const auto& GetLoggedFrames()
		{
			return this->loggedFrames;
		}
	};
}

#endif