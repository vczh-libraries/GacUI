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

	class RemotingChannelServerBase : public GuiRemoteProtocolChannelServer
	{
		using Base = GuiRemoteProtocolChannelServer;

	protected:
		// covers eventRendererConnected, connectingCoreClient and rendererClientId
		SpinLock						lockConnection;
		EventObject*					eventRendererConnected = nullptr;
		bool							connectingCoreClient = false;
		bool							acceptMultipleRenderers = false;
		vint							rendererClientId = -1;

	public:
		using Base::OnClientConnected;

		RemotingChannelServerBase(Ptr<glr::json::Parser> parser, bool _acceptMultipleRenderers)
			: Base(parser)
			, acceptMultipleRenderers(_acceptMultipleRenderers)
		{
		}

		void SetRendererConnectedEvent(EventObject* eventObject)
		{
			SPIN_LOCK(lockConnection)
			{
				eventRendererConnected = eventObject;
			}
		}

		void BeginConnectCoreClient()
		{
			SPIN_LOCK(lockConnection)
			{
				connectingCoreClient = true;
			}
		}

		void EndConnectCoreClient()
		{
			SPIN_LOCK(lockConnection)
			{
				connectingCoreClient = false;
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

		inter_process::WaitForClientResult OnClientConnected(vint clientId, const IJsonChannelClient::ChannelNameList& availableChannels) override
		{
			CHECK_ERROR(
				availableChannels.Contains(WString::Unmanaged(GacUIRemoteProtocolChannelName)),
				L"RemotingChannelServerBase::OnClientConnected(vint, const ChannelNameList&)#The client does not have the GacUI remote protocol channel."
				);

			EventObject* eventToSignal = nullptr;
			bool rejected = false;
			SPIN_LOCK(lockConnection)
			{
				if (!connectingCoreClient)
				{
					if (!acceptMultipleRenderers && rendererClientId != -1)
					{
						rejected = true;
					}
					else
					{
						rendererClientId = clientId;
						eventToSignal = eventRendererConnected;
					}
				}
			}

			if (rejected)
			{
				return inter_process::WaitForClientResult::Reject;
			}
			if (eventToSignal)
			{
				eventToSignal->Signal();
			}
			return inter_process::WaitForClientResult::Accept;
		}
	};

	class NamedPipeRemotingChannelServer : public RemotingChannelServerBase, public inter_process::NamedPipeServer
	{
	public:
		NamedPipeRemotingChannelServer(Ptr<glr::json::Parser> parser, const WString& pipeName)
			: RemotingChannelServerBase(parser, false)
			, inter_process::NamedPipeServer(pipeName)
		{
		}

		inter_process::WaitForClientResult OnClientConnected(inter_process::INetworkProtocolConnection* connection) override
		{
			return RemotingChannelServerBase::OnClientConnected(connection);
		}

		void Start() override
		{
			RemotingChannelServerBase::Start();
			inter_process::NamedPipeServer::Start();
		}

		void Stop() override
		{
			RemotingChannelServerBase::Stop();
			inter_process::NamedPipeServer::Stop();
		}

		bool IsStopped() override
		{
			return RemotingChannelServerBase::IsStopped() || inter_process::NamedPipeServer::IsStopped();
		}
	};

	class HttpRemotingChannelServer : public RemotingChannelServerBase, public inter_process::HttpServer
	{
	public:
		HttpRemotingChannelServer(Ptr<glr::json::Parser> parser, const WString& baseUrl, vint port)
			: RemotingChannelServerBase(parser, true)
			, inter_process::HttpServer(baseUrl, port)
		{
		}

		inter_process::WaitForClientResult OnClientConnected(inter_process::INetworkProtocolConnection* connection) override
		{
			return RemotingChannelServerBase::OnClientConnected(connection);
		}

		void Start() override
		{
			RemotingChannelServerBase::Start();
			inter_process::HttpServer::Start();
		}

		void Stop() override
		{
			inter_process::HttpServer::Stop();
			RemotingChannelServerBase::Stop();
		}

		bool IsStopped() override
		{
			return RemotingChannelServerBase::IsStopped() || inter_process::HttpServer::IsStopped();
		}
	};

	vint WaitForRenderer(RemotingChannelServerBase& channelServer, EventObject& eventRendererConnected)
	{
		eventRendererConnected.Wait();
		auto rendererClientId = channelServer.GetRendererClientId();
		CHECK_ERROR(
			channelServer.GetClientChannels().Contains(rendererClientId, WString::Unmanaged(GacUIRemoteProtocolChannelName)),
			L"WaitForRenderer(RemotingChannelServerBase&, EventObject&)#The renderer client does not have the GacUI remote protocol channel."
			);
		return rendererClientId;
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

void StartServer(RemotingChannelServerBase& channelServer, Ptr<glr::json::Parser> jsonParser)
{
	EventObject eventRendererConnected;
	CHECK_ERROR(eventRendererConnected.CreateManualUnsignal(false), L"StartServer(RemotingChannelServerBase&, Ptr<Parser>)#Failed to create eventRendererConnected.");
	channelServer.SetRendererConnectedEvent(&eventRendererConnected);
	channelServer.Start();

	auto coreClient = Ptr(new GuiRemoteProtocolLocalChannelClient(jsonParser));
	channelServer.BeginConnectCoreClient();
	auto coreClientId = channelServer.ConnectLocalClient(coreClient);
	channelServer.EndConnectCoreClient();
	CHECK_ERROR(coreClientId == GacUIRemoteProtocolCoreClientId, L"StartServer(RemotingChannelServerBase&, Ptr<Parser>)#Failed to register the core channel client.");

	Console::WriteLine(L"> Waiting for a renderer ...");
	auto rendererClientId = WaitForRenderer(channelServer, eventRendererConnected);
	Console::WriteLine(L"> Renderer connected: " + itow(rendererClientId));

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

	channelServer.SetRendererConnectedEvent(nullptr);
	channelServer.Stop();
}

int StartNamedPipeServer(vint index)
{
	mainWindowConstructorIndex = index;
	Console::WriteLine(L"> Named pipe created, waiting on: " + WString::Unmanaged(GacUIRemoteProtocolNamedPipeName));
	auto jsonParser = Ptr(new glr::json::Parser);
	NamedPipeRemotingChannelServer channelServer(jsonParser, WString::Unmanaged(GacUIRemoteProtocolNamedPipeName));
	StartServer(channelServer, jsonParser);
	return 0;
}

int StartHttpServer(vint index)
{
	mainWindowConstructorIndex = index;
	Console::WriteLine(L"> HTTP server created, waiting on: http://localhost:" + itow(GacUIRemoteProtocolHttpPort) + WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl));
	auto jsonParser = Ptr(new glr::json::Parser);
	HttpRemotingChannelServer channelServer(jsonParser, WString::Unmanaged(GacUIRemoteProtocolHttpBaseUrl), GacUIRemoteProtocolHttpPort);
	StartServer(channelServer, jsonParser);
	return 0;
}
