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
				stack->ForceCalculateSizeImmediately();

				TEST_ASSERT(stack->GetCachedClientArea() == Rect({ 0,0 }, { 64 + 330,66 + 140 }));
				TEST_ASSERT(stack->GetCachedMinSize() == stack->GetCachedClientArea().GetSize());
				TEST_ASSERT(stack->GetCachedBounds() == stack->GetCachedClientArea());
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
					TEST_ASSERT(stackItems[i]->GetCachedBounds() == expected);
				}
			}
			{
				stack->SetDirection(GuiStackComposition::ReversedHorizontal);
				TEST_ASSERT(stack->GetDirection() == GuiStackComposition::ReversedHorizontal);
				stack->ForceCalculateSizeImmediately();

				TEST_ASSERT(stack->GetCachedClientArea() == Rect({ 0,0 }, { 64 + 330,66 + 140 }));
				TEST_ASSERT(stack->GetCachedMinSize() == stack->GetCachedClientArea().GetSize());
				TEST_ASSERT(stack->GetCachedBounds() == stack->GetCachedClientArea());
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
					TEST_ASSERT(stackItems[i]->GetCachedBounds() == expected);
				}
			}
			{
				stack->SetDirection(GuiStackComposition::Vertical);
				TEST_ASSERT(stack->GetDirection() == GuiStackComposition::Vertical);
				stack->ForceCalculateSizeImmediately();

				TEST_ASSERT(stack->GetCachedClientArea() == Rect({ 0,0 }, { 44 + 120,86 + 360 }));
				TEST_ASSERT(stack->GetCachedMinSize() == stack->GetCachedClientArea().GetSize());
				TEST_ASSERT(stack->GetCachedBounds() == stack->GetCachedClientArea());
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
					TEST_ASSERT(stackItems[i]->GetCachedBounds() == expected);
				}
			}
			{
				stack->SetDirection(GuiStackComposition::ReversedVertical);
				TEST_ASSERT(stack->GetDirection() == GuiStackComposition::ReversedVertical);
				stack->ForceCalculateSizeImmediately();

				TEST_ASSERT(stack->GetCachedClientArea() == Rect({ 0,0 }, { 44 + 120,86 + 360 }));
				TEST_ASSERT(stack->GetCachedMinSize() == stack->GetCachedClientArea().GetSize());
				TEST_ASSERT(stack->GetCachedBounds() == stack->GetCachedClientArea());
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
					TEST_ASSERT(stackItems[i]->GetCachedBounds() == expected);
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
		stack->ForceCalculateSizeImmediately();
		TEST_ASSERT(stack->GetCachedClientArea() == Rect({ 0,0 }, { 170,70 }));
		TEST_ASSERT(stack->GetCachedMinSize() == stack->GetCachedClientArea().GetSize());
		TEST_ASSERT(stack->GetCachedBounds() == stack->GetCachedClientArea());
		{
			stack->SetDirection(GuiStackComposition::Horizontal);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ i * 60,0 }, { 50,70 }));
			}

			stack->EnsureVisible(5);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ i * 60 - 180,0 }, { 50,70 }));
			}

			stack->EnsureVisible(9);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ i * 60 - 420,0 }, { 50,70 }));
			}

			stack->EnsureVisible(0);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ i * 60,0 }, { 50,70 }));
			}
		}
		{
			stack->SetDirection(GuiStackComposition::ReversedHorizontal);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 120 - i * 60,0 }, { 50,70 }));
			}

			stack->EnsureVisible(5);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 300 - i * 60,0 }, { 50,70 }));
			}

			stack->EnsureVisible(9);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 540 - i * 60,0 }, { 50,70 }));
			}

			stack->EnsureVisible(0);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 120 - i * 60,0 }, { 50,70 }));
			}
		}

		stack->SetPreferredMinSize(Size(70, 170));
		stack->ForceCalculateSizeImmediately();
		TEST_ASSERT(stack->GetCachedClientArea() == Rect({ 0,0 }, { 70,170 }));
		TEST_ASSERT(stack->GetCachedMinSize() == stack->GetCachedClientArea().GetSize());
		TEST_ASSERT(stack->GetCachedBounds() == stack->GetCachedClientArea());
		{
			stack->SetDirection(GuiStackComposition::Vertical);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 0,i * 60 }, { 70,50 }));
			}

			stack->EnsureVisible(5);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 0,i * 60 - 180 }, { 70,50 }));
			}

			stack->EnsureVisible(9);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 0,i * 60 - 420 }, { 70,50 }));
			}

			stack->EnsureVisible(0);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 0,i * 60 }, { 70,50 }));
			}
		}
		{
			stack->SetDirection(GuiStackComposition::ReversedVertical);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 0,120 - i * 60 }, { 70,50 }));
			}

			stack->EnsureVisible(5);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 0,300 - i * 60 }, { 70,50 }));
			}

			stack->EnsureVisible(9);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 0,540 - i * 60 }, { 70,50 }));
			}

			stack->EnsureVisible(0);
			stack->ForceCalculateSizeImmediately();
			for (vint i = 0; i < ITEM_COUNT; i++)
			{
				TEST_ASSERT(stackItems[i]->GetCachedBounds() == Rect({ 0,120 - i * 60 }, { 70,50 }));
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

		vint contextValue = -1;
		auto checkStackItems = [&]()
		{
			stack->ForceCalculateSizeImmediately();
			TEST_ASSERT(stack->GetCachedClientArea() == Rect({ 0,0 }, {
				(objects.Count() == 0 ? 0 : 60 * objects.Count() - 10),
				(objects.Count() == 0 ? 0 : 50)
				}));
			TEST_ASSERT(stack->GetCachedMinSize() == stack->GetCachedClientArea().GetSize());
			TEST_ASSERT(stack->GetCachedBounds() == stack->GetCachedClientArea());

			TEST_ASSERT(stack->GetStackItems().Count() == objects.Count());
			for (auto [text, i] : indexed(objects))
			{
				auto stackItem = stack->GetStackItems()[i];
				TEST_ASSERT(stackItem->GetCachedBounds() == Rect({ 60 * i,0 }, { 50,50 }));
				TEST_ASSERT(stackItem->Children().Count() == 1);
				auto itemTemplate = dynamic_cast<GuiTemplate*>(stackItem->Children()[0]);
				TEST_ASSERT(itemTemplate->GetText() == text);
				TEST_ASSERT(itemTemplate->GetCachedBounds() == Rect({ 0,0 }, { 50,50 }));

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
			stack->SetContext(BoxValue(contextValue));
			TEST_ASSERT(UnboxValue<vint>(stack->GetContext()) == contextValue);
		};

		vint eventInsertCount = 0;
		vint eventRemoveCount = 0;
		vint eventContextCount = 0;
		vint eventIndex = -1;

		stack->ItemInserted.AttachLambda([&](auto self, GuiItemEventArgs& arguments) { eventInsertCount++; eventIndex = arguments.itemIndex; });
		stack->ItemRemoved.AttachLambda([&](auto self, GuiItemEventArgs& arguments) { eventRemoveCount++; eventIndex = arguments.itemIndex; });
		stack->ContextChanged.AttachLambda([&](auto self, auto& arguments) { eventContextCount++; eventIndex = -1; });
#define ASSERT_EVENT(IC, RC, CC, I) TEST_ASSERT(eventInsertCount == IC && eventRemoveCount == RC && eventContextCount == CC && eventIndex == I);

		checkStackItems();				ASSERT_EVENT(0, 0, 1, -1);
		objects.Add(L"A");				ASSERT_EVENT(1, 0, 1, 0);
		checkStackItems();				ASSERT_EVENT(1, 0, 2, -1);
		objects.Insert(0, L"B");		ASSERT_EVENT(2, 0, 2, 0);
		checkStackItems();				ASSERT_EVENT(2, 0, 3, -1);
		objects.Insert(1, L"C");		ASSERT_EVENT(3, 0, 3, 1);
		checkStackItems();				ASSERT_EVENT(3, 0, 4, -1);
		objects.RemoveAt(1);			ASSERT_EVENT(3, 1, 4, 1);
		checkStackItems();				ASSERT_EVENT(3, 1, 5, -1);
		objects.RemoveRange(0, 2);		ASSERT_EVENT(3, 3, 5, 0);
		checkStackItems();				ASSERT_EVENT(3, 3, 6, -1);

		objects.Add(L"A");				ASSERT_EVENT(4, 3, 6, 0);
		objects.Add(L"B");				ASSERT_EVENT(5, 3, 6, 1);
		objects.Add(L"C");				ASSERT_EVENT(6, 3, 6, 2);
		checkStackItems();				ASSERT_EVENT(6, 3, 7, -1);
		objects.Clear();				ASSERT_EVENT(6, 6, 7, 0);
		checkStackItems();				ASSERT_EVENT(6, 6, 8, -1);

		TEST_ASSERT(stack->Children().Count() == 0);

		SafeDeleteComposition(stack);
	});
}