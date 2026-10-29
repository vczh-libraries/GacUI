#include "../../RemotingHelpers/Rvmt/ViewModelHostClient.h"
#include "../Generated_RemoteViewModelTest/RemoteViewModelTestInitialize.h"
#include "RemoteViewModelTestRpc.h"
#include <VlppOS.h>
#if defined VCZH_MSVC
#include <VlppOS.Windows.h>
#include <crtdbg.h>
#endif
#include "../SharedArguments.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::console;
using namespace vl::inter_process;
using namespace vl::inter_process::stdio_redirection;
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

int RunHost(Ptr<INetworkProtocolClient> networkClient, bool showBanner = true)
{
	auto parser = Ptr(new glr::json::Parser);
	auto taskQueue = Ptr(new TaskQueue);
	auto channelClient = Ptr(new presentation::remoting::ViewModelHostClient(
		networkClient,
		parser,
		taskQueue
		));
	auto service = Ptr(new ViewModel);
	auto dispatcher = channelClient->GetDispatcher();

	channelClient->Connect();
	RemoteViewModelTestInitialize::InitializeRpc(dispatcher, channelClient->GetClientId());
	auto lifecycle = dispatcher->GetRpcLifecycle();
	auto typeId = lifecycle->GetTypeIdFromName(L"rvmt::IViewModel");
	CHECK_ERROR(typeId != RpcTypeId_NotFound, L"RunHost(Ptr<INetworkProtocolClient>)#Failed to find the rvmt::IViewModel type ID.");
	lifecycle->RegisterLocalService(typeId, service);
	channelClient->SendReady();
	dispatcher->Initialize();

	if (showBanner)
	{
		Console::WriteLine(L"> RemotingTest_RvmHost declared rvmt::IViewModel.");
	}
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
	gacui_test::CoreHostArguments coreHost;
	WString transport;
	for (int i = 1; i < argc; i++)
	{
		auto argument = u8tow(U8String::Unmanaged((const char8_t*)argv[i]));
		auto consumed = coreHost.Consume(argument);
		if (consumed < 0) return 1;
		if (consumed > 0) continue;
		if (transport.Length() != 0) return 1;
		bool supported = argument == L"/MiniHttp" || argument == L"/Cli";
#if defined VCZH_MSVC
		supported |= argument == L"/Pipe" || argument == L"/Http";
#endif
		if (!supported) return 1;
		transport = argument;
	}
	if (transport.Length() == 0 || (transport == L"/Cli" && coreHost.specified)) return 1;

	int result = 1;

#if defined VCZH_MSVC
	if (transport == L"/Pipe")
	{
		if (coreHost.host.IndexOf(L':') != -1) return 1;
		result = RunHost(Ptr(new named_pipe::NamedPipeClient(coreHost.host, WString::Unmanaged(RemotingNamedPipeName))));
	}
	else if (transport == L"/Http")
	{
		result = RunHost(Ptr(new windows_http::HttpClient(coreHost.host, WString::Unmanaged(RemotingHttpBaseUrl), RemotingHttpPort)));
	}
#endif
	if (transport == L"/MiniHttp")
	{
		auto socketClient = async_tcp_socket::CreateDefaultAsyncSocketClient(coreHost.host, RemotingHttpPort);
		result = RunHost(Ptr(new async_tcp_socket::SocketHttpClient(
			socketClient,
			coreHost.host,
			WString::Unmanaged(RemotingHttpBaseUrl)
			)));
	}
	else if (transport == L"/Cli")
	{
		result = RunHost(Ptr(new StdioRedirectionClient), false);
	}

#if defined VCZH_MSVC && VCZH_CHECK_MEMORY_LEAKS
	_CrtDumpMemoryLeaks();
#endif
	return result;
}
