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

	TEST_CATEGORY(L"RemoteViewModelLeaseState")
	{
		TEST_CASE(L"Startup grace expires at its exact deadline")
		{
			RemoteViewModelLeaseState state;
			state.Start(100, 1000);
			TEST_ASSERT(state.IsActive());
			TEST_ASSERT(!state.IsExpired(1099));
			TEST_ASSERT(state.IsExpired(1100));
		});

		TEST_CASE(L"Only increasing heartbeat sequences renew the lease")
		{
			RemoteViewModelLeaseState state;
			state.Start(100, 1000);
			TEST_ASSERT(state.Renew(1, 500, 1000));
			TEST_ASSERT(state.GetLastSequence() == 1);
			TEST_ASSERT(!state.Renew(1, 1200, 1000));
			TEST_ASSERT(!state.Renew(0, 1200, 1000));
			TEST_ASSERT(!state.IsExpired(1499));
			TEST_ASSERT(state.IsExpired(1500));
		});

		TEST_CASE(L"A delayed heartbeat cannot revive an expired lease")
		{
			RemoteViewModelLeaseState state;
			state.Start(100, 1000);
			TEST_ASSERT(!state.Renew(1, 1100, 1000));
			TEST_ASSERT(state.GetLastSequence() == -1);
			TEST_ASSERT(state.IsExpired(1100));
			TEST_ASSERT(!state.Renew(2, 1101, 1000));
		});

		TEST_CASE(L"Stopping disables lease expiry")
		{
			RemoteViewModelLeaseState state;
			state.Start(100, 1000);
			state.Stop();
			TEST_ASSERT(!state.IsActive());
			TEST_ASSERT(!state.IsExpired(100000));
			TEST_ASSERT(!state.Renew(1, 100000, 1000));
		});
	});

	TEST_CATEGORY(L"RemoteViewModelTerminalState")
	{
		TEST_CASE(L"The first failure and exact message win")
		{
			RemoteViewModelTerminalState state;
			TEST_ASSERT(state.TryClaimFailure(L"first"));
			TEST_ASSERT(!state.TryClaimFailure(L"second"));
			TEST_ASSERT(!state.TryClaimNormal());
			TEST_ASSERT(!state.BeginStopping());
			TEST_ASSERT(state.HasFailure());
			TEST_ASSERT(state.GetMessage() == L"first");

			bool normal = true;
			WString message;
			TEST_ASSERT(state.TryTake(normal, message));
			TEST_ASSERT(!normal);
			TEST_ASSERT(message == L"first");
			TEST_ASSERT(!state.TryTake(normal, message));
		});

		TEST_CASE(L"Normal requester stopping is a one-shot terminal outcome")
		{
			RemoteViewModelTerminalState state;
			TEST_ASSERT(state.TryClaimNormal());
			TEST_ASSERT(!state.TryClaimFailure(L"late"));

			bool normal = false;
			WString message;
			TEST_ASSERT(state.TryTake(normal, message));
			TEST_ASSERT(normal);
			TEST_ASSERT(message == L"");
		});

		TEST_CASE(L"Local stopping suppresses later terminal callbacks")
		{
			RemoteViewModelTerminalState state;
			TEST_ASSERT(state.BeginStopping());
			TEST_ASSERT(!state.BeginStopping());
			TEST_ASSERT(state.IsStopping());
			TEST_ASSERT(!state.TryClaimFailure(L"late"));
			TEST_ASSERT(!state.TryClaimNormal());

			bool normal = false;
			WString message;
			TEST_ASSERT(!state.TryTake(normal, message));
		});
	});
}
