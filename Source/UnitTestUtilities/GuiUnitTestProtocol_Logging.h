/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_LOGGING
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_LOGGING

#include "GuiUnitTestProtocol_Rendering.h"

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
		Ptr<UnitTestLoggedFrame>			candidateFrame;

		bool LogRenderingResult()
		{
			if (auto lastFrame = this->TryGetLastRenderingFrameAndReset())
			{
				candidateFrame = lastFrame;
				everRendered = true;
			}
			else if (everRendered)
			{
				if (candidateFrame)
				{
					auto descs = Ptr(new collections::Dictionary<vint, remoteprotocol::ElementDescVariant>);
					CopyFrom(*descs.Obj(), this->lastElementDescs);
					this->loggedTrace.frames->Add({
						candidateFrame->frameId,
						{},
						this->sizingConfig,
						descs,
						candidateFrame->renderingDom
						});
					candidateFrame = {};
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