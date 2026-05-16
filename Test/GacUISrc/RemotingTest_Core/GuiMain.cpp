#include "DarkSkin.h"
#include "RpMainWindow.h"
#include "MainWindow.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Import/VlppOS.Windows.h"

using namespace vl;
using namespace vl::console;
using namespace vl::presentation;
using namespace vl::presentation::templates;
using namespace vl::presentation::controls;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remoteprotocol::repeatfiltering;

namespace
{
	constexpr const wchar_t* GacUIRemoteProtocolNamedPipeName = L"GacUIRemoteProtocolNamedPipe";
	constexpr const wchar_t* GacUIRemoteProtocolHttpBaseUrl = L"/GacUIRemoteProtocolHttp";
	constexpr vint GacUIRemoteProtocolHttpPort = 8888;

	vint WaitForRenderer(GuiRemoteProtocolChannelServer& channelServer)
	{
		auto rendererClientId = channelServer.WaitForClient();
		CHECK_ERROR(
			channelServer.GetClientChannels().Contains(rendererClientId, WString::Unmanaged(GacUIRemoteProtocolChannelName)),
			L"WaitForRenderer(GuiRemoteProtocolChannelServer&)#The renderer client does not have the GacUI remote protocol channel."
			);
		return rendererClientId;
	}

	void AcceptRenderers(GuiRemoteProtocolChannelServer* channelServer)
	{
		while (!channelServer->IsStopped())
		{
			try
			{
				auto rendererClientId = WaitForRenderer(*channelServer);
				Console::WriteLine(L"> Renderer connected: " + itow(rendererClientId));
			}
			catch (const Exception& e)
			{
				if (!channelServer->IsStopped())
				{
					channelServer->BroadcastError(e.Message());
					Console::WriteLine(L"Error: " + e.Message());
				}
				break;
			}
			catch (const Error& e)
			{
				auto message = WString::Unmanaged(e.Description());
				if (!channelServer->IsStopped())
				{
					channelServer->BroadcastError(message);
					Console::WriteLine(L"Error: " + message);
				}
				break;
			}
		}
	}
}

GuiRemoteProtocolChannelServer* protocolServer = nullptr;
vint mainWindowConstructorIndex = 0;

void GuiMain()
{
	theme::RegisterTheme(Ptr(new darkskin::Theme));
	{
		Ptr<GuiWindow> window;
		switch (mainWindowConstructorIndex)
		{
		case 1:
			window = Ptr(new rptest::RpMainWindow);
			break;
		default:
		case 0:
			window = Ptr(new demo::MainWindow);
		}
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();
		try
		{
			GetApplication()->Run(window.Obj());
		}
		catch (const Exception& e)
		{
			if (protocolServer)
			{
				protocolServer->BroadcastError(e.Message());
			}
			Console::WriteLine(L"Error: " + e.Message());
		}
		catch (const Error& e)
		{
			auto message = WString::Unmanaged(e.Description());
			if (protocolServer)
			{
				protocolServer->BroadcastError(message);
			}
			Console::WriteLine(L"Error: " + message);
		}
	}
}

void StartServer(Ptr<inter_process::INetworkProtocolServer> networkServer, bool acceptMultipleRenderers)
{
	auto jsonParser = Ptr(new glr::json::Parser);
	GuiRemoteProtocolChannelServer channelServer(networkServer, jsonParser);

	auto coreClient = Ptr(new GuiRemoteProtocolLocalChannelClient(jsonParser));
	auto coreClientId = channelServer.ConnectLocalClient(coreClient);
	CHECK_ERROR(coreClientId > 0, L"StartServer(Ptr<INetworkProtocolServer>)#Failed to register the core channel client.");

	Console::WriteLine(L"> Waiting for a renderer ...");
	auto rendererClientId = WaitForRenderer(channelServer);
	Console::WriteLine(L"> Renderer connected: " + itow(rendererClientId));

	Thread* acceptingThread = nullptr;
	if (acceptMultipleRenderers)
	{
		acceptingThread = Thread::CreateAndStart(Func<void()>([&channelServer]()
		{
			AcceptRenderers(&channelServer);
		}), false);
	}

	GuiRemoteProtocolAsyncJsonChannelSerializer asyncChannelSender(coreClient->GetProtocolChannel());
	GuiRemoteProtocolCoreChannel channelSender(
		coreClient.Obj(),
		&asyncChannelSender,
		WString::Unmanaged(L"RemotingTest_Core.vcxproj"),
		asyncChannelSender.GetRemoteEventProcessor()
		);
	GuiRemoteProtocolFilter filteredProtocol(&channelSender);
	GuiRemoteProtocolDomDiffConverter diffConverterProtocol(&filteredProtocol);

	protocolServer = &channelServer;
	SetupRemoteNativeController(&diffConverterProtocol);
	protocolServer = nullptr;

	channelServer.Stop();
	if (acceptingThread)
	{
		acceptingThread->Wait();
		delete acceptingThread;
	}
}

int StartNamedPipeServer(vint index)
{
	mainWindowConstructorIndex = index;
	Console::WriteLine(L"> Named pipe created, waiting on: " + WString::Unmanaged(GacUIRemoteProtocolNamedPipeName));
	StartServer(Ptr(new inter_process::NamedPipeServer(WString::Unmanaged(GacUIRemoteProtocolNamedPipeName))), false);
	return 0;
}

int StartHttpServer(vint index)
{
	mainWindowConstructorIndex = index;
	Console::WriteLine(L"> HTTP server created, waiting on: http://localhost:" + itow(GacUIRemoteProtocolHttpPort) + WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl));
	StartServer(Ptr(new inter_process::HttpServer(WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl), GacUIRemoteProtocolHttpPort)), true);
	return 0;
}
