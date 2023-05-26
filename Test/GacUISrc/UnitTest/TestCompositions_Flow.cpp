#include "TestCompositions.h"

using namespace vl::collections;
using namespace vl::presentation::templates;
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test <Flow> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiFlowComposition>();
	});

	TEST_CATEGORY(L"Test <Flow> with <FlowItem> in different directions")
	{
		constexpr vint ITEM_COUNT = 6;

		auto testFlowItemBounds = [&](GuiFlowItemComposition*(&flowItems)[ITEM_COUNT], Rect(&flowItemBounds)[ITEM_COUNT], bool expand)
		{
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				auto expected = flowItemBounds[i];
				if (expand)
				{
					expected.x1 -= 1 * i;
					expected.y1 -= 2 * i;
					expected.x2 += 3 * i;
					expected.y2 += 4 * i;
				}
				TEST_ASSERT(flowItems[i]->GetBounds() == expected);
			}
		};

		auto testHorizontalDirections = [&](GuiFlowComposition* flow, GuiFlowItemComposition*(&flowItems)[ITEM_COUNT])
		{
			auto testHorizontal = [&]
			{
				TEST_ASSERT(flow->GetClientArea() == Rect({ 0,0 }, { 200,386 }));
				TEST_ASSERT(flow->GetMinPreferredClientSize() == flow->GetClientArea().GetSize());
				TEST_ASSERT(flow->GetPreferredBounds() == flow->GetClientArea());
				TEST_ASSERT(flow->GetBounds() == flow->GetClientArea());
			};

			auto testRightDown = [&](bool expand)
			{
				Rect flowItemBounds[ITEM_COUNT] = {
					{{ 11,62 }, { 30,20 }},
					{{ 51,42 }, { 40,40 }},
					{{ 101,22 }, { 50,60 }},
					{{ 11,122 }, { 60,80 }},
					{{ 81,102 }, { 70,100 }},
					{{ 11,222 }, { 80,120 }},
				};

				testHorizontal();
				testFlowItemBounds(flowItems, flowItemBounds, expand);
			};

			auto testLeftDown = [&](bool expand)
			{
				Rect flowItemBounds[ITEM_COUNT] = {
					{{ 137,62 }, { 30,20 }},
					{{ 87,42 }, { 40,40 }},
					{{ 27,22 }, { 50,60 }},
					{{ 107,122 }, { 60,80 }},
					{{ 27,102 }, { 70,100 }},
					{{ 87,222 }, { 80,120 }},
				};

				testHorizontal();
				testFlowItemBounds(flowItems, flowItemBounds, expand);
			};

			auto testRightUp = [&](bool expand)
			{
				Rect flowItemBounds[ITEM_COUNT] = {
					{{ 11,282 }, { 30,20 }},
					{{ 51,282 }, { 40,40 }},
					{{ 101,282 }, { 50,60 }},
					{{ 11,162 }, { 60,80 }},
					{{ 81,162 }, { 70,100 }},
					{{ 11,22 }, { 80,120 }},
				};

				testHorizontal();
				testFlowItemBounds(flowItems, flowItemBounds, expand);
			};

			auto testLeftUp = [&](bool expand)
			{
				Rect flowItemBounds[ITEM_COUNT] = {
					{{ 137,282 }, { 30,20 }},
					{{ 87,282 }, { 40,40 }},
					{{ 27,282 }, { 50,60 }},
					{{ 107,162 }, { 60,80 }},
					{{ 27,162 }, { 70,100 }},
					{{ 87,22 }, { 80,120 }},
				};

				testHorizontal();
				testFlowItemBounds(flowItems, flowItemBounds, expand);
			};

			auto testAllDirections = [&](bool expand)
			{
				flow->SetAxis(Ptr(new GuiDefaultAxis));
				TEST_ASSERT(flow->GetAxis().Cast<GuiDefaultAxis>());
				testRightDown(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::RightDown);
				testRightDown(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftDown)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::LeftDown);
				testLeftDown(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::RightUp)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::RightUp);
				testRightUp(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::LeftUp);
				testLeftUp(expand);
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

		auto testVerticalDirections = [&]<vint ITEM_COUNT>(GuiFlowComposition* flow, GuiFlowItemComposition*(&flowItems)[ITEM_COUNT])
		{
			auto testVertical = [&]
			{
				TEST_ASSERT(flow->GetClientArea() == Rect({ 0,0 }, { 386,200 }));
				TEST_ASSERT(flow->GetMinPreferredClientSize() == flow->GetClientArea().GetSize());
				TEST_ASSERT(flow->GetPreferredBounds() == flow->GetClientArea());
				TEST_ASSERT(flow->GetBounds() == flow->GetClientArea());
			};

			auto testDownRight = [&](bool expand)
			{
				Rect flowItemBounds[ITEM_COUNT] = {
					{{ 62,11 }, { 20,30 }},
					{{ 42,51 }, { 40,40 }},
					{{ 22,101 }, { 60,50 }},
					{{ 122,11 }, { 80,60 }},
					{{ 102,81 }, { 100,70 }},
					{{ 222,11 }, { 120,80 }},
				};

				testVertical();
				testFlowItemBounds(flowItems, flowItemBounds, expand);
			};

			auto testUpRight = [&](bool expand)
			{
				Rect flowItemBounds[ITEM_COUNT] = {
					{{ 62,137 }, { 20,30 }},
					{{ 42,87 }, { 40,40 }},
					{{ 22,27 }, { 60,50 }},
					{{ 122,107 }, { 80,60 }},
					{{ 102,27 }, { 100,70 }},
					{{ 222,87 }, { 120,80 }},
				};

				testVertical();
				testFlowItemBounds(flowItems, flowItemBounds, expand);
			};

			auto testDownLeft = [&](bool expand)
			{
				Rect flowItemBounds[ITEM_COUNT] = {
					{{ 282,11 }, { 20,30 }},
					{{ 282,51 }, { 40,40 }},
					{{ 282,101 }, { 60,50 }},
					{{ 162,11 }, { 80,60 }},
					{{ 162,81 }, { 100,70 }},
					{{ 22,11 }, { 120,80 }},
				};

				testVertical();
				testFlowItemBounds(flowItems, flowItemBounds, expand);
			};

			auto testUpLeft = [&](bool expand)
			{
				Rect flowItemBounds[ITEM_COUNT] = {
					{{ 282,137 }, { 20,30 }},
					{{ 282,87 }, { 40,40 }},
					{{ 282,27 }, { 60,50 }},
					{{ 162,107 }, { 80,60 }},
					{{ 162,27 }, { 100,70 }},
					{{ 22,87 }, { 120,80 }},
				};

				testVertical();
			};

			auto testAllDirections = [&](bool expand)
			{
				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::DownRight)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::DownRight);
				testDownRight(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::UpRight);
				testUpRight(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::DownLeft);
				testDownLeft(expand);

				flow->SetAxis(Ptr(new GuiAxis(AxisDirection::UpLeft)));
				TEST_ASSERT(flow->GetAxis().Cast<GuiAxis>()->GetDirection() == AxisDirection::UpLeft);
				testUpLeft(expand);
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

			GuiFlowItemComposition* flowItems[ITEM_COUNT];
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

	TEST_CASE(L"Test <RepeatFlow>")
	{
		ObservableList<WString> objects;

		auto flow = new GuiRepeatFlowComposition;
		flow->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		flow->SetRowPadding(20);
		flow->SetColumnPadding(10);
		flow->SetPreferredMinSize(Size(110, 0));
		flow->SetItemTemplate([](const Value& obj)
		{
			auto itemTemplate = new GuiTemplate;
			itemTemplate->SetText(UnboxValue<WString>(obj));
			itemTemplate->SetAlignmentToParent(Margin(0, 0, 0, 0));
			itemTemplate->SetPreferredMinSize(Size(50, 50));
			return itemTemplate;
		});
		flow->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(objects)));

		vint contextValue = -1;
		auto checkFlowItems = [&]()
		{
			vint rows = (objects.Count() + 1) / 2;
			TEST_ASSERT(flow->GetClientArea() == Rect({ 0,0 }, {
				110,
				(rows == 0 ? 0 : rows == 1 ? 50 : 70 * rows - 20)
				}));
			TEST_ASSERT(flow->GetMinPreferredClientSize() == flow->GetClientArea().GetSize());
			TEST_ASSERT(flow->GetPreferredBounds() == flow->GetClientArea());
			TEST_ASSERT(flow->GetBounds() == flow->GetClientArea());

			TEST_ASSERT(flow->GetFlowItems().Count() == objects.Count());
			for (auto [text, i] : indexed(objects))
			{
				vint row = i / 2;
				vint column = i % 2;

				auto flowItem = flow->GetFlowItems()[i];
				TEST_ASSERT(flowItem->GetBounds() == Rect({ 60 * column,70 * row }, { 50,50 }));
				TEST_ASSERT(flowItem->Children().Count() == 1);
				auto itemTemplate = dynamic_cast<GuiTemplate*>(flowItem->Children()[0]);
				TEST_ASSERT(itemTemplate->GetText() == text);
				TEST_ASSERT(itemTemplate->GetBounds() == Rect({ 0,0 }, { 50,50 }));

				if (contextValue == -1)
				{
					TEST_ASSERT(itemTemplate->GetContext().IsNull());
				}
				else
				{
					TEST_ASSERT(UnboxValue<vint>(itemTemplate->GetContext()) == contextValue);
				}
			}

			contextValue++;
			flow->SetContext(BoxValue(contextValue));
			TEST_ASSERT(UnboxValue<vint>(flow->GetContext()) == contextValue);
		};

		vint eventInsertCount = 0;
		vint eventRemoveCount = 0;
		vint eventContextCount = 0;
		vint eventIndex = -1;

		flow->ItemInserted.AttachLambda([&](auto self, GuiItemEventArgs& arguments) { eventInsertCount++; eventIndex = arguments.itemIndex; });
		flow->ItemRemoved.AttachLambda([&](auto self, GuiItemEventArgs& arguments) { eventRemoveCount++; eventIndex = arguments.itemIndex; });
		flow->ContextChanged.AttachLambda([&](auto self, auto& arguments) { eventContextCount++; eventIndex = -1; });
#define ASSERT_EVENT(IC, RC, CC, I) TEST_ASSERT(eventInsertCount == IC && eventRemoveCount == RC && eventContextCount == CC && eventIndex == I);

		checkFlowItems();				ASSERT_EVENT(0, 0, 1, -1);
		objects.Add(L"A");				ASSERT_EVENT(1, 0, 1, 0);
		checkFlowItems();				ASSERT_EVENT(1, 0, 2, -1);
		objects.Insert(0, L"B");		ASSERT_EVENT(2, 0, 2, 0);
		checkFlowItems();				ASSERT_EVENT(2, 0, 3, -1);
		objects.Insert(1, L"C");		ASSERT_EVENT(3, 0, 3, 1);
		checkFlowItems();				ASSERT_EVENT(3, 0, 4, -1);
		objects.RemoveAt(1);			ASSERT_EVENT(3, 1, 4, 1);
		checkFlowItems();				ASSERT_EVENT(3, 1, 5, -1);
		objects.RemoveRange(0, 2);		ASSERT_EVENT(3, 3, 5, 0);
		checkFlowItems();				ASSERT_EVENT(3, 3, 6, -1);

		objects.Add(L"A");				ASSERT_EVENT(4, 3, 6, 0);
		objects.Add(L"B");				ASSERT_EVENT(5, 3, 6, 1);
		objects.Add(L"C");				ASSERT_EVENT(6, 3, 6, 2);
		checkFlowItems();				ASSERT_EVENT(6, 3, 7, -1);
		objects.Clear();				ASSERT_EVENT(6, 6, 7, 0);
		checkFlowItems();				ASSERT_EVENT(6, 6, 8, -1);

		TEST_ASSERT(flow->Children().Count() == 0);

		SafeDeleteComposition(flow);
	});
}