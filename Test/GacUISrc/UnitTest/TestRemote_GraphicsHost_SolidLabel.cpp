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
			// Size of the composition becomes (60,12)
			L"Render(1, {10,10:60,12}, {0,0:640,480})"
			);

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Update SolidLabel and get measuring requests")
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
				L"Render(1, {0,0:100,0}, {0,0:640,480})",
				L"Render(2, {0,0:100,0}, {0,0:640,480})",
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
			L"Render(1, {0,0:100,12}, {0,0:640,480})",
			L"Render(2, {0,12:100,12}, {0,0:640,480})"
			);

		SetGuiMainProxy(MakeGuiMain(protocol, eventLogs, controlHost));
		StartRemoteControllerTest(protocol);
	});

	TEST_CATEGORY(L"Multiple SolidLabel sharing different font heights")
	{
	});
}