#ifndef VCZH_PRESENTATION_REMOTEVIEWMODELTEST_SHARED
#define VCZH_PRESENTATION_REMOTEVIEWMODELTEST_SHARED

#include "../../../Source/GacUI.h"
#include "../../RemotingHelpers/RemotingClient/ViewModelHostClient.h"
#include "RemoteViewModelTestRpc.h"

namespace vl::presentation::remote_view_model_test
{
	template<typename TChannelNameList>
	bool IsRemoteViewModelHostChannel(const TChannelNameList& availableChannels)
	{
		return
			availableChannels.Count() == 2 &&
			availableChannels.IndexOf(WString::Unmanaged(remoting::ViewModelChannelName)) != -1 &&
			availableChannels.IndexOf(WString::Unmanaged(remoting::ViewModelReadyChannelName)) != -1;
	}

#ifdef VCZH_MSVC
	inline constexpr const wchar_t* RemotingNamedPipeName = L"GacUIRemoteProtocolNamedPipe";
#endif
	inline constexpr const wchar_t* RemotingHttpBaseUrl = L"/GacUIRemoteProtocolHttp";
	inline constexpr vint RemotingHttpPort = 8888;

	extern remoting::RemotingDispatcherFactory CreateDispatcherFactory();
}

#endif
