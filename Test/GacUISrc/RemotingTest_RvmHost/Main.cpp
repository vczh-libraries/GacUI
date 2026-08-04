#include "RemoteViewModelTestShared.h"
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

constexpr const wchar_t* RemotingNamedPipeName = L"GacUIRemoteProtocolNamedPipe";
constexpr const wchar_t* RemotingHttpBaseUrl = L"/GacUIRemoteProtocolHttp";
constexpr vint RemotingHttpPort = 8888;

class ViewModel : public Object, public virtual rvmt::IViewModel
{
public:
	WString Translate(const WString& name) override
	{
		return WString::Unmanaged(L"Hello, ") + name + WString::Unmanaged(L"!");
	}
};

class RemoteViewModelHostClient : public presentation::remoting::ViewModelHostClient
{
public:
	RemoteViewModelHostClient(
		Ptr<inter_process::INetworkProtocolClient> networkClient,
		Ptr<glr::json::Parser> parser,
		Ptr<rpc_controller::channeling::TaskQueue> taskQueue
	) : presentation::remoting::ViewModelHostClient(
		networkClient,
		CreateDispatcherFactory(),
		parser,
		taskQueue
	)
	{}
};

int RunHost(Ptr<INetworkProtocolClient> networkClient)
{
	auto parser = Ptr(new glr::json::Parser);
	auto taskQueue = Ptr(new TaskQueue);
	auto channelClient = Ptr(new RemoteViewModelHostClient(
		networkClient,
		parser,
		taskQueue
		));
	auto service = Ptr(new ViewModel);
	auto dispatcher = channelClient->GetDispatcher();

	channelClient->Connect();
	auto lifecycle = dispatcher->GetRpcLifecycle();
	auto typeId = lifecycle->GetTypeIdFromName(WString::Unmanaged(presentation::remoting::ViewModelServiceName));
	CHECK_ERROR(typeId != RpcTypeId_NotFound, L"RunHost(Ptr<INetworkProtocolClient>)#Failed to find the rvmt::IViewModel type ID.");
	lifecycle->RegisterLocalService(typeId, service);
	channelClient->SendReady();
	dispatcher->Initialize();

	Console::WriteLine(L"> RemotingTest_RvmHost declared rvmt::IViewModel.");
	taskQueue->RunTaskQueue();
	return 0;
}

void GuiMain()
{
	// This function would not be needed if GacUI source files are not included in this project.
	// Doing this reduces complexity of project dependencies in this solution.
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		return 1;
	}

	int result = 1;

#ifdef VCZH_MSVC
	if (strcmp(argv[1], "/Pipe") == 0)
	{
		result = RunHost(Ptr(new named_pipe::NamedPipeClient(WString::Unmanaged(RemotingNamedPipeName))));
	}
	else if (strcmp(argv[1], "/Http") == 0)
	{
		result = RunHost(Ptr(new windows_http::HttpClient(WString::Unmanaged(RemotingHttpBaseUrl), RemotingHttpPort)));
	}
	else
#endif
	if (strcmp(argv[1], "/MiniHttp") == 0)
	{
		auto socketClient = async_tcp_socket::CreateDefaultAsyncSocketClient(RemotingHttpPort);
		result = RunHost(Ptr(new async_tcp_socket::SocketHttpClient(
			socketClient,
			WString::Unmanaged(L"localhost"),
			WString::Unmanaged(RemotingHttpBaseUrl)
			)));
	}

#if defined VCZH_MSVC && VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
