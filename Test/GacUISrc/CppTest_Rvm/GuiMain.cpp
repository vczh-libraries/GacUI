#include "DarkSkin.h"
#include "RemoteViewModelTestIncludes.h"
#include "../RemotingTest_RvmHost/RemoteViewModelTestRuntime.h"
#include "../../../Source/RemotingHelpers/AutomationService/MiniHttpAutomationService.h"
#include "../../../Source/RemotingHelpers/AutomationService/Windows/WindowsAutomationService.Windows.h"
#include "../../../Source/RemotingHelpers/RemotingServer/RemotingChannelServer.h"
#include <VlppOS.Windows.h>
#include <cstdlib>
#include "resource.h"

using namespace vl;
using namespace vl::inter_process;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remoting;
using namespace vl::presentation::remote_view_model_test;
using namespace vl::rpc_controller::channeling;

namespace
{
	struct RvmGuiContext
	{
		RemoteViewModelRequesterSession*						session = nullptr;
		Ptr<rvmt::IViewModel>									viewModel;
		Ptr<async_tcp_socket::IAsyncSocketServer>			miniHttpSocketServer;
	};

	RvmGuiContext* currentGuiContext = nullptr;

	RemotingChannelServerCallbacks CreateCallbacks(RemoteViewModelRequesterSession& session)
	{
		RemotingChannelServerCallbacks callbacks;
		callbacks.canAcceptLocalClient = Func<bool(JsonChannelClient*)>([&session](JsonChannelClient* client)
		{
			return session.CanAcceptLocalClient(client);
		});
		callbacks.isRemoteClient = Func<bool(const JsonChannelClient::ChannelNameList&)>(
			[](const JsonChannelClient::ChannelNameList& channels)
			{
				return IsRemoteViewModelHostChannel(channels);
			});
		callbacks.tryAcceptRemoteClient = Func<bool(vint)>([&session](vint clientId)
		{
			return session.TryAcceptViewModelHost(clientId);
		});
		callbacks.clientDisconnected = Func<void(vint)>([&session](vint clientId)
		{
			session.OnClientDisconnected(clientId);
		});
		return callbacks;
	}
	template<typename TServer, typename ...TArgs>
	int StartServer(
		Ptr<async_tcp_socket::IAsyncSocketServer> miniHttpSocketServer,
		TArgs&& ...args
		)
	{
		auto parser = Ptr(new glr::json::Parser);
		RemoteViewModelRequesterSession session(
			parser,
			Func<void(const WString&)>([](const WString&) { std::_Exit(1); })
			);
		auto callbacks = CreateCallbacks(session);
		RemotingChannelServer<TServer> server(
			parser,
			false,
			callbacks,
			std::forward<TArgs>(args)...
			);
		server.Start();
		session.Start(&server);
		auto viewModel = session.RequestViewModel();
		RvmGuiContext context{ &session, viewModel, miniHttpSocketServer };
		CHECK_ERROR(!currentGuiContext, L"StartServer(...)#The GUI context has already been bound.");
		currentGuiContext = &context;
		auto result = SetupHostedWindowsDirect2DRenderer();
		currentGuiContext = nullptr;
		session.Stop(Func<void()>([&server]()
		{
			server.Stop();
		}));
		return result;
	}
}

void GuiMain()
{
	CHECK_ERROR(currentGuiContext, L"GuiMain()#The RVM GUI context is null.");
	CHECK_ERROR(currentGuiContext->session, L"GuiMain()#The RVM requester session is null.");
	CHECK_ERROR(currentGuiContext->viewModel, L"GuiMain()#The rvmt::IViewModel proxy is null.");

	theme::RegisterTheme(Ptr(new darkskin::Theme));
	auto window = Ptr(new rvmt::MainWindow(currentGuiContext->viewModel));
	window->ForceCalculateSizeImmediately();
	window->MoveToScreenCenter();
	windows::SetWindowDefaultIcon(MAINICON);
	windows::WindowsAutomationServiceHosted automationService;
	GetNativeServiceSubstitution()->Substitute(&automationService, false);
	if (currentGuiContext->miniHttpSocketServer)
	{
		StartMiniHttpAutomationService(
			currentGuiContext->miniHttpSocketServer,
			WString::Unmanaged(L"CppTest_Rvm")
			);
	}
	else
	{
		windows::StartWindowsHttpAutomationService(WString::Unmanaged(L"Automation/CppTest_Rvm"), RemotingHttpPort);
	}

	CHECK_ERROR(
		currentGuiContext->session->BeginRunning(),
		L"GuiMain()#RemotingTest_RvmHost was not available before window startup."
		);
	GetApplication()->Run(window.Obj());
	currentGuiContext->session->BeginStopping();
	if (currentGuiContext->miniHttpSocketServer)
	{
		StopMiniHttpAutomationService();
	}
	else
	{
		windows::StopWindowsHttpAutomationService();
	}
	automationService.Stop();
	GetNativeServiceSubstitution()->Unsubstitute(&automationService);
}

int StartNamedPipeServer()
{
	return StartServer<named_pipe::NamedPipeServer>(
		nullptr,
		WString::Unmanaged(RemotingNamedPipeName)
		);
}

int StartHttpServer()
{
	return StartServer<windows_http::HttpServer>(
		nullptr,
		WString::Unmanaged(RemotingHttpBaseUrl),
		RemotingHttpPort
		);
}

int StartMiniHttpServer()
{
	auto socketServer = async_tcp_socket::CreateDefaultAsyncSocketServer(RemotingHttpPort);
	return StartServer<async_tcp_socket::SocketHttpServer>(
		socketServer,
		socketServer,
		WString::Unmanaged(RemotingHttpBaseUrl)
		);
}
