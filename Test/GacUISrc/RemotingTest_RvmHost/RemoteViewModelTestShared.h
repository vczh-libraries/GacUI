#ifndef VCZH_PRESENTATION_REMOTEVIEWMODELTEST_SHARED
#define VCZH_PRESENTATION_REMOTEVIEWMODELTEST_SHARED

#include "../../../Source/GacUI.h"

namespace vl::presentation::remote_view_model_test
{
	inline constexpr const wchar_t* ViewModelChannelName = L"ViewModelChannel";
	inline constexpr const wchar_t* ViewModelReadyChannelName = L"ViewModelReadyChannel";
	inline constexpr const wchar_t* ViewModelServiceName = L"rvmt::IViewModel";
	inline constexpr const wchar_t* ViewModelReadyMessage = L"Ready";
	inline constexpr const wchar_t* ViewModelHeartbeatMessagePrefix = L"Heartbeat:";
	inline constexpr const wchar_t* ViewModelHeartbeatAckMessagePrefix = L"HeartbeatAck:";
	inline constexpr const wchar_t* ViewModelRequesterStoppingMessage = L"RequesterStopping";
	inline constexpr const wchar_t* ViewModelRequesterStoppingAckMessage = L"RequesterStoppingAck";
	inline constexpr const wchar_t* RemoteViewModelHostDisconnectedError = L"RemotingTest_RvmHost disconnected.";
	inline constexpr const wchar_t* RemoteViewModelRequesterDisconnectedError = L"RVM requester disconnected.";

	inline constexpr vint RemoteViewModelHeartbeatIntervalMilliseconds = 500;
	inline constexpr vint RemoteViewModelLeaseTimeoutMilliseconds = 5000;
	inline constexpr vint RemoteViewModelStartupGraceMilliseconds = 10000;
	inline constexpr vint RemoteViewModelTerminalDeliveryMilliseconds = 200;

#ifdef VCZH_MSVC
	inline constexpr const wchar_t* RemotingNamedPipeName = L"GacUIRemoteProtocolNamedPipe";
#endif
	inline constexpr const wchar_t* RemotingHttpBaseUrl = L"/GacUIRemoteProtocolHttp";
	inline constexpr vint RemotingHttpPort = 8888;
}

#endif
