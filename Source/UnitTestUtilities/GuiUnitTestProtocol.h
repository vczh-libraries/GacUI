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

namespace vl::presentation::unittest
{
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
		using Type = TMixin<typename Mixin<TBase, TOtherMixins...>::Type>;
	};

	using UnitTestRemoteProtocolFeatures = Mixin<
		UnitTestRemoteProtocolBase,
		UnitTestRemoteProtocol_Rendering,
		UnitTestRemoteProtocol_IO,
		UnitTestRemoteProtocol_MainWindow
	>::Type;

	class UnitTestRemoteProtocol : public UnitTestRemoteProtocolFeatures
	{
	public:
		collections::List<Func<void()>>		processRemoteEvents;
		vint								nextEventIndex = 0;
		bool								rendered = false;
		bool								stopped = false;

		UnitTestRemoteProtocol(UnitTestScreenConfig _globalConfig)
			: UnitTestRemoteProtocolFeatures(_globalConfig)
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
			this->createdElements.Clear();
			this->createdImages.Clear();
		}

		void RequestControllerConnectionStopped() override
		{
			stopped = true;
		}

/***********************************************************************
IGuiRemoteProtocolMessages (Rendering)
***********************************************************************/

		void RequestRendererBeginRendering() override
		{
			rendered = true;
			UnitTestRemoteProtocolFeatures::RequestRendererBeginRendering();
		}

		void RequestRendererEndRendering(vint id) override
		{
			UnitTestRemoteProtocolFeatures::RequestRendererEndRendering(id);
		}

/***********************************************************************
IGuiRemoteProtocol
***********************************************************************/

		void Submit() override
		{
			rendered = false;
		}

		void ProcessRemoteEvents() override
		{
			if (!rendered && !stopped)
			{
				TEST_CASE(L"Execute idle frame[" + itow(nextEventIndex) + L"]")
				{
					processRemoteEvents[nextEventIndex]();
				});
				nextEventIndex++;
			}
		}
	};
}

#endif