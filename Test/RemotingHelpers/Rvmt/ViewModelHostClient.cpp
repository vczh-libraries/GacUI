#include "ViewModelHostClient.h"
#include <cstdlib>

namespace vl::presentation::remoting
{
	using namespace collections;
	using namespace rpc_controller::channeling;

	[[noreturn]] void ExitViewModelHostProcess(vint exitCode)
	{
		std::_Exit((int)exitCode);
	}

	class ViewModelHostClient::Impl : public Object
	{
	public:
		JsonChannelClient::ChannelMap						channelNames;
		Ptr<remote_view_model_test::RemoteViewModelJsonDispatcherClient> dispatcher;
		JsonChannel*										controlChannel = nullptr;

		Impl(Ptr<TaskQueue> taskQueue)
		{
			channelNames.Add(ViewModelChannelName, nullptr);
			channelNames.Add(ViewModelReadyChannelName, nullptr);
			dispatcher = Ptr(new remote_view_model_test::RemoteViewModelJsonDispatcherClient(taskQueue));
		}

		void Connect(JsonChannelClient* channelClient)
		{
			List<WString> waitingForServices;
			dispatcher->WaitForServer(
				channelClient,
				channelClient->GetChannels()[ViewModelChannelName],
				waitingForServices
				);
			dispatcher->InitializeRpc(channelClient->GetClientId());
			controlChannel = channelClient->GetChannels()[ViewModelReadyChannelName];
			CHECK_ERROR(controlChannel, L"ViewModelHostClient::Impl::Connect(...)#The control channel is null.");
		}

		void SendReady()
		{
			CHECK_ERROR(controlChannel, L"ViewModelHostClient::Impl::SendReady()#The control channel is null.");
			controlChannel->BroadcastFromClient(CreateViewModelReadyMessage());
			bool disconnected = false;
			controlChannel->BatchWrite(disconnected);
			if (disconnected)
			{
				ExitViewModelHostProcess(1);
			}
		}
	};

	ViewModelHostClient::ViewModelHostClient(
		Ptr<inter_process::INetworkProtocolClient> networkClient,
		Ptr<glr::json::Parser> parser,
		Ptr<TaskQueue> taskQueue
		)
		: JsonNetworkChannelClient(networkClient, parser)
		, impl(Ptr(new Impl(taskQueue)))
	{
	}

	const JsonChannelClient::ChannelNameList& ViewModelHostClient::OnGetChannelNames()
	{
		return impl->channelNames.Keys();
	}

	void ViewModelHostClient::OnConnected(vint)
	{
	}

	void ViewModelHostClient::OnDisconnected()
	{
		ExitViewModelHostProcess(1);
	}

	void ViewModelHostClient::OnReadError(const WString&)
	{
		ExitViewModelHostProcess(1);
	}

	void ViewModelHostClient::OnLocalError(const WString&, bool fatal)
	{
		if (fatal)
		{
			ExitViewModelHostProcess(1);
		}
	}

	void ViewModelHostClient::Connect()
	{
		impl->Connect(this);
	}

	void ViewModelHostClient::SendReady()
	{
		impl->SendReady();
	}

	RpcDispatcherClient* ViewModelHostClient::GetDispatcher()
	{
		return impl->dispatcher.Obj();
	}
}
