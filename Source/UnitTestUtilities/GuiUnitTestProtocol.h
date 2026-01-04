/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL

#include "GuiUnitTestProtocol_IO.h"
#include "GuiUnitTestProtocol_MainWindow.h"
#include "GuiUnitTestProtocol_Rendering.h"
#include "GuiUnitTestProtocol_IOCommands.h"

namespace vl::presentation::unittest
{

/***********************************************************************
UnitTestFrameworkConfig
***********************************************************************/

	struct UnitTestFrameworkConfig
	{
		filesystem::FilePath					snapshotFolder;
		filesystem::FilePath					resourceFolder;
	};

	extern const UnitTestFrameworkConfig&		GetUnitTestFrameworkConfig();

/***********************************************************************
UnitTestRemoteProtocol
***********************************************************************/

	class UnitTestRemoteProtocolFeatures :
		public UnitTestRemoteProtocol_MainWindow,
		public UnitTestRemoteProtocol_IO,
		public UnitTestRemoteProtocol_Rendering,
		public UnitTestRemoteProtocol_IOCommands
	{
	protected:
		bool								stopped = false;
		bool								everRendered = false;
		Ptr<UnitTestLoggedFrame>			candidateFrame;
		vint								idlingCounter = 0;

		bool LogRenderingResult()
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocolFeatures::LogRenderingResult()#"
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
		UnitTestRemoteProtocolFeatures(const UnitTestScreenConfig& _globalConfig)
			: UnitTestRemoteProtocol_MainWindow(_globalConfig)
			, UnitTestRemoteProtocol_IO(_globalConfig)
			, UnitTestRemoteProtocol_Rendering(_globalConfig)
			, UnitTestRemoteProtocol_IOCommands(_globalConfig)
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

/***********************************************************************
IGuiRemoteProtocolMessages (Initialization)
***********************************************************************/

	protected:

		void Impl_ControllerConnectionEstablished()
		{
			ResetCreatedObjects();
		}

		void Impl_ControllerConnectionStopped()
		{
			stopped = true;
		}
	};

	class UnitTestRemoteProtocol 
		: public UnitTestRemoteProtocolFeatures
		, protected virtual IGuiRemoteEventProcessor
	{
		using EventPair = collections::Pair<Nullable<WString>, Func<void()>>;
	protected:
		const UnitTestFrameworkConfig&		frameworkConfig;
		WString								appName;
		collections::List<EventPair>		processRemoteEvents;
		vint								lastFrameIndex = -1;
		vint								nextEventIndex = 0;
		bool								frameExecuting = false;

	public:

		UnitTestRemoteProtocol(const WString& _appName, UnitTestScreenConfig _globalConfig)
			: UnitTestRemoteProtocolBase(_globalConfig)
			, UnitTestRemoteProtocolFeatures(_globalConfig)
			, frameworkConfig(GetUnitTestFrameworkConfig())
			, appName(_appName)
		{
		}

		template<typename TCallback>
		void OnNextIdleFrame(TCallback&& callback)
		{
			processRemoteEvents.Add({ Nullable<WString>{},std::forward<TCallback&&>(callback) });
		}

		template<typename TCallback>
		void OnNextIdleFrame(const WString& name, TCallback&& callback)
		{
			processRemoteEvents.Add({ name,std::forward<TCallback&&>(callback) });
		}

/***********************************************************************
IGuiRemoteProtocolMessages
***********************************************************************/

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					void Request ## NAME() override { UnitTestRemoteProtocolFeatures::Impl_ ## NAME(); }
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id) override { UnitTestRemoteProtocolFeatures::Impl_ ## NAME(id); }
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						void Request ## NAME(const REQUEST& arguments) override { UnitTestRemoteProtocolFeatures::Impl_ ## NAME(arguments); }
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id, const REQUEST& arguments) override { UnitTestRemoteProtocolFeatures::Impl_ ## NAME(id, arguments); }
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES

/***********************************************************************
IGuiRemoteProtocol
***********************************************************************/

		void Submit(bool& disconnected) override
		{
			// TODO: Failure injection to disconnected
		}

		void ProcessRemoteEvents() override
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::unittest::UnitTestRemoteProtocol::ProcessRemoteEvents()#"
			if (!stopped)
			{
				if (LogRenderingResult())
				{
					auto [name, func] = processRemoteEvents[nextEventIndex];
					CHECK_ERROR(!frameExecuting, ERROR_MESSAGE_PREFIX L"The action registered by OnNextIdleFrame should not call any blocking function, consider using InvokeInMainThread.");
					vl::unittest::UnitTest::PrintMessage(L"Execute idle frame[" + (name ? name.Value() : itow(nextEventIndex)) + L"]", vl::unittest::UnitTest::MessageKind::Info);
					CHECK_ERROR(lastFrameIndex != loggedTrace.frames->Count() - 1, ERROR_MESSAGE_PREFIX L"No rendering occured after the last idle frame.");
					lastFrameIndex = loggedTrace.frames->Count() - 1;

					if (name)
					{
						auto&& lastFrame = (*loggedTrace.frames.Obj())[loggedTrace.frames->Count() - 1];
						lastFrame.frameName = name;
					}
					frameExecuting = true;
					func();
					frameExecuting = false;
					nextEventIndex++;
				}
			}
#undef ERROR_MESSAGE_PREFIX
		}

		IGuiRemoteEventProcessor* GetRemoteEventProcessor() override
		{
			return this;
		}
	};
}

#endif