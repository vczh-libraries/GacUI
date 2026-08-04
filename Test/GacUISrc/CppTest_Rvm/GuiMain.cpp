#include "DarkSkin.h"
#include "RemoteViewModelTestIncludes.h"
#include "../RemotingTest_RvmHost/RemoteViewModelTestRuntime.h"
#include "../../../Source/RemotingHelpers/AutomationService/MiniHttpAutomationService.h"
#include "../../../Source/RemotingHelpers/AutomationService/Windows/WindowsAutomationService.Windows.h"
#include <VlppOS.Windows.h>
#include "resource.h"

using namespace vl;
using namespace vl::inter_process;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remoting;
using namespace vl::presentation::remote_view_model_test;

struct RvmGuiContext
{
	RemoteViewModelRequesterSession*						session = nullptr;
	Ptr<rvmt::IViewModel>									viewModel;
	Ptr<async_tcp_socket::IAsyncSocketServer>			miniHttpSocketServer;
};

RvmGuiContext* currentGuiContext = nullptr;

template<typename TServerBase>
int StartServer(
	RemoteViewModelChannelServer<TServerBase>& server,
	Ptr<async_tcp_socket::IAsyncSocketServer> miniHttpSocketServer
	)
{
	auto session = server.GetSession();
	server.Start();
	session->Start(&server);
	auto viewModel = session->RequestViewModel();
	RvmGuiContext context{ session, viewModel, miniHttpSocketServer };
	CHECK_ERROR(!currentGuiContext, L"StartServer(...)#The GUI context has already been bound.");
	currentGuiContext = &context;
	auto result = SetupHostedWindowsDirect2DRenderer();
	currentGuiContext = nullptr;
	session->Stop(Func<void()>([&server]()
	{
		server.Stop();
	}));
	return result;
}

void GuiMain()
{
	CHECK_ERROR(currentGuiContext, L"GuiMain()#The RVM GUI context is null.");
	CHECK_ERROR(currentGuiContext->session, L"GuiMain()#The RVM requester session is null.");
	CHECK_ERROR(currentGuiContext->viewModel, L"GuiMain()#The rvmt::IViewModel proxy is null.");

	theme::RegisterTheme(Ptr(new darkskin::Theme));
	rvmt::MainWindow window(currentGuiContext->viewModel);
	window.ForceCalculateSizeImmediately();
	window.MoveToScreenCenter();
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
	GetApplication()->Run(&window);
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
	auto parser = Ptr(new glr::json::Parser);
	RemoteViewModelChannelServer<named_pipe::NamedPipeServer> server(
		parser,
		false,
		WString::Unmanaged(RemotingNamedPipeName)
		);
	return StartServer(server, nullptr);
}

int StartHttpServer()
{
	auto parser = Ptr(new glr::json::Parser);
	RemoteViewModelChannelServer<windows_http::HttpServer> server(
		parser,
		false,
		WString::Unmanaged(RemotingHttpBaseUrl),
		RemotingHttpPort
		);
	return StartServer(server, nullptr);
}

int StartMiniHttpServer()
{
	auto parser = Ptr(new glr::json::Parser);
	auto socketServer = async_tcp_socket::CreateDefaultAsyncSocketServer(RemotingHttpPort);
	RemoteViewModelChannelServer<async_tcp_socket::SocketHttpServer> server(
		parser,
		false,
		socketServer,
		WString::Unmanaged(RemotingHttpBaseUrl)
		);
	return StartServer(server, socketServer);
}
