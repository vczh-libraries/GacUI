#ifndef VCZH_PRESENTATION_RVMT_VIEWMODEL_SHARED
#define VCZH_PRESENTATION_RVMT_VIEWMODEL_SHARED

#include "../../../Source/GacUI.h"

namespace vl::presentation::remoting
{
	using JsonPackage = rpc_controller::channeling::JsonPackage;
	using JsonChannel = rpc_controller::channeling::JsonChannel;
	using JsonChannelClient = rpc_controller::channeling::JsonChannelClient;
	using JsonChannelServer = rpc_controller::channeling::JsonChannelServer;
	using JsonLocalChannelClient = rpc_controller::channeling::JsonLocalChannelClient;
	using TaskQueue = rpc_controller::channeling::TaskQueue;
	using RpcDispatcherClient = rpc_controller::channeling::RpcJsonDispatcherClientForTaskQueue;

	inline constexpr const wchar_t*						ViewModelChannelName = L"ViewModelChannel";
	inline constexpr const wchar_t*						ViewModelReadyChannelName = L"ViewModelReadyChannel";
	inline constexpr const wchar_t*						ViewModelReadyMessage = L"Ready";
	inline constexpr const wchar_t*						RemoteViewModelHostDisconnectedError = L"RemotingTest_RvmHost disconnected.";
	inline constexpr vint								InvalidRemoteViewModelClientId = -1;

	extern JsonPackage CreateViewModelReadyMessage();
	extern bool IsViewModelReadyMessage(const JsonPackage& package);
}

namespace vl::presentation::remote_view_model_test
{
	class RemoteViewModelJsonDispatcherClient
		: public rpc_controller::channeling::RpcJsonDispatcherClientForTaskQueue
	{
	public:
		RemoteViewModelJsonDispatcherClient(Ptr<remoting::TaskQueue> taskQueue);

		void InitializeRpc(vint clientId);
	};

}

#endif
