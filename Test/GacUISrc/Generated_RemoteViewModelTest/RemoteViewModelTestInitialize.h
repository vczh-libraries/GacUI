#ifndef VCZH_PRESENTATION_REMOTE_VIEW_MODEL_TEST_INITIALIZE
#define VCZH_PRESENTATION_REMOTE_VIEW_MODEL_TEST_INITIALIZE

#include <VlppWorkflowLibrary.h>

namespace vl::presentation::remote_view_model_test
{
	class RemoteViewModelTestInitialize
	{
	public:
		static void InitializeRpc(
			rpc_controller::channeling::RpcJsonDispatcherClient* dispatcher,
			vint clientId
			);
	};
}

#endif
