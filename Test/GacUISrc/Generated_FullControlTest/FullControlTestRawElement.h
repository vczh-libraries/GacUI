#ifndef VCZH_DEMO_FULLCONTROLTESTRAWELEMENT
#define VCZH_DEMO_FULLCONTROLTESTRAWELEMENT

#include "../../../Source/GacUI.h"
#include "../../../Source/PlatformProviders/RemoteRenderer/GuiRemoteRendererSingle.h"

namespace demo
{
	// Keep the returned attachment alive while drawing is needed. Destruction detaches all handlers.
	// Data is HH:mm:ss (24-hour time); painting uses only this value, never the local clock.
	extern vl::Ptr<vl::Object> AttachFullControlTestRawElement(vl::Ptr<vl::presentation::elements::GuiRawElementBase> element);
	extern vl::Ptr<vl::Object> AttachFullControlTestRawElements(vl::presentation::remote_renderer::GuiRemoteRendererSingle& renderer);
}

#endif
