#ifndef VCZH_PRESENTATION_REMOTING_REMOTINGCHANNELSERVER
#define VCZH_PRESENTATION_REMOTING_REMOTINGCHANNELSERVER

#include "../../PlatformProviders/Remote/GuiRemoteProtocol.h"

namespace vl::presentation::remoting
{
	using JsonChannelClient = remoteprotocol::channeling::IJsonChannelClient;
	using JsonChannel = remoteprotocol::channeling::IJsonChannel;
	using JsonChannelServer = remoteprotocol::channeling::IJsonChannelServer;

	inline bool IsRendererChannel(const JsonChannelClient::ChannelNameList& availableChannels)
	{
		return
			availableChannels.Count() == 1 &&
			availableChannels[0] == WString::Unmanaged(remoteprotocol::channeling::GacUIRemoteProtocolChannelName);
	}

	template<typename TServerBase>
	class RemotingChannelServer
		: public remoteprotocol::channeling::GuiRemoteProtocolNetworkChannelServer<TServerBase>
	{
		using Base = remoteprotocol::channeling::GuiRemoteProtocolNetworkChannelServer<TServerBase>;

	private:
		SpinLock											lockConnection;
		JsonChannel*										coreJsonChannel = nullptr;
		remoteprotocol::channeling::GuiRemoteProtocolCoreChannel* coreProtocolChannel = nullptr;
		bool												acceptRenderer = false;
		vint												rendererClientId = -1;

	protected:
		virtual inter_process::WaitForClientResult OnRemoteClientConnected(
			vint clientId,
			const JsonChannelClient::ChannelNameList& availableChannels
			)
		{
			using WaitForClientResult = inter_process::WaitForClientResult;
			auto rendererChannel = IsRendererChannel(availableChannels);
			if (acceptRenderer && rendererChannel)
			{
				JsonChannel* jsonChannelToOldRenderer = nullptr;
				vint oldRendererClientId = -1;
				SPIN_LOCK(lockConnection)
				{
					if (clientId == rendererClientId)
					{
						return WaitForClientResult::Reject;
					}
					oldRendererClientId = rendererClientId;
					rendererClientId = clientId;
					if (oldRendererClientId != -1)
					{
						jsonChannelToOldRenderer = coreJsonChannel;
						if (coreProtocolChannel)
						{
							coreProtocolChannel->DetachRenderer(oldRendererClientId);
						}
					}
				}

				if (oldRendererClientId != -1)
				{
					bool rendererNotifiedToStop = false;
					if (jsonChannelToOldRenderer)
					{
						try
						{
							Ptr<glr::json::JsonObject> package;
							remoteprotocol::channeling::ChannelPackageInfo info
							{
								remoteprotocol::channeling::ChannelPackageSemantic::Message,
								-1,
								WString::Unmanaged(L"ControllerConnectionStopped")
							};
							remoteprotocol::channeling::JsonChannelPack(info, {}, package);
							jsonChannelToOldRenderer->SendToClient(oldRendererClientId, package);
							bool disconnected = false;
							jsonChannelToOldRenderer->BatchWrite(disconnected);
							rendererNotifiedToStop = !disconnected;
						}
						catch (const Error&)
						{
						}
						catch (const Exception&)
						{
						}
					}
					if (!rendererNotifiedToStop)
					{
						this->DisconnectClient(oldRendererClientId);
					}
				}
				return WaitForClientResult::Accept;
			}
			return WaitForClientResult::Reject;
		}

	public:
		using Base::OnClientConnected;

		template<typename... TArgs>
		RemotingChannelServer(
			Ptr<glr::json::Parser> parser,
			bool _acceptRenderer,
			TArgs&&... args
			)
			: Base(parser, std::forward<TArgs>(args)...)
			, acceptRenderer(_acceptRenderer)
		{
		}

		void SetCoreChannels(
			JsonChannel* jsonChannel,
			remoteprotocol::channeling::GuiRemoteProtocolCoreChannel* protocolChannel
			)
		{
			SPIN_LOCK(lockConnection)
			{
				coreJsonChannel = jsonChannel;
				coreProtocolChannel = protocolChannel;
			}
		}

		void ClearCoreChannels()
		{
			SPIN_LOCK(lockConnection)
			{
				coreProtocolChannel = nullptr;
				coreJsonChannel = nullptr;
			}
		}

		vint GetRendererClientId()
		{
			vint clientId = -1;
			SPIN_LOCK(lockConnection)
			{
				clientId = rendererClientId;
			}
			return clientId;
		}

		inter_process::WaitForClientResult OnClientConnected(
			vint clientId,
			const JsonChannelClient::ChannelNameList& availableChannels,
			JsonChannelClient* localClient
			) override
		{
			if (localClient)
			{
				return inter_process::WaitForClientResult::Accept;
			}
			return OnRemoteClientConnected(clientId, availableChannels);
		}

		void OnClientDisconnected(vint clientId) override
		{
			SPIN_LOCK(lockConnection)
			{
				if (rendererClientId == clientId)
				{
					rendererClientId = -1;
					if (coreProtocolChannel)
					{
						coreProtocolChannel->DetachRenderer(clientId);
					}
				}
			}
		}
	};
}

#endif
