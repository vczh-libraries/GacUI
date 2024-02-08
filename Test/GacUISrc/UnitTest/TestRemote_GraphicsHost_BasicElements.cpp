#include "TestRemote_GraphicsHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"GuiFocusRectangleElement")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			auto bounds = new GuiBoundsComposition;
			bounds->SetAlignmentToParent(Margin(10, 10, 10, 10));

			auto element = Ptr(GuiFocusRectangleElement::Create());
			bounds->SetOwnedElement(element);

			controlHost->GetContainerComposition()->AddChild(bounds);
			controlHost->ForceCalculateSizeImmediately();
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 10,10 }, { 620,460 }));
		});

		protocol.OnNextFrame([&]()
		{
			AssertEventLogs(eventLogs);
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});
}