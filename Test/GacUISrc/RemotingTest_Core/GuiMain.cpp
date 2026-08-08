#include "DarkSkin.h"
#include "RpMainWindow.h"
#include "MainWindow.h"
#include "RemoteViewModelTestIncludes.h"
#include "../Generated_RemoteViewModelTest/RemoteViewModelTestInitialize.h"
#include "../../RemotingHelpers/Rvmt/ViewModelHostServer.h"
#include "../../../Source/Utilities/AutomationService/MiniHttpAutomationService.h"
#include "../../../Source/Utilities/SharedServices/GuiSharedAutomationService_Controls.h"
#ifdef VCZH_MSVC
#include <VlppOS.Windows.h>
#include "../../../Source/Utilities/AutomationService/Windows/WindowsAutomationService.Windows.h"
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
	Ptr<rvmt::IViewModel>								viewModel;
	Ptr<async_tcp_socket::IAsyncSocketServer>			miniHttpSocketServer;
	Func<void(const WString&)>							broadcastFatalError;
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

	std::exception_ptr uiException;
	try
	{
		GetApplication()->Run(window.Obj());
	}
	catch (const Exception& ex)
	{
		currentGuiContext->broadcastFatalError(ex.Message());
		uiException = std::current_exception();
	}
	catch (const Error& ex)
	{
		currentGuiContext->broadcastFatalError(WString::Unmanaged(ex.Description()));
		uiException = std::current_exception();
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
	if (uiException)
	{
		std::rethrow_exception(uiException);
	}
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
	remoting::RemotingChannelServer<TServerBase>& channelServer
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

	int result = 0;
	bool fatalErrorBroadcasted = false;
	auto broadcastFatalError = [&channelServer, &fatalErrorBroadcasted](const WString& message)
	{
		if (fatalErrorBroadcasted)
		{
			return;
		}
		fatalErrorBroadcasted = true;
		try
		{
			channelServer.BroadcastError(message);
		}
		catch (...)
		{
		}
	};
	CoreGuiContext context{
		mainWindowConstructorIndex,
		nullptr,
		miniHttpSocketServer,
		broadcastFatalError
		};
	CHECK_ERROR(!currentGuiContext, L"StartServer(...)#The GUI context has already been bound.");
	currentGuiContext = &context;
	try
	{
		if (mainWindowConstructorIndex == 2)
		{
			auto& rvmChannelServer = dynamic_cast<RemoteViewModelChannelServer<TServerBase>&>(channelServer);
			collections::List<WString> requiredServiceNames;
			requiredServiceNames.Add(L"rvmt::IViewModel");
			auto requesterClientId = rvmChannelServer.Connect(requiredServiceNames);
			RemoteViewModelTestInitialize::InitializeRpc(rvmChannelServer.GetDispatcher(), requesterClientId);
			context.viewModel = rvmChannelServer.RequestService(L"rvmt::IViewModel").template Cast<rvmt::IViewModel>();
		}
		SetupRemoteNativeController(&diffConverterProtocol);
	}
	catch (const Exception& ex)
	{
		result = 1;
		broadcastFatalError(ex.Message());
	}
	catch (const Error& ex)
	{
		result = 1;
		broadcastFatalError(WString::Unmanaged(ex.Description()));
	}
	currentGuiContext = nullptr;

	channelServer.ClearCoreChannels();
	channelServer.Stop();
	return result;
}

template<typename TServerBase, typename ...TArgs>
int StartServerHelper(
	vint index,
	Ptr<async_tcp_socket::IAsyncSocketServer> miniHttpSocketServer,
	TArgs&&... args)
{
	auto jsonParser = Ptr(new glr::json::Parser);
	if (index == 2)
	{
		RemoteViewModelChannelServer<TServerBase> channelServer(
			jsonParser,
			true,
			std::forward<TArgs&&>(args)...
		);
		return StartServer<TServerBase>(index, miniHttpSocketServer, jsonParser, channelServer);
	}
	else
	{
		remoting::RemotingChannelServer<TServerBase> channelServer(
			jsonParser,
			true,
			std::forward<TArgs&&>(args)...
		);
		return StartServer<TServerBase>(index, miniHttpSocketServer, jsonParser, channelServer);
	}
}

#ifdef VCZH_MSVC
int StartNamedPipeServer(vint index)
{
	return StartServerHelper<named_pipe::NamedPipeServer>(
		index,
		nullptr,
		WString::Unmanaged(RemotingNamedPipeName)
		);
}

int StartHttpServer(vint index)
{
	return StartServerHelper<windows_http::HttpServer>(
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
	return StartServerHelper<async_tcp_socket::SocketHttpServer>(
		index,
		socketServer,
		socketServer,
		WString::Unmanaged(RemotingHttpBaseUrl)
		);
}
