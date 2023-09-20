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
		// TODO: change item template in the middle (text)
		// TODO: assign item template in the middle
		ObservableList<vint> xs;
		auto root = new GuiRepeatFreeHeightItemComposition;
		root->SetPreferredMinSize({ 100,100 });
		root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));
		root->SetItemTemplate([](const Value& value)
		{
			auto style = new GuiTemplate;
			style->SetText(itow(UnboxValue<vint>(value)));
			style->SetPreferredMinSize({ 10,20 });
			return style;
		});

		auto checkItems = [&]()
		{
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->Children().Count() == xs.Count());
			for (vint i = 0; i < xs.Count(); i++)
			{
				auto style = root->GetVisibleStyle(i);
				TEST_ASSERT(root->GetVisibleIndex(style) == i);
				TEST_ASSERT(style->GetCachedBounds() == Rect({ 0,i * 20 }, { 100,20 }));
				TEST_ASSERT(style->GetText() == itow(xs[i]));
				TEST_ASSERT(style->GetContext() == root->GetContext());
			}
		};

		checkItems();

		xs.Add(0);
		checkItems();

		xs.Add(1);
		checkItems();

		root->SetContext(BoxValue<vint>(100));
		checkItems();

		xs.Insert(1, 2);
		checkItems();

		xs.Insert(0, 3);
		checkItems();

		xs.Add(4);
		checkItems();

		xs.Remove(2);
		checkItems();

		root->SetContext({});
		checkItems();

		xs.RemoveRange(1, 2);
		checkItems();

		xs.Clear();
		checkItems();

		SafeDeleteComposition(root);
	});

	TEST_CATEGORY(L"Test <RepeatFreeHeightItem> binding with partial items visible")
	{
	});

	TEST_CATEGORY(L"Test <RepeatFreeHeightItem> layout in different direction")
	{
	});
}