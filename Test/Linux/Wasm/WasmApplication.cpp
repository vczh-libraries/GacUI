#include <Vlpp.h>

#if defined VCZH_WASM
#include "DarkSkin.h"
#include "../../RemotingHelpers/RemotingServer/Wasm/WasmNetworkProtocol.h"
#include "../../RemotingHelpers/RemotingServer/RemotingChannelServer.h"

#if defined GACUI_WASM_FCT
#include "MainWindow.h"
#include "../../GacUISrc/Generated_FullControlTest/FullControlTestPalette.h"
#elif defined GACUI_WASM_RPT
#include "RpMainWindow.h"
#elif defined GACUI_WASM_RVMT
#include "RemoteViewModelTestIncludes.h"
#include "../../GacUISrc/Generated_RemoteViewModelTest/RemoteViewModelTestInitialize.h"
#include "../../RemotingHelpers/Rvmt/ViewModelHostServer.h"
#endif

using namespace vl;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remoting;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remoteprotocol::repeatfiltering;

#if defined GACUI_WASM_RVMT
using WasmChannelServer = remote_view_model_test::RemoteViewModelChannelServer<WasmNetworkProtocolServer>;
Ptr<rvmt::IViewModel>* wasmViewModel = nullptr;
#else
using WasmChannelServer = RemotingChannelServer<WasmNetworkProtocolServer>;
#endif
WasmChannelServer* wasmServer = nullptr;

void GuiMain()
{
	theme::RegisterTheme(Ptr(new darkskin::Theme));
#if defined GACUI_WASM_FCT
	auto window = Ptr(new demo::MainWindow);
	window->PaletteSelected.Add(&demo::OnPaletteSelected);
#elif defined GACUI_WASM_RPT
	auto window = Ptr(new rptest::RpMainWindow);
#elif defined GACUI_WASM_RVMT
	auto window = Ptr(new rvmt::MainWindow(*wasmViewModel));
#endif
	window->ForceCalculateSizeImmediately();
	try
	{
		GetApplication()->Run(window.Obj());
	}
	catch (const Exception& error)
	{
		wasmServer->BroadcastError(error.Message());
		throw;
	}
	catch (const Error& error)
	{
		wasmServer->BroadcastError(error.Description());
		throw;
	}
}

class WasmCoreChannel : public GuiRemoteProtocolCoreChannel
{
private:
	WasmChannelServer&						server;

protected:
	bool IsCorrectRendererClientId(vint clientId) override
	{
		return clientId != -1 && clientId == server.GetRendererClientId();
	}

public:
	WasmCoreChannel(GuiRemoteProtocolLocalChannelClient* client, GuiRemoteProtocolAsyncJsonChannel& channel, WasmChannelServer& _server)
		: GuiRemoteProtocolCoreChannel(client, &channel, L"/GacUIWasmApplication", channel.GetRemoteEventProcessor())
		, server(_server)
	{
	}
};

vint WasmMain()
{
	auto parser = Ptr(new glr::json::Parser);
	WasmChannelServer server(parser, true);
	server.Start();
	auto coreClient = Ptr(new GuiRemoteProtocolLocalChannelClient(parser));
	auto coreClientId = server.ConnectLocalClient(coreClient);
	CHECK_ERROR(coreClientId == GacUIRemoteProtocolCoreClientId, L"WasmMain()#Unexpected Core client ID.");
	GuiRemoteProtocolAsyncJsonChannel asyncChannel(coreClient->GetProtocolChannel());
	WasmCoreChannel coreChannel(coreClient.Obj(), asyncChannel, server);
	GuiRemoteProtocolFilter filteredProtocol(&coreChannel);
	GuiRemoteProtocolDomDiffConverter protocol(&filteredProtocol);
	server.SetCoreChannels(coreClient->GetProtocolChannel(), &coreChannel);
	wasmServer = &server;

	vint result = 0;
	try
	{
#if defined GACUI_WASM_RVMT
		collections::List<WString> services;
		services.Add(L"rvmt::IViewModel");
		auto requesterId = server.Connect(services);
		remote_view_model_test::RemoteViewModelTestInitialize::InitializeRpc(server.GetDispatcher(), requesterId);
		NotifyWasmApplication(L"ready");
		auto viewModel = server.RequestService(L"rvmt::IViewModel").Cast<rvmt::IViewModel>();
		wasmViewModel = &viewModel;
#else
		NotifyWasmApplication(L"ready");
#endif
		NotifyWasmApplication(L"renderer-ready");
		SetupRemoteNativeController(&protocol);
#if defined GACUI_WASM_RVMT
		wasmViewModel = nullptr;
#endif
	}
	catch (const Exception& error)
	{
		result = 1;
		server.BroadcastError(error.Message());
	}
	catch (const Error& error)
	{
		result = 1;
		server.BroadcastError(error.Description());
	}
	server.ClearCoreChannels();
	wasmServer = nullptr;
	server.Stop();
	ThreadPoolLite::Stop(true);
	return result;
}
#endif

#if defined VCZH_GCC
void GuiMain()
{
}

int main()
{
	return 0;
}
#endif
