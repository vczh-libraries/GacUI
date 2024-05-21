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
#include "GuiUnitTestProtocol_Logging.h"

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

	template<typename TBase, template<typename> class ...TMixins>
	struct Mixin;

	template<typename TBase>
	struct Mixin<TBase>
	{
		using Type = TBase;
	};

	template<typename TBase, template<typename> class TMixin, template<typename> class ...TOtherMixins>
	struct Mixin<TBase, TMixin, TOtherMixins...>
	{
		using Type = typename Mixin<TMixin<TBase>, TOtherMixins...>::Type;
	};

	using UnitTestRemoteProtocolFeatures = Mixin<
		UnitTestRemoteProtocolBase,
		UnitTestRemoteProtocol_MainWindow,
		UnitTestRemoteProtocol_IO,
		UnitTestRemoteProtocol_Rendering,
		UnitTestRemoteProtocol_Logging
	>::Type;

	class UnitTestRemoteProtocol : public UnitTestRemoteProtocolFeatures
	{
	protected:
		const UnitTestFrameworkConfig&		frameworkConfig;
		WString								appName;
		collections::List<Func<void()>>		processRemoteEvents;
		vint								nextEventIndex = 0;
		bool								stopped = false;


	public:

		UnitTestRemoteProtocol(const WString& _appName, UnitTestScreenConfig _globalConfig)
			: UnitTestRemoteProtocolFeatures(_globalConfig)
			, frameworkConfig(GetUnitTestFrameworkConfig())
			, appName(_appName)
		{
		}

		template<typename TCallback>
		void OnNextIdleFrame(TCallback&& callback)
		{
			processRemoteEvents.Add(std::move(callback));
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Initialization)
***********************************************************************/

		void RequestControllerConnectionEstablished() override
		{
			ResetCreatedObjects();
		}

		void RequestControllerConnectionStopped() override
		{
			stopped = true;
		}

/***********************************************************************
IGuiRemoteProtocol
***********************************************************************/

		void Submit() override
		{
		}

		void ProcessRemoteEvents() override
		{
			if (!stopped)
			{
				if (LogRenderingResult())
				{
					vl::unittest::UnitTest::PrintMessage(L"Execute idle frame[" + itow(nextEventIndex) + L"]", vl::unittest::UnitTest::MessageKind::Info);
					processRemoteEvents[nextEventIndex]();
					nextEventIndex++;
				}
			}
		}
	};
}

#endif