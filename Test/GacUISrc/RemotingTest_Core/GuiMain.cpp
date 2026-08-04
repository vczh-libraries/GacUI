#include "DarkSkin.h"
#include "RpMainWindow.h"
#include "MainWindow.h"
#include "RemoteViewModelTestIncludes.h"
#include "../../RemotingHelpers/Rvmt/ViewModelHostServer.h"
#include "../../RemotingHelpers/AutomationService/MiniHttpAutomationService.h"
#include "../../../Source/Utilities/SharedServices/GuiSharedAutomationService_Controls.h"
#ifdef VCZH_MSVC
#include <VlppOS.Windows.h>
#include "../../RemotingHelpers/AutomationService/Windows/WindowsAutomationService.Windows.h"
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

constexpr const wchar_t* RemotingNamedPipeName = L"GacUIRemoteProtocolNamedPipe";
constexpr const wchar_t* RemotingHttpBaseUrl = L"/GacUIRemoteProtocolHttp";
constexpr vint RemotingHttpPort = 8888;
constexpr vint GacUIAutomationHttpPort = 8888;

struct CoreGuiContext
{
	vint												mainWindowConstructorIndex = 0;
	remoting::RemotingRequesterSession*					session = nullptr;
	Ptr<rvmt::IViewModel>								viewModel;
	Ptr<async_tcp_socket::IAsyncSocketServer>			miniHttpSocketServer;
};

CoreGuiContext* currentGuiContext = nullptr;

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
			GacUIAutomationHttpPort
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
	remoting::RemotingChannelServer<TServerBase>& channelServer,
	remoting::RemotingRequesterSession* requesterSession
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
		viewModel = requesterSession->RequestService().Cast<rvmt::IViewModel>();
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

template<template<typename> class TChannelServer, typename TServerBase, typename ...TArgs>
int StartServerHelper(
	vint index,
	Ptr<async_tcp_socket::IAsyncSocketServer> miniHttpSocketServer,
	TArgs&&... args)
{
	auto jsonParser = Ptr(new glr::json::Parser);
	TChannelServer<TServerBase> channelServer(
		jsonParser,
		true,
		std::forward<TArgs&&>(args)...
	);

	remoting::RemotingRequesterSession* session = nullptr;
	if constexpr (std::is_same_v<TChannelServer<TServerBase>, RemoteViewModelChannelServer<TServerBase>>)
	{
		session = channelServer.GetSession();
	}
	return StartServer<TServerBase>(index, miniHttpSocketServer, jsonParser, channelServer, session);
}

#ifdef VCZH_MSVC
int StartNamedPipeServer(vint index)
{
	if (index == 2)
	{
		return StartServerHelper<RemoteViewModelChannelServer, named_pipe::NamedPipeServer>(
			index,
			nullptr,
			WString::Unmanaged(RemotingNamedPipeName)
			);
	}
	else
	{
		return StartServerHelper<remoting::RemotingChannelServer, named_pipe::NamedPipeServer>(
			index,
			nullptr,
			WString::Unmanaged(RemotingNamedPipeName)
			);
	}
}

int StartHttpServer(vint index)
{
	if (index == 2)
	{
		return StartServerHelper<RemoteViewModelChannelServer, windows_http::HttpServer>(
			index,
			nullptr,
			WString::Unmanaged(RemotingHttpBaseUrl),
			RemotingHttpPort
			);
	}
	else
	{
		return StartServerHelper<remoting::RemotingChannelServer, windows_http::HttpServer>(
			index,
			nullptr,
			WString::Unmanaged(RemotingHttpBaseUrl),
			RemotingHttpPort
			);
	}
}
#endif

int StartMiniHttpServer(vint index)
{
	auto socketServer = async_tcp_socket::CreateDefaultAsyncSocketServer(RemotingHttpPort);
	if (index == 2)
	{
		return StartServerHelper<RemoteViewModelChannelServer, async_tcp_socket::SocketHttpServer>(
			index,
			socketServer,
			socketServer,
			WString::Unmanaged(RemotingHttpBaseUrl)
			);
	}
	else
	{
		return StartServerHelper<remoting::RemotingChannelServer, async_tcp_socket::SocketHttpServer>(
			index,
			socketServer,
			socketServer,
			WString::Unmanaged(RemotingHttpBaseUrl)
			);
	}
}
