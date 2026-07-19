#include "DarkSkin.h"
#include "RpMainWindow.h"
#include "MainWindow.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include <VlppOS.Windows.h>
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"
#include <type_traits>

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

	template<typename TServerBase>
	class RemotingChannelServerBase : public GuiRemoteProtocolNetworkChannelServer<TServerBase>
	{
		using Base = GuiRemoteProtocolNetworkChannelServer<TServerBase>;

	protected:
		// covers eventRendererConnected, rendererClientId and coreProtocolChannel
		SpinLock						lockConnection;
		EventObject*					eventRendererConnected = nullptr;
		GuiRemoteProtocolCoreChannel*	coreProtocolChannel = nullptr;
		vint							rendererClientId = -1;

	public:
		using Base::OnClientConnected;

		template<typename... TArgs>
		RemotingChannelServerBase(Ptr<glr::json::Parser> parser, TArgs&&... args)
			: Base(parser, std::forward<TArgs>(args)...)
		{
		}

		void SetRendererConnectedEvent(EventObject* eventObject)
		{
			SPIN_LOCK(lockConnection)
			{
				eventRendererConnected = eventObject;
			}
		}

		void SetCoreProtocolChannel(GuiRemoteProtocolCoreChannel* channel)
		{
			SPIN_LOCK(lockConnection)
			{
				coreProtocolChannel = channel;
			}
		}

		vint GetRendererClientId()
		{
			vint clientId = -1;
			SPIN_LOCK(lockConnection)
			{
				clientId = rendererClientId;
			}
			return clientId;
		}

		inter_process::WaitForClientResult OnClientConnected(vint clientId, const IJsonChannelClient::ChannelNameList& availableChannels, IJsonChannelClient* localClient) override
		{
			CHECK_ERROR(
				availableChannels.Contains(WString::Unmanaged(GacUIRemoteProtocolChannelName)),
				L"RemotingChannelServerBase::OnClientConnected(vint, const ChannelNameList&, IJsonChannelClient*)#The client does not have the GacUI remote protocol channel."
				);

			EventObject* eventToSignal = nullptr;
			GuiRemoteProtocolCoreChannel* channelToOldRenderer = nullptr;
			vint oldRendererClientId = -1;
			SPIN_LOCK(lockConnection)
			{
				if (!localClient)
				{
					if (rendererClientId != -1 && rendererClientId != clientId)
					{
						oldRendererClientId = rendererClientId;
						channelToOldRenderer = coreProtocolChannel;
					}
					rendererClientId = clientId;
					eventToSignal = eventRendererConnected;
				}
			}

			if (oldRendererClientId != -1)
			{
				if (channelToOldRenderer)
				{
					channelToOldRenderer->DetachRenderer(oldRendererClientId);
				}
				this->DisconnectClient(oldRendererClientId);
			}
			if (eventToSignal)
			{
				eventToSignal->Signal();
			}
			return inter_process::WaitForClientResult::Accept;
		}
	};

	class NamedPipeRemotingChannelServer : public RemotingChannelServerBase<inter_process::named_pipe::NamedPipeServer>
	{
		using Base = RemotingChannelServerBase<inter_process::named_pipe::NamedPipeServer>;

	public:
		NamedPipeRemotingChannelServer(Ptr<glr::json::Parser> parser, const WString& pipeName)
			: Base(parser, pipeName)
		{
		}
	};

	class HttpRemotingChannelServer : public RemotingChannelServerBase<inter_process::windows_http::HttpServer>
	{
		using Base = RemotingChannelServerBase<inter_process::windows_http::HttpServer>;

	public:
		HttpRemotingChannelServer(Ptr<glr::json::Parser> parser, const WString& baseUrl, vint port)
			: Base(parser, baseUrl, port)
		{
		}
	};

	class MiniHttpRemotingChannelServer : public RemotingChannelServerBase<inter_process::async_tcp_socket::SocketHttpServer>
	{
		using Base = RemotingChannelServerBase<inter_process::async_tcp_socket::SocketHttpServer>;

	public:
		MiniHttpRemotingChannelServer(Ptr<glr::json::Parser> parser, const WString& baseUrl, vint port)
			: Base(parser, baseUrl, port)
		{
		}

		~MiniHttpRemotingChannelServer()
		{
			this->Stop();
		}
	};

	template<typename TServerBase>
	vint WaitForRenderer(RemotingChannelServerBase<TServerBase>& channelServer, EventObject& eventRendererConnected)
	{
		eventRendererConnected.Wait();
		auto rendererClientId = channelServer.GetRendererClientId();
		CHECK_ERROR(rendererClientId != -1, L"WaitForRenderer(RemotingChannelServerBase&, EventObject&)#The renderer client has not been recorded.");
		return rendererClientId;
	}
}

IJsonLocalChannelServer* protocolServer = nullptr;
vint mainWindowConstructorIndex = 0;
bool useWindowsHttpAutomationService = true;

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
			RemoteProtocolAutomationService automationService;
			GetNativeServiceSubstitution()->Substitute(&automationService, false);
			if (useWindowsHttpAutomationService)
			{
				windows::StartWindowsHttpAutomationService(WString::Unmanaged(L"Automation/RemotingTest_Core"), 8888);
			}
			GetApplication()->Run(window.Obj());
			if (useWindowsHttpAutomationService)
			{
				windows::StopWindowsHttpAutomationService();
			}
			GetNativeServiceSubstitution()->Unsubstitute(&automationService);
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

template<typename TServerBase>
void StartServer(RemotingChannelServerBase<TServerBase>& channelServer, Ptr<glr::json::Parser> jsonParser)
{
	EventObject eventRendererConnected;
	CHECK_ERROR(eventRendererConnected.CreateManualUnsignal(false), L"StartServer(RemotingChannelServerBase&, Ptr<Parser>)#Failed to create eventRendererConnected.");
	channelServer.SetRendererConnectedEvent(&eventRendererConnected);
	channelServer.Start();

	auto coreClient = Ptr(new GuiRemoteProtocolLocalChannelClient(jsonParser));
	auto coreClientId = channelServer.ConnectLocalClient(coreClient);
	CHECK_ERROR(coreClientId == GacUIRemoteProtocolCoreClientId, L"StartServer(RemotingChannelServerBase&, Ptr<Parser>)#Failed to register the core channel client.");
	Console::WriteLine(L"> Waiting for a renderer ...");
	auto rendererClientId = WaitForRenderer(channelServer, eventRendererConnected);
	Console::WriteLine(L"> Renderer connected: " + itow(rendererClientId));

	GuiRemoteProtocolAsyncJsonChannel asyncChannelSender(coreClient->GetProtocolChannel());
	GuiRemoteProtocolCoreChannel channelSender(
		coreClient.Obj(),
		&asyncChannelSender,
		WString::Unmanaged(L"RemotingTest_Core.vcxproj"),
		asyncChannelSender.GetRemoteEventProcessor()
		);
	GuiRemoteProtocolFilter filteredProtocol(&channelSender);
	GuiRemoteProtocolDomDiffConverter diffConverterProtocol(&filteredProtocol);

	channelServer.SetCoreProtocolChannel(&channelSender);
	protocolServer = &channelServer;
	SetupRemoteNativeController(&diffConverterProtocol);
	if constexpr (std::is_same_v<TServerBase, inter_process::named_pipe::NamedPipeServer>)
	{
		if (rendererClientId != -1)
		{
			channelServer.DisconnectClient(rendererClientId);
		}
	}
	protocolServer = nullptr;
	channelServer.SetCoreProtocolChannel(nullptr);

	channelServer.SetRendererConnectedEvent(nullptr);
	channelServer.Stop();
}

int StartNamedPipeServer(vint index)
{
	mainWindowConstructorIndex = index;
	useWindowsHttpAutomationService = true;
	Console::WriteLine(L"> Named pipe created, waiting on: " + WString::Unmanaged(GacUIRemoteProtocolNamedPipeName));
	auto jsonParser = Ptr(new glr::json::Parser);
	NamedPipeRemotingChannelServer channelServer(jsonParser, WString::Unmanaged(GacUIRemoteProtocolNamedPipeName));
	StartServer(channelServer, jsonParser);
	return 0;
}

int StartHttpServer(vint index)
{
	mainWindowConstructorIndex = index;
	useWindowsHttpAutomationService = true;
	Console::WriteLine(L"> HTTP server created, waiting on: http://localhost:" + itow(GacUIRemoteProtocolHttpPort) + WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl));
	auto jsonParser = Ptr(new glr::json::Parser);
	HttpRemotingChannelServer channelServer(jsonParser, WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl), GacUIRemoteProtocolHttpPort);
	StartServer(channelServer, jsonParser);
	return 0;
}

int StartMiniHttpServer(vint index)
{
	mainWindowConstructorIndex = index;
	useWindowsHttpAutomationService = false;
	Console::WriteLine(L"> Mini HTTP server created, waiting on: http://localhost:" + itow(GacUIRemoteProtocolHttpPort) + WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl));
	auto jsonParser = Ptr(new glr::json::Parser);
	MiniHttpRemotingChannelServer channelServer(jsonParser, WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl), GacUIRemoteProtocolHttpPort);
	StartServer(channelServer, jsonParser);
	return 0;
}
