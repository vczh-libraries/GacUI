#include "TestRemote_GraphicsHost_Shared.h"

TEST_FILE
{
	TEST_CATEGORY(L"HitTest in empty composition")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			auto bounds = new GuiBoundsComposition;
			bounds->SetAlignmentToParent(Margin(10, 10, 10, 10));
			bounds->SetAssociatedHitTestResult(INativeWindowListener::Title);

			controlHost->GetContainerComposition()->AddChild(bounds);
			controlHost->ForceCalculateSizeImmediately();
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 10,10 }, { 620,460 }));
		});

		protocol.OnNextFrame([&]()
		{
			AssertEventLogs(
				eventLogs,
				L"Begin()",
				L"BeginBoundary(Title, {10,10:620,460}, {10,10:620,460})",
				L"EndBoundary()",
				L"End()"
				);
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"HitTest in composition with element")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			auto bounds = new GuiBoundsComposition;
			bounds->SetAlignmentToParent(Margin(10, 10, 10, 10));
			bounds->SetAssociatedHitTestResult(INativeWindowListener::Title);

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
				L"BeginBoundary(Title, {10,10:620,460}, {10,10:620,460})",
				L"EndBoundary()",
				L"End()"
				);
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"HitTest in composition with children")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			auto bounds = new GuiBoundsComposition;
			bounds->SetAlignmentToParent(Margin(10, 10, 10, 10));
			bounds->SetAssociatedHitTestResult(INativeWindowListener::Title);

			auto childBounds = new GuiBoundsComposition;
			childBounds->SetAlignmentToParent(Margin(10, 10, 10, 10));

			auto element = Ptr(GuiFocusRectangleElement::Create());
			childBounds->SetOwnedElement(element);
			bounds->AddChild(childBounds);

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
				L"BeginBoundary(Title, {10,10:620,460}, {10,10:620,460})",
				L"Render(1, {20,20:600,440}, {10,10:620,460})",
				L"EndBoundary()",
				L"End()"
				);
			controlHost->Hide();
		});

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Nested HitTest of the same value")
	{
	});

	TEST_CATEGORY(L"HitTest in non-main window")
	{
	});
}