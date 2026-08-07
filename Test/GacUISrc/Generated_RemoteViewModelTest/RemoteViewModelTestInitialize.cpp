#include "RemoteViewModelTestInitialize.h"
#include "RemoteViewModelTestRpc.h"

namespace vl::presentation::remote_view_model_test
{
	using namespace collections;
	using namespace reflection;
	using namespace reflection::description;
	using namespace rpc_controller;

	void RemoteViewModelTestInitialize::InitializeRpc(
		rpc_controller::channeling::RpcJsonDispatcherClient* dispatcher,
		vint clientId
		)
	{
		CHECK_ERROR(dispatcher, L"RemoteViewModelTestInitialize::InitializeRpc(...)#The dispatcher is null.");
		auto& app = vl_workflow_global::RemoteViewModelTestRpc::Instance();
		auto rpcDispatcher = Ptr(new RpcJsonDispatcher(clientId, dispatcher));
		auto lifecycle = Ptr(new RpcJsonLifecycle(clientId, rpcDispatcher.Obj()));
		dispatcher->SetRpcObjects(rpcDispatcher, lifecycle);

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
