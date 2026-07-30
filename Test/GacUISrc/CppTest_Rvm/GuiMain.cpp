#include "DarkSkin.h"
#include "RemoteViewModelTestIncludes.h"
#include "../RemotingTest_RvmHost/RemoteViewModelRoleState.h"
#include "../RemotingTest_RvmHost/RemoteViewModelTestRuntime.h"
#include <cstdlib>

#ifdef VCZH_MSVC
#include "../../../Source/PlatformProviders/Windows/WinNativeWindow.h"
#include <VlppOS.Windows.h>
#include "resource.h"
#endif

using namespace vl;
using namespace vl::console;
using namespace vl::inter_process;
using namespace vl::presentation;
using namespace vl::presentation::controls;
using namespace vl::presentation::remote_view_model_test;
using namespace vl::rpc_controller::channeling;

extern void StartMiniHttpAutomationService(Ptr<async_tcp_socket::IAsyncSocketServer> socketServer, const WString& applicationName);
extern void StopMiniHttpAutomationService();

namespace
{
	class IRemoteViewModelUiServer
	{
	public:
		virtual bool					BeginRunning(const Func<void()>& callback) = 0;
		virtual void					BeginStopping() = 0;
		virtual Nullable<WString>		GetFatalError() = 0;
	};

	template<typename TServerBase>
	class RemoteViewModelUiServer
		: public JsonNetworkChannelServer<TServerBase>
		, public IRemoteViewModelUiServer
	{
		using Base = JsonNetworkChannelServer<TServerBase>;

	private:
		SpinLock								lockState;
		RemoteViewModelRoleState				roleState;
		WString									fatalError;
		Func<void()>							fatalCallback;
		RpcJsonDispatcherServer*				brokerDispatcher = nullptr;
		vint									brokerViewModelHostId = InvalidRemoteViewModelClientId;

	public:
		using Base::OnClientConnected;

		template<typename... TArgs>
		RemoteViewModelUiServer(Ptr<glr::json::Parser> parser, TArgs&&... args)
			: Base(parser, std::forward<TArgs>(args)...)
		{
			CHECK_ERROR(roleState.EnableRemoteViewModel(), L"RemoteViewModelUiServer::RemoteViewModelUiServer(...)#Failed to enable remote view model mode.");
		}

		WaitForClientResult OnClientConnected(
			vint clientId,
			const JsonChannelClient::ChannelNameList& availableChannels,
			JsonChannelClient* localClient
			) override
		{
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
					SPIN_LOCK(lockState)
					{
						return
							brokerDispatcher &&
							brokerDispatcher->HasServerClientId()
							? WaitForClientResult::Accept
							: WaitForClientResult::Reject;
					}
				}
				return WaitForClientResult::Reject;
			}

			if (ClassifyRemoteViewModelChannel(availableChannels) != RemoteViewModelChannelRole::ViewModelHost)
			{
				return WaitForClientResult::Reject;
			}

			SPIN_LOCK(lockState)
			{
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

		void OnClientDisconnected(vint clientId) override
		{
			RpcJsonDispatcherServer* targetBroker = nullptr;
			bool brokerClient = false;
			bool fatalViewModelHost = false;
			SPIN_LOCK(lockState)
			{
				targetBroker = brokerDispatcher;
				brokerClient = roleState.GetViewModelHostId() == clientId;
				fatalViewModelHost = roleState.FailViewModelHost(clientId);
				if (brokerViewModelHostId == clientId)
				{
					brokerViewModelHostId = InvalidRemoteViewModelClientId;
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
		}

		void SetBrokerDispatcher(RpcJsonDispatcherServer* dispatcher)
		{
			SPIN_LOCK(lockState)
			{
				brokerDispatcher = dispatcher;
			}
		}

		void RegisterViewModelHost(vint clientId)
		{
			RpcJsonDispatcherServer* targetBroker = nullptr;
			SPIN_LOCK(lockState)
			{
				CHECK_ERROR(
					roleState.GetViewModelHostId() == clientId,
					L"RemoteViewModelUiServer::RegisterViewModelHost(...)#The ready client is not the accepted view-model host."
					);
				CHECK_ERROR(
					brokerViewModelHostId == InvalidRemoteViewModelClientId,
					L"RemoteViewModelUiServer::RegisterViewModelHost(...)#The view-model host is already registered."
					);
				targetBroker = brokerDispatcher;
				brokerViewModelHostId = clientId;
			}
			CHECK_ERROR(targetBroker, L"RemoteViewModelUiServer::RegisterViewModelHost(...)#The broker dispatcher is null.");
			targetBroker->RegisterClient(clientId);
		}

		void RegisterRequester(vint clientId)
		{
			RpcJsonDispatcherServer* targetBroker = nullptr;
			SPIN_LOCK(lockState)
			{
				CHECK_ERROR(roleState.RegisterRequester(clientId), L"RemoteViewModelUiServer::RegisterRequester(...)#A requester has already been registered.");
				targetBroker = brokerDispatcher;
			}
			CHECK_ERROR(targetBroker, L"RemoteViewModelUiServer::RegisterRequester(...)#The broker dispatcher is null.");
			targetBroker->RegisterClient(clientId);
		}

		void FailRpcTaskQueue(const WString& message)
		{
			bool claimed = false;
			Func<void()> targetCallback;
			SPIN_LOCK(lockState)
			{
				if (fatalError == L"" && roleState.FailRemoteViewModel())
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
			SPIN_LOCK(lockState)
			{
				if (fatalError == L"" && roleState.BeginRunning())
				{
					fatalCallback = callback;
					succeeded = true;
				}
			}
			return succeeded;
		}

		void BeginStopping() override
		{
			SPIN_LOCK(lockState)
			{
				roleState.BeginStopping();
				fatalCallback = {};
			}
		}

		Nullable<WString> GetFatalError() override
		{
			Nullable<WString> result;
			SPIN_LOCK(lockState)
			{
				if (fatalError != L"")
				{
					result = fatalError;
				}
			}
			return result;
		}
	};

#ifdef VCZH_MSVC
	class NamedPipeRemoteViewModelUiServer
		: public RemoteViewModelUiServer<named_pipe::NamedPipeServer>
	{
		using Base = RemoteViewModelUiServer<named_pipe::NamedPipeServer>;

	public:
		NamedPipeRemoteViewModelUiServer(Ptr<glr::json::Parser> parser)
			: Base(parser, WString::Unmanaged(RemotingNamedPipeName))
		{
		}
	};

	class HttpRemoteViewModelUiServer
		: public RemoteViewModelUiServer<windows_http::HttpServer>
	{
		using Base = RemoteViewModelUiServer<windows_http::HttpServer>;

	public:
		HttpRemoteViewModelUiServer(Ptr<glr::json::Parser> parser)
			: Base(parser, WString::Unmanaged(RemotingHttpBaseUrl), RemotingHttpPort)
		{
		}
	};
#endif

	class MiniHttpRemoteViewModelUiServer
		: public RemoteViewModelUiServer<async_tcp_socket::SocketHttpServer>
	{
		using Base = RemoteViewModelUiServer<async_tcp_socket::SocketHttpServer>;

	public:
		MiniHttpRemoteViewModelUiServer(
			Ptr<glr::json::Parser> parser,
			Ptr<async_tcp_socket::IAsyncSocketServer> socketServer
			)
			: Base(parser, socketServer, WString::Unmanaged(RemotingHttpBaseUrl))
		{
		}

		~MiniHttpRemoteViewModelUiServer()
		{
			this->Stop();
		}
	};

	IRemoteViewModelUiServer* currentServer = nullptr;
	Ptr<rvmt::IViewModel> currentViewModel;
	int guiProcessResult = 0;
#ifdef VCZH_MSVC
	bool useWindowsHttpAutomationService = true;
#endif
	Ptr<async_tcp_socket::IAsyncSocketServer>* miniHttpAutomationSocketServer = nullptr;

	template<typename TServer>
	int StartServer(
		TServer& channelServer,
		Ptr<glr::json::Parser> parser,
		bool useWindowsAutomation
		)
	{
		auto taskQueue = Ptr(new TaskQueue);
		auto taskQueueThread = Ptr(new RemoteViewModelTaskQueueThread(taskQueue));
		auto broadcastingClient = Ptr(new RemoteViewModelBroadcastingLocalClient(parser));
		auto readyClient = Ptr(new RemoteViewModelReadyLocalClient(
			parser,
			Func<void(vint)>([&channelServer](vint clientId)
			{
				channelServer.RegisterViewModelHost(clientId);
			})
			));
		auto requesterClient = Ptr(new RemoteViewModelRequesterLocalClient(parser));
		RemoteViewModelJsonDispatcherClient* requesterDispatcher = nullptr;
		bool serverStarted = false;
		bool taskQueueStarted = false;
		int result = 1;

		(void)useWindowsAutomation;
		try
		{
			channelServer.Start();
			serverStarted = true;

			broadcastingClient->Connect(&channelServer, broadcastingClient, taskQueue);
			auto brokerDispatcher = broadcastingClient->GetDispatcher();
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
			auto viewModel = requesterDispatcher
				->GetRpcLifecycle()
				->RequestService(WString::Unmanaged(ViewModelServiceName))
				.Cast<rvmt::IViewModel>();
			CHECK_ERROR(viewModel, L"StartServer(...)#Failed to request rvmt::IViewModel.");

			currentServer = &channelServer;
			currentViewModel = viewModel;
			guiProcessResult = 0;
#ifdef VCZH_MSVC
			useWindowsHttpAutomationService = useWindowsAutomation;
			result = SetupHostedWindowsDirect2DRenderer();
#else
			result = 0;
#endif
			if (guiProcessResult != 0)
			{
				result = guiProcessResult;
			}
			currentViewModel = nullptr;
			currentServer = nullptr;
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

		channelServer.BeginStopping();
		currentViewModel = nullptr;
		currentServer = nullptr;

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
		if (serverStarted)
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
}

void GuiMain()
{
	try
	{
		CHECK_ERROR(currentServer, L"GuiMain()#The RVM channel server is null.");
		CHECK_ERROR(currentViewModel, L"GuiMain()#The rvmt::IViewModel proxy is null.");

		theme::RegisterTheme(Ptr(new darkskin::Theme));
		auto window = Ptr(new rvmt::MainWindow(currentViewModel));
		window->ForceCalculateSizeImmediately();
		window->MoveToScreenCenter();
#ifdef VCZH_MSVC
		windows::SetWindowDefaultIcon(MAINICON);
#endif

		if (!currentServer->BeginRunning(Func<void()>([window]()
		{
			GetApplication()->InvokeInMainThread(window.Obj(), [window]()
			{
				window->Close();
			});
		})))
		{
			auto failure = currentServer->GetFatalError();
			throw Exception(failure ? failure.Value() : WString::Unmanaged(L"RemotingTest_RvmHost was not available before window startup."));
		}
		Console::WriteLine(L"> rvmt::IViewModel acquired; local window is ready.");

		bool automationStarted = false;
		try
		{
#ifdef VCZH_MSVC
			if (useWindowsHttpAutomationService)
			{
				windows::StartWindowsHttpAutomationService(WString::Unmanaged(L"Automation/CppTest_Rvm"), RemotingHttpPort);
			}
			else
#endif
			{
				StartMiniHttpAutomationService(
					*miniHttpAutomationSocketServer,
					WString::Unmanaged(L"CppTest_Rvm")
					);
			}
			automationStarted = true;
			GetApplication()->Run(window.Obj());
		}
		catch (...)
		{
			currentServer->BeginStopping();
			if (automationStarted)
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
			}
			throw;
		}

		currentServer->BeginStopping();
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

		if (auto failure = currentServer->GetFatalError())
		{
			throw Exception(failure.Value());
		}
	}
	catch (const Exception& e)
	{
		if (currentServer)
		{
			currentServer->BeginStopping();
		}
		Console::WriteLine(L"Error: " + e.Message());
		guiProcessResult = 1;
	}
	catch (const Error& e)
	{
		if (currentServer)
		{
			currentServer->BeginStopping();
		}
		Console::WriteLine(L"Error: " + WString::Unmanaged(e.Description()));
		guiProcessResult = 1;
	}
}

#ifdef VCZH_MSVC
int StartNamedPipeServer()
{
	Console::WriteLine(L"> Named pipe created, waiting on: " + WString::Unmanaged(RemotingNamedPipeName));
	auto parser = Ptr(new glr::json::Parser);
	NamedPipeRemoteViewModelUiServer channelServer(parser);
	return StartServer(channelServer, parser, true);
}

int StartHttpServer()
{
	Console::WriteLine(L"> HTTP server created, waiting on: http://localhost:" + itow(RemotingHttpPort) + WString::Unmanaged(RemotingHttpBaseUrl));
	auto parser = Ptr(new glr::json::Parser);
	HttpRemoteViewModelUiServer channelServer(parser);
	return StartServer(channelServer, parser, true);
}
#endif

int StartMiniHttpServer()
{
	Console::WriteLine(L"> Mini HTTP server created, waiting on: http://localhost:" + itow(RemotingHttpPort) + WString::Unmanaged(RemotingHttpBaseUrl));
	auto parser = Ptr(new glr::json::Parser);
	auto socketServer = async_tcp_socket::CreateDefaultAsyncSocketServer(RemotingHttpPort);
	MiniHttpRemoteViewModelUiServer channelServer(parser, socketServer);
	miniHttpAutomationSocketServer = &socketServer;
	try
	{
		auto result = StartServer(channelServer, parser, false);
		miniHttpAutomationSocketServer = nullptr;
		return result;
	}
	catch (...)
	{
		miniHttpAutomationSocketServer = nullptr;
		throw;
	}
}
