#include "TestCompositions.h"

using namespace vl::collections;
using namespace vl::presentation::templates;
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test <RepeatFreeHeightItem> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiRepeatFreeHeightItemComposition>();
	});

	TEST_CASE(L"Test <RepeatFreeHeightItem> binding without item template")
	{
		ObservableList<vint> xs;
		auto root = new GuiRepeatFreeHeightItemComposition;
		root->SetPreferredMinSize({ 100,100 });
		root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));
		TEST_ASSERT(root->Children().Count() == 0);

		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(root->Children().Count() == 0);

		xs.Add(0);
		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(root->Children().Count() == 0);

		xs.Insert(0, 1);
		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(root->Children().Count() == 0);

		xs.Insert(1, 2);
		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(root->Children().Count() == 0);

		xs.RemoveAt(1);
		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(root->Children().Count() == 0);

		xs.Clear();
		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(root->Children().Count() == 0);

		SafeDeleteComposition(root);
	});

	TEST_CASE(L"Test <RepeatFreeHeightItem> binding with all items visible")
	{
		ObservableList<vint> xs;
		auto root = new GuiRepeatFreeHeightItemComposition;
		root->SetPreferredMinSize({ 100,100 });
		root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));

		auto checkItems = [&](vint phase)
		{
			// TODO:
			// the first call update ExpectedBounds of item styles
			// but since the CachedMinSize has already been calculated
			// the size of ExpectedBounds doesn't take effect in time
			// the next call fix the issue
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			if (phase == 0)
			{
				TEST_ASSERT(root->Children().Count() == 0);
			}
			else
			{
				TEST_ASSERT(root->Children().Count() == xs.Count());
				for (vint i = 0; i < xs.Count(); i++)
				{
					auto style = root->GetVisibleStyle(i);
					TEST_ASSERT(root->GetVisibleIndex(style) == i);
					TEST_ASSERT(style->GetCachedBounds() == Rect({ 0,i * 20 }, { 100,20 }));
					if (phase == 1)
					{
						TEST_ASSERT(style->GetText() == itow(xs[i]));
					}
					else
					{
						TEST_ASSERT(style->GetText() == L"[" + itow(xs[i]) + L"]");
					}
					TEST_ASSERT(style->GetContext() == root->GetContext());
				}
			}
		};

		checkItems(0);

		xs.Add(0);
		checkItems(0);

		root->SetItemTemplate([](const Value& value)
		{
			auto style = new GuiTemplate;
			style->SetText(itow(UnboxValue<vint>(value)));
			style->SetPreferredMinSize({ 10,20 });
			return style;
		});

		xs.Add(1);
		checkItems(1);

		root->SetContext(BoxValue<vint>(100));
		checkItems(1);

		xs.Insert(1, 2);
		checkItems(1);

		xs.Insert(0, 3);
		checkItems(1);

		xs.Add(4);
		checkItems(1);

		root->SetItemTemplate([](const Value& value)
		{
			auto style = new GuiTemplate;
			style->SetText(L"[" + itow(UnboxValue<vint>(value)) + L"]");
			style->SetPreferredMinSize({ 10,20 });
			return style;
		});

		xs.Remove(2);
		checkItems(2);

		root->SetContext({});
		checkItems(2);

		xs.RemoveRange(1, 2);
		checkItems(2);

		xs.Clear();
		checkItems(2);

		SafeDeleteComposition(root);
	});

	TEST_CATEGORY(L"Test <RepeatFreeHeightItem> binding with scrolling")
	{
		ObservableList<vint> xs;
		GuiRepeatFreeHeightItemComposition* root = nullptr;

		auto itemTemplate = [](const Value& value)
		{
			auto style = new GuiTemplate;
			style->SetText(itow(UnboxValue<vint>(value)));
			style->SetPreferredMinSize({ 20,10 });
			return style;
		};

		auto checkItems = [&](vint first, vint count, vint x, vint y)
		{
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->Children().Count() == count);
			for (vint i = 0; i < count; i++)
			{
				auto style = root->GetVisibleStyle(first + i);
				TEST_ASSERT(root->GetVisibleIndex(style) == first + i);
				TEST_ASSERT(style->GetCachedBounds() == Rect({ x,i * 10 + y }, { 95,10 }));
				TEST_ASSERT(style->GetText() == itow(xs[first + i]));
				TEST_ASSERT(style->GetContext() == root->GetContext());
			}
		};

		TEST_CASE(L"Simple Scrolling")
		{
			root = new GuiRepeatFreeHeightItemComposition;
			root->SetPreferredMinSize({ 95,95 });
			root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));
			for (vint i = 0; i < 9; i++) xs.Add(i);
			root->SetItemTemplate(itemTemplate);

			checkItems(0, 9, 0, 0);

			for (vint i = 9; i < 20; i++) xs.Insert(i - 4, i);
			checkItems(0, 10, 0, 0);

			auto messingAround = [&]()
			{
				root->SetViewLocation({ 0,52 });
				checkItems(5, 10, 0, -2);

				root->SetViewLocation({ 0,105 });
				checkItems(10, 10, 0, -5);

				xs.RemoveRange(15, 5);
				checkItems(10, 5, 0, -5);

				root->SetViewLocation({ 0,-42 });
				checkItems(0, 6, 0, 42);

				for (vint i = 0; i < 5; i++) xs.Add(i + 20);
				checkItems(0, 6, 0, 42);
			};
			messingAround();

			xs.Clear();
			checkItems(0, 0, 0, 0);

			root->SetItemTemplate({});
			for (vint i = 0; i < 20; i++) xs.Add(i + 20);
			checkItems(0, 0, 0, 0);

			root->SetItemTemplate(itemTemplate);
			checkItems(0, 10, 0, 0);

			messingAround();

			root->SetItemTemplate({});
			checkItems(0, 0, 0, 0);

			xs.Clear();
			checkItems(0, 0, 0, 0);

			SafeDeleteComposition(root);
			root = nullptr;
		});

		TEST_CASE(L"Setting ViewBounds with X Offset")
		{
		});

		TEST_CASE(L"Changing Axis")
		{
		});
	});

	TEST_CATEGORY(L"Test <RepeatFreeHeightItem> layout in different direction")
	{
		// setup and add items with different size until overflow and thens scroll in all directions
	});
}