#ifndef VCZH_PRESENTATION_REMOTEVIEWMODELTEST_SHARED
#define VCZH_PRESENTATION_REMOTEVIEWMODELTEST_SHARED

#include "../../../Source/GacUI.h"

namespace vl::presentation::remote_view_model_test
{
	inline constexpr const wchar_t* ViewModelChannelName = L"ViewModelChannel";
	inline constexpr const wchar_t* ViewModelReadyChannelName = L"ViewModelReadyChannel";
	inline constexpr const wchar_t* ViewModelServiceName = L"rvmt::IViewModel";

#ifdef VCZH_MSVC
	inline constexpr const wchar_t* RemotingNamedPipeName = L"GacUIRemoteProtocolNamedPipe";
#endif
	inline constexpr const wchar_t* RemotingHttpBaseUrl = L"/GacUIRemoteProtocolHttp";
	inline constexpr vint RemotingHttpPort = 8888;
}

#endif
