#ifndef VCZH_DEMO_FULLCONTROLTESTRAWELEMENTREMOTE
#define VCZH_DEMO_FULLCONTROLTESTRAWELEMENTREMOTE

#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"

namespace demo
{
	// Keep the attachment alive for the renderer lifetime. Only FullControlTest.Clock is attached.
	extern vl::Ptr<vl::Object> AttachFullControlTestRawElements(vl::presentation::remote_renderer::GuiRemoteRendererSingle& renderer);
}

#endif
