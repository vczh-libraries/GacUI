#include "DarkSkin.h"
#include "RemoteViewModelTestIncludes.h"
#include "../RemotingTest_RvmHost/RemoteViewModelTestRuntime.h"
#include <cstdlib>

#ifdef VCZH_MSVC
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"
#include <VlppOS.Windows.h>
#include "resource.h"
#endif

using namespace vl;
using namespace vl::console;
using namespace vl::inter_process;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remote_view_model_test;
using namespace vl::rpc_controller::channeling;

extern void StartMiniHttpAutomationService(Ptr<async_tcp_socket::IAsyncSocketServer> socketServer, const WString& applicationName);
extern void StopMiniHttpAutomationService();

namespace
{
	template<typename TServerBase>
	class RemoteViewModelUiServer
		: public JsonNetworkChannelServer<TServerBase>
	{
		using Base = JsonNetworkChannelServer<TServerBase>;

	private:
		RemoteViewModelRequesterSession*			session = nullptr;

	public:
		using Base::OnClientConnected;

		template<typename... TArgs>
		RemoteViewModelUiServer(Ptr<glr::json::Parser> parser, TArgs&&... args)
			: Base(parser, std::forward<TArgs>(args)...)
		{
		}

		void SetSession(RemoteViewModelRequesterSession* _session)
		{
			CHECK_ERROR(_session, L"RemoteViewModelUiServer::SetSession(...)#The requester session is null.");
			CHECK_ERROR(!session, L"RemoteViewModelUiServer::SetSession(...)#The requester session has already been set.");
			session = _session;
		}

		void ClearSession()
		{
			session = nullptr;
		}

		WaitForClientResult OnClientConnected(
			vint clientId,
			const JsonChannelClient::ChannelNameList& availableChannels,
			JsonChannelClient* localClient
			) override
		{
			if (localClient)
			{
				return
					session && session->CanAcceptLocalClient(localClient)
					? WaitForClientResult::Accept
					: WaitForClientResult::Reject;
			}

			if (
				!session ||
				ClassifyRemoteViewModelChannel(availableChannels) != RemoteViewModelChannelRole::ViewModelHost ||
				!session->TryAcceptViewModelHost(clientId)
				)
			{
				return WaitForClientResult::Reject;
			}

			Console::WriteLine(L"> RemotingTest_RvmHost transport connected: " + itow(clientId));
			return WaitForClientResult::Accept;
		}

		void OnClientDisconnected(vint clientId) override
		{
			if (session)
			{
				session->OnClientDisconnected(clientId);
			}
		}
	};

#ifdef VCZH_MSVC
	class NamedPipeRemoteViewModelUiServer
		: public RemoteViewModelUiServer<named_pipe::NamedPipeServer>
	{
		using Base = RemoteViewModelUiServer<named_pipe::NamedPipeServer>;

	public:
		NamedPipeRemoteViewModelUiServer(Ptr<glr::json::Parser> parser)
			: Base(parser, WString::Unmanaged(RemotingNamedPipeName))
		{
		}
	};

	class HttpRemoteViewModelUiServer
		: public RemoteViewModelUiServer<windows_http::HttpServer>
	{
		using Base = RemoteViewModelUiServer<windows_http::HttpServer>;

	public:
		HttpRemoteViewModelUiServer(Ptr<glr::json::Parser> parser)
			: Base(parser, WString::Unmanaged(RemotingHttpBaseUrl), RemotingHttpPort)
		{
		}
	};
#endif

	class MiniHttpRemoteViewModelUiServer
		: public RemoteViewModelUiServer<async_tcp_socket::SocketHttpServer>
	{
		using Base = RemoteViewModelUiServer<async_tcp_socket::SocketHttpServer>;

	public:
		MiniHttpRemoteViewModelUiServer(
			Ptr<glr::json::Parser> parser,
			Ptr<async_tcp_socket::IAsyncSocketServer> socketServer
			)
			: Base(parser, socketServer, WString::Unmanaged(RemotingHttpBaseUrl))
		{
		}
	};

	RemoteViewModelRequesterSession* currentSession = nullptr;
	Ptr<rvmt::IViewModel> currentViewModel;
	int guiProcessResult = 0;
#ifdef VCZH_MSVC
	bool useWindowsHttpAutomationService = true;
#endif
	Ptr<async_tcp_socket::IAsyncSocketServer>* miniHttpAutomationSocketServer = nullptr;

	template<typename TServer>
	int StartServer(
		TServer& channelServer,
		Ptr<glr::json::Parser> parser,
		bool useWindowsAutomation
		)
	{
		RemoteViewModelRequesterSession session(
			parser,
			Func<void(const WString&)>([](const WString& message)
			{
				Console::WriteLine(L"Error: " + message);
				std::_Exit(1);
			})
			);
		bool serverStarted = false;
		int result = 1;

		(void)useWindowsAutomation;
		channelServer.SetSession(&session);
		try
		{
			channelServer.Start();
			serverStarted = true;
			session.Start(&channelServer);

			Console::WriteLine(L"> Waiting for RemotingTest_RvmHost on ViewModelChannel.");
			auto viewModel = session.RequestViewModel();

			currentSession = &session;
			currentViewModel = viewModel;
			guiProcessResult = 0;
#ifdef VCZH_MSVC
			useWindowsHttpAutomationService = useWindowsAutomation;
			result = SetupHostedWindowsDirect2DRenderer();
#else
			result = 0;
#endif
			if (guiProcessResult != 0)
			{
				result = guiProcessResult;
			}
			currentViewModel = nullptr;
			currentSession = nullptr;
		}
		catch (const Exception& e)
		{
			Console::WriteLine(L"Error: " + e.Message());
			result = 1;
		}
		catch (const Error& e)
		{
			Console::WriteLine(L"Error: " + WString::Unmanaged(e.Description()));
			result = 1;
		}

		currentViewModel = nullptr;
		currentSession = nullptr;

		Func<void()> stopServer;
		if (serverStarted)
		{
			stopServer = Func<void()>([&channelServer]()
			{
				channelServer.Stop();
			});
		}
		if (auto failure = session.Stop(stopServer))
		{
			Console::WriteLine(L"Error during RVM cleanup: " + failure.Value());
			result = 1;
		}
		channelServer.ClearSession();
		return result;
	}
}

void GuiMain()
{
	try
	{
		CHECK_ERROR(currentSession, L"GuiMain()#The RVM requester session is null.");
		CHECK_ERROR(currentViewModel, L"GuiMain()#The rvmt::IViewModel proxy is null.");

		theme::RegisterTheme(Ptr(new darkskin::Theme));
		auto window = Ptr(new rvmt::MainWindow(currentViewModel));
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();
#ifdef VCZH_MSVC
		windows::SetWindowDefaultIcon(MAINICON);
#endif

		if (!currentSession->BeginRunning())
		{
			auto failure = currentSession->GetFatalError();
			throw Exception(failure ? failure.Value() : WString::Unmanaged(L"RemotingTest_RvmHost was not available before window startup."));
		}
		Console::WriteLine(L"> rvmt::IViewModel acquired; local window is ready.");

		bool automationStarted = false;
		try
		{
#ifdef VCZH_MSVC
			if (useWindowsHttpAutomationService)
			{
				windows::StartWindowsHttpAutomationService(WString::Unmanaged(L"Automation/CppTest_Rvm"), RemotingHttpPort);
			}
			else
#endif
			{
				StartMiniHttpAutomationService(
					*miniHttpAutomationSocketServer,
					WString::Unmanaged(L"CppTest_Rvm")
					);
			}
			automationStarted = true;
			GetApplication()->Run(window.Obj());
		}
		catch (...)
		{
			currentSession->BeginStopping();
			if (automationStarted)
			{
#ifdef VCZH_MSVC
				if (useWindowsHttpAutomationService)
				{
					windows::StopWindowsHttpAutomationService();
				}
				else
#endif
				{
					StopMiniHttpAutomationService();
				}
			}
			throw;
		}

		currentSession->BeginStopping();
#ifdef VCZH_MSVC
		if (useWindowsHttpAutomationService)
		{
			windows::StopWindowsHttpAutomationService();
		}
		else
#endif
		{
			StopMiniHttpAutomationService();
		}

		if (auto failure = currentSession->GetFatalError())
		{
			throw Exception(failure.Value());
		}
	}
	catch (const Exception& e)
	{
		if (currentSession)
		{
			currentSession->BeginStopping();
		}
		Console::WriteLine(L"Error: " + e.Message());
		guiProcessResult = 1;
	}
	catch (const Error& e)
	{
		if (currentSession)
		{
			currentSession->BeginStopping();
		}
		Console::WriteLine(L"Error: " + WString::Unmanaged(e.Description()));
		guiProcessResult = 1;
	}
}

#ifdef VCZH_MSVC
int StartNamedPipeServer()
{
	Console::WriteLine(L"> Named pipe created, waiting on: " + WString::Unmanaged(RemotingNamedPipeName));
	auto parser = Ptr(new glr::json::Parser);
	NamedPipeRemoteViewModelUiServer channelServer(parser);
	return StartServer(channelServer, parser, true);
}

int StartHttpServer()
{
	Console::WriteLine(L"> HTTP server created, waiting on: http://localhost:" + itow(RemotingHttpPort) + WString::Unmanaged(RemotingHttpBaseUrl));
	auto parser = Ptr(new glr::json::Parser);
	HttpRemoteViewModelUiServer channelServer(parser);
	return StartServer(channelServer, parser, true);
}
#endif

int StartMiniHttpServer()
{
	Console::WriteLine(L"> Mini HTTP server created, waiting on: http://localhost:" + itow(RemotingHttpPort) + WString::Unmanaged(RemotingHttpBaseUrl));
	auto parser = Ptr(new glr::json::Parser);
	auto socketServer = async_tcp_socket::CreateDefaultAsyncSocketServer(RemotingHttpPort);
	MiniHttpRemoteViewModelUiServer channelServer(parser, socketServer);
	miniHttpAutomationSocketServer = &socketServer;
	try
	{
		auto result = StartServer(channelServer, parser, false);
		miniHttpAutomationSocketServer = nullptr;
		return result;
	}
	catch (...)
	{
		miniHttpAutomationSocketServer = nullptr;
		throw;
	}
}
