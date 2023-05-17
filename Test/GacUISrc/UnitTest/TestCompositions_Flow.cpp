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
		flow->SetRowPadding(10);
		flow->SetColumnPadding(20);
		TEST_ASSERT(flow->GetExtraMargin() == Margin(11, 22, 33, 44));
		TEST_ASSERT(flow->GetRowPadding() == 10);
		TEST_ASSERT(flow->GetColumnPadding() == 20);

		GuiFlowItemComposition* flowItems[6];
		const vint ITEM_COUNT = sizeof(flowItems) / sizeof(*flowItems);
		for (vint i = 0; i < ITEM_COUNT; i++)
		{
			flowItems[i] = new GuiFlowItemComposition;
			flowItems[i]->SetPreferredMinSize(Size(30 + i * 10, 20 + i * 20));
			flow->InsertFlowItem(i, flowItems[i]);
		}

		auto testAllDirections = [&](bool expand)
		{
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