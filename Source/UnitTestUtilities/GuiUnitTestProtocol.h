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

		UnitTestRemoteProtocol(UnitTestScreenConfig _globalConfig)
			: UnitTestRemoteProtocolFeatures(_globalConfig)
		{
		}

/***********************************************************************
IGuiRemoteProtocol
***********************************************************************/

		void Submit() override
		{
			CHECK_FAIL(L"Not Implemented!");
		}

		void ProcessRemoteEvents() override
		{
			CHECK_FAIL(L"Not Implemented!");
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
		}
	};
}

#endif