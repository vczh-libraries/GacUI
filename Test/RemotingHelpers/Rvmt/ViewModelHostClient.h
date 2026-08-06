#ifndef VCZH_PRESENTATION_RVMT_VIEWMODEL_HOST_CLIENT
#define VCZH_PRESENTATION_RVMT_VIEWMODEL_HOST_CLIENT

#include "ViewModelShared.h"

namespace vl::presentation::remoting
{
	class ViewModelHostClient
		: public rpc_controller::channeling::JsonNetworkChannelClient
	{
	private:
		JsonChannelClient::ChannelMap					channelNames;
		Ptr<remote_view_model_test::RemoteViewModelJsonDispatcherClient> dispatcher;
		JsonChannel*									controlChannel = nullptr;

	public:
		ViewModelHostClient(
			Ptr<inter_process::INetworkProtocolClient> networkClient,
			Ptr<glr::json::Parser> parser,
			Ptr<TaskQueue> taskQueue
			);
		const JsonChannelClient::ChannelNameList&		OnGetChannelNames() override;
		void											OnConnected(vint clientId) override;
		void											OnDisconnected() override;
		void											OnReadError(const WString& errorMessage) override;
		void											OnLocalError(const WString& errorMessage, bool fatal) override;

		void											Connect();
		void											SendReady();
		RpcDispatcherClient*							GetDispatcher();
	};
}

#endif
