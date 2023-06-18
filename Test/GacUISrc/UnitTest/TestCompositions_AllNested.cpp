#include "TestCompositions.h"
using namespace vl::collections;
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test all compositions nested in other compositions")
	{
		auto root = new GuiSharedSizeRootComposition;
		root->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		root->SetPreferredMinSize(Size(340, 340));

		// <Table> ======================================================

		auto table = new GuiTableComposition;
		root->AddChild(table);
		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetAlignmentToParent(Margin(10, 10, 10, 10));
		table->SetBorderVisible(false);
		table->SetCellPadding(10);
		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::MinSizeOption());
		table->SetRowOption(1, GuiCellOption::AbsoluteOption(100));
		table->SetRowOption(2, GuiCellOption::PercentageOption(0.1));
		table->SetColumnOption(0, GuiCellOption::MinSizeOption());
		table->SetColumnOption(1, GuiCellOption::AbsoluteOption(100));
		table->SetColumnOption(2, GuiCellOption::PercentageOption(0.1));

		auto cellStack = new GuiCellComposition;
		table->AddChild(cellStack);
		cellStack->SetSite(0, 0, 1, 1);

		auto cellFlow = new GuiCellComposition;
		table->AddChild(cellFlow);
		cellFlow->SetSite(1, 1, 1, 1);

		auto cellSpec = new GuiCellComposition;
		table->AddChild(cellSpec);
		cellSpec->SetSite(2, 2, 1, 1);

		// <Stack> ======================================================

		auto stack = new GuiStackComposition;
		cellStack->AddChild(stack);
		stack->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		stack->SetAlignmentToParent(Margin(10, 10, 10, 10));
		stack->SetDirection(GuiStackComposition::Vertical);
		stack->SetPadding(10);

		GuiStackItemComposition* stackItems[3];
		for (vint i = 0; i < sizeof(stackItems) / sizeof(*stackItems); i++)
		{
			auto& item = stackItems[i];
			item = new GuiStackItemComposition;
			stack->AddChild(item);

			auto shared = new GuiSharedSizeItemComposition;
			item->AddChild(shared);
			shared->SetAlignmentToParent(Margin(0, 0, 0, 0));
			shared->SetPreferredMinSize(Size(20 + i * 30, 20 - i * 5));
			shared->SetSharedWidth(true);
			shared->SetSharedHeight(true);
			shared->SetGroup(L"stack");
		}

		// <Flow> =======================================================

		auto flow = new GuiFlowComposition;
		cellFlow->AddChild(flow);
		flow->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		flow->SetAlignmentToParent(Margin(10, 10, 10, 10));
		flow->SetRowPadding(10);
		flow->SetColumnPadding(10);

		GuiFlowItemComposition* flowItems[3];
		for (vint i = 0; i < sizeof(flowItems) / sizeof(*flowItems); i++)
		{
			auto& item = flowItems[i];
			item = new GuiFlowItemComposition;
			flow->AddChild(item);

			auto shared = new GuiSharedSizeItemComposition;
			item->AddChild(shared);
			shared->SetAlignmentToParent(Margin(0, 0, 0, 0));
			shared->SetPreferredMinSize(Size(10 + i * 20, 15 - i * 10));
			shared->SetSharedWidth(true);
			shared->SetSharedHeight(true);
			shared->SetGroup(L"flow");
		}

		// <Specialized> ================================================

		// <SharedSize> =================================================

		// <Bounds> =====================================================

		root->ForceCalculateSizeImmediately();

		TEST_CASE(L"<Table>")
		{
			TEST_ASSERT(table->GetCachedMinSize() == Size(320, 320));
			TEST_ASSERT(table->GetCachedBounds() == Rect({ 10,10 }, { 320,320 }));
			TEST_ASSERT(cellStack->GetCachedMinSize() == Size(100, 100));
			TEST_ASSERT(cellStack->GetCachedBounds() == Rect({ 0,0 }, { 100,100 }));
			TEST_ASSERT(cellFlow->GetCachedMinSize() == Size(100, 100));
			TEST_ASSERT(cellFlow->GetCachedBounds() == Rect({ 110,110 }, { 100,100 }));
			TEST_ASSERT(cellSpec->GetCachedMinSize() == Size(100, 100));
			TEST_ASSERT(cellSpec->GetCachedBounds() == Rect({ 220,220 }, { 100,100 }));
		});

		TEST_CASE(L"<Stack>")
		{
			TEST_ASSERT(stack->GetCachedMinSize() == Size(80, 80));
			TEST_ASSERT(stack->GetCachedBounds() == Rect({ 10,10 }, { 80,80 }));
			for (auto [item, i] : indexed(From(stackItems)))
			{
				TEST_ASSERT(item->GetCachedMinSize() == Size(80, 20));
				TEST_ASSERT(item->GetCachedBounds() == Rect({ 0,i * 30 }, { 80,20 }));
			}
		});

		TEST_CASE(L"<Flow>")
		{
			TEST_ASSERT(flow->GetCachedMinSize() == Size(50, 65));
			TEST_ASSERT(flow->GetCachedBounds() == Rect({ 10,10 }, { 80,80 }));
			for (auto [item, i] : indexed(From(flowItems)))
			{
				TEST_ASSERT(item->GetCachedMinSize() == Size(50, 15));
				TEST_ASSERT(item->GetCachedBounds() == Rect({ 0,i * 30 }, { 50,15 }));
			}
		});

		TEST_CASE(L"<Specialized>")
		{
		});

		TEST_CASE(L"<SharedSize>")
		{
			TEST_ASSERT(root->GetCachedMinSize() == Size(340, 340));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 340,340 }));
			for (auto item : stackItems)
			{
				auto shared = dynamic_cast<GuiSharedSizeItemComposition*>(item->Children()[0]);
				TEST_ASSERT(shared->GetCachedMinSize() == Size(80, 20));
				TEST_ASSERT(shared->GetCachedBounds() == Rect({ 0,0 }, { 80,20 }));
			}
			for (auto item : flowItems)
			{
				auto shared = dynamic_cast<GuiSharedSizeItemComposition*>(item->Children()[0]);
				TEST_ASSERT(shared->GetCachedMinSize() == Size(50, 15));
				TEST_ASSERT(shared->GetCachedBounds() == Rect({ 0,0 }, { 50,15 }));
			}
		});

		TEST_CASE(L"<Bounds>")
		{
		});

		TEST_CASE(L"MinSizeLimitation <- NoLimite")
		{
		});

		SafeDeleteComposition(root);
	});
}