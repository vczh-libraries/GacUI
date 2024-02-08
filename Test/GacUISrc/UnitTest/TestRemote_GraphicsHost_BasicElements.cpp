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
			AssertEventLogs(
				eventLogs,
				L"Created(<1:FocusRectangle>)",
				L"Begin()",
				L"Render(1, {10,10:620,460}, {0,0:640,480})",
				L"End()"
				);
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"GuiSolidBorderElement")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;
		GuiBoundsComposition* bounds = nullptr;
		Ptr<GuiSolidBorderElement> element = nullptr;

		protocol.OnNextFrame([&]()
		{
			bounds = new GuiBoundsComposition;
			bounds->SetAlignmentToParent(Margin(10, 10, 10, 10));

			element = Ptr(GuiSolidBorderElement::Create());
			element->SetColor(Color(255, 0, 0));
			element->SetShape({ ElementShapeType::RoundRect,2,4 });
			bounds->SetOwnedElement(element);

			controlHost->GetContainerComposition()->AddChild(bounds);
		});

		protocol.OnNextFrame([&]()
		{
			AssertEventLogs(
				eventLogs,
				L"Created(<1:SolidBorder>)",
				L"Updated(1, #FF0000, {RoundRect,2,4})",
				L"Begin()",
				L"Render(1, {0,0:0,0}, {0,0:640,480})",
				L"End()"
				);
		});

		protocol.OnNextFrame([&]()
		{
			AssertEventLogs(
				eventLogs,
				L"Begin()",
				L"Render(1, {10,10:620,460}, {0,0:640,480})",
				L"End()"
				);

			element->SetColor(Color(0, 0, 255));
			element->SetShape({ ElementShapeType::Ellipse });
			bounds->SetAlignmentToParent(Margin(20, 20, 20, 20));
		});

		protocol.OnNextFrame([&]()
		{
			AssertEventLogs(
				eventLogs,
				L"Updated(1, #0000FF, Ellipse)",
				L"Begin()",
				L"Render(1, {10,10:620,460}, {0,0:640,480})",
				L"End()"
				);
		});

		protocol.OnNextFrame([&]()
		{
			AssertEventLogs(
				eventLogs,
				L"Begin()",
				L"Render(1, {20,20:600,440}, {0,0:640,480})",
				L"End()"
				);
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});
}