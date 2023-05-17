#include "TestCompositions.h"
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test <Flow> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiFlowComposition>();
	});

	TEST_CATEGORY(L"Test <Flow> with <FlowItem> in different directions")
	{
		auto testHorizontalDirections = []<vint ITEM_COUNT>(GuiFlowComposition * flow, GuiFlowItemComposition * (&flowItems)[ITEM_COUNT])
		{
			auto testHorizontal = [&]
			{
				TEST_ASSERT(flow->GetClientArea() == Rect({ 0,0 }, { 200,386 }));
				TEST_ASSERT(flow->GetMinPreferredClientSize() == flow->GetClientArea().GetSize());
				TEST_ASSERT(flow->GetPreferredBounds() == flow->GetClientArea());
				TEST_ASSERT(flow->GetBounds() == flow->GetClientArea());
			};

			auto testLeftDown = [&](bool expand)
			{
				testHorizontal();
			};

			auto testRightDown = [&](bool expand)
			{
				testHorizontal();
			};

			auto testLeftUp = [&](bool expand)
			{
				testHorizontal();
			};

			auto testRightUp = [&](bool expand)
			{
				testHorizontal();
			};

			auto testAllDirections = [&](bool expand)
			{
				flow->SetAxis(Ptr(new GuiDefaultAxis));
				TEST_ASSERT(flow->GetAxis().Cast<GuiDefaultAxis>());
				testLeftDown(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftDown)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::LeftDown);
				testLeftDown(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::RightDown);
				testRightDown(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::LeftUp);
				testLeftUp(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::RightUp)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::RightUp);
				testRightUp(expand);
			};

			testAllDirections(false);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				Margin extraMargin(1 * i, 2 * i, 3 * i, 4 * i);
				flowItems[i]->SetExtraMargin(extraMargin);
				TEST_ASSERT(flowItems[i]->GetExtraMargin() == extraMargin);
			}
			testAllDirections(true);
		};

		auto testVerticalDirections = []<vint ITEM_COUNT>(GuiFlowComposition * flow, GuiFlowItemComposition * (&flowItems)[ITEM_COUNT])
		{
			auto testVertical = [&]
			{
				auto fuck = flow->GetClientArea();
				TEST_ASSERT(flow->GetClientArea() == Rect({ 0,0 }, { 386,200 }));
				TEST_ASSERT(flow->GetMinPreferredClientSize() == flow->GetClientArea().GetSize());
				TEST_ASSERT(flow->GetPreferredBounds() == flow->GetClientArea());
				TEST_ASSERT(flow->GetBounds() == flow->GetClientArea());
			};

			auto testDownLeft = [&](bool expand)
			{
				testVertical();
			};

			auto testDownRight = [&](bool expand)
			{
				testVertical();
			};

			auto testUpLeft = [&](bool expand)
			{
				testVertical();
			};

			auto testUpRight = [&](bool expand)
			{
				testVertical();
			};

			auto testAllDirections = [&](bool expand)
			{
				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::DownLeft);
				testDownLeft(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::DownRight)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::DownRight);
				testDownRight(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::UpLeft)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::UpLeft);
				testUpLeft(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::UpRight);
				testUpRight(expand);
			};

			testAllDirections(false);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				Margin extraMargin(1 * i, 2 * i, 3 * i, 4 * i);
				flowItems[i]->SetExtraMargin(extraMargin);
				TEST_ASSERT(flowItems[i]->GetExtraMargin() == extraMargin);
			}
			testAllDirections(true);
		};

		auto testAllDirections = [&testHorizontalDirections, &testVerticalDirections](bool horizontal)
		{
			auto flow = new GuiFlowComposition;

			flow->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			flow->SetPreferredMinSize(Size(200, 200));
			flow->ForceCalculateSizeImmediately();

			TEST_ASSERT(flow->GetExtraMargin() == Margin(0, 0, 0, 0));
			TEST_ASSERT(flow->GetRowPadding() == 0);
			TEST_ASSERT(flow->GetColumnPadding() == 0);
			TEST_ASSERT(flow->GetAlignment() == FlowAlignment::Left);
			TEST_ASSERT(flow->GetAxis().Cast<GuiDefaultAxis>());
			if (horizontal)
			{
				flow->SetExtraMargin(Margin(11, 22, 33, 44));
				TEST_ASSERT(flow->GetExtraMargin() == Margin(11, 22, 33, 44));
			}
			else
			{
				flow->SetExtraMargin(Margin(22, 11, 44, 33));
				TEST_ASSERT(flow->GetExtraMargin() == Margin(22, 11, 44, 33));
			}
			flow->SetRowPadding(20);
			flow->SetColumnPadding(10);
			TEST_ASSERT(flow->GetRowPadding() == 20);
			TEST_ASSERT(flow->GetColumnPadding() == 10);

			GuiFlowItemComposition* flowItems[6];
			const vint ITEM_COUNT = sizeof(flowItems) / sizeof(*flowItems);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				flowItems[i] = new GuiFlowItemComposition;
				if (horizontal)
				{
					flowItems[i]->SetPreferredMinSize(Size(30 + i * 10, 20 + i * 20));
				}
				else
				{
					flowItems[i]->SetPreferredMinSize(Size(20 + i * 20, 30 + i * 10));
				}
				flow->InsertFlowItem(i, flowItems[i]);
			}

			if (horizontal)
			{
				testHorizontalDirections(flow, flowItems);
			}
			else
			{
				testVerticalDirections(flow, flowItems);
			}

			SafeDeleteComposition(flow);
		};

		TEST_CASE(L"Horizontal directions")
		{
			testAllDirections(true);
		});

		TEST_CASE(L"Vertical directions")
		{
			testAllDirections(false);
		});
	});

	TEST_CASE(L"Test <Flow> with different <Flow> alignment")
	{
	});

	TEST_CASE(L"Test <Flow> with different <FlowItem> option")
	{
	});
}