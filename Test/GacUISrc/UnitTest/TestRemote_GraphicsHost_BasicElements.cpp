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
			// ForceCalculateSizeImmediately causes the layout to ready
			AssertEventLogs(
				eventLogs,
				L"Created(<1:FocusRectangle>)",
				L"Begin()",
				L"Render(1, {10,10:620,460}, {0,0:640,480})",
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
			L"Render(1, {10,10:620,460}, {0,0:640,480})"
			);

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
			// Layout is ready after rendering
			AssertEventLogs(
				eventLogs,
				L"Created(<1:SolidBorder>)",
				L"Updated(1, #FF0000, {RoundRect,2,4})",
				L"Begin()",
				L"Render(1, {0,0:0,0}, {0,0:640,480})",
				L"End()"
				);
		});

		AssertRenderingEventLogs(
			protocol,
			eventLogs,
			[&]()
			{
				element->SetColor(Color(0, 0, 255));
				element->SetShape({ ElementShapeType::Ellipse });
				bounds->SetAlignmentToParent(Margin(20, 20, 20, 20));
			},
			L"Render(1, {10,10:620,460}, {0,0:640,480})"
			);

		protocol.OnNextFrame([&]()
		{
			// Element updated, layout is ready after rendering
			AssertEventLogs(
				eventLogs,
				L"Updated(1, #0000FF, Ellipse)",
				L"Begin()",
				L"Render(1, {10,10:620,460}, {0,0:640,480})",
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
			L"Render(1, {20,20:600,440}, {0,0:640,480})"
			);

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Other simple elements")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;

		protocol.OnNextFrame([&]()
		{
			auto table = new GuiTableComposition;
			table->SetAlignmentToParent(Margin(10, 10, 10, 10));
			table->SetRowsAndColumns(3, 2);
			table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
			table->SetRowOption(1, GuiCellOption::MinSizeOption());
			table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
			table->SetColumnOption(0, GuiCellOption::MinSizeOption());
			table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(0, 0, 1, 1);
				auto element = Ptr(Gui3DBorderElement::Create());
				cell->SetOwnedElement(element);
				table->AddChild(cell);

				element->SetColor1(Color(255, 0, 0));
				element->SetColor2(Color(0, 255, 0));
			}
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(1, 0, 1, 1);
				auto element = Ptr(GuiPolygonElement::Create());
				cell->SetOwnedElement(element);
				table->AddChild(cell);

				Point points[] = {
					{0,1},
					{2,3},
					{4,5},
					{6,7}
				};
				element->SetSize({ 200,100 });
				element->SetPoints(&*points, sizeof(points) / sizeof(*points));
				element->SetBorderColor(Color(255, 0, 0));
				element->SetBackgroundColor(Color(0, 255, 0));
			}
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(2, 0, 1, 1);
				auto element = Ptr(Gui3DSplitterElement::Create());
				cell->SetOwnedElement(element);
				table->AddChild(cell);

				element->SetColor1(Color(255, 0, 0));
				element->SetColor2(Color(0, 255, 0));
				element->SetDirection(Gui3DSplitterElement::Horizontal);
			}
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(0, 1, 1, 1);
				auto element = Ptr(GuiSolidBackgroundElement::Create());
				cell->SetOwnedElement(element);
				table->AddChild(cell);

				element->SetColor(Color(255, 0, 0));
				element->SetShape({ ElementShapeType::Rectangle });
			}
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(1, 1, 1, 1);
				auto element = Ptr(GuiGradientBackgroundElement::Create());
				cell->SetOwnedElement(element);
				table->AddChild(cell);

				element->SetColor1(Color(255, 0, 0));
				element->SetColor2(Color(0, 255, 0));
				element->SetDirection(GuiGradientBackgroundElement::Vertical);
				element->SetShape({ ElementShapeType::Ellipse });
			}
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(2, 1, 1, 1);
				auto element = Ptr(GuiInnerShadowElement::Create());
				cell->SetOwnedElement(element);
				table->AddChild(cell);

				element->SetColor(Color(255, 0, 0));
				element->SetThickness(5);
			}

			controlHost->GetContainerComposition()->AddChild(table);
			controlHost->ForceCalculateSizeImmediately();
		});

		protocol.OnNextFrame([&]()
		{
			// Layout is ready after rendering, the size of the polygon is (200,100), but the cell is still (0,0)
			AssertEventLogs(
				eventLogs,

				L"Created(<1:SinkBorder>, <2:Polygon>, <3:SinkSplitter>, <4:SolidBackground>, <5:GradientBackground>, <6:InnerShadow>)",
				L"Updated(1, #FF0000, #00FF00)",
				L"Updated(2, {200,100}, {0,1:2,3:4,5:6,7}, #FF0000, #00FF00)",
				L"Updated(3, #FF0000, #00FF00, Horizontal)",
				L"Updated(4, #FF0000, Rectangle)",
				L"Updated(5, #FF0000, #00FF00, Vertical, Ellipse)",
				L"Updated(6, #FF0000, 5)",

				L"Begin()",
				L"Render(1, {10,10:0,230}, {10,10:620,460})",
				L"Render(2, {10,240:0,0}, {10,10:620,460})",
				L"Render(3, {10,240:0,230}, {10,10:620,460})",
				L"Render(4, {10,10:620,230}, {10,10:620,460})",
				L"Render(5, {10,240:620,0}, {10,10:620,460})",
				L"Render(6, {10,240:620,230}, {10,10:620,460})",
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
			// Layout is ready, the size of the polygon cell is updated to (200,100)
			L"Render(1, {10,10:200,180}, {10,10:620,460})",
			L"Render(2, {10,190:200,100}, {10,10:620,460})",
			L"Render(3, {10,290:200,180}, {10,10:620,460})",
			L"Render(4, {210,10:420,180}, {10,10:620,460})",
			L"Render(5, {210,190:420,100}, {10,10:620,460})",
			L"Render(6, {210,290:420,180}, {10,10:620,460})"
			);

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});
}