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

	inline JsonPackage CreateViewModelReadyMessage()
	{
		auto package = Ptr(new glr::json::JsonString);
		package->content.value = ViewModelReadyMessage;
		return package;
	}

	inline bool IsViewModelReadyMessage(const JsonPackage& package)
	{
		auto jsonString = package.Cast<glr::json::JsonString>();
		return jsonString && jsonString->content.value == ViewModelReadyMessage;
	}
}

#endif
