#include "TestRemote.h"
#include "../../RemotingHelpers/Rvmt/ViewModelHostServer.h"

using namespace vl;
using namespace vl::inter_process;
using namespace vl::presentation;
using namespace vl::presentation::remoting;
using namespace vl::presentation::remote_view_model_test;

namespace remote_channel_server_tests
{
	class TestNetworkProtocolServer : public virtual INetworkProtocolServer
	{
	private:
		bool									stopped = false;

	public:
		void Start() override
		{
			stopped = false;
		}

		void Stop() override
		{
			stopped = true;
		}

		bool IsStopped() override
		{
			return stopped;
		}
	};

	class TestRendererAdmission
	{
	public:
		bool									running = false;

		bool CanAdmitRenderer()
		{
			return running;
		}
	};

	class TestRendererChannelServer
		: public RemoteViewModelRendererChannelServer<TestNetworkProtocolServer, TestRendererAdmission>
	{
		using Base = RemoteViewModelRendererChannelServer<TestNetworkProtocolServer, TestRendererAdmission>;

	public:
		TestRendererChannelServer(
			Ptr<glr::json::Parser> parser,
			TestRendererAdmission* admission
			)
			: Base(parser)
		{
			SetRvmChannelServer(admission);
		}

		WaitForClientResult AdmitRenderer(
			vint clientId,
			const JsonChannelClient::ChannelNameList& availableChannels
			)
		{
			return OnRemoteClientConnected(clientId, availableChannels);
		}
	};
}
using namespace remote_channel_server_tests;

TEST_FILE
{
	TEST_CASE(L"The split RVM renderer server gates admission and preserves replacement")
	{
		auto parser = Ptr(new glr::json::Parser);
		TestRendererAdmission admission;
		TestRendererChannelServer server(parser, &admission);
		JsonChannelClient::ChannelNameList rendererChannels;
		rendererChannels.Add(WString::Unmanaged(remoteprotocol::channeling::GacUIRemoteProtocolChannelName));

		TEST_ASSERT(server.AdmitRenderer(2, rendererChannels) == WaitForClientResult::Reject);
		TEST_ASSERT(server.AdmitRenderer(3, rendererChannels) == WaitForClientResult::Reject);
		TEST_ASSERT(server.GetRendererClientId() == -1);

		admission.running = true;
		TEST_ASSERT(server.AdmitRenderer(4, rendererChannels) == WaitForClientResult::Accept);
		TEST_ASSERT(server.GetRendererClientId() == 4);
		TEST_ASSERT(server.AdmitRenderer(5, rendererChannels) == WaitForClientResult::Accept);
		TEST_ASSERT(server.GetRendererClientId() == 5);
	});
}
