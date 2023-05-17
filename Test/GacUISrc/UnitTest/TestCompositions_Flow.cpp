#include "TestCompositions.h"
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test <Flow> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiFlowComposition>();
	});

	TEST_CASE(L"Test <Flow> with <FlowItem> in different direction")
	{
		auto flow = new GuiFlowComposition;

		flow->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		flow->SetPreferredMinSize(Size(50, 50));

		TEST_ASSERT(flow->GetExtraMargin() == Margin(0, 0, 0, 0));
		TEST_ASSERT(flow->GetRowPadding() == 0);
		TEST_ASSERT(flow->GetColumnPadding() == 0);
		TEST_ASSERT(flow->GetAlignment() == FlowAlignment::Left);
		TEST_ASSERT(flow->GetAxis().Cast<GuiDefaultAxis>());
		flow->SetPreferredMinSize(Size(200, 200));
		flow->SetExtraMargin(Margin(11, 22, 33, 44));
		flow->SetRowPadding(20);
		flow->SetColumnPadding(10);
		TEST_ASSERT(flow->GetExtraMargin() == Margin(11, 22, 33, 44));
		TEST_ASSERT(flow->GetRowPadding() == 20);
		TEST_ASSERT(flow->GetColumnPadding() == 10);

		GuiFlowItemComposition* flowItems[6];
		const vint ITEM_COUNT = sizeof(flowItems) / sizeof(*flowItems);
		for (vint i = 0; i < ITEM_COUNT; i++)
		{
			flowItems[i] = new GuiFlowItemComposition;
			flowItems[i]->SetPreferredMinSize(Size(30 + i * 10, 20 + i * 20));
			flow->InsertFlowItem(i, flowItems[i]);
		}

		auto testLeftDown = [&](bool expand)
		{
			TEST_ASSERT(flow->GetClientArea() == Rect({ 0,0 }, { 200,386 }));
			TEST_ASSERT(flow->GetMinPreferredClientSize() == flow->GetClientArea().GetSize());
			TEST_ASSERT(flow->GetPreferredBounds() == flow->GetClientArea());
			TEST_ASSERT(flow->GetBounds() == flow->GetClientArea());
		};

		auto testRightDown = [&](bool expand)
		{
		};

		auto testLeftUp = [&](bool expand)
		{
		};

		auto testRightUp = [&](bool expand)
		{
		};

		auto testDownLeft = [&](bool expand)
		{
		};

		auto testDownRight = [&](bool expand)
		{
		};

		auto testUpLeft = [&](bool expand)
		{
		};

		auto testUpRight = [&](bool expand)
		{
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

		SafeDeleteComposition(flow);
	});

	TEST_CASE(L"Test <Flow> with different <Flow> alignment")
	{
	});

	TEST_CASE(L"Test <Flow> with different <FlowItem> option")
	{
	});

	TEST_CASE(L"Test <Stack> with clipped <StackItem>")
	{
	});
}