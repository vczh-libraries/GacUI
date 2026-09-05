#include "TestRemote.h"
#include "../../RemotingHelpers/Rvmt/ViewModelHostServer.h"

using namespace vl;
using namespace vl::inter_process;
using namespace vl::presentation;
using namespace vl::presentation::remoting;
using namespace vl::presentation::remote_view_model_test;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;

namespace remote_channel_server_tests
{
	class TakeoverJsonChannel : public Object, public virtual IJsonChannel
	{
	private:
		WString						channelName = L"Takeover";
		IJsonChannelReader*			reader = nullptr;

	public:
		Func<void()>				duringBatch;

		const WString& GetChannelName() override { return channelName; }
		IJsonChannelReader* GetReader() override { return reader; }
		void Initialize(IJsonChannelReader* value) override { reader = value; }
		void SendToClient(vint, const JsonPackage&) override {}
		void BroadcastFromClient(const JsonPackage&) override {}
		void BroadcastFromClient(const JsonPackage&, const collections::List<vint>&) override {}

		void BatchWrite(bool& disconnected) override
		{
			disconnected = false;
			duringBatch();
		}

		void Respond(vint clientId, vint requestId)
		{
			Ptr<glr::json::JsonObject> package;
			ChannelPackageInfo info{ ChannelPackageSemantic::Response, requestId, L"WindowGetBounds" };
			JsonChannelPack(info, ConvertCustomTypeToJson(WindowSizingConfig{}), package);
			reader->OnRead(clientId, package);
		}
	};

	class TakeoverCoreChannel : public GuiRemoteProtocolCoreChannel
	{
	protected:
		bool IsCorrectRendererClientId(vint clientId) override
		{
			return clientId == transportClientId;
		}

	public:
		vint						transportClientId = 17;

		TakeoverCoreChannel(IJsonChannel* channel)
			: GuiRemoteProtocolCoreChannel(nullptr, channel, L"Takeover")
		{
			SetRendererClientId(transportClientId);
		}

		void Connect(vint clientId)
		{
			transportClientId = clientId;
			SetRendererClientId(clientId);
		}
	};

	class TakeoverEvents : public GuiRemoteEventCombinator_PassingThrough
	{
	public:
		vint						responses = 0;

		void RespondWindowGetBounds(vint, const WindowSizingConfig&) override
		{
			responses++;
		}
	};

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
	TEST_CASE(L"Renderer detachment during a batch cancels outstanding responses")
	{
		TakeoverJsonChannel channel;
		TakeoverCoreChannel core(&channel);
		repeatfiltering::GuiRemoteProtocolFilter filter(&core);
		TakeoverEvents events;
		filter.Initialize(&events);

		channel.duringBatch = [&]()
		{
			core.DetachRenderer(17);
			channel.Respond(17, 42);
		};
		filter.RequestWindowGetBounds(42);
		bool disconnected = false;
		filter.Submit(disconnected);
		TEST_ASSERT(disconnected);
		TEST_ASSERT(events.responses == 0);

		core.Connect(18);
		channel.duringBatch = [&]() { channel.Respond(18, 43); };
		filter.RequestWindowGetBounds(43);
		filter.Submit(disconnected);
		TEST_ASSERT(!disconnected);
		TEST_ASSERT(events.responses == 1);
	});

	TEST_CASE(L"Renderer admission changing during a batch reports disconnection")
	{
		TakeoverJsonChannel channel;
		TakeoverCoreChannel core(&channel);
		repeatfiltering::GuiRemoteProtocolFilter filter(&core);
		TakeoverEvents events;
		filter.Initialize(&events);

		channel.duringBatch = [&]()
		{
			core.transportClientId = 18;
			channel.Respond(17, 42);
		};
		filter.RequestWindowGetBounds(42);
		bool disconnected = false;
		filter.Submit(disconnected);
		TEST_ASSERT(disconnected);
		TEST_ASSERT(events.responses == 0);

		core.Connect(18);
		channel.duringBatch = [&]() { channel.Respond(18, 43); };
		filter.RequestWindowGetBounds(43);
		filter.Submit(disconnected);
		TEST_ASSERT(!disconnected);
		TEST_ASSERT(events.responses == 1);
	});

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
