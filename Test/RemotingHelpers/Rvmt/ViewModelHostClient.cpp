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

	ViewModelHostClient::ViewModelHostClient(
		Ptr<inter_process::INetworkProtocolClient> networkClient,
		Ptr<glr::json::Parser> parser,
		Ptr<TaskQueue> taskQueue
		)
		: JsonNetworkChannelClient(networkClient, parser)
	{
		channelNames.Add(ViewModelChannelName, nullptr);
		channelNames.Add(ViewModelReadyChannelName, nullptr);
		dispatcher = Ptr(new RpcDispatcherClient(taskQueue));
	}

	const JsonChannelClient::ChannelNameList& ViewModelHostClient::OnGetChannelNames()
	{
		return channelNames.Keys();
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
		List<WString> waitingForServices;
		dispatcher->WaitForServer(
			this,
			GetChannels()[ViewModelChannelName],
			waitingForServices
			);
		controlChannel = GetChannels()[ViewModelReadyChannelName];
		CHECK_ERROR(controlChannel, L"ViewModelHostClient::Connect()#The control channel is null.");
	}

	void ViewModelHostClient::SendReady()
	{
		CHECK_ERROR(controlChannel, L"ViewModelHostClient::SendReady()#The control channel is null.");
		controlChannel->BroadcastFromClient(CreateViewModelReadyMessage());
		bool disconnected = false;
		controlChannel->BatchWrite(disconnected);
		if (disconnected)
		{
			ExitViewModelHostProcess(1);
		}
	}

	RpcDispatcherClient* ViewModelHostClient::GetDispatcher()
	{
		return dispatcher.Obj();
	}
}
