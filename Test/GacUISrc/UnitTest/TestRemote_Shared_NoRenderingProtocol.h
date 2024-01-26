#ifndef GACUISRC_REMOTE_NORENDERINGPROTOCOL
#define GACUISRC_REMOTE_NORENDERINGPROTOCOL

#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteController.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation;
using namespace vl::presentation::remoteprotocol;

namespace remote_protocol_tests
{
	class NotImplementedProtocolBase : public Object, public virtual IGuiRemoteProtocol
	{
	public:
#define MESSAGE_NOREQ_NORES(NAME, REQUEST, RESPONSE)					void Request ## NAME()									override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_NOREQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id)							override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_REQ_NORES(NAME, REQUEST, RESPONSE)						void Request ## NAME(const REQUEST& arguments)			override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_REQ_RES(NAME, REQUEST, RESPONSE)						void Request ## NAME(vint id, const REQUEST& arguments)	override { CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!"); }
#define MESSAGE_HANDLER(NAME, REQUEST, RESPONSE, REQTAG, RESTAG, ...)	MESSAGE_ ## REQTAG ## _ ## RESTAG(NAME, REQUEST, RESPONSE)
		GACUI_REMOTEPROTOCOL_MESSAGES(MESSAGE_HANDLER)
#undef MESSAGE_HANDLER
#undef MESSAGE_REQ_RES
#undef MESSAGE_REQ_NORES
#undef MESSAGE_NOREQ_RES
#undef MESSAGE_NOREQ_NORES
	
		IGuiRemoteProtocolEvents* events = nullptr;
		
		void Initialize(IGuiRemoteProtocolEvents* _events) override
		{
			events = _events;
		}
		
		void Submit() override
		{
		}
		
		void ProcessRemoteEvents() override
		{
			CHECK_FAIL(L"Not Implemented in NotImplementedProtocolBase!");
		}
	};

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

	struct SingleScreenConfig
	{
		NativeMargin				customFramePadding;
		FontConfig					fontConfig;
		ScreenConfig				screenConfig;
	};
	
	class SingleScreenProtocol : public NotImplementedProtocolBase
	{
	public:
		SingleScreenConfig			globalConfig;
		List<Func<void()>>			processRemoteEvents;
		vint						nextEventIndex = 0;
	
		WindowSizingConfig			sizingConfig;
		WindowStyleConfig			styleConfig;
		NativeRect					lastRestoredSize;
	
		SingleScreenProtocol(SingleScreenConfig _globalConfig)
			: globalConfig(_globalConfig)
		{
			sizingConfig.bounds = { 0,0,0,0 };
			sizingConfig.clientBounds = { 0,0,0,0 };
			sizingConfig.customFramePadding = globalConfig.customFramePadding;
			sizingConfig.sizeState = INativeWindow::Restored;
		}
	
		template<typename TCallback>
		void OnNextFrame(TCallback&& callback)
		{
			processRemoteEvents.Add(std::move(callback));
		}
	
		void Submit() override
		{
		}
	
		void ProcessRemoteEvents() override
		{
			TEST_ASSERT(nextEventIndex < processRemoteEvents.Count());
			processRemoteEvents[nextEventIndex]();
			nextEventIndex++;
		}
	
		WString GetExecutablePath() override
		{
			return L"/Remote/Protocol.exe";
		}
	
		void RequestControllerGetFontConfig(vint id) override
		{
			events->RespondControllerGetFontConfig(id, globalConfig.fontConfig);
		}
	
		void RequestControllerGetScreenConfig(vint id) override
		{
			events->RespondControllerGetScreenConfig(id, globalConfig.screenConfig);
		}
	
		void RequestControllerConnectionEstablished() override
		{
		}
	
		void RequestControllerConnectionStopped() override
		{
		}
	
		void RequestWindowGetBounds(vint id) override
		{
			events->RespondWindowGetBounds(id, sizingConfig);
		}
	
		void RequestWindowNotifySetTitle(const ::vl::WString& arguments) override
		{
			styleConfig.title = arguments;
		}
	
		void RequestWindowNotifySetEnabled(const bool& arguments) override
		{
			styleConfig.enabled = arguments;
		}
	
		void RequestWindowNotifySetTopMost(const bool& arguments) override
		{
			styleConfig.topMost = arguments;
		}
	
		void RequestWindowNotifySetShowInTaskBar(const bool& arguments) override
		{
			styleConfig.showInTaskBar = arguments;
		}
	
		void OnBoundsUpdated()
		{
			sizingConfig.clientBounds = sizingConfig.bounds;
			if (sizingConfig.sizeState == INativeWindow::Restored)
			{
				lastRestoredSize = sizingConfig.bounds;
			}
			events->OnWindowBoundsUpdated(sizingConfig);
		}
	
		void RequestWindowNotifySetBounds(const NativeRect& arguments) override
		{
			sizingConfig.bounds = arguments;
			OnBoundsUpdated();
		}
	
		void RequestWindowNotifySetClientSize(const NativeSize& arguments) override
		{
			sizingConfig.bounds = { sizingConfig.bounds.LeftTop(), arguments };
			OnBoundsUpdated();
		}
	
		void RequestWindowNotifySetCustomFrameMode(const bool& arguments) override	{ styleConfig.customFrameMode = arguments;	events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetMaximizedBox(const bool& arguments) override		{ styleConfig.maximizedBox = arguments;		events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetMinimizedBox(const bool& arguments) override		{ styleConfig.minimizedBox = arguments;		events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetBorder(const bool& arguments) override			{ styleConfig.border = arguments;			events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetSizeBox(const bool& arguments) override			{ styleConfig.sizeBox = arguments;			events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetIconVisible(const bool& arguments) override		{ styleConfig.iconVisible = arguments;		events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifySetTitleBar(const bool& arguments) override			{ styleConfig.titleBar = arguments;			events->OnWindowBoundsUpdated(sizingConfig); }
		void RequestWindowNotifyActivate() override									{ styleConfig.activated = true; }
	
		void RequestWindowNotifyShow(const WindowShowing& arguments) override
		{
			styleConfig.activated = arguments.activate;
			if (sizingConfig.sizeState != arguments.sizeState)
			{
				sizingConfig.sizeState = arguments.sizeState;
				switch (arguments.sizeState)
				{
				case INativeWindow::Maximized:
					sizingConfig.bounds = globalConfig.screenConfig.clientBounds;
					OnBoundsUpdated();
					break;
				case INativeWindow::Minimized:
					sizingConfig.bounds = NativeRect(
						{ globalConfig.screenConfig.bounds.x2,globalConfig.screenConfig.bounds.y2 },
						{ 1,1 }
					);
					OnBoundsUpdated();
					break;
				case INativeWindow::Restored:
					if (sizingConfig.bounds != lastRestoredSize)
					{
						sizingConfig.bounds = lastRestoredSize;
						OnBoundsUpdated();
					}
					else
					{
						events->OnWindowBoundsUpdated(sizingConfig);
					}
					break;
				}
			}
		}
	};
}

#endif