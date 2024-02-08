#include "TestRemote_GraphicsHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"Simple SolidLabel")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			auto bounds = new GuiBoundsComposition;
			bounds->SetAlignmentToParent(Margin(10, 10, -1, -1));
			bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);

			auto element = Ptr(GuiSolidLabelElement::Create());
			element->SetText(L"Hello");
			element->SetFont(GetCurrentController()->ResourceService()->GetDefaultFont());
			bounds->SetOwnedElement(element);

			controlHost->GetContainerComposition()->AddChild(bounds);

			protocol.measuringForNextRendering.minSizes = Ptr(new List<ElementMeasuring_ElementMinSize>);
			protocol.measuringForNextRendering.minSizes->Add({ 1,{60,12} });
		});

		protocol.OnNextFrame([&]()
		{
			// Render for the first time and send back size of Hello
			AssertEventLogs(
				eventLogs,
				L"Created(<1:SolidLabel>)",
				L"Updated(1, #000000, Left, Top, <flags:>, <font:One:12>, <text:Hello>, <request:TotalSize>)",
				L"Begin()",
				L"Render(1, {0,0:0,0}, {0,0:640,480})",
				L"End()"
				);
			TEST_ASSERT(!protocol.measuringForNextRendering.fontHeights);
			TEST_ASSERT(!protocol.measuringForNextRendering.minSizes);
		});

		protocol.OnNextFrame([&]()
		{
			// Render for the second time and the size of Hello is updated to the composition
			AssertEventLogs(
				eventLogs,
				L"Begin()",
				L"Render(1, {10,10:0,0}, {0,0:640,480})",
				L"End()"
				);
		});

		AssertRenderingEventLogs(
			protocol,
			eventLogs,
			[&]()
			{
				controlHost->Hide();
			},
			// Size of Hello becomes (60,12)
			L"Render(1, {10,10:60,12}, {0,0:640,480})"
			);

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Update SolidLabel and get measuring requests")
	{
	});

	TEST_CATEGORY(L"Multiple SolidLabel sharing different font heights")
	{
	});
}