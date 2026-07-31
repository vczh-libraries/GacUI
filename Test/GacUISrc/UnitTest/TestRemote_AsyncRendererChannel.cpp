#include "TestRemote.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::glr;
using namespace vl::presentation::remoteprotocol::channeling;

namespace remote_async_renderer_channel_tests
{
	class FakeJsonChannel : public Object, public virtual IJsonChannel
	{
	protected:
		WString						channelName = L"FakeJsonChannel";
		IJsonChannelReader*				reader = nullptr;

	public:
		const WString& GetChannelName() override
		{
			return channelName;
		}

		IJsonChannelReader* GetReader() override
		{
			return reader;
		}

		void Initialize(IJsonChannelReader* _reader) override
		{
			CHECK_ERROR(_reader, L"FakeJsonChannel::Initialize(IJsonChannelReader*)#The reader must not be null.");
			CHECK_ERROR(!reader, L"FakeJsonChannel::Initialize(IJsonChannelReader*)#The reader has already been initialized.");
			reader = _reader;
		}

		void SendToClient(vint, const JsonPackage&) override
		{
		}

		void BroadcastFromClient(const JsonPackage&) override
		{
		}

		void BroadcastFromClient(const JsonPackage&, const List<vint>&) override
		{
		}

		void BatchWrite(bool& disconnected) override
		{
			disconnected = false;
		}

		void Push(vint senderClientId, const JsonPackage& package)
		{
			CHECK_ERROR(reader, L"FakeJsonChannel::Push(vint, const JsonPackage&)#The reader has not been initialized.");
			reader->OnRead(senderClientId, package);
		}
	};

	class RecordingJsonReader : public Object, public virtual IJsonChannelReader
	{
	protected:
		List<WString>&					log;

	public:
		RecordingJsonReader(List<WString>& _log)
			: log(_log)
		{
		}

		void OnRead(vint, const JsonPackage& package) override
		{
			log.Add(package.Cast<json::JsonString>()->content.value);
		}
	};

	class ManualInvoker : public Object, public virtual IGuiRemoteProtocolAsyncRendererInvoker
	{
	public:
		List<Func<void()>>				tasks;

		void InvokeInMainThread(const Func<void()>& proc) override
		{
			tasks.Add(proc);
		}

		void RunAll()
		{
			while (tasks.Count() > 0)
			{
				auto task = tasks[0];
				tasks.RemoveAt(0);
				task();
			}
		}
	};

	JsonPackage CreatePackage(const WString& value)
	{
		auto package = Ptr(new json::JsonString);
		package->content.value = value;
		return package;
	}
}
using namespace remote_async_renderer_channel_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Remote Async Renderer Channel")
	{
		TEST_CASE(L"Cached startup messages and tasks preserve FIFO order")
		{
			FakeJsonChannel channel;
			GuiRemoteProtocolAsyncJsonChannelRenderer asyncChannel(&channel);
			List<WString> log;
			RecordingJsonReader reader(log);
			auto invoker = Ptr(new ManualInvoker);

			asyncChannel.Initialize(&reader);
			channel.Push(1, CreatePackage(L"Package 1"));
			asyncChannel.QueueMainThreadTask([&]()
			{
				log.Add(L"Task 1");
			});
			channel.Push(1, CreatePackage(L"Package 2"));

			TEST_ASSERT(log.Count() == 0);
			asyncChannel.SetInvokeInMainThread(invoker);
			TEST_ASSERT(invoker->tasks.Count() == 1);

			asyncChannel.ProcessPendingMessages();
			TEST_ASSERT(log.Count() == 3);
			TEST_ASSERT(log[0] == L"Package 1");
			TEST_ASSERT(log[1] == L"Task 1");
			TEST_ASSERT(log[2] == L"Package 2");

			invoker->RunAll();
			TEST_ASSERT(log.Count() == 3);
			asyncChannel.Detach();
		});

		TEST_CASE(L"Detach cancels an already scheduled batch")
		{
			FakeJsonChannel channel;
			GuiRemoteProtocolAsyncJsonChannelRenderer asyncChannel(&channel);
			List<WString> log;
			RecordingJsonReader reader(log);
			auto invoker = Ptr(new ManualInvoker);

			asyncChannel.Initialize(&reader);
			asyncChannel.SetInvokeInMainThread(invoker);
			channel.Push(1, CreatePackage(L"Package 1"));
			TEST_ASSERT(invoker->tasks.Count() == 1);

			asyncChannel.Detach();
			invoker->RunAll();
			TEST_ASSERT(log.Count() == 0);

			channel.Push(1, CreatePackage(L"Package 2"));
			asyncChannel.QueueMainThreadTask([&]()
			{
				log.Add(L"Task 1");
			});
			TEST_ASSERT(invoker->tasks.Count() == 0);
			TEST_ASSERT(log.Count() == 0);
		});

		TEST_CASE(L"Detach in a task cancels the rest of the same batch")
		{
			FakeJsonChannel channel;
			GuiRemoteProtocolAsyncJsonChannelRenderer asyncChannel(&channel);
			List<WString> log;
			RecordingJsonReader reader(log);

			asyncChannel.Initialize(&reader);
			channel.Push(1, CreatePackage(L"Package 1"));
			asyncChannel.QueueMainThreadTask([&]()
			{
				log.Add(L"Task 1");
				asyncChannel.Detach();
			});
			channel.Push(1, CreatePackage(L"Package 2"));

			asyncChannel.ProcessPendingMessages();
			TEST_ASSERT(log.Count() == 2);
			TEST_ASSERT(log[0] == L"Package 1");
			TEST_ASSERT(log[1] == L"Task 1");
		});

		TEST_CASE(L"Reentrant draining cannot overtake the active batch")
		{
			FakeJsonChannel channel;
			GuiRemoteProtocolAsyncJsonChannelRenderer asyncChannel(&channel);
			List<WString> log;
			RecordingJsonReader reader(log);

			asyncChannel.Initialize(&reader);
			asyncChannel.QueueMainThreadTask([&]()
			{
				log.Add(L"Task Begin");
				channel.Push(1, CreatePackage(L"Package 1"));
				asyncChannel.ProcessPendingMessages();
				log.Add(L"Task End");
			});

			asyncChannel.ProcessPendingMessages();
			TEST_ASSERT(log.Count() == 3);
			TEST_ASSERT(log[0] == L"Task Begin");
			TEST_ASSERT(log[1] == L"Task End");
			TEST_ASSERT(log[2] == L"Package 1");
		});
	});
}
