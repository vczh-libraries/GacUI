#ifndef VCZH_PRESENTATION_REMOTING_REMOTEPROTOCOLRENDERERCLIENT
#define VCZH_PRESENTATION_REMOTING_REMOTEPROTOCOLRENDERERCLIENT

#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"
#include "../../../Source/Utilities/SharedServices/GuiSharedAutomationService_Controls.h"

namespace vl::presentation::remoting
{
	class RemoteProtocolRendererClient : public remoteprotocol::channeling::GuiRemoteProtocolChannelClient
	{
		using Base = remoteprotocol::channeling::GuiRemoteProtocolChannelClient;

	private:
		SpinLock											lockState;
		WString												fatalTitle;
		bool												triggeredFatalError = false;
		bool												retainedFatalError = false;
		bool												stopping = false;
		remote_renderer::GuiRemoteRendererSingle*			renderer = nullptr;
		remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannelRenderer*
															asyncRendererChannel = nullptr;
		AutomationServiceRenderer*							rendererAutomationService = nullptr;

		void												QueueMainThreadTask(const Func<void()>& task);
		void												ProcessFatalError(const WString& errorMessage);
		void												ProcessDisconnected();

	public:
		RemoteProtocolRendererClient(
			Ptr<inter_process::INetworkProtocolClient> client,
			Ptr<glr::json::Parser> parser,
			const WString& fatalTitle
			);

		void												SetRenderer(remote_renderer::GuiRemoteRendererSingle* renderer);
		void												SetAsyncRendererChannel(
																remoteprotocol::channeling::GuiRemoteProtocolAsyncJsonChannelRenderer* asyncRendererChannel
																);
		void												SetRendererAutomationService(AutomationServiceRenderer* automationService);
		void												BeginStopping();
		bool												IsFatalErrorRetained();

		void												OnReadError(const WString& errorMessage) override;
		void												OnLocalError(const WString& errorMessage, bool fatal) override;
		void												OnDisconnected() override;
	};
}

#endif
