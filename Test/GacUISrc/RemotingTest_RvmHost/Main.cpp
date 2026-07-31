#include "RemoteViewModelTestRuntime.h"
#ifdef VCZH_MSVC
#include <VlppOS.Windows.h>
#include <crtdbg.h>
#endif

using namespace vl;
using namespace vl::collections;
using namespace vl::console;
using namespace vl::inter_process;
using namespace vl::rpc_controller;
using namespace vl::rpc_controller::channeling;
using namespace vl::presentation::remote_view_model_test;

namespace
{
	class ViewModel : public Object, public virtual rvmt::IViewModel
	{
	public:
		WString Translate(const WString& name) override
		{
			return WString::Unmanaged(L"Hello, ") + name + WString::Unmanaged(L"!");
		}
	};

	int RunHost(Ptr<INetworkProtocolClient> networkClient)
	{
		auto parser = Ptr(new glr::json::Parser);
		auto taskQueue = Ptr(new TaskQueue);
		auto channelClient = Ptr(new RemoteViewModelHostingClient(
			networkClient,
			parser,
			taskQueue
			));
		auto service = Ptr(new ViewModel);
		auto dispatcher = channelClient->GetDispatcher();

		channelClient->Connect();
		auto lifecycle = dispatcher->GetRpcLifecycle();
		auto typeId = lifecycle->GetTypeIdFromName(WString::Unmanaged(ViewModelServiceName));
		CHECK_ERROR(typeId != RpcTypeId_NotFound, L"RunHost(Ptr<INetworkProtocolClient>)#Failed to find the rvmt::IViewModel type ID.");
		lifecycle->RegisterLocalService(typeId, service);
		channelClient->SendReady();
		dispatcher->Initialize();
		channelClient->StartHeartbeat();

		Console::WriteLine(L"> RemotingTest_RvmHost declared rvmt::IViewModel.");
		taskQueue->RunTaskQueue();
		return 0;
	}

	enum class Transport
	{
		None,
		Pipe,
		Http,
		MiniHttp,
	};

	bool ParseTransport(int argc, char* argv[], Transport& transport)
	{
		if (argc != 2)
		{
			Console::WriteLine(L"Error: exactly one of /Pipe, /Http or /MiniHttp must be provided.");
			return false;
		}

		if (strcmp(argv[1], "/Pipe") == 0)
		{
			transport = Transport::Pipe;
		}
		else if (strcmp(argv[1], "/Http") == 0)
		{
			transport = Transport::Http;
		}
		else if (strcmp(argv[1], "/MiniHttp") == 0)
		{
			transport = Transport::MiniHttp;
		}
		else
		{
			Console::WriteLine(L"Error: unknown command line argument.");
			return false;
		}
		return true;
	}
}

int main(int argc, char* argv[])
{
#ifdef VCZH_MSVC
	_set_abort_behavior(0, _WRITE_ABORT_MSG);
#endif
	Transport transport = Transport::None;
	if (!ParseTransport(argc, argv, transport))
	{
		return 1;
	}

	int result = 1;
	switch (transport)
	{
	case Transport::Pipe:
#ifdef VCZH_MSVC
		result = RunHost(Ptr(new named_pipe::NamedPipeClient(WString::Unmanaged(RemotingNamedPipeName))));
#else
		Console::WriteLine(L"Error: /Pipe is only supported on Windows.");
#endif
		break;
	case Transport::Http:
#ifdef VCZH_MSVC
		result = RunHost(Ptr(new windows_http::HttpClient(WString::Unmanaged(RemotingHttpBaseUrl), RemotingHttpPort)));
#else
		Console::WriteLine(L"Error: /Http is only supported on Windows.");
#endif
		break;
	case Transport::MiniHttp:
		{
			auto socketClient = async_tcp_socket::CreateDefaultAsyncSocketClient(RemotingHttpPort);
			result = RunHost(Ptr(new async_tcp_socket::SocketHttpClient(
				socketClient,
				WString::Unmanaged(L"localhost"),
				WString::Unmanaged(RemotingHttpBaseUrl)
				)));
		}
		break;
	default:
		break;
	}

#if defined VCZH_MSVC && VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
