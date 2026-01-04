/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Unit Test Snapsnot and other Utilities
***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_SHARED
#define VCZH_PRESENTATION_GUIUNITTESTPROTOCOL_SHARED

#include "../GacUI.h"
#include "../PlatformProviders/Remote/GuiRemoteController.h"

namespace vl::presentation::unittest
{
	struct WindowStyleConfig
	{
		WString						title;
		bool						enabled = true;
		bool						topMost = false;
		bool						showInTaskBar = true;

		bool						customFrameMode = false;
		bool						maximizedBox = true;
		bool						minimizedBox = true;
		bool						border = true;
		bool						sizeBox = true;
		bool						iconVisible = true;
		bool						titleBar = true;
		bool						activated = false;

		auto operator<=>(const WindowStyleConfig&) const = default;
	};

	enum class UnitTestRemoteChannel
	{
		None,
		Sync,
		Async,
	};

	struct UnitTestScreenConfig
	{
		using FontConfig = vl::presentation::remoteprotocol::FontConfig;
		using ScreenConfig = vl::presentation::remoteprotocol::ScreenConfig;

		WString						executablePath;
		NativeMargin				customFramePadding;
		FontConfig					fontConfig;
		ScreenConfig				screenConfig;
		bool						useDomDiff = false;
		UnitTestRemoteChannel		useChannel = UnitTestRemoteChannel::None;

		void						FastInitialize(vint width, vint height, vint taskBarHeight = 0);
	};

	class UnitTestRemoteProtocolBase : public Object, protected virtual IGuiRemoteProtocol
	{
	protected:
		IGuiRemoteProtocolEvents*	events = nullptr;
		UnitTestScreenConfig		globalConfig;

	public:
		UnitTestRemoteProtocolBase(const UnitTestScreenConfig& _globalConfig)
			: globalConfig(_globalConfig)
		{
		}

		IGuiRemoteProtocol* GetProtocol()
		{
			return this;
		}

		IGuiRemoteProtocolEvents* GetEvents() const
		{
			return events;
		}

		const UnitTestScreenConfig& GetGlobalConfig() const
		{
			return globalConfig;
		}

	protected:

#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					void Impl_ ## NAME()									{ CHECK_FAIL(L"Not Implemented!"); }
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						void Impl_ ## NAME(vint id)								{ CHECK_FAIL(L"Not Implemented!"); }
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						void Impl_ ## NAME(const REQUEST& arguments)			{ CHECK_FAIL(L"Not Implemented!"); }
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						void Impl_ ## NAME(vint id, const REQUEST& arguments)	{ CHECK_FAIL(L"Not Implemented!"); }
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

		void Initialize(IGuiRemoteProtocolEvents* _events) override
		{
			events = _events;
		}

		WString GetExecutablePath() override
		{
			return globalConfig.executablePath;
		}
	};
}

#endif