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
				L"Render(1, {10,10:1,1}, {0,0:640,480})",
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
			// Size of the composition becomes (60,12)
			L"Render(1, {10,10:60,12}, {0,0:640,480})"
			);

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Update SolidLabel and cache font heights")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;
		Ptr<GuiSolidLabelElement> e1, e2;

		protocol.OnNextFrame([&]()
		{
			auto table = new GuiTableComposition;
			table->SetAlignmentToParent(Margin(0, 0, 0, 0));
			table->SetRowsAndColumns(3, 2);
			table->SetRowOption(0, GuiCellOption::MinSizeOption());
			table->SetRowOption(1, GuiCellOption::MinSizeOption());
			table->SetRowOption(2, GuiCellOption::PercentageOption(1.0));
			table->SetColumnOption(0, GuiCellOption::AbsoluteOption(100));
			table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(0, 0, 1, 1);
				auto element = Ptr(GuiSolidLabelElement::Create());
				cell->SetOwnedElement(element);
				table->AddChild(cell);

				element->SetText(L"Hello");
				element->SetEllipse(true);
				element->SetFont(GetCurrentController()->ResourceService()->GetDefaultFont());
				e1 = element;
			}
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(1, 0, 1, 1);
				auto element = Ptr(GuiSolidLabelElement::Create());
				cell->SetOwnedElement(element);
				table->AddChild(cell);

				element->SetText(L"World");
				element->SetEllipse(true);
				element->SetFont(GetCurrentController()->ResourceService()->GetDefaultFont());
				e2 = element;
			}

			controlHost->GetContainerComposition()->AddChild(table);

			protocol.measuringForNextRendering.fontHeights = Ptr(new List<ElementMeasuring_FontHeight>);
			protocol.measuringForNextRendering.fontHeights->Add({ L"One",12,12 });
		});

		protocol.OnNextFrame([&]()
		{
			// Render for the first time and send back height of texts
			// No rendered element because the table is still 0x0
			AssertEventLogs(
				eventLogs,
				L"Created(<1:SolidLabel>, <2:SolidLabel>)",
				L"Updated(1, #000000, Left, Top, <flags:[e]>, <font:One:12>, <text:Hello>, <request:FontHeight>)",
				L"Updated(2, #000000, Left, Top, <flags:[e]>, <font:One:12>, <text:World>, <request:FontHeight>)",
				L"Begin()",
				L"End()"
				);
			TEST_ASSERT(!protocol.measuringForNextRendering.fontHeights);
			TEST_ASSERT(!protocol.measuringForNextRendering.minSizes);
		});

		protocol.OnNextFrame([&]()
		{
			// Render for the second time and the height of texts is updated to the cells
			AssertEventLogs(
				eventLogs,
				L"Begin()",
				L"Render(1, {0,0:100,1}, {0,0:640,480})",
				L"Render(2, {0,1:100,1}, {0,0:640,480})",
				L"End()"
				);
		});

		AssertRenderingEventLogs(
			protocol,
			eventLogs,
			[&]()
			{
				FontProperties f1;
				f1.fontFamily = L"Two";
				f1.size = 14;
				e1->SetFont(f1);

				FontProperties f2;
				f2.fontFamily = L"Three";
				f2.size = 16;
				e2->SetFont(f2);

				protocol.measuringForNextRendering.fontHeights = Ptr(new List<ElementMeasuring_FontHeight>);
				protocol.measuringForNextRendering.fontHeights->Add({ L"Two",14,14 });
				protocol.measuringForNextRendering.fontHeights->Add({ L"Three",16,16 });
			},
			// Layout stablized
			L"Render(1, {0,0:100,12}, {0,0:640,480})",
			L"Render(2, {0,12:100,12}, {0,0:640,480})"
			);

		protocol.OnNextFrame([&]()
		{
			// Heights updated to texts
			AssertEventLogs(
				eventLogs,
				L"Updated(1, #000000, Left, Top, <flags:[e]>, <font:Two:14>, <notext>, <request:FontHeight>)",
				L"Updated(2, #000000, Left, Top, <flags:[e]>, <font:Three:16>, <notext>, <request:FontHeight>)",
				L"Begin()",
				L"Render(1, {0,0:100,12}, {0,0:640,480})",
				L"Render(2, {0,12:100,12}, {0,0:640,480})",
				L"End()"
				);
			TEST_ASSERT(!protocol.measuringForNextRendering.fontHeights);
			TEST_ASSERT(!protocol.measuringForNextRendering.minSizes);
		});

		protocol.OnNextFrame([&]()
		{
			// Height updated to cells
			AssertEventLogs(
				eventLogs,
				L"Begin()",
				L"Render(1, {0,0:100,12}, {0,0:640,480})",
				L"Render(2, {0,12:100,12}, {0,0:640,480})",
				L"End()"
				);
		});

		AssertRenderingEventLogs(
			protocol,
			eventLogs,
			[&]()
			{
				auto f1 = e1->GetFont();
				auto f2 = e2->GetFont();
				e1->SetFont(f2);
				e2->SetFont(f1);
			},
			// Layout stablized
			L"Render(1, {0,0:100,14}, {0,0:640,480})",
			L"Render(2, {0,14:100,16}, {0,0:640,480})"
			);

		protocol.OnNextFrame([&]()
		{
			// Heights updated to texts
			// No need to send new font heights since they are already cached
			// So heights already updated to cells
			// It skips one frame comparing to before
			AssertEventLogs(
				eventLogs,
				L"Updated(1, #000000, Left, Top, <flags:[e]>, <font:Three:16>, <notext>, <request:FontHeight>)",
				L"Updated(2, #000000, Left, Top, <flags:[e]>, <font:Two:14>, <notext>, <request:FontHeight>)",
				L"Begin()",
				L"Render(1, {0,0:100,14}, {0,0:640,480})",
				L"Render(2, {0,14:100,16}, {0,0:640,480})",
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
			// Layout stablized
			L"Render(1, {0,0:100,16}, {0,0:640,480})",
			L"Render(2, {0,16:100,14}, {0,0:640,480})"
			);

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Disconnect after caching font heights to trigger recache")
	{
		List<WString> eventLogs;
		GraphicsHostRenderingProtocol protocol(eventLogs);
		GuiWindow* controlHost = nullptr;
		Ptr<GuiSolidLabelElement> e1, e2;

		protocol.OnNextFrame([&]()
		{
			auto table = new GuiTableComposition;
			table->SetAlignmentToParent(Margin(0, 0, 0, 0));
			table->SetRowsAndColumns(3, 2);
			table->SetRowOption(0, GuiCellOption::MinSizeOption());
			table->SetRowOption(1, GuiCellOption::MinSizeOption());
			table->SetRowOption(2, GuiCellOption::PercentageOption(1.0));
			table->SetColumnOption(0, GuiCellOption::AbsoluteOption(100));
			table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(0, 0, 1, 1);
				auto element = Ptr(GuiSolidLabelElement::Create());
				cell->SetOwnedElement(element);
				table->AddChild(cell);

				element->SetText(L"Hello");
				element->SetFont(GetCurrentController()->ResourceService()->GetDefaultFont());
				e1 = element;
			}
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(1, 0, 1, 1);
				auto element = Ptr(GuiSolidLabelElement::Create());
				cell->SetOwnedElement(element);
				table->AddChild(cell);

				element->SetText(L"World");
				element->SetEllipse(true);
				element->SetFont(GetCurrentController()->ResourceService()->GetDefaultFont());
				e2 = element;
			}

			controlHost->GetContainerComposition()->AddChild(table);

			protocol.measuringForNextRendering.minSizes = Ptr(new List<ElementMeasuring_ElementMinSize>);
			protocol.measuringForNextRendering.minSizes->Add({ 1,{60,12} });

			protocol.measuringForNextRendering.fontHeights = Ptr(new List<ElementMeasuring_FontHeight>);
			protocol.measuringForNextRendering.fontHeights->Add({ L"One",12,12 });
		});

		protocol.OnNextFrame([&]()
		{
			// Render for the first time and send back size/height of texts
			// No rendered element because the table is still 0x0
			AssertEventLogs(
				eventLogs,
				L"Created(<1:SolidLabel>, <2:SolidLabel>)",
				L"Updated(1, #000000, Left, Top, <flags:>, <font:One:12>, <text:Hello>, <request:TotalSize>)",
				L"Updated(2, #000000, Left, Top, <flags:[e]>, <font:One:12>, <text:World>, <request:FontHeight>)",
				L"Begin()",
				L"End()"
				);
			TEST_ASSERT(!protocol.measuringForNextRendering.fontHeights);
			TEST_ASSERT(!protocol.measuringForNextRendering.minSizes);
		});

		protocol.OnNextFrame([&]()
		{
			// Render for the second time and the size/height of texts is updated to the cells
			AssertEventLogs(
				eventLogs,
				L"Begin()",
				L"Render(1, {0,0:100,1}, {0,0:640,480})",
				L"Render(2, {0,1:100,1}, {0,0:640,480})",
				L"End()"
				);
		});

		AssertRenderingEventLogs(
			protocol,
			eventLogs,
			[&]()
			{
				protocol.GetEvents()->OnControllerDisconnect();
				protocol.GetEvents()->OnControllerConnect();

				protocol.measuringForNextRendering.minSizes = Ptr(new List<ElementMeasuring_ElementMinSize>);
				protocol.measuringForNextRendering.minSizes->Add({ 1,{60,12} });

				protocol.measuringForNextRendering.fontHeights = Ptr(new List<ElementMeasuring_FontHeight>);
				protocol.measuringForNextRendering.fontHeights->Add({ L"One",12,12 });
			},
			// Layout stablized
			L"Render(1, {0,0:100,12}, {0,0:640,480})",
			L"Render(2, {0,12:100,12}, {0,0:640,480})"
			);

		protocol.OnNextFrame([&]()
		{
			// Reconnect and send back size/height of texts
			// Force rendering
			AssertEventLogs(
				eventLogs,
				L"Created(<1:SolidLabel>, <2:SolidLabel>)",
				L"Updated(1, #000000, Left, Top, <flags:>, <font:One:12>, <text:Hello>, <request:TotalSize>)",
				L"Updated(2, #000000, Left, Top, <flags:[e]>, <font:One:12>, <text:World>, <request:FontHeight>)",
				L"Begin()",
				L"Render(1, {0,0:100,12}, {0,0:640,480})",
				L"Render(2, {0,12:100,12}, {0,0:640,480})",
				L"End()"
				);
			TEST_ASSERT(!protocol.measuringForNextRendering.fontHeights);
			TEST_ASSERT(!protocol.measuringForNextRendering.minSizes);
		});

		protocol.OnNextFrame([&]()
		{
			// No rendered element because sizes are not changed
			AssertEventLogs(eventLogs);

			// Reconnect again but text heights will be doubled
			protocol.GetEvents()->OnControllerDisconnect();
			protocol.GetEvents()->OnControllerConnect();

			protocol.measuringForNextRendering.minSizes = Ptr(new List<ElementMeasuring_ElementMinSize>);
			protocol.measuringForNextRendering.minSizes->Add({ 1,{60,24} });

			protocol.measuringForNextRendering.fontHeights = Ptr(new List<ElementMeasuring_FontHeight>);
			protocol.measuringForNextRendering.fontHeights->Add({ L"One",12,24 });
		});

		protocol.OnNextFrame([&]()
		{
			// Reconnect and send back size/height of texts
			// Force rendering
			AssertEventLogs(
				eventLogs,
				L"Created(<1:SolidLabel>, <2:SolidLabel>)",
				L"Updated(1, #000000, Left, Top, <flags:>, <font:One:12>, <text:Hello>, <request:TotalSize>)",
				L"Updated(2, #000000, Left, Top, <flags:[e]>, <font:One:12>, <text:World>, <request:FontHeight>)",
				L"Begin()",
				L"Render(1, {0,0:100,12}, {0,0:640,480})",
				L"Render(2, {0,12:100,12}, {0,0:640,480})",
				L"End()"
				);
			TEST_ASSERT(!protocol.measuringForNextRendering.fontHeights);
			TEST_ASSERT(!protocol.measuringForNextRendering.minSizes);
		});

		protocol.OnNextFrame([&]()
		{
			// Render again since sizes were updated in the last frame
			// Now size/height of texts is updated to the cells
			AssertEventLogs(
				eventLogs,
				L"Begin()",
				L"Render(1, {0,0:100,12}, {0,0:640,480})",
				L"Render(2, {0,12:100,12}, {0,0:640,480})",
				L"End()"
				);
			TEST_ASSERT(!protocol.measuringForNextRendering.fontHeights);
			TEST_ASSERT(!protocol.measuringForNextRendering.minSizes);
		});

		AssertRenderingEventLogs(
			protocol,
			eventLogs,
			[&]()
			{
				controlHost->Hide();
			},
			// Layout stablized
			L"Render(1, {0,0:100,24}, {0,0:640,480})",
			L"Render(2, {0,24:100,24}, {0,0:640,480})"
			);

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});
}