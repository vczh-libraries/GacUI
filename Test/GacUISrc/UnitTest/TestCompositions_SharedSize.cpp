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

		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		for (vint group = 0; group < 2; group++)
		{
			for (vint item = 0; item < 3; item++)
			{
				TEST_ASSERT(items[group][item]->GetBounds() == Rect({ 0,0 }, { (group + 1) * 30,(item + 1) * 10 }));
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

		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		for (vint group = 0; group < 2; group++)
		{
			for (vint item = 0; item < 3; item++)
			{
				TEST_ASSERT(items[group][item]->GetBounds() == Rect({ 0,0 }, { (item + 1) * 10,(group + 1) * 30 }));
			}
		}

		SafeDeleteComposition(root);
	});

	TEST_CASE(L"Test <SharedSizeRoot> with widths and heights")
	{
	});
}