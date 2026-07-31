#ifndef VCZH_PRESENTATION_REMOTING_REMOTINGCHANNELSERVER
#define VCZH_PRESENTATION_REMOTING_REMOTINGCHANNELSERVER

#include "../../PlatformProviders/Remote/GuiRemoteProtocol.h"

namespace vl::presentation::remoting
{
	using JsonChannelClient = remoteprotocol::channeling::IJsonChannelClient;
	using JsonChannel = remoteprotocol::channeling::IJsonChannel;
	using JsonChannelServer = remoteprotocol::channeling::IJsonChannelServer;
	using JsonChannelPackage = remoteprotocol::channeling::JsonPackage;

	struct RemotingChannelServerCallbacks
	{
		Func<bool(JsonChannelClient*)>						canAcceptLocalClient;
		Func<bool(const JsonChannelClient::ChannelNameList&)>	isRemoteClient;
		Func<bool(vint)>									tryAcceptRemoteClient;
		Func<void(vint)>									clientDisconnected;
		Func<bool()>										canAdmitRenderer;
		Func<void(vint, bool)>								rendererConnectionChanged;
	};

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
		RemotingChannelServerCallbacks						callbacks;
		bool												acceptRenderer = false;
		vint												rendererClientId = -1;

	public:
		using Base::OnClientConnected;

		template<typename... TArgs>
		RemotingChannelServer(
			Ptr<glr::json::Parser> parser,
			bool _acceptRenderer,
			const RemotingChannelServerCallbacks& _callbacks,
			TArgs&&... args
			)
			: Base(parser, std::forward<TArgs>(args)...)
			, callbacks(_callbacks)
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
			using WaitForClientResult = inter_process::WaitForClientResult;
			auto rendererChannel = IsRendererChannel(availableChannels);
			if (localClient)
			{
				if (callbacks.canAcceptLocalClient && callbacks.canAcceptLocalClient(localClient))
				{
					return WaitForClientResult::Accept;
				}
				if (
					acceptRenderer &&
					rendererChannel &&
					dynamic_cast<remoteprotocol::channeling::GuiRemoteProtocolLocalChannelClient*>(localClient)
					)
				{
					return WaitForClientResult::Accept;
				}
				return WaitForClientResult::Reject;
			}

			if (acceptRenderer && rendererChannel)
			{
				if (callbacks.canAdmitRenderer && !callbacks.canAdmitRenderer())
				{
					return WaitForClientResult::Reject;
				}

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
				if (callbacks.rendererConnectionChanged)
				{
					callbacks.rendererConnectionChanged(clientId, true);
				}
				return WaitForClientResult::Accept;
			}

			if (
				callbacks.isRemoteClient &&
				callbacks.isRemoteClient(availableChannels) &&
				callbacks.tryAcceptRemoteClient &&
				callbacks.tryAcceptRemoteClient(clientId)
				)
			{
				return WaitForClientResult::Accept;
			}
			return WaitForClientResult::Reject;
		}

		void OnClientDisconnected(vint clientId) override
		{
			bool rendererDisconnected = false;
			SPIN_LOCK(lockConnection)
			{
				if (rendererClientId == clientId)
				{
					rendererClientId = -1;
					if (coreProtocolChannel)
					{
						coreProtocolChannel->DetachRenderer(clientId);
					}
					rendererDisconnected = true;
				}
			}

			if (callbacks.clientDisconnected)
			{
				callbacks.clientDisconnected(clientId);
			}
			if (rendererDisconnected && callbacks.rendererConnectionChanged)
			{
				callbacks.rendererConnectionChanged(clientId, false);
			}
		}
	};

	class RemotingCoreChannel : public remoteprotocol::channeling::GuiRemoteProtocolCoreChannel
	{
		using Base = remoteprotocol::channeling::GuiRemoteProtocolCoreChannel;
		Func<vint()>										getTransportRendererClientId;

	public:
		RemotingCoreChannel(
			JsonChannelClient* client,
			JsonChannel* channel,
			const WString& executablePath,
			IGuiRemoteEventProcessor* eventProcessor,
			const Func<vint()>& _getTransportRendererClientId
			)
			: Base(client, channel, executablePath, eventProcessor)
			, getTransportRendererClientId(_getTransportRendererClientId)
		{
		}

		void Submit(bool& disconnected) override
		{
			auto receiverClientId = GetRendererClientId();
			collections::List<JsonChannelPackage> packages;
			SPIN_LOCK(lockPackagesBeforeRenderer)
			{
				packages = std::move(packagesBeforeRenderer);
			}

			if (receiverClientId == -1 || receiverClientId != getTransportRendererClientId())
			{
				disconnected = true;
				return;
			}

			for (auto&& package : packages)
			{
				channel->SendToClient(receiverClientId, package);
			}
			channel->BatchWrite(disconnected);
		}
	};
}

#endif
