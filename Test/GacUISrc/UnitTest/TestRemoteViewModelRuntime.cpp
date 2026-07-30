#include "../RemotingTest_RvmHost/RemoteViewModelRoleState.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation::remote_view_model_test;
using namespace vl::presentation::remoteprotocol::channeling;

TEST_FILE
{
	TEST_CATEGORY(L"RemoteViewModelRoleState")
	{
		TEST_CASE(L"Exact channel-list classification")
		{
			List<WString> channels;
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Unknown);

			channels.Add(WString::Unmanaged(ViewModelChannelName));
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Unknown);

			channels.Add(WString::Unmanaged(ViewModelReadyChannelName));
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::ViewModelHost);

			channels.Clear();
			channels.Add(WString::Unmanaged(ViewModelReadyChannelName));
			channels.Add(WString::Unmanaged(ViewModelChannelName));
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::ViewModelHost);

			channels.Clear();
			channels.Add(WString::Unmanaged(GacUIRemoteProtocolChannelName));
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Renderer);

			channels.Clear();
			channels.Add(L"Unknown");
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Unknown);

			channels.Clear();
			channels.Add(WString::Unmanaged(ViewModelChannelName));
			channels.Add(WString::Unmanaged(ViewModelChannelName));
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Unknown);

			channels.Clear();
			channels.Add(WString::Unmanaged(ViewModelReadyChannelName));
			channels.Add(WString::Unmanaged(ViewModelReadyChannelName));
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Unknown);

			channels.Clear();
			channels.Add(WString::Unmanaged(GacUIRemoteProtocolChannelName));
			channels.Add(WString::Unmanaged(GacUIRemoteProtocolChannelName));
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Unknown);

			channels.Clear();
			channels.Add(WString::Unmanaged(ViewModelChannelName));
			channels.Add(WString::Unmanaged(GacUIRemoteProtocolChannelName));
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Unknown);

			channels.Clear();
			channels.Add(WString::Unmanaged(GacUIRemoteProtocolChannelName));
			channels.Add(WString::Unmanaged(ViewModelChannelName));
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Unknown);

			channels.Clear();
			channels.Add(WString::Unmanaged(ViewModelChannelName));
			channels.Add(L"Unknown");
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Unknown);

			channels.Clear();
			channels.Add(WString::Unmanaged(GacUIRemoteProtocolChannelName));
			channels.Add(L"Unknown");
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Unknown);

			channels.Clear();
			channels.Add(L"Unknown");
			channels.Add(L"Unknown");
			TEST_ASSERT(ClassifyRemoteViewModelChannel(channels) == RemoteViewModelChannelRole::Unknown);
		});

		TEST_CASE(L"The first host is accepted for the whole process run")
		{
			RemoteViewModelRoleState state;
			TEST_ASSERT(state.EnableRemoteViewModel());
			TEST_ASSERT(state.RegisterRequester(10));
			TEST_ASSERT(state.TryAcceptViewModelHost(20));
			TEST_ASSERT(state.HasAcceptedViewModelHost());
			TEST_ASSERT(state.GetViewModelHostId() == 20);
			TEST_ASSERT(!state.TryAcceptViewModelHost(21));
		});

		TEST_CASE(L"A second requester is rejected without replacing the first requester")
		{
			RemoteViewModelRoleState state;
			TEST_ASSERT(state.EnableRemoteViewModel());
			TEST_ASSERT(state.RegisterRequester(10));
			TEST_ASSERT(!state.RegisterRequester(11));
			TEST_ASSERT(state.GetRequesterClientId() == 10);
		});

		TEST_CASE(L"Host loss before window startup is fatal")
		{
			RemoteViewModelRoleState state;
			TEST_ASSERT(state.EnableRemoteViewModel());
			TEST_ASSERT(state.RegisterRequester(10));
			TEST_ASSERT(state.TryAcceptViewModelHost(20));
			TEST_ASSERT(state.FailViewModelHost(20));
			TEST_ASSERT(state.HasFatalError());
			TEST_ASSERT(state.GetViewModelHostId() == InvalidRemoteViewModelClientId);
			TEST_ASSERT(!state.BeginRunning());
			TEST_ASSERT(!state.TryAcceptViewModelHost(21));
		});

		TEST_CASE(L"Phase gates host and renderer acceptance")
		{
			RemoteViewModelRoleState state;
			TEST_ASSERT(state.GetPhase() == RemoteViewModelApplicationPhase::Running);
			TEST_ASSERT(state.CanAdmitRenderer());
			TEST_ASSERT(!state.TryAcceptViewModelHost(20));
			TEST_ASSERT(!state.BeginRunning());

			TEST_ASSERT(state.EnableRemoteViewModel());
			TEST_ASSERT(state.GetPhase() == RemoteViewModelApplicationPhase::Starting);
			TEST_ASSERT(!state.CanAdmitRenderer());
			TEST_ASSERT(state.RegisterRequester(10));
			TEST_ASSERT(!state.TryAcceptViewModelHost(10));
			TEST_ASSERT(state.TryAcceptViewModelHost(20));
			TEST_ASSERT(state.GetViewModelHostId() == 20);
			TEST_ASSERT(state.BeginRunning());
			TEST_ASSERT(state.GetPhase() == RemoteViewModelApplicationPhase::Running);
			TEST_ASSERT(state.CanAdmitRenderer());
			TEST_ASSERT(!state.BeginRunning());
			TEST_ASSERT(!state.TryAcceptViewModelHost(21));

			state.BeginStopping();
			TEST_ASSERT(state.GetPhase() == RemoteViewModelApplicationPhase::Stopping);
			TEST_ASSERT(!state.CanAdmitRenderer());
			TEST_ASSERT(!state.FailViewModelHost(20));
			TEST_ASSERT(state.GetViewModelHostId() == InvalidRemoteViewModelClientId);
			TEST_ASSERT(!state.HasFatalError());
		});

		TEST_CASE(L"Renderer replacement preserves RPC role IDs")
		{
			RemoteViewModelRoleState state;
			TEST_ASSERT(state.EnableRemoteViewModel());
			TEST_ASSERT(state.RegisterRequester(10));
			TEST_ASSERT(state.TryAcceptViewModelHost(20));
			TEST_ASSERT(state.BeginRunning());

			vint replacedClientId = 999;
			TEST_ASSERT(!state.TryAcceptRenderer(10, replacedClientId));
			TEST_ASSERT(replacedClientId == InvalidRemoteViewModelClientId);
			TEST_ASSERT(!state.TryAcceptRenderer(20, replacedClientId));
			TEST_ASSERT(replacedClientId == InvalidRemoteViewModelClientId);
			TEST_ASSERT(state.TryAcceptRenderer(30, replacedClientId));
			TEST_ASSERT(replacedClientId == InvalidRemoteViewModelClientId);
			TEST_ASSERT(state.GetRendererClientId() == 30);
			TEST_ASSERT(state.GetRequesterClientId() == 10);
			TEST_ASSERT(state.GetViewModelHostId() == 20);

			TEST_ASSERT(state.TryAcceptRenderer(31, replacedClientId));
			TEST_ASSERT(replacedClientId == 30);
			TEST_ASSERT(state.GetRendererClientId() == 31);
			TEST_ASSERT(state.GetRequesterClientId() == 10);
			TEST_ASSERT(state.GetViewModelHostId() == 20);
			TEST_ASSERT(!state.DisconnectRenderer(30));
			TEST_ASSERT(state.DisconnectRenderer(31));
			TEST_ASSERT(state.GetRendererClientId() == InvalidRemoteViewModelClientId);
			TEST_ASSERT(state.GetRequesterClientId() == 10);
			TEST_ASSERT(state.GetViewModelHostId() == 20);
		});

		TEST_CASE(L"Host loss keeps other roles distinct and admission closed")
		{
			RemoteViewModelRoleState state;
			TEST_ASSERT(state.EnableRemoteViewModel());
			TEST_ASSERT(state.RegisterRequester(10));
			TEST_ASSERT(state.TryAcceptViewModelHost(20));
			TEST_ASSERT(state.BeginRunning());

			vint replacedClientId = InvalidRemoteViewModelClientId;
			TEST_ASSERT(state.TryAcceptRenderer(30, replacedClientId));
			TEST_ASSERT(state.FailViewModelHost(20));
			TEST_ASSERT(state.HasFatalError());
			TEST_ASSERT(state.GetViewModelHostId() == InvalidRemoteViewModelClientId);
			TEST_ASSERT(state.GetRequesterClientId() == 10);
			TEST_ASSERT(state.GetRendererClientId() == 30);
			TEST_ASSERT(state.HasAcceptedViewModelHost());
			TEST_ASSERT(!state.CanAdmitRenderer());
			TEST_ASSERT(!state.TryAcceptViewModelHost(21));
			TEST_ASSERT(!state.FailViewModelHost(20));
		});

		TEST_CASE(L"Host loss remains immediately fatal after another fatal error")
		{
			RemoteViewModelRoleState state;
			TEST_ASSERT(state.EnableRemoteViewModel());
			TEST_ASSERT(state.RegisterRequester(10));
			TEST_ASSERT(state.TryAcceptViewModelHost(20));
			TEST_ASSERT(state.FailRemoteViewModel());
			TEST_ASSERT(state.HasFatalError());
			TEST_ASSERT(state.FailViewModelHost(20));
			TEST_ASSERT(state.GetViewModelHostId() == InvalidRemoteViewModelClientId);
		});
	});
}
