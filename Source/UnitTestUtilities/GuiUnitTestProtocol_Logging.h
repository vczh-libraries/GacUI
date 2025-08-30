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
	protected:
		bool								everRendered = false;
		Ptr<UnitTestLoggedFrame>			candidateFrame;
		vint								idlingCounter = 0;

		bool LogRenderingResult()
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol_Logging<TProtocol>::ProcessRemoteEvents()#"
			if (auto lastFrame = this->TryGetLastRenderingFrameAndReset())
			{
				candidateFrame = lastFrame;
				everRendered = true;
			}
			else if (everRendered)
			{
				if (candidateFrame)
				{
					idlingCounter = 0;
					auto descs = Ptr(new collections::Dictionary<vint, ElementDescVariant>);
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
				else
				{
					idlingCounter++;
					if (idlingCounter == 100)
					{
						CHECK_ERROR(
							idlingCounter < 100,
							ERROR_MESSAGE_PREFIX L"The last frame didn't trigger UI updating. The action registered by OnNextIdleFrame should always make any element or layout to change."
							);
					}
				}
			}
			return false;
#undef ERROR_MESSAGE_PREFIX
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