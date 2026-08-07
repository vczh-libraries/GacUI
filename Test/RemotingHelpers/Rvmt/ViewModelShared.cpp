#include "ViewModelShared.h"
#include "RemoteViewModelTestRpc.h"

namespace vl::presentation::remoting
{
	JsonPackage CreateViewModelReadyMessage()
	{
		auto package = Ptr(new glr::json::JsonString);
		package->content.value = ViewModelReadyMessage;
		return package;
	}

	bool IsViewModelReadyMessage(const JsonPackage& package)
	{
		auto jsonString = package.Cast<glr::json::JsonString>();
		return jsonString && jsonString->content.value == ViewModelReadyMessage;
	}
}

namespace vl::presentation::remote_view_model_test
{
	using namespace collections;
	using namespace reflection;
	using namespace reflection::description;
	using namespace rpc_controller;
	using namespace rpc_controller::channeling;

	RemoteViewModelJsonDispatcherClient::RemoteViewModelJsonDispatcherClient(Ptr<remoting::TaskQueue> taskQueue)
		: RpcJsonDispatcherClientForTaskQueue(taskQueue)
	{
	}

	void RemoteViewModelJsonDispatcherClient::InitializeRpc(vint clientId)
	{
		auto& app = vl_workflow_global::RemoteViewModelTestRpc::Instance();
		auto rpcDispatcher = Ptr(new RpcJsonDispatcher(clientId, this));
		auto lifecycle = Ptr(new RpcJsonLifecycle(clientId, rpcDispatcher.Obj()));
		this->SetRpcObjects(rpcDispatcher, lifecycle);

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
}
