#include "TestCompositions.h"
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test <SharedSizeRoot> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiSharedSizeRootComposition>();
	});

	TEST_CATEGORY(L"Test <SharedSizeItem> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiSharedSizeItemComposition>(
			[](GuiSharedSizeItemComposition* sharedSizeItem)
			{
				sharedSizeItem->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
			});
	});

	TEST_CASE(L"Test <SharedSizeRoot> with widths")
	{
		auto root = new GuiSharedSizeRootComposition;

		GuiSharedSizeItemComposition* items[2][3];
		for (vint group = 0; group < 2; group++)
		{
			for (vint item = 0; item < 3; item++)
			{
				auto bounds = new GuiBoundsComposition;
				root->AddChild(bounds);

				auto shared = new GuiSharedSizeItemComposition;
				shared->SetGroup(itow(group));
				shared->SetSharedWidth(true);
				bounds->AddChild(shared);

				auto content = new GuiBoundsComposition;
				content->SetPreferredMinSize(Size((group * 3 + item + 1) * 10, (item + 1) * 10));
				shared->AddChild(content);

				items[group][item] = shared;
			}
		}

		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 0,0 }));
		for (vint group = 0; group < 2; group++)
		{
			for (vint item = 0; item < 3; item++)
			{
				TEST_ASSERT(items[group][item]->GetCachedBounds() == Rect({ 0,0 }, { (group + 1) * 30,(item + 1) * 10 }));
			}
		}

		SafeDeleteComposition(root);
	});

	TEST_CASE(L"Test <SharedSizeRoot> with heights")
	{
		auto root = new GuiSharedSizeRootComposition;

		GuiSharedSizeItemComposition* items[2][3];
		for (vint group = 0; group < 2; group++)
		{
			for (vint item = 0; item < 3; item++)
			{
				auto bounds = new GuiBoundsComposition;
				root->AddChild(bounds);

				auto shared = new GuiSharedSizeItemComposition;
				shared->SetGroup(itow(group));
				shared->SetSharedHeight(true);
				bounds->AddChild(shared);

				auto content = new GuiBoundsComposition;
				content->SetPreferredMinSize(Size((item + 1) * 10, (group * 3 + item + 1) * 10));
				shared->AddChild(content);

				items[group][item] = shared;
			}
		}

		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 0,0 }));
		for (vint group = 0; group < 2; group++)
		{
			for (vint item = 0; item < 3; item++)
			{
				TEST_ASSERT(items[group][item]->GetCachedBounds() == Rect({ 0,0 }, { (item + 1) * 10,(group + 1) * 30 }));
			}
		}

		SafeDeleteComposition(root);
	});

	TEST_CASE(L"Test <SharedSizeRoot> with widths and heights")
	{
		auto root = new GuiSharedSizeRootComposition;

		auto itemW = new GuiSharedSizeItemComposition;
		{
			itemW->SetGroup(L"group");
			itemW->SetSharedWidth(true);
			root->AddChild(itemW);

			auto bounds = new GuiBoundsComposition;
			bounds->SetPreferredMinSize(Size(10, 10));
			itemW->AddChild(bounds);
		}

		auto itemH = new GuiSharedSizeItemComposition;
		{
			itemH->SetGroup(L"group");
			itemH->SetSharedHeight(true);
			root->AddChild(itemH);

			auto bounds = new GuiBoundsComposition;
			bounds->SetPreferredMinSize(Size(20, 20));
			itemH->AddChild(bounds);
		}

		auto itemWH = new GuiSharedSizeItemComposition;
		{
			itemWH->SetGroup(L"group");
			itemWH->SetSharedWidth(true);
			itemWH->SetSharedHeight(true);
			root->AddChild(itemWH);

			auto bounds = new GuiBoundsComposition;
			bounds->SetPreferredMinSize(Size(30, 30));
			itemWH->AddChild(bounds);
		}

		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(itemW->GetCachedBounds() == Rect({ 0,0 }, { 30,10 }));
		TEST_ASSERT(itemH->GetCachedBounds() == Rect({ 0,0 }, { 20,30 }));
		TEST_ASSERT(itemWH->GetCachedBounds() == Rect({ 0,0 }, { 30,30 }));

		SafeDeleteComposition(root);
	});

	TEST_CASE(L"Test <SharedSizeRoot> auto shrink")
	{
		auto root = new GuiSharedSizeRootComposition;

		constexpr vint ITEM_COUNT = 3;
		GuiSharedSizeItemComposition* items[ITEM_COUNT];
		for (vint i = 0; i < ITEM_COUNT; i++)
		{
			auto item = new GuiSharedSizeItemComposition;
			items[i] = item;

			item->SetGroup(L"group");
			item->SetSharedWidth(true);
			item->SetSharedHeight(true);
			root->AddChild(item);

			auto bounds = new GuiBoundsComposition;
			bounds->SetPreferredMinSize(Size((i + 1) * 10, (i + 1) * 10));
			item->AddChild(bounds);
		}

		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 0,0 }));

		for (vint j = ITEM_COUNT; j >= 1; j--)
		{
			if (j < ITEM_COUNT)
			{
				SafeDeleteComposition(items[j]);
				root->ForceCalculateSizeImmediately();
			}

			for (vint i = 0; i < j; i++)
			{
				TEST_ASSERT(items[i]->GetCachedBounds() == Rect({0,0}, { j * 10,j * 10 }));
			}
		}

		SafeDeleteComposition(root);
	});
}