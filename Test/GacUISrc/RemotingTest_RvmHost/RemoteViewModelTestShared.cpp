#include "RemoteViewModelTestShared.h"

namespace vl::presentation::remote_view_model_test
{
	using namespace collections;
	using namespace reflection;
	using namespace reflection::description;
	using namespace rpc_controller;
	using namespace rpc_controller::channeling;

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
