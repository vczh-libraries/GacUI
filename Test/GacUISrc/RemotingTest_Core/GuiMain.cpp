#include "DarkSkin.h"
#include "RpMainWindow.h"
#include "MainWindow.h"
#include "RemoteViewModelTestIncludes.h"
#include "../RemotingTest_RvmHost/RemoteViewModelTestRuntime.h"
#include "../../../Source/RemotingHelpers/AutomationService/MiniHttpAutomationService.h"
#include "../../../Source/Utilities/SharedServices/GuiSharedAutomationService_Controls.h"
#ifdef VCZH_MSVC
#include <VlppOS.Windows.h>
#include "../../../Source/RemotingHelpers/AutomationService/Windows/WindowsAutomationService.Windows.h"
#endif

using namespace vl;
using namespace vl::inter_process;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remoting;
using namespace vl::presentation::remote_view_model_test;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remoteprotocol::repeatfiltering;

struct CoreGuiContext
{
	vint												mainWindowConstructorIndex = 0;
	RemoteViewModelRequesterSession*					session = nullptr;
	Ptr<rvmt::IViewModel>								viewModel;
	Ptr<async_tcp_socket::IAsyncSocketServer>			miniHttpSocketServer;
};

CoreGuiContext* currentGuiContext = nullptr;

template<typename TServerBase>
class SwitchableRenderersCoreChannel : public GuiRemoteProtocolCoreChannel
{
	using Base = GuiRemoteProtocolCoreChannel;

private:
	RemotingChannelServer<TServerBase>*					channelServer = nullptr;

protected:
	bool IsCorrectRendererClientId(vint clientId) override
	{
		return clientId != -1 && clientId == channelServer->GetRendererClientId();
	}

public:
	SwitchableRenderersCoreChannel(
		JsonChannelClient* client,
		JsonChannel* channel,
		const WString& executablePath,
		IGuiRemoteEventProcessor* eventProcessor,
		RemotingChannelServer<TServerBase>* _channelServer
		)
		: Base(client, channel, executablePath, eventProcessor)
		, channelServer(_channelServer)
	{
	}
};

template<typename TServerBase>
int StartServer(
	vint mainWindowConstructorIndex,
	Ptr<async_tcp_socket::IAsyncSocketServer> miniHttpSocketServer,
	Ptr<glr::json::Parser> jsonParser,
	RemotingChannelServer<TServerBase>& channelServer,
	RemoteViewModelRequesterSession* requesterSession
	)
{
	channelServer.Start();

	auto coreClient = Ptr(new GuiRemoteProtocolLocalChannelClient(jsonParser));
	auto coreClientId = channelServer.ConnectLocalClient(coreClient);
	CHECK_ERROR(coreClientId == GacUIRemoteProtocolCoreClientId, L"StartServer(...)#Failed to register the core channel client.");

	GuiRemoteProtocolAsyncJsonChannel asyncChannelSender(coreClient->GetProtocolChannel());
	SwitchableRenderersCoreChannel<TServerBase> channelSender(
		coreClient.Obj(),
		&asyncChannelSender,
		WString::Unmanaged(L"RemotingTest_Core.vcxproj"),
		asyncChannelSender.GetRemoteEventProcessor(),
		&channelServer
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
		requesterSession,
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
	if (currentGuiContext->session)
	{
		currentGuiContext->session->BeginStopping();
	}

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
	auto jsonParser = Ptr(new glr::json::Parser);
	if (index == 2)
	{
		RemoteViewModelChannelServer<named_pipe::NamedPipeServer> channelServer(
			jsonParser,
			true,
			WString::Unmanaged(RemotingNamedPipeName)
			);
		return StartServer<named_pipe::NamedPipeServer>(index, nullptr, jsonParser, channelServer, channelServer.GetSession());
	}
	RemotingChannelServer<named_pipe::NamedPipeServer> channelServer(
		jsonParser,
		true,
		WString::Unmanaged(RemotingNamedPipeName)
		);
	return StartServer<named_pipe::NamedPipeServer>(index, nullptr, jsonParser, channelServer, nullptr);
}

int StartHttpServer(vint index)
{
	auto jsonParser = Ptr(new glr::json::Parser);
	if (index == 2)
	{
		RemoteViewModelChannelServer<windows_http::HttpServer> channelServer(
			jsonParser,
			true,
			WString::Unmanaged(RemotingHttpBaseUrl),
			RemotingHttpPort
			);
		return StartServer<windows_http::HttpServer>(index, nullptr, jsonParser, channelServer, channelServer.GetSession());
	}
	RemotingChannelServer<windows_http::HttpServer> channelServer(
		jsonParser,
		true,
		WString::Unmanaged(RemotingHttpBaseUrl),
		RemotingHttpPort
		);
	return StartServer<windows_http::HttpServer>(index, nullptr, jsonParser, channelServer, nullptr);
}
#endif

int StartMiniHttpServer(vint index)
{
	auto jsonParser = Ptr(new glr::json::Parser);
	auto socketServer = async_tcp_socket::CreateDefaultAsyncSocketServer(RemotingHttpPort);
	if (index == 2)
	{
		RemoteViewModelChannelServer<async_tcp_socket::SocketHttpServer> channelServer(
			jsonParser,
			true,
			socketServer,
			WString::Unmanaged(RemotingHttpBaseUrl)
			);
		return StartServer<async_tcp_socket::SocketHttpServer>(index, socketServer, jsonParser, channelServer, channelServer.GetSession());
	}
	RemotingChannelServer<async_tcp_socket::SocketHttpServer> channelServer(
		jsonParser,
		true,
		socketServer,
		WString::Unmanaged(RemotingHttpBaseUrl)
		);
	return StartServer<async_tcp_socket::SocketHttpServer>(index, socketServer, jsonParser, channelServer, nullptr);
}
