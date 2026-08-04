#ifndef VCZH_PRESENTATION_REMOTEVIEWMODELTEST_SHARED
#define VCZH_PRESENTATION_REMOTEVIEWMODELTEST_SHARED

#include "../../../Source/GacUI.h"
#include "../../RemotingHelpers/ViewModelHostClient/ViewModelHostClient.h"
#include "RemoteViewModelTestRpc.h"

namespace vl::presentation::remote_view_model_test
{
	extern remoting::RemotingDispatcherFactory CreateDispatcherFactory();
}

#endif
