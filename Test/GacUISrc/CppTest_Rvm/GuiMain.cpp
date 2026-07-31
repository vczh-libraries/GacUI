#include "DarkSkin.h"
#include "RemoteViewModelTestIncludes.h"
#include "../RemotingTest_RvmHost/RemoteViewModelTestRuntime.h"
#include "../../../Source/RemotingHelpers/AutomationService/MiniHttpAutomationService.h"
#include "../../../Source/RemotingHelpers/RemotingServer/RemotingChannelServer.h"
#include <cstdlib>

#ifdef VCZH_MSVC
#include "../../../Source/RemotingHelpers/AutomationService/Windows/WindowsAutomationService.Windows.h"
#include <VlppOS.Windows.h>
#include "resource.h"
#endif

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
		RemotingHostContext									host;
	};

	RvmGuiContext* currentGuiContext = nullptr;

	template<typename TServer>
	class RequesterServerScope
	{
	private:
		TServer&												server;
		RemoteViewModelRequesterSession&						session;

	public:
		RequesterServerScope(TServer& _server, RemoteViewModelRequesterSession& _session)
			: server(_server)
			, session(_session)
		{
			server.Start();
			session.Start(&server);
		}

		~RequesterServerScope()
		{
			session.Stop(Func<void()>([this]()
			{
				server.Stop();
			}));
		}
	};

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
#ifdef VCZH_MSVC
	windows::SetWindowDefaultIcon(MAINICON);
	windows::WindowsAutomationServiceScope automation(
		windows::WindowsAutomationServiceType::Hosted,
		currentGuiContext->host.automationService,
		WString::Unmanaged(L"CppTest_Rvm"),
		RemotingHttpPort,
		currentGuiContext->host.miniHttpSocketServer
		);
#else
	MiniHttpAutomationServiceScope automation(
		currentGuiContext->host.miniHttpSocketServer,
		WString::Unmanaged(L"CppTest_Rvm")
		);
#endif

	CHECK_ERROR(
		currentGuiContext->session->BeginRunning(),
		L"GuiMain()#RemotingTest_RvmHost was not available before window startup."
		);
	GetApplication()->Run(window.Obj());
	currentGuiContext->session->BeginStopping();
	CHECK_ERROR(
		!currentGuiContext->session->GetFatalError(),
		L"GuiMain()#RemotingTest_RvmHost disconnected while the window was running."
		);
}

#ifdef VCZH_MSVC
int StartNamedPipeServer()
{
	auto parser = Ptr(new glr::json::Parser);
	RemoteViewModelRequesterSession session(
		parser,
		Func<void(const WString&)>([](const WString&) { std::_Exit(1); })
		);
	auto boundCallbacks = CreateCallbacks(session);
	RemotingChannelServer<named_pipe::NamedPipeServer> boundServer(
		parser,
		false,
		boundCallbacks,
		WString::Unmanaged(RemotingNamedPipeName)
		);
	RequesterServerScope scope(boundServer, session);
	auto viewModel = session.RequestViewModel();
	RvmGuiContext context{ &session, viewModel, { RemotingAutomationService::WindowsHttp, nullptr } };
	currentGuiContext = &context;
	auto result = SetupHostedWindowsDirect2DRenderer();
	currentGuiContext = nullptr;
	return result;
}

int StartHttpServer()
{
	auto parser = Ptr(new glr::json::Parser);
	RemoteViewModelRequesterSession session(
		parser,
		Func<void(const WString&)>([](const WString&) { std::_Exit(1); })
		);
	auto callbacks = CreateCallbacks(session);
	RemotingChannelServer<windows_http::HttpServer> server(
		parser,
		false,
		callbacks,
		WString::Unmanaged(RemotingHttpBaseUrl),
		RemotingHttpPort
		);
	RequesterServerScope scope(server, session);
	auto viewModel = session.RequestViewModel();
	RvmGuiContext context{ &session, viewModel, { RemotingAutomationService::WindowsHttp, nullptr } };
	currentGuiContext = &context;
	auto result = SetupHostedWindowsDirect2DRenderer();
	currentGuiContext = nullptr;
	return result;
}
#endif

int StartMiniHttpServer()
{
	auto parser = Ptr(new glr::json::Parser);
	auto socketServer = async_tcp_socket::CreateDefaultAsyncSocketServer(RemotingHttpPort);
	RemoteViewModelRequesterSession session(
		parser,
		Func<void(const WString&)>([](const WString&) { std::_Exit(1); })
		);
	auto callbacks = CreateCallbacks(session);
	RemotingChannelServer<async_tcp_socket::SocketHttpServer> server(
		parser,
		false,
		callbacks,
		socketServer,
		WString::Unmanaged(RemotingHttpBaseUrl)
		);
	RequesterServerScope scope(server, session);
	auto viewModel = session.RequestViewModel();
	RvmGuiContext context{ &session, viewModel, { RemotingAutomationService::MiniHttp, socketServer } };
	currentGuiContext = &context;
#ifdef VCZH_MSVC
	auto result = SetupHostedWindowsDirect2DRenderer();
#else
	auto result = 0;
#endif
	currentGuiContext = nullptr;
	return result;
}
