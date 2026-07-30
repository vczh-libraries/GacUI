#include "DarkSkin.h"
#include "RpMainWindow.h"
#include "MainWindow.h"
#include "RemoteViewModelTestIncludes.h"
#include "../RemotingTest_RvmHost/RemoteViewModelRoleState.h"
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

namespace rpc_channeling = vl::rpc_controller::channeling;

extern void StartMiniHttpAutomationService(Ptr<async_tcp_socket::IAsyncSocketServer> socketServer);
extern void StopMiniHttpAutomationService();

namespace
{
	class IRemoteViewModelCoreState
	{
	public:
		virtual bool					BeginRunning(const Func<void()>& callback) = 0;
		virtual void					BeginStopping() = 0;
		virtual Nullable<WString>		GetFatalError() = 0;
	};

	template<typename TServerBase>
	class RemotingChannelServerBase
		: public GuiRemoteProtocolNetworkChannelServer<TServerBase>
		, public IRemoteViewModelCoreState
	{
		using Base = GuiRemoteProtocolNetworkChannelServer<TServerBase>;

	protected:
		// Covers all role IDs, channel pointers, phase, and fatal state.
		SpinLock								lockConnection;
		IJsonChannel*							coreJsonChannel = nullptr;
		GuiRemoteProtocolCoreChannel*			coreProtocolChannel = nullptr;

		RemoteViewModelRoleState				roleState;
		WString									fatalError;
		Func<void()>							fatalCallback;
		rpc_channeling::RpcJsonDispatcherServer*
												brokerDispatcher = nullptr;
		vint									brokerViewModelHostId = InvalidRemoteViewModelClientId;

	public:
		using Base::OnClientConnected;

		template<typename... TArgs>
		RemotingChannelServerBase(Ptr<glr::json::Parser> parser, TArgs&&... args)
			: Base(parser, std::forward<TArgs>(args)...)
		{
		}

		void EnableRemoteViewModel()
		{
			SPIN_LOCK(lockConnection)
			{
				CHECK_ERROR(roleState.EnableRemoteViewModel(), L"RemotingChannelServerBase::EnableRemoteViewModel()#Remote view model mode has already been enabled.");
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
				clientId = roleState.GetRendererClientId();
			}
			return clientId;
		}

		WaitForClientResult OnClientConnected(
			vint clientId,
			const IJsonChannelClient::ChannelNameList& availableChannels,
			IJsonChannelClient* localClient
			) override
		{
			auto role = ClassifyRemoteViewModelChannel(availableChannels);
			if (localClient)
			{
				if (
					dynamic_cast<RemoteViewModelBroadcastingLocalClient*>(localClient) ||
					dynamic_cast<RemoteViewModelReadyLocalClient*>(localClient)
					)
				{
					return WaitForClientResult::Accept;
				}
				if (dynamic_cast<RemoteViewModelRequesterLocalClient*>(localClient))
				{
					SPIN_LOCK(lockConnection)
					{
						return
							brokerDispatcher &&
							brokerDispatcher->HasServerClientId()
							? WaitForClientResult::Accept
							: WaitForClientResult::Reject;
					}
				}
				if (
					role == RemoteViewModelChannelRole::Renderer &&
					dynamic_cast<GuiRemoteProtocolLocalChannelClient*>(localClient)
					)
				{
					return WaitForClientResult::Accept;
				}
				return WaitForClientResult::Reject;
			}

			if (role == RemoteViewModelChannelRole::Renderer)
			{
				IJsonChannel* jsonChannelToOldRenderer = nullptr;
				vint oldRendererClientId = InvalidRemoteViewModelClientId;
				SPIN_LOCK(lockConnection)
				{
					if (!roleState.TryAcceptRenderer(clientId, oldRendererClientId))
					{
						return WaitForClientResult::Reject;
					}
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
			else if (role == RemoteViewModelChannelRole::ViewModelHost)
			{
				SPIN_LOCK(lockConnection)
				{
					if (!roleState.IsRemoteViewModelEnabled())
					{
						return WaitForClientResult::Reject;
					}
					if (
						!brokerDispatcher ||
						!brokerDispatcher->HasServerClientId() ||
						!roleState.TryAcceptViewModelHost(clientId)
						)
					{
						return WaitForClientResult::Reject;
					}
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
			bool brokerClient = false;
			bool fatalViewModelHost = false;
			rpc_channeling::RpcJsonDispatcherServer* targetBroker = nullptr;
			SPIN_LOCK(lockConnection)
			{
				targetBroker = brokerDispatcher;
				brokerClient = roleState.GetViewModelHostId() == clientId;
				fatalViewModelHost = roleState.FailViewModelHost(clientId);
				if (brokerViewModelHostId == clientId)
				{
					brokerViewModelHostId = InvalidRemoteViewModelClientId;
				}
				if (roleState.DisconnectRenderer(clientId))
				{
					if (coreProtocolChannel)
					{
						coreProtocolChannel->DetachRenderer(clientId);
					}
					rendererDisconnected = true;
				}
				if (roleState.DisconnectRequester(clientId))
				{
					brokerClient = true;
				}
			}

			if (fatalViewModelHost)
			{
				Console::WriteLine(L"Fatal error: RemotingTest_RvmHost disconnected.");
				std::_Exit(1);
			}
			if (brokerClient && targetBroker)
			{
				targetBroker->DisconnectClient(clientId);
			}
			if (rendererDisconnected)
			{
				Console::WriteLine(L"> Renderer transport disconnected: " + itow(clientId));
			}
		}

		void SetBrokerDispatcher(rpc_channeling::RpcJsonDispatcherServer* dispatcher)
		{
			SPIN_LOCK(lockConnection)
			{
				brokerDispatcher = dispatcher;
			}
		}

		void RegisterViewModelHost(vint clientId)
		{
			rpc_channeling::RpcJsonDispatcherServer* targetBroker = nullptr;
			SPIN_LOCK(lockConnection)
			{
				CHECK_ERROR(
					roleState.GetViewModelHostId() == clientId,
					L"RemotingChannelServerBase::RegisterViewModelHost(...)#The ready client is not the accepted view-model host."
					);
				CHECK_ERROR(
					brokerViewModelHostId == InvalidRemoteViewModelClientId,
					L"RemotingChannelServerBase::RegisterViewModelHost(...)#The view-model host is already registered."
					);
				targetBroker = brokerDispatcher;
				brokerViewModelHostId = clientId;
			}
			CHECK_ERROR(targetBroker, L"RemotingChannelServerBase::RegisterViewModelHost(...)#The broker dispatcher is null.");
			targetBroker->RegisterClient(clientId);
		}

		void RegisterRequester(vint clientId)
		{
			rpc_channeling::RpcJsonDispatcherServer* targetBroker = nullptr;
			SPIN_LOCK(lockConnection)
			{
				CHECK_ERROR(roleState.RegisterRequester(clientId), L"RemotingChannelServerBase::RegisterRequester(...)#A requester has already been registered.");
				targetBroker = brokerDispatcher;
			}
			CHECK_ERROR(targetBroker, L"RemotingChannelServerBase::RegisterRequester(...)#The broker dispatcher is null.");
			targetBroker->RegisterClient(clientId);
		}

		void FailRpcTaskQueue(const WString& message)
		{
			bool claimed = false;
			Func<void()> targetCallback;
			SPIN_LOCK(lockConnection)
			{
				if (
					fatalError == L"" &&
					roleState.FailRemoteViewModel()
					)
				{
					fatalError = message;
					targetCallback = fatalCallback;
					claimed = true;
				}
			}
			if (claimed && targetCallback)
			{
				targetCallback();
			}
			else if (claimed)
			{
				Console::WriteLine(L"Fatal error: " + message);
				std::_Exit(1);
			}
		}

		bool BeginRunning(const Func<void()>& callback) override
		{
			bool succeeded = false;
			SPIN_LOCK(lockConnection)
			{
				if (
					fatalError == L"" &&
					roleState.BeginRunning()
					)
				{
					fatalCallback = callback;
					succeeded = true;
				}
			}
			return succeeded;
		}

		void BeginStopping() override
		{
			SPIN_LOCK(lockConnection)
			{
				if (roleState.IsRemoteViewModelEnabled())
				{
					roleState.BeginStopping();
					fatalCallback = {};
				}
			}
		}

		Nullable<WString> GetFatalError() override
		{
			Nullable<WString> result;
			SPIN_LOCK(lockConnection)
			{
				if (fatalError != L"")
				{
					result = fatalError;
				}
			}
			return result;
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

		~MiniHttpRemotingChannelServer()
		{
			this->Stop();
		}
	};
}

IJsonLocalChannelServer* protocolServer = nullptr;
IGuiRemoteProtocol* rendererProtocol = nullptr;
IRemoteViewModelCoreState* remoteViewModelCoreState = nullptr;
Ptr<rvmt::IViewModel> remoteViewModel;
vint mainWindowConstructorIndex = 0;
int mainProcessResult = 0;
#ifdef VCZH_MSVC
bool useWindowsHttpAutomationService = true;
#endif
Ptr<async_tcp_socket::IAsyncSocketServer>* miniHttpAutomationSocketServer = nullptr;

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
			CHECK_ERROR(remoteViewModelCoreState, L"GuiMain()#The RVM Core state is null.");
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
				if (!remoteViewModelCoreState->BeginRunning(Func<void()>([window]()
				{
					GetApplication()->InvokeInMainThread(window.Obj(), [window]()
					{
						window->Close();
					});
				})))
				{
					auto failure = remoteViewModelCoreState->GetFatalError();
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
					remoteViewModelCoreState->BeginStopping();
				}
				throw;
			}

			if (mainWindowConstructorIndex == 2)
			{
				remoteViewModelCoreState->BeginStopping();
				if (auto failure = remoteViewModelCoreState->GetFatalError())
				{
					throw Exception(failure.Value());
				}
			}

			if (rendererProtocol)
			{
				rendererProtocol->RequestControllerConnectionStopped();
				bool disconnected = false;
				rendererProtocol->Submit(disconnected);
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
		if (mainWindowConstructorIndex == 2 && remoteViewModelCoreState)
		{
			remoteViewModelCoreState->BeginStopping();
		}
		if (protocolServer)
		{
			try
			{
				protocolServer->BroadcastError(e.Message());
			}
			catch (...)
			{
			}
		}
		Console::WriteLine(L"Error: " + e.Message());
	}
	catch (const Error& e)
	{
		mainProcessResult = 1;
		if (mainWindowConstructorIndex == 2 && remoteViewModelCoreState)
		{
			remoteViewModelCoreState->BeginStopping();
		}
		auto message = WString::Unmanaged(e.Description());
		if (protocolServer)
		{
			try
			{
				protocolServer->BroadcastError(message);
			}
			catch (...)
			{
			}
		}
		Console::WriteLine(L"Error: " + message);
	}
}

template<typename TServerBase>
int StartServer(RemotingChannelServerBase<TServerBase>& channelServer, Ptr<glr::json::Parser> jsonParser)
{
	const bool useRemoteViewModel = mainWindowConstructorIndex == 2;
	if (useRemoteViewModel)
	{
		channelServer.EnableRemoteViewModel();
	}

	bool channelServerStarted = false;
	Ptr<rpc_channeling::TaskQueue> taskQueue;
	Ptr<RemoteViewModelTaskQueueThread> taskQueueThread;
	Ptr<RemoteViewModelBroadcastingLocalClient> broadcastingClient;
	Ptr<RemoteViewModelReadyLocalClient> readyClient;
	Ptr<RemoteViewModelRequesterLocalClient> requesterClient;
	RemoteViewModelJsonDispatcherClient* requesterDispatcher = nullptr;
	bool taskQueueStarted = false;
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
	rendererProtocol = &diffConverterProtocol;
	mainProcessResult = 0;

	try
	{
		if (useRemoteViewModel)
		{
			taskQueue = Ptr(new rpc_channeling::TaskQueue);
			taskQueueThread = Ptr(new RemoteViewModelTaskQueueThread(taskQueue));
			broadcastingClient = Ptr(new RemoteViewModelBroadcastingLocalClient(jsonParser));
			requesterClient = Ptr(new RemoteViewModelRequesterLocalClient(jsonParser));

			broadcastingClient->Connect(&channelServer, broadcastingClient, taskQueue);
			auto brokerDispatcher = broadcastingClient->GetDispatcher();

			readyClient = Ptr(new RemoteViewModelReadyLocalClient(
				jsonParser,
				Func<void(vint)>([&channelServer](vint clientId)
				{
					channelServer.RegisterViewModelHost(clientId);
				})
				));
			readyClient->Connect(&channelServer, readyClient);
			channelServer.SetBrokerDispatcher(brokerDispatcher);

			auto requesterClientId = requesterClient->Connect(
				&channelServer,
				requesterClient,
				taskQueue,
				brokerDispatcher->GetServerClientId()
			);
			requesterDispatcher = requesterClient->GetDispatcher();
			channelServer.RegisterRequester(requesterClientId);
			taskQueueThread->SetFailureCallback(Func<void(const WString&)>([&channelServer](const WString& message)
			{
				channelServer.FailRpcTaskQueue(WString::Unmanaged(L"RPC task queue failed: ") + message);
			}));

			CHECK_ERROR(taskQueueThread->Start(), L"StartServer(...)#Failed to start the RPC task queue thread.");
			taskQueueStarted = true;

			Console::WriteLine(L"> Waiting for RemotingTest_RvmHost on ViewModelChannel.");
			requesterDispatcher->Initialize();
			remoteViewModel = requesterDispatcher
				->GetRpcLifecycle()
				->RequestService(WString::Unmanaged(ViewModelServiceName))
				.Cast<rvmt::IViewModel>();
			CHECK_ERROR(remoteViewModel, L"StartServer(...)#Failed to request rvmt::IViewModel.");
			remoteViewModelCoreState = &channelServer;
		}

		SetupRemoteNativeController(&diffConverterProtocol);
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

	if (useRemoteViewModel)
	{
		channelServer.BeginStopping();
	}
	remoteViewModelCoreState = nullptr;
	remoteViewModel = nullptr;
	rendererProtocol = nullptr;

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

	if (requesterDispatcher)
	{
		try
		{
			if (taskQueueStarted)
			{
				FinalizeRpcOnTaskQueue(taskQueue, requesterDispatcher);
			}
			else
			{
				requesterDispatcher->FinalizeRpc();
			}
		}
		catch (const Exception& e)
		{
			Console::WriteLine(L"Error during RPC cleanup: " + e.Message());
			result = 1;
		}
		catch (const Error& e)
		{
			Console::WriteLine(L"Error during RPC cleanup: " + WString::Unmanaged(e.Description()));
			result = 1;
		}
	}

	protocolServer = nullptr;
	channelServer.SetCoreProtocolChannel(nullptr);
	channelServer.SetCoreJsonChannel(nullptr);

	if (channelServerStarted)
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
	if (taskQueueStarted)
	{
		if (auto failure = StopRpcTaskQueue(taskQueue, taskQueueThread.Obj()))
		{
			if (result == 0)
			{
				Console::WriteLine(L"Error during RPC task queue cleanup: " + failure.Value());
			}
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
