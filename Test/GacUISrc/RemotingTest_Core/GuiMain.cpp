#include "DarkSkin.h"
#include "RpMainWindow.h"
#include "MainWindow.h"
#include "RemoteViewModelTestIncludes.h"
#include "../RemotingTest_RvmHost/RemoteViewModelTestRuntime.h"
#include "../../../Source/PlatformProviders/Remote/GuiRemoteProtocol.h"
#include "../../../Source/Utilities/SharedServices/GuiSharedAutomationService_Controls.h"
#ifdef VCZH_MSVC
#include <VlppOS.Windows.h>
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"
#endif
#include <cstdlib>
#include <type_traits>

using namespace vl;
using namespace vl::console;
using namespace vl::inter_process;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remote_view_model_test;
using namespace vl::presentation::remoteprotocol;
using namespace vl::presentation::remoteprotocol::channeling;
using namespace vl::presentation::remoteprotocol::repeatfiltering;

extern void StartMiniHttpAutomationService(Ptr<async_tcp_socket::IAsyncSocketServer> socketServer);
extern void StopMiniHttpAutomationService();

namespace
{
	bool IsRendererChannel(const IJsonChannelClient::ChannelNameList& availableChannels)
	{
		return
			availableChannels.Count() == 1 &&
			availableChannels[0] == WString::Unmanaged(GacUIRemoteProtocolChannelName);
	}

	template<typename TServerBase>
	class RemotingChannelServerBase
		: public GuiRemoteProtocolNetworkChannelServer<TServerBase>
	{
		using Base = GuiRemoteProtocolNetworkChannelServer<TServerBase>;

	protected:
		SpinLock								lockConnection;
		IJsonChannel*							coreJsonChannel = nullptr;
		GuiRemoteProtocolCoreChannel*			coreProtocolChannel = nullptr;
		RemoteViewModelRequesterSession*			remoteViewModelSession = nullptr;
		bool									remoteViewModelRequired = false;
		vint									rendererClientId = InvalidRemoteViewModelClientId;

	public:
		using Base::OnClientConnected;

		template<typename... TArgs>
		RemotingChannelServerBase(Ptr<glr::json::Parser> parser, TArgs&&... args)
			: Base(parser, std::forward<TArgs>(args)...)
		{
		}

		void RequireRemoteViewModel()
		{
			SPIN_LOCK(lockConnection)
			{
				CHECK_ERROR(!remoteViewModelRequired, L"RemotingChannelServerBase::RequireRemoteViewModel()#Remote view model mode has already been enabled.");
				remoteViewModelRequired = true;
			}
		}

		void SetRemoteViewModelSession(RemoteViewModelRequesterSession* session)
		{
			CHECK_ERROR(session, L"RemotingChannelServerBase::SetRemoteViewModelSession(...)#The requester session is null.");
			SPIN_LOCK(lockConnection)
			{
				CHECK_ERROR(remoteViewModelRequired, L"RemotingChannelServerBase::SetRemoteViewModelSession(...)#Remote view model mode is not enabled.");
				CHECK_ERROR(!remoteViewModelSession, L"RemotingChannelServerBase::SetRemoteViewModelSession(...)#Remote view model mode has already been enabled.");
				remoteViewModelSession = session;
			}
		}

		void ClearRemoteViewModelSession()
		{
			SPIN_LOCK(lockConnection)
			{
				remoteViewModelSession = nullptr;
			}
		}

		void SetCoreJsonChannel(IJsonChannel* channel)
		{
			SPIN_LOCK(lockConnection)
			{
				coreJsonChannel = channel;
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

		WaitForClientResult OnClientConnected(
			vint clientId,
			const IJsonChannelClient::ChannelNameList& availableChannels,
			IJsonChannelClient* localClient
			) override
		{
			auto rendererChannel = IsRendererChannel(availableChannels);
			auto viewModelHostChannel = IsRemoteViewModelHostChannel(availableChannels);
			RemoteViewModelRequesterSession* requesterSession = nullptr;
			bool requesterSessionRequired = false;
			SPIN_LOCK(lockConnection)
			{
				requesterSession = remoteViewModelSession;
				requesterSessionRequired = remoteViewModelRequired;
			}
			if (localClient)
			{
				if (requesterSession && requesterSession->CanAcceptLocalClient(localClient))
				{
					return WaitForClientResult::Accept;
				}
				if (
					rendererChannel &&
					dynamic_cast<GuiRemoteProtocolLocalChannelClient*>(localClient)
					)
				{
					return WaitForClientResult::Accept;
				}
				return WaitForClientResult::Reject;
			}

			if (rendererChannel)
			{
				if (
					requesterSessionRequired &&
					(!requesterSession || !requesterSession->CanAdmitRenderer())
					)
				{
					return WaitForClientResult::Reject;
				}

				IJsonChannel* jsonChannelToOldRenderer = nullptr;
				vint oldRendererClientId = InvalidRemoteViewModelClientId;
				SPIN_LOCK(lockConnection)
				{
					if (clientId == rendererClientId)
					{
						return WaitForClientResult::Reject;
					}
					oldRendererClientId = rendererClientId;
					rendererClientId = clientId;
					if (oldRendererClientId != InvalidRemoteViewModelClientId)
					{
						jsonChannelToOldRenderer = coreJsonChannel;
						if (coreProtocolChannel)
						{
							coreProtocolChannel->DetachRenderer(oldRendererClientId);
						}
					}
				}

				if (oldRendererClientId != InvalidRemoteViewModelClientId)
				{
					bool rendererNotifiedToStop = false;
					if (jsonChannelToOldRenderer)
					{
						try
						{
							Ptr<glr::json::JsonObject> package;
							ChannelPackageInfo info{ ChannelPackageSemantic::Message, -1, WString::Unmanaged(L"ControllerConnectionStopped") };
							JsonChannelPack(info, {}, package);
							jsonChannelToOldRenderer->SendToClient(oldRendererClientId, package);
							bool disconnected = false;
							jsonChannelToOldRenderer->BatchWrite(disconnected);
							rendererNotifiedToStop = !disconnected;
						}
						catch (const Error&)
						{
						}
						catch (const Exception&)
						{
						}
					}
					if (!rendererNotifiedToStop)
					{
						this->DisconnectClient(oldRendererClientId);
					}
				}
				Console::WriteLine(L"> Renderer transport connected: " + itow(clientId));
				return WaitForClientResult::Accept;
			}
			else if (viewModelHostChannel)
			{
				if (
					!requesterSession ||
					!requesterSession->TryAcceptViewModelHost(clientId)
					)
				{
					return WaitForClientResult::Reject;
				}
				Console::WriteLine(L"> RemotingTest_RvmHost transport connected: " + itow(clientId));
				return WaitForClientResult::Accept;
			}
			else
			{
				return WaitForClientResult::Reject;
			}
		}

		void OnClientDisconnected(vint clientId) override
		{
			bool rendererDisconnected = false;
			RemoteViewModelRequesterSession* requesterSession = nullptr;
			SPIN_LOCK(lockConnection)
			{
				requesterSession = remoteViewModelSession;
				if (rendererClientId == clientId)
				{
					rendererClientId = InvalidRemoteViewModelClientId;
					if (coreProtocolChannel)
					{
						coreProtocolChannel->DetachRenderer(clientId);
					}
					rendererDisconnected = true;
				}
			}

			if (requesterSession)
			{
				requesterSession->OnClientDisconnected(clientId);
			}
			if (rendererDisconnected)
			{
				Console::WriteLine(L"> Renderer transport disconnected: " + itow(clientId));
			}
		}
	};

	class RemotingTestCoreChannel : public GuiRemoteProtocolCoreChannel
	{
		using Base = GuiRemoteProtocolCoreChannel;
		Func<vint()> getTransportRendererClientId;

	public:
		RemotingTestCoreChannel(
			IJsonChannelClient* client,
			IJsonChannel* channel,
			const WString& executablePath,
			IGuiRemoteEventProcessor* eventProcessor,
			const Func<vint()>& _getTransportRendererClientId
			)
			: Base(client, channel, executablePath, eventProcessor)
			, getTransportRendererClientId(_getTransportRendererClientId)
		{
		}

		void Submit(bool& disconnected) override
		{
			auto receiverClientId = GetRendererClientId();
			collections::List<JsonPackage> packages;
			SPIN_LOCK(lockPackagesBeforeRenderer)
			{
				packages = std::move(packagesBeforeRenderer);
			}

			if (receiverClientId == -1 || receiverClientId != getTransportRendererClientId())
			{
				disconnected = true;
				return;
			}

			for (auto&& package : packages)
			{
				channel->SendToClient(receiverClientId, package);
			}
			channel->BatchWrite(disconnected);
		}
	};

#ifdef VCZH_MSVC
	class NamedPipeRemotingChannelServer : public RemotingChannelServerBase<named_pipe::NamedPipeServer>
	{
		using Base = RemotingChannelServerBase<named_pipe::NamedPipeServer>;

	public:
		NamedPipeRemotingChannelServer(Ptr<glr::json::Parser> parser, const WString& pipeName)
			: Base(parser, pipeName)
		{
		}
	};

	class HttpRemotingChannelServer : public RemotingChannelServerBase<windows_http::HttpServer>
	{
		using Base = RemotingChannelServerBase<windows_http::HttpServer>;

	public:
		HttpRemotingChannelServer(Ptr<glr::json::Parser> parser, const WString& baseUrl, vint port)
			: Base(parser, baseUrl, port)
		{
		}
	};
#endif

	class MiniHttpRemotingChannelServer : public RemotingChannelServerBase<async_tcp_socket::SocketHttpServer>
	{
		using Base = RemotingChannelServerBase<async_tcp_socket::SocketHttpServer>;

	public:
		MiniHttpRemotingChannelServer(
			Ptr<glr::json::Parser> parser,
			Ptr<async_tcp_socket::IAsyncSocketServer> socketServer,
			const WString& baseUrl
			)
			: Base(parser, socketServer, baseUrl)
		{
		}
	};
}

IJsonLocalChannelServer* protocolServer = nullptr;
RemoteViewModelRequesterSession* remoteViewModelSession = nullptr;
Ptr<rvmt::IViewModel> remoteViewModel;
vint mainWindowConstructorIndex = 0;
int mainProcessResult = 0;
#ifdef VCZH_MSVC
bool useWindowsHttpAutomationService = true;
#endif
Ptr<async_tcp_socket::IAsyncSocketServer>* miniHttpAutomationSocketServer = nullptr;

namespace
{
	class CoreFatalState
	{
	private:
		SpinLock					lockState;
		EventObject					delivered;
		bool						initialized = false;
		bool						claimed = false;
		bool						deliveryFinished = false;
		WString						message;

	public:
		void Initialize()
		{
			SPIN_LOCK(lockState)
			{
				if (!initialized)
				{
					CHECK_ERROR(delivered.CreateManualUnsignal(false), L"CoreFatalState::Initialize()#Failed to create the fatal-delivery event.");
					initialized = true;
				}
			}
		}

		WString Deliver(IJsonLocalChannelServer* server, const WString& errorMessage)
		{
			bool deliver = false;
			bool wait = false;
			SPIN_LOCK(lockState)
			{
				CHECK_ERROR(initialized, L"CoreFatalState::Deliver(...)#The fatal state has not been initialized.");
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

			WString result;
			SPIN_LOCK(lockState)
			{
				result = message;
			}
			return result;
		}
	};

	CoreFatalState* coreFatalState = nullptr;
}

void GuiMain()
{
	try
	{
		theme::RegisterTheme(Ptr(new darkskin::Theme));
		Ptr<GuiWindow> window;
		switch (mainWindowConstructorIndex)
		{
		case 1:
			window = Ptr(new rptest::RpMainWindow);
			break;
		case 2:
			CHECK_ERROR(remoteViewModelSession, L"GuiMain()#The RVM requester session is null.");
			CHECK_ERROR(remoteViewModel, L"GuiMain()#The rvmt::IViewModel proxy is null.");
			window = Ptr(new rvmt::MainWindow(remoteViewModel));
			break;
		default:
		case 0:
			window = Ptr(new demo::MainWindow);
		}
		window->ForceCalculateSizeImmediately();

		RemoteProtocolAutomationService automationService;
		GetNativeServiceSubstitution()->Substitute(&automationService, false);
		auto cleanup = [&]()
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
			GetNativeServiceSubstitution()->Unsubstitute(&automationService);
		};
		try
		{
#ifdef VCZH_MSVC
			if (useWindowsHttpAutomationService)
			{
				windows::StartWindowsHttpAutomationService(WString::Unmanaged(L"Automation/RemotingTest_Core"), RemotingHttpPort);
			}
			else
#endif
			{
				StartMiniHttpAutomationService(*miniHttpAutomationSocketServer);
			}

			if (mainWindowConstructorIndex == 2)
			{
				if (!remoteViewModelSession->BeginRunning())
				{
					auto failure = remoteViewModelSession->GetFatalError();
					throw Exception(failure ? failure.Value() : WString::Unmanaged(L"RemotingTest_RvmHost was not available before window startup."));
				}
				Console::WriteLine(L"> rvmt::IViewModel acquired; renderer admission is open.");
			}

			try
			{
				GetApplication()->Run(window.Obj());
			}
			catch (...)
			{
				if (mainWindowConstructorIndex == 2)
				{
					remoteViewModelSession->BeginStopping();
				}
				throw;
			}

			if (mainWindowConstructorIndex == 2)
			{
				remoteViewModelSession->BeginStopping();
				if (auto failure = remoteViewModelSession->GetFatalError())
				{
					throw Exception(failure.Value());
				}
			}
		}
		catch (...)
		{
			cleanup();
			throw;
		}
		cleanup();
	}
	catch (const Exception& e)
	{
		mainProcessResult = 1;
		auto message = e.Message();
		if (mainWindowConstructorIndex == 2 && remoteViewModelSession)
		{
			remoteViewModelSession->BeginStopping();
			if (auto failure = remoteViewModelSession->GetFatalError())
			{
				message = failure.Value();
			}
		}
		message = coreFatalState->Deliver(protocolServer, message);
		Console::WriteLine(L"Error: " + message);
	}
	catch (const Error& e)
	{
		mainProcessResult = 1;
		auto message = WString::Unmanaged(e.Description());
		if (mainWindowConstructorIndex == 2 && remoteViewModelSession)
		{
			remoteViewModelSession->BeginStopping();
			if (auto failure = remoteViewModelSession->GetFatalError())
			{
				message = failure.Value();
			}
		}
		message = coreFatalState->Deliver(protocolServer, message);
		Console::WriteLine(L"Error: " + message);
	}
}

template<typename TServerBase>
int StartServer(RemotingChannelServerBase<TServerBase>& channelServer, Ptr<glr::json::Parser> jsonParser)
{
	const bool useRemoteViewModel = mainWindowConstructorIndex == 2;
	CoreFatalState fatalState;
	fatalState.Initialize();
	Ptr<RemoteViewModelRequesterSession> requesterSession;
	if (useRemoteViewModel)
	{
		requesterSession = Ptr(new RemoteViewModelRequesterSession(
			jsonParser,
			Func<void(const WString&)>([&channelServer, &fatalState](const WString& message)
			{
				auto deliveredMessage = fatalState.Deliver(&channelServer, message);
				Console::WriteLine(L"Error: " + deliveredMessage);
				std::_Exit(1);
			})
			));
		channelServer.RequireRemoteViewModel();
	}

	bool channelServerStarted = false;
	bool requesterSessionStarted = false;
	int result = 1;

	channelServer.Start();
	channelServerStarted = true;

	auto coreClient = Ptr(new GuiRemoteProtocolLocalChannelClient(jsonParser));
	auto coreClientId = channelServer.ConnectLocalClient(coreClient);
	CHECK_ERROR(coreClientId == GacUIRemoteProtocolCoreClientId, L"StartServer(RemotingChannelServerBase&, Ptr<Parser>)#Failed to register the core channel client.");
	channelServer.SetCoreJsonChannel(coreClient->GetProtocolChannel());

	GuiRemoteProtocolAsyncJsonChannel asyncChannelSender(coreClient->GetProtocolChannel());
	RemotingTestCoreChannel channelSender(
		coreClient.Obj(),
		&asyncChannelSender,
		WString::Unmanaged(L"RemotingTest_Core.vcxproj"),
		asyncChannelSender.GetRemoteEventProcessor(),
		[&channelServer]()
		{
			return channelServer.GetRendererClientId();
		}
		);
	GuiRemoteProtocolFilter filteredProtocol(&channelSender);
	GuiRemoteProtocolDomDiffConverter diffConverterProtocol(&filteredProtocol);

	channelServer.SetCoreProtocolChannel(&channelSender);
	protocolServer = &channelServer;
	mainProcessResult = 0;

	try
	{
		if (useRemoteViewModel)
		{
			channelServer.SetRemoteViewModelSession(requesterSession.Obj());
			requesterSession->Start(&channelServer);
			requesterSessionStarted = true;
			Console::WriteLine(L"> Waiting for RemotingTest_RvmHost on ViewModelChannel.");
			remoteViewModel = requesterSession->RequestViewModel();
			remoteViewModelSession = requesterSession.Obj();
		}

		coreFatalState = &fatalState;
		try
		{
			SetupRemoteNativeController(&diffConverterProtocol);
		}
		catch (...)
		{
			coreFatalState = nullptr;
			throw;
		}
		coreFatalState = nullptr;
		result = mainProcessResult;
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

	remoteViewModelSession = nullptr;
	remoteViewModel = nullptr;

#ifdef VCZH_MSVC
	if constexpr (std::is_same_v<TServerBase, named_pipe::NamedPipeServer>)
	{
		auto rendererClientId = channelServer.GetRendererClientId();
		if (rendererClientId != -1)
		{
			channelServer.DisconnectClient(rendererClientId);
		}
	}
#endif

	protocolServer = nullptr;
	channelServer.SetCoreProtocolChannel(nullptr);
	channelServer.SetCoreJsonChannel(nullptr);

	if (requesterSessionStarted)
	{
		requesterSession->Stop(Func<void()>([&channelServer]()
			{
				channelServer.Stop();
			}));
		channelServer.ClearRemoteViewModelSession();
	}
	else if (channelServerStarted)
	{
		try
		{
			channelServer.Stop();
		}
		catch (const Exception& e)
		{
			Console::WriteLine(L"Error during channel cleanup: " + e.Message());
			result = 1;
		}
		catch (const Error& e)
		{
			Console::WriteLine(L"Error during channel cleanup: " + WString::Unmanaged(e.Description()));
			result = 1;
		}
	}
	return result;
}

#ifdef VCZH_MSVC
int StartNamedPipeServer(vint index)
{
	mainWindowConstructorIndex = index;
	useWindowsHttpAutomationService = true;
	Console::WriteLine(L"> Named pipe created, waiting on: " + WString::Unmanaged(RemotingNamedPipeName));
	auto jsonParser = Ptr(new glr::json::Parser);
	NamedPipeRemotingChannelServer channelServer(jsonParser, WString::Unmanaged(RemotingNamedPipeName));
	return StartServer(channelServer, jsonParser);
}

int StartHttpServer(vint index)
{
	mainWindowConstructorIndex = index;
	useWindowsHttpAutomationService = true;
	Console::WriteLine(L"> HTTP server created, waiting on: http://localhost:" + itow(RemotingHttpPort) + WString::Unmanaged(RemotingHttpBaseUrl));
	auto jsonParser = Ptr(new glr::json::Parser);
	HttpRemotingChannelServer channelServer(jsonParser, WString::Unmanaged(RemotingHttpBaseUrl), RemotingHttpPort);
	return StartServer(channelServer, jsonParser);
}
#endif

int StartMiniHttpServer(vint index)
{
	mainWindowConstructorIndex = index;
#ifdef VCZH_MSVC
	useWindowsHttpAutomationService = false;
#endif
	Console::WriteLine(L"> Mini HTTP server created, waiting on: http://localhost:" + itow(RemotingHttpPort) + WString::Unmanaged(RemotingHttpBaseUrl));
	auto jsonParser = Ptr(new glr::json::Parser);
	auto socketServer = async_tcp_socket::CreateDefaultAsyncSocketServer(RemotingHttpPort);
	MiniHttpRemotingChannelServer channelServer(jsonParser, socketServer, WString::Unmanaged(RemotingHttpBaseUrl));
	miniHttpAutomationSocketServer = &socketServer;
	try
	{
		auto result = StartServer(channelServer, jsonParser);
		miniHttpAutomationSocketServer = nullptr;
		return result;
	}
	catch (...)
	{
		miniHttpAutomationSocketServer = nullptr;
		throw;
	}
}
