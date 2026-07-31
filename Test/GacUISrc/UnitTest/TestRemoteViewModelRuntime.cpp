#include "../RemotingTest_RvmHost/RemoteViewModelTestShared.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::presentation::remote_view_model_test;

TEST_FILE
{
	TEST_CATEGORY(L"RemoteViewModelHostChannel")
	{
		TEST_CASE(L"Only the exact RPC and control channel pair is accepted")
		{
			List<WString> channels;
			TEST_ASSERT(!IsRemoteViewModelHostChannel(channels));

			channels.Add(WString::Unmanaged(ViewModelChannelName));
			TEST_ASSERT(!IsRemoteViewModelHostChannel(channels));

			channels.Add(WString::Unmanaged(ViewModelReadyChannelName));
			TEST_ASSERT(IsRemoteViewModelHostChannel(channels));

			channels.Clear();
			channels.Add(WString::Unmanaged(ViewModelReadyChannelName));
			channels.Add(WString::Unmanaged(ViewModelChannelName));
			TEST_ASSERT(IsRemoteViewModelHostChannel(channels));

			channels.Clear();
			channels.Add(WString::Unmanaged(ViewModelChannelName));
			channels.Add(WString::Unmanaged(ViewModelChannelName));
			TEST_ASSERT(!IsRemoteViewModelHostChannel(channels));

			channels.Clear();
			channels.Add(WString::Unmanaged(ViewModelReadyChannelName));
			channels.Add(WString::Unmanaged(ViewModelReadyChannelName));
			TEST_ASSERT(!IsRemoteViewModelHostChannel(channels));

			channels.Clear();
			channels.Add(WString::Unmanaged(ViewModelChannelName));
			channels.Add(WString::Unmanaged(ViewModelReadyChannelName));
			channels.Add(L"Unknown");
			TEST_ASSERT(!IsRemoteViewModelHostChannel(channels));
		});
	});
}
