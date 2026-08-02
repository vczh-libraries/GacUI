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
	struct CoreGuiContext
	{
		vint												mainWindowConstructorIndex = 0;
		RemoteViewModelRequesterSession*					session = nullptr;
		Ptr<rvmt::IViewModel>								viewModel;
		Ptr<async_tcp_socket::IAsyncSocketServer>			miniHttpSocketServer;
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

	template<typename TServer, typename ...TArgs>
	int StartServer(
		vint mainWindowConstructorIndex,
		Ptr<async_tcp_socket::IAsyncSocketServer> miniHttpSocketServer,
		TArgs&& ...args
		)
	{
		auto jsonParser = Ptr(new glr::json::Parser);
		Ptr<RemoteViewModelRequesterSession> requesterSession;
		RemotingChannelServer<TServer>* serverForTerminal = nullptr;
		if (mainWindowConstructorIndex == 2)
		{
			requesterSession = Ptr(new RemoteViewModelRequesterSession(
				jsonParser,
				Func<void(const WString&)>([&serverForTerminal](const WString& message)
				{
					if (serverForTerminal)
					{
						try
						{
							serverForTerminal->BroadcastError(message);
						}
						catch (...)
						{
						}
					}
					std::_Exit(1);
				})
				));
		}

		auto callbacks = CreateCallbacks(requesterSession.Obj());
		RemotingChannelServer<TServer> channelServer(
			jsonParser,
			true,
			callbacks,
			std::forward<TArgs>(args)...
			);
		serverForTerminal = &channelServer;
		channelServer.Start();

		auto coreClient = Ptr(new GuiRemoteProtocolLocalChannelClient(jsonParser));
		auto coreClientId = channelServer.ConnectLocalClient(coreClient);
		CHECK_ERROR(coreClientId == GacUIRemoteProtocolCoreClientId, L"StartServer(...)#Failed to register the core channel client.");

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

		CoreGuiContext context{
			mainWindowConstructorIndex,
			requesterSession.Obj(),
			viewModel,
			miniHttpSocketServer
			};
		CHECK_ERROR(!currentGuiContext, L"StartServer(...)#The GUI context has already been bound.");
		currentGuiContext = &context;
		SetupRemoteNativeController(&diffConverterProtocol);
		currentGuiContext = nullptr;

		channelServer.ClearCoreChannels();
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
	GetNativeServiceSubstitution()->Substitute(&automationService, false);
#ifdef VCZH_MSVC
	if (currentGuiContext->miniHttpSocketServer)
	{
		StartMiniHttpAutomationService(
			currentGuiContext->miniHttpSocketServer,
			WString::Unmanaged(L"RemotingTest_Core")
			);
	}
	else
	{
		windows::StartWindowsHttpAutomationService(
			WString::Unmanaged(L"Automation/RemotingTest_Core"),
			RemotingHttpPort
			);
	}
#else
	StartMiniHttpAutomationService(
		currentGuiContext->miniHttpSocketServer,
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

#ifdef VCZH_MSVC
	if (currentGuiContext->miniHttpSocketServer)
	{
		StopMiniHttpAutomationService();
	}
	else
	{
		windows::StopWindowsHttpAutomationService();
	}
#else
	StopMiniHttpAutomationService();
#endif
	automationService.Stop();
	GetNativeServiceSubstitution()->Unsubstitute(&automationService);
}

#ifdef VCZH_MSVC
int StartNamedPipeServer(vint index)
{
	return StartServer<named_pipe::NamedPipeServer>(
		index,
		nullptr,
		WString::Unmanaged(RemotingNamedPipeName)
		);
}

int StartHttpServer(vint index)
{
	return StartServer<windows_http::HttpServer>(
		index,
		nullptr,
		WString::Unmanaged(RemotingHttpBaseUrl),
		RemotingHttpPort
		);
}
#endif

int StartMiniHttpServer(vint index)
{
	auto socketServer = async_tcp_socket::CreateDefaultAsyncSocketServer(RemotingHttpPort);
	return StartServer<async_tcp_socket::SocketHttpServer>(
		index,
		socketServer,
		socketServer,
		WString::Unmanaged(RemotingHttpBaseUrl)
		);
}
