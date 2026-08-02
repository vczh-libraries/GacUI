#include "RemoteViewModelTestRuntime.h"
#include <cstdlib>

namespace vl::presentation::remote_view_model_test
{
	using namespace collections;
	using namespace reflection;
	using namespace reflection::description;
	using namespace rpc_controller;
	using namespace rpc_controller::channeling;

	namespace
	{
		constexpr vint RemoteViewModelCallTimeoutMilliseconds = 10000;

		class RemoteViewModelCall : public Object
		{
		public:
			EventObject									completed;
			WString										result;
			bool										failed = false;

			RemoteViewModelCall()
			{
				CHECK_ERROR(completed.CreateAutoUnsignal(false), L"RemoteViewModelCall::RemoteViewModelCall()#Failed to create the completion event.");
			}
		};

		class RemoteViewModelFailFastProxy : public Object, public virtual rvmt::IViewModel
		{
		private:
			Ptr<rvmt::IViewModel>							viewModel;
			Func<void(const WString&)>						terminalAction;

			[[noreturn]] void Fail()
			{
				terminalAction(WString::Unmanaged(RemoteViewModelHostDisconnectedError));
				std::_Exit(1);
			}

		public:
			RemoteViewModelFailFastProxy(
				Ptr<rvmt::IViewModel> _viewModel,
				const Func<void(const WString&)>& _terminalAction
				)
				: viewModel(_viewModel)
				, terminalAction(_terminalAction)
			{
			}

			WString Translate(const WString& name) override
			{
				auto call = Ptr(new RemoteViewModelCall);
				auto worker = Thread::CreateAndStart(Func<void()>([call, viewModel = viewModel, name]()
				{
					try
					{
						call->result = viewModel->Translate(name);
					}
					catch (...)
					{
						call->failed = true;
					}
					call->completed.Signal();
				}), false);
				CHECK_ERROR(worker, L"RemoteViewModelFailFastProxy::Translate(...)#Failed to start the RPC worker thread.");

				if (!call->completed.WaitForTime(RemoteViewModelCallTimeoutMilliseconds))
				{
					Fail();
				}
				worker->Wait();
				delete worker;
				if (call->failed)
				{
					Fail();
				}
				return call->result;
			}
		};

		class RemoteViewModelJsonDispatcherClient
			: public remoting::RemotingJsonDispatcherClient
		{
		public:
			RemoteViewModelJsonDispatcherClient(Ptr<TaskQueue> taskQueue)
				: RemotingJsonDispatcherClient(taskQueue)
			{
			}

			void InitializeRpc(vint clientId) override
			{
				auto& app = vl_workflow_global::RemoteViewModelTestRpc::Instance();
				auto rpcDispatcher = Ptr(new RpcJsonDispatcher(clientId, this));
				auto lifecycle = Ptr(new RpcJsonLifecycle(clientId, rpcDispatcher.Obj()));
				SetRpcObjects(rpcDispatcher, lifecycle);

				auto idMap = UnboxParameter<Dictionary<WString, vint>>(BoxParameter(app.rpc_GetIds()));
				lifecycle->SetIdMap(idMap.Ref());

				auto serializer = app.rpcops_IRpcSerializer();
				auto objectOps = app.rpcops_IRpcObjectOpsJson(lifecycle.Obj());
				auto objectEventOps = app.rpcops_IRpcObjectEventOpsJson(lifecycle.Obj());
				auto ops = app.rpcops_IOps_CreateJson(lifecycle.Obj());

				lifecycle->Register(
					serializer,
					objectOps,
					objectEventOps,
					Func<vint(IDescriptable*)>([](IDescriptable* obj)
					{
						return vl_workflow_global::RemoteViewModelTestRpc::Instance().rpcwrapper_GetTypeId(BoxValue<IDescriptable*>(obj));
					}),
					Func<void(RpcObjectReference, IDescriptable*)>()
					);

				lifecycle->RegisterWrapperFactory(Func<Ptr<IRpcWrapperBase>(RpcObjectReference, IRpcLifecycle*)>(
					[ops](RpcObjectReference ref, IRpcLifecycle* lifecycle)
					{
						return vl_workflow_global::RemoteViewModelTestRpc::Instance().rpcwrapper_Create(ref, lifecycle, ops);
					}));
			}
		};

		remoting::RemotingRpcConfiguration CreateConfiguration()
		{
			remoting::RemotingRpcConfiguration configuration;
			configuration.rpcChannelName = WString::Unmanaged(ViewModelChannelName);
			configuration.controlChannelName = WString::Unmanaged(ViewModelReadyChannelName);
			configuration.serviceName = WString::Unmanaged(ViewModelServiceName);
			configuration.readyMessage = WString::Unmanaged(ViewModelReadyMessage);
			configuration.hostDisconnectedError = WString::Unmanaged(RemoteViewModelHostDisconnectedError);
			configuration.invalidClientId = InvalidRemoteViewModelClientId;
			return configuration;
		}

		remoting::RemotingDispatcherFactory CreateDispatcherFactory()
		{
			return remoting::RemotingDispatcherFactory([](Ptr<TaskQueue> taskQueue)
			{
				return Ptr<remoting::RemotingJsonDispatcherClient>(
					new RemoteViewModelJsonDispatcherClient(taskQueue)
					);
			});
		}
	}

	RemoteViewModelRequesterSession::RemoteViewModelRequesterSession(
		Ptr<glr::json::Parser> parser,
		const Func<void(const WString&)>& _terminalAction
		)
		: session(Ptr(new remoting::RemotingRequesterSession(
			CreateConfiguration(),
			CreateDispatcherFactory(),
			parser,
			_terminalAction
			)))
		, terminalAction(_terminalAction)
	{
	}

	bool RemoteViewModelRequesterSession::CanAcceptLocalClient(JsonChannelClient* localClient)
	{
		return session->CanAcceptLocalClient(localClient);
	}

	bool RemoteViewModelRequesterSession::TryAcceptViewModelHost(vint clientId)
	{
		return session->TryAcceptHost(clientId);
	}

	void RemoteViewModelRequesterSession::OnClientDisconnected(vint clientId)
	{
		session->OnClientDisconnected(clientId);
	}

	void RemoteViewModelRequesterSession::Start(JsonChannelServer* channelServer)
	{
		session->Start(channelServer);
	}

	Ptr<rvmt::IViewModel> RemoteViewModelRequesterSession::RequestViewModel()
	{
		auto viewModel = session->RequestService().Cast<rvmt::IViewModel>();
		CHECK_ERROR(viewModel, L"RemoteViewModelRequesterSession::RequestViewModel()#Failed to request rvmt::IViewModel.");
		return Ptr(new RemoteViewModelFailFastProxy(viewModel, terminalAction));
	}

	bool RemoteViewModelRequesterSession::BeginRunning()
	{
		return session->BeginRunning();
	}

	bool RemoteViewModelRequesterSession::CanAdmitRenderer()
	{
		return session->CanAdmitRenderer();
	}

	void RemoteViewModelRequesterSession::Stop(const Func<void()>& stopServer)
	{
		session->Stop(stopServer);
	}

	RemoteViewModelHostingClient::RemoteViewModelHostingClient(
		Ptr<inter_process::INetworkProtocolClient> networkClient,
		Ptr<glr::json::Parser> parser,
		Ptr<TaskQueue> taskQueue
		)
		: RemotingHostingClient(
			networkClient,
			CreateConfiguration(),
			CreateDispatcherFactory(),
			parser,
			taskQueue
			)
	{
	}
}
