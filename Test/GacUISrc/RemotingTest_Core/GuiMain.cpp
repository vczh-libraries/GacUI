#include "DarkSkin.h"
#include "RpMainWindow.h"
#include "MainWindow.h"
#include "RemoteViewModelTestIncludes.h"
#include "../RemotingTest_RvmHost/RemoteViewModelTestRuntime.h"
#include "../../../Source/RemotingHelpers/AutomationService/MiniHttpAutomationService.h"
#include "../../../Source/RemotingHelpers/RemotingServer/RemotingChannelServer.h"
#include "../../../Source/Utilities/SharedServices/GuiSharedAutomationService_Controls.h"
#ifdef VCZH_MSVC
#include <VlppOS.Windows.h>
#include "../../../Source/RemotingHelpers/AutomationService/Windows/WindowsAutomationService.Windows.h"
#endif
#include <cstdlib>

using namespace vl;
using namespace vl::inter_process;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remoting;
using namespace vl::presentation::remote_view_model_test;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remoteprotocol::repeatfiltering;

namespace
{
	class CoreFatalState
	{
	private:
		SpinLock											lockState;
		EventObject											delivered;
		bool												claimed = false;
		bool												deliveryFinished = false;
		WString												message;

	public:
		CoreFatalState()
		{
			CHECK_ERROR(delivered.CreateManualUnsignal(false), L"CoreFatalState::CoreFatalState()#Failed to create the fatal-delivery event.");
		}

		WString Deliver(IJsonLocalChannelServer* server, const WString& errorMessage)
		{
			bool deliver = false;
			bool wait = false;
			SPIN_LOCK(lockState)
			{
				if (!claimed)
				{
					claimed = true;
					message = errorMessage;
					deliver = true;
				}
				else
				{
					wait = !deliveryFinished;
				}
			}

			if (deliver)
			{
				if (server)
				{
					try
					{
						server->BroadcastError(message);
					}
					catch (...)
					{
					}
				}
				SPIN_LOCK(lockState)
				{
					deliveryFinished = true;
				}
				delivered.Signal();
			}
			else if (wait)
			{
				delivered.Wait();
			}
			return message;
		}
	};

	struct CoreGuiContext
	{
		vint												mainWindowConstructorIndex = 0;
		RemoteViewModelRequesterSession*						session = nullptr;
		Ptr<rvmt::IViewModel>									viewModel;
		RemotingHostContext									host;
	};

	CoreGuiContext* currentGuiContext = nullptr;

	RemotingChannelServerCallbacks CreateCallbacks(RemoteViewModelRequesterSession* session)
	{
		RemotingChannelServerCallbacks callbacks;
		if (session)
		{
			callbacks.canAcceptLocalClient = Func<bool(JsonChannelClient*)>([session](JsonChannelClient* client)
			{
				return session->CanAcceptLocalClient(client);
			});
			callbacks.isRemoteClient = Func<bool(const JsonChannelClient::ChannelNameList&)>(
				[](const JsonChannelClient::ChannelNameList& channels)
				{
					return IsRemoteViewModelHostChannel(channels);
				});
			callbacks.tryAcceptRemoteClient = Func<bool(vint)>([session](vint clientId)
			{
				return session->TryAcceptViewModelHost(clientId);
			});
			callbacks.clientDisconnected = Func<void(vint)>([session](vint clientId)
			{
				session->OnClientDisconnected(clientId);
			});
			callbacks.canAdmitRenderer = Func<bool()>([session]()
			{
				return session->CanAdmitRenderer();
			});
		}
		return callbacks;
	}

	template<typename TServer>
	int RunServer(
		TServer& channelServer,
		Ptr<glr::json::Parser> jsonParser,
		vint mainWindowConstructorIndex,
		RemotingHostContext host,
		RemoteViewModelRequesterSession* requesterSession
		)
	{
		channelServer.Start();

		auto coreClient = Ptr(new GuiRemoteProtocolLocalChannelClient(jsonParser));
		auto coreClientId = channelServer.ConnectLocalClient(coreClient);
		CHECK_ERROR(coreClientId == GacUIRemoteProtocolCoreClientId, L"RunServer(...)#Failed to register the core channel client.");

		GuiRemoteProtocolAsyncJsonChannel asyncChannelSender(coreClient->GetProtocolChannel());
		RemotingCoreChannel channelSender(
			coreClient.Obj(),
			&asyncChannelSender,
			WString::Unmanaged(L"RemotingTest_Core.vcxproj"),
			asyncChannelSender.GetRemoteEventProcessor(),
			Func<vint()>([&channelServer]()
			{
				return channelServer.GetRendererClientId();
			})
			);
		GuiRemoteProtocolFilter filteredProtocol(&channelSender);
		GuiRemoteProtocolDomDiffConverter diffConverterProtocol(&filteredProtocol);
		channelServer.SetCoreChannels(coreClient->GetProtocolChannel(), &channelSender);

		Ptr<rvmt::IViewModel> viewModel;
		if (requesterSession)
		{
			requesterSession->Start(&channelServer);
			viewModel = requesterSession->RequestViewModel();
		}

		CoreGuiContext context{ mainWindowConstructorIndex, requesterSession, viewModel, host };
		CHECK_ERROR(!currentGuiContext, L"RunServer(...)#The GUI context has already been bound.");
		currentGuiContext = &context;
		SetupRemoteNativeController(&diffConverterProtocol);
		currentGuiContext = nullptr;

		channelServer.ClearCoreChannels();
		auto rendererClientId = channelServer.GetRendererClientId();
		if (rendererClientId != -1)
		{
			channelServer.DisconnectClient(rendererClientId);
		}
		if (requesterSession)
		{
			requesterSession->Stop(Func<void()>([&channelServer]()
			{
				channelServer.Stop();
			}));
		}
		else
		{
			channelServer.Stop();
		}
		return 0;
	}

	template<typename TServer>
	int StartConfiguredServer(
		Ptr<glr::json::Parser> parser,
		vint index,
		RemotingHostContext host,
		const Func<Ptr<TServer>(const RemotingChannelServerCallbacks&)>& createServer
		)
	{
		const bool useRemoteViewModel = index == 2;
		CoreFatalState fatalState;
		Ptr<RemoteViewModelRequesterSession> requesterSession;
		TServer* serverForTerminal = nullptr;
		if (useRemoteViewModel)
		{
			requesterSession = Ptr(new RemoteViewModelRequesterSession(
				parser,
				Func<void(const WString&)>([&fatalState, &serverForTerminal](const WString& message)
				{
					fatalState.Deliver(serverForTerminal, message);
					std::_Exit(1);
				})
				));
		}
		auto server = createServer(CreateCallbacks(requesterSession.Obj()));
		serverForTerminal = server.Obj();
		return RunServer(*server.Obj(), parser, index, host, requesterSession.Obj());
	}
}

void GuiMain()
{
	CHECK_ERROR(currentGuiContext, L"GuiMain()#The Core GUI context is null.");
	theme::RegisterTheme(Ptr(new darkskin::Theme));
	Ptr<GuiWindow> window;
	switch (currentGuiContext->mainWindowConstructorIndex)
	{
	case 1:
		window = Ptr(new rptest::RpMainWindow);
		break;
	case 2:
		CHECK_ERROR(currentGuiContext->session, L"GuiMain()#The RVM requester session is null.");
		CHECK_ERROR(currentGuiContext->viewModel, L"GuiMain()#The rvmt::IViewModel proxy is null.");
		window = Ptr(new rvmt::MainWindow(currentGuiContext->viewModel));
		break;
	default:
	case 0:
		window = Ptr(new demo::MainWindow);
	}
	window->ForceCalculateSizeImmediately();

	RemoteProtocolAutomationService automationService;
	NativeAutomationServiceScope substitution(&automationService);
#ifdef VCZH_MSVC
	windows::AutomationServiceEndpointScope endpoint(
		currentGuiContext->host.automationService,
		WString::Unmanaged(L"RemotingTest_Core"),
		RemotingHttpPort,
		currentGuiContext->host.miniHttpSocketServer
		);
#else
	MiniHttpAutomationServiceScope endpoint(
		currentGuiContext->host.miniHttpSocketServer,
		WString::Unmanaged(L"RemotingTest_Core")
		);
#endif

	if (currentGuiContext->mainWindowConstructorIndex == 2)
	{
		CHECK_ERROR(
			currentGuiContext->session->BeginRunning(),
			L"GuiMain()#RemotingTest_RvmHost was not available before window startup."
			);
	}
	GetApplication()->Run(window.Obj());
	if (currentGuiContext->mainWindowConstructorIndex == 2)
	{
		currentGuiContext->session->BeginStopping();
		CHECK_ERROR(
			!currentGuiContext->session->GetFatalError(),
			L"GuiMain()#RemotingTest_RvmHost disconnected while the window was running."
			);
	}
}

#ifdef VCZH_MSVC
int StartNamedPipeServer(vint index)
{
	using Server = RemotingChannelServer<named_pipe::NamedPipeServer>;
	auto parser = Ptr(new glr::json::Parser);
	return StartConfiguredServer<Server>(
		parser,
		index,
		{ RemotingAutomationService::WindowsHttp, nullptr },
		Func<Ptr<Server>(const RemotingChannelServerCallbacks&)>(
			[parser](const RemotingChannelServerCallbacks& callbacks)
			{
				return Ptr(new Server(
					parser,
					true,
					callbacks,
					WString::Unmanaged(RemotingNamedPipeName)
					));
			})
		);
}

int StartHttpServer(vint index)
{
	using Server = RemotingChannelServer<windows_http::HttpServer>;
	auto parser = Ptr(new glr::json::Parser);
	return StartConfiguredServer<Server>(
		parser,
		index,
		{ RemotingAutomationService::WindowsHttp, nullptr },
		Func<Ptr<Server>(const RemotingChannelServerCallbacks&)>(
			[parser](const RemotingChannelServerCallbacks& callbacks)
			{
				return Ptr(new Server(
					parser,
					true,
					callbacks,
					WString::Unmanaged(RemotingHttpBaseUrl),
					RemotingHttpPort
					));
			})
		);
}
#endif

int StartMiniHttpServer(vint index)
{
	using Server = RemotingChannelServer<async_tcp_socket::SocketHttpServer>;
	auto parser = Ptr(new glr::json::Parser);
	auto socketServer = async_tcp_socket::CreateDefaultAsyncSocketServer(RemotingHttpPort);
	return StartConfiguredServer<Server>(
		parser,
		index,
		{ RemotingAutomationService::MiniHttp, socketServer },
		Func<Ptr<Server>(const RemotingChannelServerCallbacks&)>(
			[parser, socketServer](const RemotingChannelServerCallbacks& callbacks)
			{
				return Ptr(new Server(
					parser,
					true,
					callbacks,
					socketServer,
					WString::Unmanaged(RemotingHttpBaseUrl)
					));
			})
		);
}
