#include "FullControlTestRawElementRemote.h"
#include "../Generated_FullControlTest/FullControlTestRawElement.h"

namespace demo
{
	using namespace vl;
	using namespace vl::collections;
	using namespace vl::presentation::elements;
	using namespace vl::presentation::remote_renderer;

	class RawCollectionAttachment : public Object
	{
	protected:
		GuiRemoteRendererSingle& renderer;
		Dictionary<GuiRawElementBase*, Pair<WString, Ptr<Object>>> attachments;
		Ptr<EventHandler> installed, changed, uninstalled;

		void Update(const GuiRemoteRawElementEventArgs& arguments)
		{
			auto element = arguments.element;
			auto id = element->GetId();
			if (attachments.Keys().Contains(element.Obj()))
			{
				if (attachments[element.Obj()].key == id) return;
				attachments.Remove(element.Obj());
			}
			if (id == L"FullControlTest.Clock" || id == L"FullControlTest.Touching")
			{
				attachments.Add(element.Obj(), { id, id == L"FullControlTest.Clock" ? AttachFullControlTestRawElement(element) : AttachFullControlTestTouchingElement(element) });
			}
		}
	public:
		RawCollectionAttachment(GuiRemoteRendererSingle& _renderer) : renderer(_renderer)
		{
			installed = renderer.RawElementInstalled.Add(this, &RawCollectionAttachment::Update);
			changed = renderer.RawElementChanged.Add(this, &RawCollectionAttachment::Update);
			uninstalled = renderer.RawElementUninstalled.Add([this](const GuiRemoteRawElementEventArgs& arguments)
			{
				attachments.Remove(arguments.element.Obj());
			});
			for (auto element : renderer.GetRawElements()) Update({ element });
		}

		~RawCollectionAttachment()
		{
			renderer.RawElementInstalled.Remove(installed);
			renderer.RawElementChanged.Remove(changed);
			renderer.RawElementUninstalled.Remove(uninstalled);
		}
	};

	Ptr<Object> AttachFullControlTestRawElements(GuiRemoteRendererSingle& renderer)
	{
		return Ptr(new RawCollectionAttachment(renderer));
	}
}
