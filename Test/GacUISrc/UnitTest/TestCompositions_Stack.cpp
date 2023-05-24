#include "TestCompositions.h"

using namespace vl::collections;
using namespace vl::presentation::templates;
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test <Stack> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiBoundsComposition>();
	});

	TEST_CASE(L"Test <Stack> with <StackItem>")
	{
		auto stack = new GuiStackComposition;

		stack->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		stack->SetPreferredMinSize(Size(50, 50));

		TEST_ASSERT(stack->GetDirection() == GuiStackComposition::Horizontal);
		TEST_ASSERT(stack->GetExtraMargin() == Margin(0, 0, 0, 0));
		TEST_ASSERT(stack->GetPadding() == 0);
		stack->SetExtraMargin(Margin(11, 22, 33, 44));
		stack->SetPadding(10);
		TEST_ASSERT(stack->GetExtraMargin() == Margin(11, 22, 33, 44));
		TEST_ASSERT(stack->GetPadding() == 10);

		const vint ITEM_COUNT = 3;
		GuiStackItemComposition* stackItems[ITEM_COUNT];
		for (vint i = 0; i < ITEM_COUNT; i++)
		{
			stackItems[i] = new GuiStackItemComposition;
			stackItems[i]->SetPreferredMinSize(Size(100 + i * 10, 100 + i * 20));
			stack->InsertStackItem(i, stackItems[i]);
		}

		auto testAllDirections = [&](bool expand)
		{
			{
				stack->SetDirection(GuiStackComposition::Horizontal);
				TEST_ASSERT(stack->GetDirection() == GuiStackComposition::Horizontal);
				TEST_ASSERT(stack->GetClientArea() == Rect({ 0,0 }, { 64 + 330,66 + 140 }));
				TEST_ASSERT(stack->GetMinPreferredClientSize() == stack->GetClientArea().GetSize());
				TEST_ASSERT(stack->GetPreferredBounds() == stack->GetClientArea());
				TEST_ASSERT(stack->GetBounds() == stack->GetClientArea());
				for (vint i = 0; i < ITEM_COUNT; i++)
				{
					vint offset = 11 + (100 + 10) * i + 10 * i * (i - 1) / 2;
					Rect expected({ offset,22 }, { 100 + i * 10,140 });
					if (expand)
					{
						expected.x1 -= 1 * i;
						expected.y1 -= 2 * i;
						expected.x2 += 3 * i;
						expected.y2 += 4 * i;
					}
					TEST_ASSERT(stackItems[i]->GetBounds() == expected);
				}
			}
			{
				stack->SetDirection(GuiStackComposition::ReversedHorizontal);
				TEST_ASSERT(stack->GetDirection() == GuiStackComposition::ReversedHorizontal);
				TEST_ASSERT(stack->GetClientArea() == Rect({ 0,0 }, { 64 + 330,66 + 140 }));
				TEST_ASSERT(stack->GetMinPreferredClientSize() == stack->GetClientArea().GetSize());
				TEST_ASSERT(stack->GetPreferredBounds() == stack->GetClientArea());
				TEST_ASSERT(stack->GetBounds() == stack->GetClientArea());
				for (vint i = 0; i < ITEM_COUNT; i++)
				{
					vint offset = 33 + (100 + 10) * i + 10 * i * (i - 1) / 2;
					offset = 64 + 330 - offset - (100 + i * 10);
					Rect expected({ offset,22 }, { 100 + i * 10,140 });
					if (expand)
					{
						expected.x1 -= 1 * i;
						expected.y1 -= 2 * i;
						expected.x2 += 3 * i;
						expected.y2 += 4 * i;
					}
					TEST_ASSERT(stackItems[i]->GetBounds() == expected);
				}
			}
			{
				stack->SetDirection(GuiStackComposition::Vertical);
				TEST_ASSERT(stack->GetDirection() == GuiStackComposition::Vertical);
				TEST_ASSERT(stack->GetClientArea() == Rect({ 0,0 }, { 44 + 120,86 + 360 }));
				TEST_ASSERT(stack->GetMinPreferredClientSize() == stack->GetClientArea().GetSize());
				TEST_ASSERT(stack->GetPreferredBounds() == stack->GetClientArea());
				TEST_ASSERT(stack->GetBounds() == stack->GetClientArea());
				for (vint i = 0; i < ITEM_COUNT; i++)
				{
					vint offset = 22 + (100 + 10) * i + 20 * i * (i - 1) / 2;
					Rect expected({ 11,offset }, { 120,100 + i * 20 });
					if (expand)
					{
						expected.x1 -= 1 * i;
						expected.y1 -= 2 * i;
						expected.x2 += 3 * i;
						expected.y2 += 4 * i;
					}
					TEST_ASSERT(stackItems[i]->GetBounds() == expected);
				}
			}
			{
				stack->SetDirection(GuiStackComposition::ReversedVertical);
				TEST_ASSERT(stack->GetDirection() == GuiStackComposition::ReversedVertical);
				TEST_ASSERT(stack->GetClientArea() == Rect({ 0,0 }, { 44 + 120,86 + 360 }));
				TEST_ASSERT(stack->GetMinPreferredClientSize() == stack->GetClientArea().GetSize());
				TEST_ASSERT(stack->GetPreferredBounds() == stack->GetClientArea());
				TEST_ASSERT(stack->GetBounds() == stack->GetClientArea());
				for (vint i = 0; i < ITEM_COUNT; i++)
				{
					vint offset = 44 + (100 + 10) * i + 20 * i * (i - 1) / 2;
					offset = 86 + 360 - offset - (100 + i * 20);
					Rect expected({ 11,offset }, { 120,100 + i * 20 });
					if (expand)
					{
						expected.x1 -= 1 * i;
						expected.y1 -= 2 * i;
						expected.x2 += 3 * i;
						expected.y2 += 4 * i;
					}
					TEST_ASSERT(stackItems[i]->GetBounds() == expected);
				}
			}
		};

		testAllDirections(false);
		for (vint i = 0; i < ITEM_COUNT; i++)
		{
			Margin extraMargin(1 * i, 2 * i, 3 * i, 4 * i);
			stackItems[i]->SetExtraMargin(extraMargin);
			TEST_ASSERT(stackItems[i]->GetExtraMargin() == extraMargin);
		}
		testAllDirections(true);

		SafeDeleteComposition(stack);
	});

	TEST_CASE(L"Test <Stack> with clipped <StackItem>")
	{
		auto stack = new GuiStackComposition;
		stack->SetPadding(10);

		const vint ITEM_COUNT = 10;
		GuiStackItemComposition* stackItems[ITEM_COUNT];
		for (vint i = 0; i < ITEM_COUNT; i++)
		{
			stackItems[i] = new GuiStackItemComposition;
			stackItems[i]->SetPreferredMinSize(Size(50, 50));
			stack->AddChild(stackItems[i]);
		}

		stack->SetPreferredMinSize(Size(170, 70));
		TEST_ASSERT(stack->GetClientArea() == Rect({ 0,0 }, { 170,70 }));
		TEST_ASSERT(stack->GetMinPreferredClientSize() == stack->GetClientArea().GetSize());
		TEST_ASSERT(stack->GetPreferredBounds() == stack->GetClientArea());
		TEST_ASSERT(stack->GetBounds() == stack->GetClientArea());
		{
			stack->SetDirection(GuiStackComposition::Horizontal);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ i * 60,0 }, { 50,70 }));
			}

			stack->EnsureVisible(5);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ i * 60 - 180,0 }, { 50,70 }));
			}

			stack->EnsureVisible(9);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ i * 60 - 420,0 }, { 50,70 }));
			}

			stack->EnsureVisible(0);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ i * 60,0 }, { 50,70 }));
			}
		}
		{
			stack->SetDirection(GuiStackComposition::ReversedHorizontal);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 120 - i * 60,0 }, { 50,70 }));
			}

			stack->EnsureVisible(5);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 300 - i * 60,0 }, { 50,70 }));
			}

			stack->EnsureVisible(9);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 540 - i * 60,0 }, { 50,70 }));
			}

			stack->EnsureVisible(0);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 120 - i * 60,0 }, { 50,70 }));
			}
		}

		stack->SetPreferredMinSize(Size(70, 170));
		TEST_ASSERT(stack->GetClientArea() == Rect({ 0,0 }, { 70,170 }));
		TEST_ASSERT(stack->GetMinPreferredClientSize() == stack->GetClientArea().GetSize());
		TEST_ASSERT(stack->GetPreferredBounds() == stack->GetClientArea());
		TEST_ASSERT(stack->GetBounds() == stack->GetClientArea());
		{
			stack->SetDirection(GuiStackComposition::Vertical);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 0,i * 60 }, { 70,50 }));
			}

			stack->EnsureVisible(5);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 0,i * 60 - 180 }, { 70,50 }));
			}

			stack->EnsureVisible(9);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 0,i * 60 - 420 }, { 70,50 }));
			}

			stack->EnsureVisible(0);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 0,i * 60 }, { 70,50 }));
			}
		}
		{
			stack->SetDirection(GuiStackComposition::ReversedVertical);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 0,120 - i * 60 }, { 70,50 }));
			}

			stack->EnsureVisible(5);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 0,300 - i * 60 }, { 70,50 }));
			}

			stack->EnsureVisible(9);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 0,540 - i * 60 }, { 70,50 }));
			}

			stack->EnsureVisible(0);
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetBounds() == Rect({ 0,120 - i * 60 }, { 70,50 }));
			}
		}

		SafeDeleteComposition(stack);
	});

	TEST_CASE(L"Test <RepeatStack>")
	{
		ObservableList<WString> objects;

		auto stack = new GuiRepeatStackComposition;
		stack->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		stack->SetPadding(10);
		stack->SetItemTemplate([](const Value& obj)
		{
			auto itemTemplate = new GuiTemplate;
			itemTemplate->SetText(UnboxValue<WString>(obj));
			itemTemplate->SetAlignmentToParent(Margin(0, 0, 0, 0));
			itemTemplate->SetPreferredMinSize(Size(50, 50));
			return itemTemplate;
		});
		stack->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(objects)));

		auto checkStackItems = [&]()
		{
			TEST_ASSERT(stack->GetClientArea() == Rect({ 0,0 }, {
				(objects.Count() == 0 ? 0 : 60 * objects.Count() - 10),
				(objects.Count() == 0 ? 0 : 50)
				}));
			TEST_ASSERT(stack->GetMinPreferredClientSize() == stack->GetClientArea().GetSize());
			TEST_ASSERT(stack->GetPreferredBounds() == stack->GetClientArea());
			TEST_ASSERT(stack->GetBounds() == stack->GetClientArea());

			TEST_ASSERT(stack->GetStackItems().Count() == objects.Count());
			for (auto [text, i] : indexed(objects))
			{
				auto stackItem = stack->GetStackItems()[i];
				TEST_ASSERT(stackItem->GetBounds() == Rect({ 60 * i,0 }, { 50,50 }));
				TEST_ASSERT(stackItem->Children().Count() == 1);
				auto itemTemplate = dynamic_cast<GuiTemplate*>(stackItem->Children()[0]);
				TEST_ASSERT(itemTemplate->GetText() == text);
				TEST_ASSERT(itemTemplate->GetBounds() == Rect({ 0,0 }, { 50,50 }));
			}
		};

		checkStackItems();
		objects.Add(L"A");
		checkStackItems();
		objects.Insert(0, L"B");
		checkStackItems();
		objects.Insert(1, L"C");
		checkStackItems();
		objects.RemoveAt(1);
		checkStackItems();
		objects.RemoveRange(0, 2);
		checkStackItems();

		objects.Add(L"A");
		objects.Add(L"B");
		objects.Add(L"C");
		checkStackItems();
		objects.Clear();
		checkStackItems();

		TEST_ASSERT(stack->Children().Count() == 0);

		SafeDeleteComposition(stack);
	});
}