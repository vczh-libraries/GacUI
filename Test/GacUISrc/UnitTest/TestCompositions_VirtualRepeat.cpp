#include "TestCompositions.h"

using namespace vl::collections;
using namespace vl::presentation::templates;
using namespace composition_bounds_tests;

#define FIND_ITEM(KEY, INPUT, OUTPUT) TEST_ASSERT(root->FindItemByRealKeyDirection(INPUT, KeyDirection::KEY) == OUTPUT)

TEST_FILE
{

/***********************************************************************
Common
***********************************************************************/

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
		TEST_ASSERT(root->GetTotalSize() == Size(0, 0));

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

		auto checkItemsCommon = [&]<typename TGetBounds>(vint first, vint count, TGetBounds&& getBounds)
		{
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->Children().Count() == count);
			for (vint i = 0; i < count; i++)
			{
				auto style = root->GetVisibleStyle(first + i);
				TEST_ASSERT(root->GetVisibleIndex(style) == first + i);
				TEST_ASSERT(style->GetText() == itow(xs[first + i]));
				TEST_ASSERT(style->GetContext() == root->GetContext());

				auto actualBounds = style->GetCachedBounds();
				auto expectedBounds = getBounds(i);
				TEST_ASSERT(actualBounds == expectedBounds);
			}
		};

		auto checkItems = [&](vint first, vint count, vint x, vint y)
		{
			checkItemsCommon(first, count, [=](vint i)
			{
				return Rect({ x,i * 10 + y }, { 85,10 });
			});
		};

		auto checkItemsR2L = [&](vint first, vint count, vint x, vint y)
		{
			checkItemsCommon(first, count, [=](vint i)
			{
				return Rect({ x - i * 20,y }, { 20,95 });
			});
		};

		TEST_CASE(L"Simple Scrolling")
		{
			root = new GuiRepeatFreeHeightItemComposition;
			root->SetPreferredMinSize({ 85,95 });
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
			root = new GuiRepeatFreeHeightItemComposition;
			root->SetPreferredMinSize({ 85,95 });
			root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));
			root->SetItemTemplate(itemTemplate);

			for (vint i = 0; i < 20; i++) xs.Add(i);
			checkItems(0, 10, 0, 0);

			root->SetViewLocation({ 52,52 });
			checkItems(5, 10, -52, -2);

			xs.RemoveRange(5, 10);
			checkItems(5, 5, -52, -2);

			xs.Clear();
			checkItems(0, 0, 0, 0);

			SafeDeleteComposition(root);
			root = nullptr;
		});

		TEST_CASE(L"Changing Axis")
		{
			root = new GuiRepeatFreeHeightItemComposition;
			root->SetPreferredMinSize({ 85,95 });
			root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));
			root->SetItemTemplate(itemTemplate);
			root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));

			for (vint i = 0; i < 3; i++) xs.Add(i);
			checkItems(0, 3, 0, 0);

			root->SetViewLocation({ -30,-20 });
			checkItems(0, 3, 30, 20);

			root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpLeft)));
			checkItemsR2L(0, 3, 65, 0);

			root->SetViewLocation({ 5,20 });
			checkItemsR2L(0, 3, 35, -20);

			xs.Clear();
			checkItems(0, 0, 0, 0);

			SafeDeleteComposition(root);
			root = nullptr;
		});
	});

/***********************************************************************
<RepeatFreeHeightItem>
***********************************************************************/

	TEST_CATEGORY(L"Test <RepeatFreeHeightItem> layout in different direction, with GetTotalSize and EnsureItemVisible")
	{
		ObservableList<vint> xs;
		for (vint i = 0; i < 20; i++) xs.Add(i);

		auto root = new GuiRepeatFreeHeightItemComposition;
		root->SetPreferredMinSize({ 100,100 });
		root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));

		root->SetItemTemplate([](const Value& value)
		{
			vint x = UnboxValue<vint>(value);
			auto style = new GuiTemplate;
			style->SetText(itow(UnboxValue<vint>(value)));
			style->SetPreferredMinSize({ (x + 1) + 2,(x + 1) + 3});
			return style;
		});

		auto checkItemsCommon = [&]<typename TGetBounds>(vint first, vint count, TGetBounds&& getBounds)
		{
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->Children().Count() == count);
			for (vint i = 0; i < count; i++)
			{
				auto style = root->GetVisibleStyle(first + i);
				TEST_ASSERT(root->GetVisibleIndex(style) == first + i);
				TEST_ASSERT(style->GetText() == itow(xs[first + i]));
				TEST_ASSERT(style->GetContext() == root->GetContext());

				auto actualBounds = style->GetCachedBounds();
				auto expectedBounds = getBounds(first + i);
				TEST_ASSERT(actualBounds == expectedBounds);
			}
		};

		auto checkItemsDown = [&](vint first, vint count, vint x0, vint y0)
		{
			checkItemsCommon(first, count, [=](vint i)
			{
				vint offset = i * (i + 1) / 2 + i * 3;
				vint thickness = (i + 1) + 3;
				return Rect({ x0,y0 + offset }, { 100,thickness });
			});
		};

		auto checkItemsUp = [&](vint first, vint count, vint x0, vint y0)
		{
			checkItemsCommon(first, count, [=](vint i)
			{
				vint offset = i * (i + 1) / 2 + i * 3;
				vint thickness = (i + 1) + 3;
				return Rect({ x0,y0 - offset - thickness }, { 100,thickness });
			});
		};

		auto checkItemsRight = [&](vint first, vint count, vint x0, vint y0)
		{
			checkItemsCommon(first, count, [=](vint i)
			{
				vint offset = i * (i + 1) / 2 + i * 2;
				vint thickness = (i + 1) + 2;
				return Rect({ x0 + offset,y0 }, { thickness,100 });
			});
		};

		auto checkItemsLeft = [&](vint first, vint count, vint x0, vint y0)
		{
			checkItemsCommon(first, count, [=](vint i)
			{
				vint offset = i * (i + 1) / 2 + i * 2;
				vint thickness = (i + 1) + 2;
				return Rect({ x0 - offset - thickness,y0 }, { thickness,100 });
			});
		};

		//   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19
		//---------------------------------------------------------------------------------------------------
		//   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22
		//   3,   7,  12,  18,  25,  33,  42,  52,  63,  75,  88, 102, 117, 133, 150, 168, 187, 207, 228, 250
		//---------------------------------------------------------------------------------------------------
		//   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23
		//   4,   9,  15,  22,  30,  39,  49,  60,  72,  85,  99, 114, 130, 147, 165, 184, 204, 225, 247, 270

		// move to the top so its page size becomes 12
		root->SetViewLocation({ 0,0 });
		root->ForceCalculateSizeImmediately();
		root->ForceCalculateSizeImmediately();

		TEST_CASE(L"FindItemByVirtualKeyDirection")
		{
			FIND_ITEM(Up		,	0	,	0	);
			FIND_ITEM(Down		,	0	,	1	);
			FIND_ITEM(Left		,	0	,	-1	);
			FIND_ITEM(Right		,	0	,	-1	);
			FIND_ITEM(Home		,	0	,	0	);
			FIND_ITEM(End		,	0	,	19	);
			FIND_ITEM(PageUp	,	0	,	0	);
			FIND_ITEM(PageDown	,	0	,	12	);
			FIND_ITEM(PageLeft	,	0	,	-1	);
			FIND_ITEM(PageRight	,	0	,	-1	);

			FIND_ITEM(Up		,	1	,	0	);
			FIND_ITEM(Down		,	1	,	2	);
			FIND_ITEM(Left		,	1	,	-1	);
			FIND_ITEM(Right		,	1	,	-1	);
			FIND_ITEM(Home		,	1	,	0	);
			FIND_ITEM(End		,	1	,	19	);
			FIND_ITEM(PageUp	,	1	,	0	);
			FIND_ITEM(PageDown	,	1	,	13	);
			FIND_ITEM(PageLeft	,	1	,	-1	);
			FIND_ITEM(PageRight	,	1	,	-1	);

			FIND_ITEM(Up		,	18	,	17	);
			FIND_ITEM(Down		,	18	,	19	);
			FIND_ITEM(Left		,	18	,	-1	);
			FIND_ITEM(Right		,	18	,	-1	);
			FIND_ITEM(Home		,	18	,	0	);
			FIND_ITEM(End		,	18	,	19	);
			FIND_ITEM(PageUp	,	18	,	6	);
			FIND_ITEM(PageDown	,	18	,	19	);
			FIND_ITEM(PageLeft	,	18	,	-1	);
			FIND_ITEM(PageRight	,	18	,	-1	);

			FIND_ITEM(Up		,	19	,	18	);
			FIND_ITEM(Down		,	19	,	19	);
			FIND_ITEM(Left		,	19	,	-1	);
			FIND_ITEM(Right		,	19	,	-1	);
			FIND_ITEM(Home		,	19	,	0	);
			FIND_ITEM(End		,	19	,	19	);
			FIND_ITEM(PageUp	,	19	,	7	);
			FIND_ITEM(PageDown	,	19	,	19	);
			FIND_ITEM(PageLeft	,	19	,	-1	);
			FIND_ITEM(PageRight	,	19	,	-1	);
		});

		TEST_CASE(L"GetAdoptedSize")
		{
			{
				auto size = root->GetAdoptedSize({ 100,100 });
				TEST_ASSERT(size == Size(100, 200));
			}
			{
				auto size = root->GetAdoptedSize({ 100,300 });
				TEST_ASSERT(size == Size(100, 300));
			}
		});

		auto testDown = [&](bool useMinimumTotalSize)
		{
			const vint w = useMinimumTotalSize ? 0 : 100;

			checkItemsDown(0, 12, 0, 0);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(w, 114 + 8));

			root->SetViewLocation({ 10,100 });
			checkItemsDown(11, 6, -10, -100);
			TEST_ASSERT(root->GetViewLocation() == Point(10, 100));
			TEST_ASSERT(root->GetTotalSize() == Size(w, 204 + 3));

			root->SetViewLocation({ 20,200 });
			checkItemsDown(16, 4, -20, -200);
			TEST_ASSERT(root->GetViewLocation() == Point(20, 200));
			TEST_ASSERT(root->GetTotalSize() == Size(w, 270));

			TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
			TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(20, 0));
			checkItemsDown(0, 12, -20, 0);
			TEST_ASSERT(root->GetTotalSize() == Size(w, 270));

			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(20, 170));
			checkItemsDown(15, 5, -20, -170);
			TEST_ASSERT(root->GetTotalSize() == Size(w, 270));
		};

		auto testUp = [&](bool useMinimumTotalSize)
		{
			const vint w = useMinimumTotalSize ? 0 : 100;

			checkItemsUp(0, 12, 0, 100);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 22));
			TEST_ASSERT(root->GetTotalSize() == Size(w, 114 + 8));

			root->SetViewLocation({ 10,-78 });
			checkItemsUp(11, 6, -10, 200);
			TEST_ASSERT(root->GetViewLocation() == Point(10, 7));
			TEST_ASSERT(root->GetTotalSize() == Size(w, 204 + 3));

			root->SetViewLocation({ 20,-93 });
			checkItemsUp(16, 4, -20, 300);
			TEST_ASSERT(root->GetViewLocation() == Point(20, -30));
			TEST_ASSERT(root->GetTotalSize() == Size(w, 270));

			TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
			TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(20, 170));
			checkItemsUp(0, 12, -20, 100);
			TEST_ASSERT(root->GetTotalSize() == Size(w, 270));

			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(20, 0));
			checkItemsUp(15, 5, -20, 270);
			TEST_ASSERT(root->GetTotalSize() == Size(w, 270));
		};

		auto testRight = [&](bool useMinimumTotalSize)
		{
			const vint h = useMinimumTotalSize ? 0 : 100;

			checkItemsRight(0, 12, 0, 0);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(102 + 8, h));

			root->SetViewLocation({ 100,10 });
			checkItemsRight(11, 7, -100, -10);
			TEST_ASSERT(root->GetViewLocation() == Point(100, 10));
			TEST_ASSERT(root->GetTotalSize() == Size(207 + 2, h));

			root->SetViewLocation({ 200,20 });
			checkItemsRight(17, 3, -200, -20);
			TEST_ASSERT(root->GetViewLocation() == Point(200, 20));
			TEST_ASSERT(root->GetTotalSize() == Size(250, h));

			TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
			TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 20));
			checkItemsRight(0, 12, 0, -20);
			TEST_ASSERT(root->GetTotalSize() == Size(250, h));

			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(150, 20));
			checkItemsRight(15, 5, -150, -20);
			TEST_ASSERT(root->GetTotalSize() == Size(250, h));
		};

		auto testLeft = [&](bool useMinimumTotalSize)
		{
			const vint h = useMinimumTotalSize ? 0 : 100;

			checkItemsLeft(0, 12, 100, 0);
			TEST_ASSERT(root->GetViewLocation() == Point(10, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(102 + 8, h));

			root->SetViewLocation({ -90,10 });
			checkItemsLeft(11, 7, 200, -10);
			TEST_ASSERT(root->GetViewLocation() == Point(9, 10));
			TEST_ASSERT(root->GetTotalSize() == Size(207 + 2, h));

			root->SetViewLocation({ -91,20 });
			checkItemsLeft(17, 3, 300, -20);
			TEST_ASSERT(root->GetViewLocation() == Point(-50, 20));
			TEST_ASSERT(root->GetTotalSize() == Size(250, h));

			TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
			TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(150, 20));
			checkItemsLeft(0, 12, 100, -20);
			TEST_ASSERT(root->GetTotalSize() == Size(250, h));

			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 20));
			checkItemsLeft(15, 5, 250, -20);
			TEST_ASSERT(root->GetTotalSize() == Size(250, h));
		};

		auto testAllDirections = [&](bool useMinimumTotalSize)
		{
			TEST_CASE(L"RightDown")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
				testDown(useMinimumTotalSize);
			});

			TEST_CASE(L"LeftDown")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftDown)));
				testDown(useMinimumTotalSize);
			});

			TEST_CASE(L"RightUp")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightUp)));
				testUp(useMinimumTotalSize);
			});

			TEST_CASE(L"LeftUp")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
				testUp(useMinimumTotalSize);
			});

			TEST_CASE(L"DownRight")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownRight)));
				testRight(useMinimumTotalSize);
			});

			TEST_CASE(L"UpRight")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
				testRight(useMinimumTotalSize);
			});

			TEST_CASE(L"DownLeft")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
				testLeft(useMinimumTotalSize);
			});

			TEST_CASE(L"UpLeft")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpLeft)));
				testLeft(useMinimumTotalSize);
			});
		};

		TEST_CATEGORY(L"UseMinimumTotalSize == false")
		{
			TEST_CASE_ASSERT(root->GetUseMinimumTotalSize() == false);
			testAllDirections(false);
		});

		TEST_CATEGORY(L"UseMinimumTotalSize == true")
		{
			root->SetUseMinimumTotalSize(true);
			TEST_CASE_ASSERT(root->GetUseMinimumTotalSize() == true);
			testAllDirections(true);
		});

		SafeDeleteComposition(root);
	});

/***********************************************************************
<RepeatFixedHeightItem>
***********************************************************************/

	TEST_CATEGORY(L"Test <RepeatFixedHeightItem> layout in different direction, with GetTotalSize and EnsureItemVisible")
	{
		ObservableList<vint> xs;
		GuiRepeatFixedHeightItemComposition* root = nullptr;

		auto checkItems = [&](vint first, vint count, vint x0, vint y0, vint ow, vint oh, vint iw = 0, vint ih = 0)
		{
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->Children().Count() == count);

			if (iw == 0) iw = (ow == 0 ? root->GetCachedBounds().Width() : (ow > 0 ? ow : -ow));
			if (ih == 0) ih = (oh == 0 ? root->GetCachedBounds().Height() : (oh > 0 ? oh : -oh));
			if (ow < 0) x0 += ow;
			if (oh < 0) y0 += oh;

			for (vint i = 0; i < count; i++)
			{
				auto style = root->GetVisibleStyle(first + i);
				TEST_ASSERT(root->GetVisibleIndex(style) == first + i);
				TEST_ASSERT(style->GetText() == itow(xs[first + i]));
				TEST_ASSERT(style->GetContext() == root->GetContext());

				auto actualBounds = style->GetCachedBounds();
				auto expectedBounds = Rect({
					x0 + (i + first) * ow,
					y0 + (i + first) * oh
				}, {
					iw,
					ih
				});
				TEST_ASSERT(actualBounds == expectedBounds);
			}
		};

		TEST_CASE(L"Item of different PreferredMinSize")
		{
			root = new GuiRepeatFixedHeightItemComposition;
			root->SetPreferredMinSize({ 100,100 });
			root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));
			root->SetItemTemplate([](const Value& value)
			{
				vint x = UnboxValue<vint>(value);
				auto style = new GuiTemplate;
				style->SetText(itow(UnboxValue<vint>(value)));
				style->SetPreferredMinSize({ 10 + x,10 + x});
				return style;
			});

			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->Children().Count() == 0);
			TEST_ASSERT(root->GetTotalSize() == Size(0, 0));

			for (vint i = 1; i <= 20; i++) xs.Add(i);
			checkItems(0, 6, 0, 0, 0, 17);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 340));

			root->EnsureItemVisible(4);
			checkItems(0, 6, 0, 0, 0, 17);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 340));

			root->EnsureItemVisible(5);
			checkItems(0, 6, 0, -2, 0, 17);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 2));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 340));

			root->EnsureItemVisible(0);
			checkItems(0, 6, 0, 0, 0, 17);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 340));

			root->EnsureItemVisible(9);
			checkItems(5, 5, 0, -100, 0, 20);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 100));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 400));

			root->EnsureItemVisible(19);
			checkItems(16, 4, 0, -500, 0, 30);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 500));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 600));

			root->EnsureItemVisible(18);
			checkItems(16, 4, 0, -500, 0, 30);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 500));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 600));

			root->EnsureItemVisible(0);
			checkItems(0, 4, 0, 0, 0, 30);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 600));

			xs.Clear();
			SafeDeleteComposition(root);
			root = nullptr;
		});

		{
			for (vint i = 0; i < 20; i++) xs.Add(i + 1);

			root = new GuiRepeatFixedHeightItemComposition;
			root->SetPreferredMinSize({ 100,100 });
			root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));

			root->SetItemTemplate([](const Value& value)
			{
				vint x = UnboxValue<vint>(value);
				auto style = new GuiTemplate;
				style->SetText(itow(UnboxValue<vint>(value)));
				style->SetPreferredMinSize({ 15,15 });
				return style;
			});

			// trigger layout so its page size becomes 6
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();

			TEST_CASE(L"FindItemByVirtualKeyDirection")
			{
				FIND_ITEM(Up, 0, 0);
				FIND_ITEM(Down, 0, 1);
				FIND_ITEM(Left, 0, -1);
				FIND_ITEM(Right, 0, -1);
				FIND_ITEM(Home, 0, 0);
				FIND_ITEM(End, 0, 19);
				FIND_ITEM(PageUp, 0, 0);
				FIND_ITEM(PageDown, 0, 6);
				FIND_ITEM(PageLeft, 0, -1);
				FIND_ITEM(PageRight, 0, -1);

				FIND_ITEM(Up, 1, 0);
				FIND_ITEM(Down, 1, 2);
				FIND_ITEM(Left, 1, -1);
				FIND_ITEM(Right, 1, -1);
				FIND_ITEM(Home, 1, 0);
				FIND_ITEM(End, 1, 19);
				FIND_ITEM(PageUp, 1, 0);
				FIND_ITEM(PageDown, 1, 7);
				FIND_ITEM(PageLeft, 1, -1);
				FIND_ITEM(PageRight, 1, -1);

				FIND_ITEM(Up, 18, 17);
				FIND_ITEM(Down, 18, 19);
				FIND_ITEM(Left, 18, -1);
				FIND_ITEM(Right, 18, -1);
				FIND_ITEM(Home, 18, 0);
				FIND_ITEM(End, 18, 19);
				FIND_ITEM(PageUp, 18, 12);
				FIND_ITEM(PageDown, 18, 19);
				FIND_ITEM(PageLeft, 18, -1);
				FIND_ITEM(PageRight, 18, -1);

				FIND_ITEM(Up, 19, 18);
				FIND_ITEM(Down, 19, 19);
				FIND_ITEM(Left, 19, -1);
				FIND_ITEM(Right, 19, -1);
				FIND_ITEM(Home, 19, 0);
				FIND_ITEM(End, 19, 19);
				FIND_ITEM(PageUp, 19, 13);
				FIND_ITEM(PageDown, 19, 19);
				FIND_ITEM(PageLeft, 19, -1);
				FIND_ITEM(PageRight, 19, -1);
			});

			TEST_CASE(L"GetAdoptedSize")
			{
				{
					auto size = root->GetAdoptedSize({ 100,100 });
					TEST_ASSERT(size == Size(100, 105));
				}
				{
					auto size = root->GetAdoptedSize({ 100,300 });
					TEST_ASSERT(size == Size(100, 300));
				}
				{
					auto size = root->GetAdoptedSize({ 100,500 });
					TEST_ASSERT(size == Size(100, 300));
				}
			});

			root->SetItemYOffset(30);
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();

			TEST_CASE(L"GetAdoptedSize with YOffset")
			{
				{
					auto size = root->GetAdoptedSize({ 100,100 });
					TEST_ASSERT(size == Size(100, 105));
				}
				{
					auto size = root->GetAdoptedSize({ 100,300 });
					TEST_ASSERT(size == Size(100, 300));
				}
				{
					auto size = root->GetAdoptedSize({ 100,500 });
					TEST_ASSERT(size == Size(100, 330));
				}
			});

			root->SetItemYOffset(0);
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();

			TEST_CASE(L"GetTotalSize")
			{
				// 15*20
				TEST_ASSERT(root->GetTotalSize() == Size(100, 300));

				// 15*20
				root->SetPreferredMinSize({ 150,200 });
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(150, 300));

				// 15*35
				for (vint i = 1; i <= 15; i++) xs.Add(i);
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(150, 525));

				// 15*35
				root->SetPreferredMinSize({ 100,100 });
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(100, 525));

				// 15*20
				xs.RemoveRange(20, 15);
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(100, 300));
			});

			auto testHorizontal = [&](bool useMinimumTotalSize, vint y0, vint y1, vint y2, vint vy0, vint vy1, vint vy2, vint h, vint cw = 0, vint ch = 0)
			{
				vint tw = cw == 0 ? (useMinimumTotalSize ? 0 : 100) : cw;
				vint th = ch == 0 ? 300 : ch;
				vint fxo = cw == 0 ? 0 : 1;

				checkItems(0, 7, 0, y0, 0, h, cw, 0);
				TEST_ASSERT(root->GetViewLocation() == Point(0, vy0));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				root->SetViewLocation({ 10,vy1 });
				checkItems(6, 8, (fxo * -10), y1, 0, h, cw, 0);
				TEST_ASSERT(root->GetViewLocation() == Point(10, vy1));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				root->SetViewLocation({ 20,vy2 });
				checkItems(13, 7, (fxo * -20), y2, 0, h, cw, 0);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy2));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
				TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy0));
				checkItems(0, 7, (fxo * -20), y0, 0, h, cw, 0);
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy2));
				checkItems(13, 7, (fxo * -20), y2, 0, h, cw, 0);
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));
			};

			auto testVertical = [&](bool useMinimumTotalSize, vint x0, vint x1, vint x2, vint vx0, vint vx1, vint vx2, vint w)
			{
				vint tw = 300;
				vint th = useMinimumTotalSize ? 0 : 100;

				checkItems(0, 7, x0, 0, w, 0);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, 0));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				root->SetViewLocation({ vx1,10 });
				checkItems(6, 8, x1, 0, w, 0);
				TEST_ASSERT(root->GetViewLocation() == Point(vx1, 10));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				root->SetViewLocation({ vx2,20 });
				checkItems(13, 7, x2, 0, w, 0);
				TEST_ASSERT(root->GetViewLocation() == Point(vx2, 20));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
				TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, 20));
				checkItems(0, 7, x0, 0, w, 0);
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx2, 20));
				checkItems(13, 7, x2, 0, w, 0);
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));
			};

			auto testDown = [&](bool useMinimumTotalSize)
			{
				testHorizontal(
					useMinimumTotalSize,
					0, -100, -200,
					0, 100, 200,
					15);
			};

			auto testUp = [&](bool useMinimumTotalSize)
			{
				testHorizontal(
					useMinimumTotalSize,
					100, 200, 300,
					200, 100, 0,
					-15);
			};

			auto testRight = [&](bool useMinimumTotalSize)
			{
				testVertical(
					useMinimumTotalSize,
					0, -100, -200,
					0, 100, 200,
					15);
			};

			auto testLeft = [&](bool useMinimumTotalSize)
			{
				testVertical(
					useMinimumTotalSize,
					100, 200, 300,
					200, 100, 0,
					-15);
			};

			auto testAllDirections = [&](bool useMinimumTotalSize)
			{
				root->SetAxis(nullptr);

				TEST_CASE(L"ItemWidth")
				{
					root->SetItemWidth(80);
					root->ResetLayout(false);
					testHorizontal(
						useMinimumTotalSize,
						0, -100, -200,
						0, 100, 200,
						15, 80, 0);
				
					root->SetItemWidth(-1);
					root->ResetLayout(false);
					testDown(useMinimumTotalSize);
				});

				TEST_CASE(L"ItemYOffset")
				{
					root->SetItemYOffset(30);
					root->ResetLayout(false);
					root->SetPreferredMinSize({ 100,130 });
					testHorizontal(
						useMinimumTotalSize,
						30, -70, -170,
						0, 100, 200,
						15, 0, 330);

					root->SetItemYOffset(0);
					root->ResetLayout(false);
					root->SetPreferredMinSize({ 100,100 });
					testDown(useMinimumTotalSize);
				});

				TEST_CASE(L"RightDown")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
					testDown(useMinimumTotalSize);
				});

				TEST_CASE(L"LeftDown")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftDown)));
					testDown(useMinimumTotalSize);
				});

				TEST_CASE(L"RightUp")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightUp)));
					testUp(useMinimumTotalSize);
				});

				TEST_CASE(L"LeftUp")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
					testUp(useMinimumTotalSize);
				});

				TEST_CASE(L"DownRight")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownRight)));
					testRight(useMinimumTotalSize);
				});

				TEST_CASE(L"UpRight")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
					testRight(useMinimumTotalSize);
				});

				TEST_CASE(L"DownLeft")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
					testLeft(useMinimumTotalSize);
				});

				TEST_CASE(L"UpLeft")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpLeft)));
					testLeft(useMinimumTotalSize);
				});
			};

			TEST_CATEGORY(L"UseMinimumTotalSize == false")
			{
				TEST_CASE_ASSERT(root->GetUseMinimumTotalSize() == false);
				testAllDirections(false);
			});

			TEST_CATEGORY(L"UseMinimumTotalSize == true")
			{
				root->SetUseMinimumTotalSize(true);
				TEST_CASE_ASSERT(root->GetUseMinimumTotalSize() == true);
				testAllDirections(true);
			});

			SafeDeleteComposition(root);
			root = nullptr;
		}
	});

/***********************************************************************
<RepeatFixedSizeMultiColumnItem>
***********************************************************************/

	TEST_CATEGORY(L"Test <RepeatFixedSizeMultiColumnItem> layout in different direction, with GetTotalSize and EnsureItemVisible")
	{
		ObservableList<vint> xs;
		GuiRepeatFixedSizeMultiColumnItemComposition* root = nullptr;

		auto checkItems = [&](vint first, vint count, vint x0, vint y0, vint w, vint h, bool horizontal)
		{
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->Children().Count() == count);

			if (w < 0) x0 += w;
			if (h < 0) y0 += h;
			vint cw = w > 0 ? w : -w;
			vint ch = h > 0 ? h : -h;

			for (vint i = 0; i < count; i++)
			{
				auto style = root->GetVisibleStyle(first + i);
				TEST_ASSERT(root->GetVisibleIndex(style) == first + i);
				TEST_ASSERT(style->GetText() == itow(xs[first + i]));
				TEST_ASSERT(style->GetContext() == root->GetContext());
				vint s = horizontal ?
					root->GetCachedBounds().Width() / cw :
					root->GetCachedBounds().Height() / ch;
				vint r = (first + i) / s;
				vint c = (first + i) % s;

				auto actualBounds = style->GetCachedBounds();
				auto expectedBounds = Rect({
					x0 + (horizontal ? c : r) * w,
					y0 + (horizontal ? r : c) * h
				}, {
					cw,
					ch
				});
				TEST_ASSERT(actualBounds == expectedBounds);
			}
		};

		TEST_CASE(L"Item of different PreferredMinSize")
		{
			root = new GuiRepeatFixedSizeMultiColumnItemComposition;
			root->SetPreferredMinSize({ 100,100 });
			root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));
			root->SetItemTemplate([](const Value& value)
			{
				vint x = UnboxValue<vint>(value);
				auto style = new GuiTemplate;
				style->SetText(itow(UnboxValue<vint>(value)));
				style->SetPreferredMinSize({ 20 + x,20 + x});
				return style;
			});

			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->Children().Count() == 0);
			TEST_ASSERT(root->GetTotalSize() == Size(0, 0));

			for (vint i = 1; i <= 30; i++) xs.Add(i);
			checkItems(0, 12, 0, 0, 32, 32, true);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(96, 320));

			root->EnsureItemVisible(8);
			checkItems(0, 12, 0, 0, 32, 32, true);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(96, 320));

			root->EnsureItemVisible(9);
			checkItems(0, 12, 0, -28, 32, 32, true);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 28));
			TEST_ASSERT(root->GetTotalSize() == Size(96, 320));

			root->EnsureItemVisible(11);
			checkItems(0, 12, 0, -28, 32, 32, true);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 28));
			TEST_ASSERT(root->GetTotalSize() == Size(96, 320));

			root->EnsureItemVisible(29);
			checkItems(26, 4, 0, -650, 50, 50, true);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 650));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 750));

			root->EnsureItemVisible(26);
			checkItems(26, 4, 0, -650, 50, 50, true);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 650));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 750));

			root->EnsureItemVisible(0);
			checkItems(0, 4, 0, 0, 50, 50, true);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 750));

			xs.Clear();
			SafeDeleteComposition(root);
			root = nullptr;
		});

		{
			for (vint i = 1; i <= 30; i++) xs.Add(i);

			root = new GuiRepeatFixedSizeMultiColumnItemComposition;
			root->SetPreferredMinSize({ 100,100 });
			root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));

			root->SetItemTemplate([](const Value& value)
			{
				vint x = UnboxValue<vint>(value);
				auto style = new GuiTemplate;
				style->SetText(itow(UnboxValue<vint>(value)));
				style->SetPreferredMinSize({ 30,30 });
				return style;
			});

			// trigger layout so its page size becomes 3x3
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();

			TEST_CASE(L"FindItemByVirtualKeyDirection")
			{
				FIND_ITEM(Up		,	0	,	0	);
				FIND_ITEM(Down		,	0	,	3	);
				FIND_ITEM(Left		,	0	,	0	);
				FIND_ITEM(Right		,	0	,	1	);
				FIND_ITEM(Home		,	0	,	0	);
				FIND_ITEM(End		,	0	,	29	);
				FIND_ITEM(PageUp	,	0	,	0	);
				FIND_ITEM(PageDown	,	0	,	9	);
				FIND_ITEM(PageLeft	,	0	,	0	);
				FIND_ITEM(PageRight	,	0	,	2	);

				FIND_ITEM(Up		,	1	,	0	);
				FIND_ITEM(Down		,	1	,	4	);
				FIND_ITEM(Left		,	1	,	0	);
				FIND_ITEM(Right		,	1	,	2	);
				FIND_ITEM(Home		,	1	,	0	);
				FIND_ITEM(End		,	1	,	29	);
				FIND_ITEM(PageUp	,	1	,	0	);
				FIND_ITEM(PageDown	,	1	,	10	);
				FIND_ITEM(PageLeft	,	1	,	0	);
				FIND_ITEM(PageRight	,	1	,	2	);

				FIND_ITEM(Up		,	2	,	0	);
				FIND_ITEM(Down		,	2	,	5	);
				FIND_ITEM(Left		,	2	,	1	);
				FIND_ITEM(Right		,	2	,	3	);
				FIND_ITEM(Home		,	2	,	0	);
				FIND_ITEM(End		,	2	,	29	);
				FIND_ITEM(PageUp	,	2	,	0	);
				FIND_ITEM(PageDown	,	2	,	11	);
				FIND_ITEM(PageLeft	,	2	,	0	);
				FIND_ITEM(PageRight	,	2	,	2	);

				FIND_ITEM(Up		,	3	,	0	);
				FIND_ITEM(Down		,	3	,	6	);
				FIND_ITEM(Left		,	3	,	2	);
				FIND_ITEM(Right		,	3	,	4	);
				FIND_ITEM(Home		,	3	,	0	);
				FIND_ITEM(End		,	3	,	29	);
				FIND_ITEM(PageUp	,	3	,	0	);
				FIND_ITEM(PageDown	,	3	,	12	);
				FIND_ITEM(PageLeft	,	3	,	3	);
				FIND_ITEM(PageRight	,	3	,	5	);

				FIND_ITEM(Up		,	4	,	1	);
				FIND_ITEM(Down		,	4	,	7	);
				FIND_ITEM(Left		,	4	,	3	);
				FIND_ITEM(Right		,	4	,	5	);
				FIND_ITEM(Home		,	4	,	0	);
				FIND_ITEM(End		,	4	,	29	);
				FIND_ITEM(PageUp	,	4	,	0	);
				FIND_ITEM(PageDown	,	4	,	13	);
				FIND_ITEM(PageLeft	,	4	,	3	);
				FIND_ITEM(PageRight	,	4	,	5	);

				FIND_ITEM(Up		,	5	,	2	);
				FIND_ITEM(Down		,	5	,	8	);
				FIND_ITEM(Left		,	5	,	4	);
				FIND_ITEM(Right		,	5	,	6	);
				FIND_ITEM(Home		,	5	,	0	);
				FIND_ITEM(End		,	5	,	29	);
				FIND_ITEM(PageUp	,	5	,	0	);
				FIND_ITEM(PageDown	,	5	,	14	);
				FIND_ITEM(PageLeft	,	5	,	3	);
				FIND_ITEM(PageRight	,	5	,	5	);

				FIND_ITEM(Up		,	27	,	24	);
				FIND_ITEM(Down		,	27	,	29	);
				FIND_ITEM(Left		,	27	,	26	);
				FIND_ITEM(Right		,	27	,	28	);
				FIND_ITEM(Home		,	27	,	0	);
				FIND_ITEM(End		,	27	,	29	);
				FIND_ITEM(PageUp	,	27	,	18	);
				FIND_ITEM(PageDown	,	27	,	29	);
				FIND_ITEM(PageLeft	,	27	,	27	);
				FIND_ITEM(PageRight	,	27	,	29	);

				FIND_ITEM(Up		,	28	,	25	);
				FIND_ITEM(Down		,	28	,	29	);
				FIND_ITEM(Left		,	28	,	27	);
				FIND_ITEM(Right		,	28	,	29	);
				FIND_ITEM(Home		,	28	,	0	);
				FIND_ITEM(End		,	28	,	29	);
				FIND_ITEM(PageUp	,	28	,	19	);
				FIND_ITEM(PageDown	,	28	,	29	);
				FIND_ITEM(PageLeft	,	28	,	27	);
				FIND_ITEM(PageRight	,	28	,	29	);

				FIND_ITEM(Up		,	29	,	26	);
				FIND_ITEM(Down		,	29	,	29	);
				FIND_ITEM(Left		,	29	,	28	);
				FIND_ITEM(Right		,	29	,	29	);
				FIND_ITEM(Home		,	29	,	0	);
				FIND_ITEM(End		,	29	,	29	);
				FIND_ITEM(PageUp	,	29	,	20	);
				FIND_ITEM(PageDown	,	29	,	29	);
				FIND_ITEM(PageLeft	,	29	,	27	);
				FIND_ITEM(PageRight	,	29	,	29	);
			});

			TEST_CASE(L"GetAdoptedSize")
			{
				{
					auto size = root->GetAdoptedSize({ 100,100 });
					TEST_ASSERT(size == Size(90, 90));
				}
				{
					auto size = root->GetAdoptedSize({ 100,300 });
					TEST_ASSERT(size == Size(90, 300));
				}
				{
					auto size = root->GetAdoptedSize({ 100,500 });
					TEST_ASSERT(size == Size(90, 300));
				}
			});

			TEST_CASE(L"GetTotalSize")
			{
				// 3x10
				TEST_ASSERT(root->GetTotalSize() == Size(90, 300));

				// 5x6
				root->SetPreferredMinSize({ 150,200 });
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(150, 180));

				// 5x9
				for (vint i = 1; i <= 15; i++) xs.Add(i);
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(150, 270));

				// 3x15
				root->SetPreferredMinSize({ 100,100 });
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(90, 450));

				// 3x10
				xs.RemoveRange(30, 15);
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(90, 300));
			});

			auto testHorizontal = [&](bool useMinimumTotalSize, vint x0, vint y0, vint y1, vint y2, vint vx0, vint vy0, vint vy1, vint vy2, vint w, vint h)
			{
				vint tw = useMinimumTotalSize ? 30 : 90;
				vint th = 300;

				checkItems(0, 12, x0, y0, w, h, true);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, vy0));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				root->SetViewLocation({ vx0 + 10,vy1 });
				checkItems(9, 12, x0 - 10, y1, w, h, true);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0 + 10, vy1));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				root->SetViewLocation({ vx0 + 20,vy2 });
				checkItems(18, 12, x0 - 20, y2, w, h, true);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0 + 20, vy2));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
				TEST_ASSERT(root->EnsureItemVisible(30) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0 + 20, vy0));
				checkItems(0, 12, x0 - 20, y0, w, h, true);
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0 + 20, vy2));
				checkItems(18, 12, x0 - 20, y2, w, h, true);
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));
			};

			auto testVertical = [&](bool useMinimumTotalSize, vint y0, vint x0, vint x1, vint x2, vint vy0, vint vx0, vint vx1, vint vx2, vint w, vint h)
			{
				vint tw = 300;
				vint th = useMinimumTotalSize ? 30 : 90;

				checkItems(0, 12, x0, y0, w, h, false);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, vy0));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				root->SetViewLocation({ vx1,vy0 + 10 });
				checkItems(9, 12, x1, y0 - 10, w, h, false);
				TEST_ASSERT(root->GetViewLocation() == Point(vx1, vy0 + 10));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				root->SetViewLocation({ vx2,vy0 + 20 });
				checkItems(18, 12, x2, y0 - 20, w, h, false);
				TEST_ASSERT(root->GetViewLocation() == Point(vx2, vy0 + 20));
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
				TEST_ASSERT(root->EnsureItemVisible(30) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, vy0 + 20));
				checkItems(0, 12, x0, y0 - 20, w, h, false);
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));

				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx2, vy0 + 20));
				checkItems(18, 12, x2, y0 - 20, w, h, false);
				TEST_ASSERT(root->GetTotalSize() == Size(tw, th));
			};

			auto testAllDirections = [&](bool useMinimumTotalSize)
			{
				TEST_CASE(L"RightDown")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
					testHorizontal(
						useMinimumTotalSize,
						0,
						0, -100, -200,
						0,
						0, 100, 200,
						30, 30);
				});

				TEST_CASE(L"LeftDown")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftDown)));
					testHorizontal(
						useMinimumTotalSize,
						100,
						0, -100, -200,
						-10,
						0, 100, 200,
						-30, 30);
				});

				TEST_CASE(L"RightUp")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightUp)));
					testHorizontal(
						useMinimumTotalSize,
						0,
						100, 200, 300,
						0,
						200, 100, 0,
						30, -30);
				});

				TEST_CASE(L"LeftUp")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
					testHorizontal(
						useMinimumTotalSize,
						100,
						100, 200, 300,
						-10,
						200, 100, 0,
						-30, -30);
				});

				TEST_CASE(L"DownRight")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownRight)));
					testVertical(
						useMinimumTotalSize,
						0,
						0, -100, -200,
						0,
						0, 100, 200,
						30, 30);
				});

				TEST_CASE(L"UpRight")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
					testVertical(
						useMinimumTotalSize,
						100,
						0, -100, -200,
						-10,
						0, 100, 200,
						30, -30);
				});

				TEST_CASE(L"DownLeft")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
					testVertical(
						useMinimumTotalSize,
						0,
						100, 200, 300,
						0,
						200, 100, 0,
						-30, 30);
				});

				TEST_CASE(L"UpLeft")
				{
					root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpLeft)));
					testVertical(
						useMinimumTotalSize,
						100,
						100, 200, 300,
						-10,
						200, 100, 0,
						-30, -30);
				});
			};

			TEST_CATEGORY(L"UseMinimumTotalSize == false")
			{
				TEST_CASE_ASSERT(root->GetUseMinimumTotalSize() == false);
				testAllDirections(false);
			});

			TEST_CATEGORY(L"UseMinimumTotalSize == true")
			{
				root->SetUseMinimumTotalSize(true);
				TEST_CASE_ASSERT(root->GetUseMinimumTotalSize() == true);
				testAllDirections(true);
			});

			SafeDeleteComposition(root);
			root = nullptr;
		}
	});

/***********************************************************************
<RepeatFixedHeightMultiColumnItem>
***********************************************************************/

	TEST_CATEGORY(L"Test <RepeatFixedHeightMultiColumnItem> layout in different direction, with GetTotalSize and EnsureItemVisible")
	{
		ObservableList<vint> xs;
		GuiRepeatFixedHeightMultiColumnItemComposition* root = nullptr;

		TEST_CASE(L"Item of different PreferredMinSize")
		{
			root = new GuiRepeatFixedHeightMultiColumnItemComposition;
			root->SetPreferredMinSize({ 100,100 });
			root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));
			root->SetItemTemplate([](const Value& value)
			{
				vint x = UnboxValue<vint>(value);
				auto style = new GuiTemplate;
				style->SetText(itow(UnboxValue<vint>(value)));
				style->SetPreferredMinSize({ 30 + x,10 + x });
				return style;
			});

			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->Children().Count() == 0);
			TEST_ASSERT(root->GetTotalSize() == Size(0, 0));

			auto checkItems = [&](vint first, vint count, vint h)
			{
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				vint itemsInColumn = root->GetCachedBounds().Height() / h;
				TEST_ASSERT(first % itemsInColumn == 0);
				TEST_ASSERT(root->Children().Count() == count);

				vint cx = 0;
				vint cf = first;
				while (cf - first < count)
				{
					vint cw = 0;
					vint stepCount = itemsInColumn;
					if (cf + stepCount > first + count) stepCount = first + count - cf;

					for (vint i = 0; i < stepCount; i++)
					{
						auto style = root->GetVisibleStyle(cf + i);
						TEST_ASSERT(root->GetVisibleIndex(style) == cf + i);
						TEST_ASSERT(style->GetText() == itow(xs[cf + i]));
						TEST_ASSERT(style->GetContext() == root->GetContext());

						vint w = 30 + cf + i + 1;
						if (cw < w) cw = w;
						auto actualBounds = style->GetCachedBounds();
						auto expectedBounds = Rect({ cx,i * h }, { w,h });
						TEST_ASSERT(actualBounds == expectedBounds);
					}

					cx += cw;
					cf += stepCount;
				}
			};

			for (vint i = 1; i <= 40; i++) xs.Add(i);
			checkItems(0, 12, 22);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(1100, 88));

			root->EnsureItemVisible(7);
			checkItems(0, 12, 22);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(1100, 88));

			root->EnsureItemVisible(8);
			checkItems(3, 9, 26);
			TEST_ASSERT(root->GetViewLocation() == Point(100, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(1500, 78));

			root->EnsureItemVisible(9);
			checkItems(6, 9, 26);
			TEST_ASSERT(root->GetViewLocation() == Point(200, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(1500, 78));

			root->EnsureItemVisible(39);
			checkItems(38, 2, 50);
			TEST_ASSERT(root->GetViewLocation() == Point(1900, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(2100, 100));

			root->EnsureItemVisible(0);
			checkItems(0, 6, 50);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(2100, 100));

			xs.Clear();
			SafeDeleteComposition(root);
			root = nullptr;
		});

		{
			for (vint i = 1; i <= 50; i++) xs.Add(i);

			root = new GuiRepeatFixedHeightMultiColumnItemComposition;
			root->SetPreferredMinSize({ 100,100 });
			root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));

			root->SetItemTemplate([](const Value& value)
			{
				vint x = UnboxValue<vint>(value);
				auto style = new GuiTemplate;
				style->SetText(itow(UnboxValue<vint>(value)));
				style->SetPreferredMinSize({ 40,10 });
				return style;
			});

			// trigger layout so its page size becomes 2x10
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();

			TEST_CASE(L"FindItemByVirtualKeyDirection")
			{
				FIND_ITEM(Up		,	0	,	0	);
				FIND_ITEM(Down		,	0	,	1	);
				FIND_ITEM(Left		,	0	,	0	);
				FIND_ITEM(Right		,	0	,	10	);
				FIND_ITEM(Home		,	0	,	0	);
				FIND_ITEM(End		,	0	,	49	);
				FIND_ITEM(PageUp	,	0	,	0	);
				FIND_ITEM(PageDown	,	0	,	9	);
				FIND_ITEM(PageLeft	,	0	,	-1	);
				FIND_ITEM(PageRight	,	0	,	-1	);

				FIND_ITEM(Up		,	1	,	0	);
				FIND_ITEM(Down		,	1	,	2	);
				FIND_ITEM(Left		,	1	,	0	);
				FIND_ITEM(Right		,	1	,	11	);
				FIND_ITEM(Home		,	1	,	0	);
				FIND_ITEM(End		,	1	,	49	);
				FIND_ITEM(PageUp	,	1	,	0	);
				FIND_ITEM(PageDown	,	1	,	9	);
				FIND_ITEM(PageLeft	,	1	,	-1	);
				FIND_ITEM(PageRight	,	1	,	-1	);

				FIND_ITEM(Up		,	9	,	8	);
				FIND_ITEM(Down		,	9	,	10	);
				FIND_ITEM(Left		,	9	,	0	);
				FIND_ITEM(Right		,	9	,	19	);
				FIND_ITEM(Home		,	9	,	0	);
				FIND_ITEM(End		,	9	,	49	);
				FIND_ITEM(PageUp	,	9	,	0	);
				FIND_ITEM(PageDown	,	9	,	9	);
				FIND_ITEM(PageLeft	,	9	,	-1	);
				FIND_ITEM(PageRight	,	9	,	-1	);

				FIND_ITEM(Up		,	10	,	9	);
				FIND_ITEM(Down		,	10	,	11	);
				FIND_ITEM(Left		,	10	,	0	);
				FIND_ITEM(Right		,	10	,	20	);
				FIND_ITEM(Home		,	10	,	0	);
				FIND_ITEM(End		,	10	,	49	);
				FIND_ITEM(PageUp	,	10	,	10	);
				FIND_ITEM(PageDown	,	10	,	19	);
				FIND_ITEM(PageLeft	,	10	,	-1	);
				FIND_ITEM(PageRight	,	10	,	-1	);

				FIND_ITEM(Up		,	11	,	10	);
				FIND_ITEM(Down		,	11	,	12	);
				FIND_ITEM(Left		,	11	,	1	);
				FIND_ITEM(Right		,	11	,	21	);
				FIND_ITEM(Home		,	11	,	0	);
				FIND_ITEM(End		,	11	,	49	);
				FIND_ITEM(PageUp	,	11	,	10	);
				FIND_ITEM(PageDown	,	11	,	19	);
				FIND_ITEM(PageLeft	,	11	,	-1	);
				FIND_ITEM(PageRight	,	11	,	-1	);

				FIND_ITEM(Up		,	19	,	18	);
				FIND_ITEM(Down		,	19	,	20	);
				FIND_ITEM(Left		,	19	,	9	);
				FIND_ITEM(Right		,	19	,	29	);
				FIND_ITEM(Home		,	19	,	0	);
				FIND_ITEM(End		,	19	,	49	);
				FIND_ITEM(PageUp	,	19	,	10	);
				FIND_ITEM(PageDown	,	19	,	19	);
				FIND_ITEM(PageLeft	,	19	,	-1	);
				FIND_ITEM(PageRight	,	19	,	-1	);

				FIND_ITEM(Up		,	40	,	39	);
				FIND_ITEM(Down		,	40	,	41	);
				FIND_ITEM(Left		,	40	,	30	);
				FIND_ITEM(Right		,	40	,	49	);
				FIND_ITEM(Home		,	40	,	0	);
				FIND_ITEM(End		,	40	,	49	);
				FIND_ITEM(PageUp	,	40	,	40	);
				FIND_ITEM(PageDown	,	40	,	49	);
				FIND_ITEM(PageLeft	,	40	,	-1	);
				FIND_ITEM(PageRight	,	40	,	-1	);

				FIND_ITEM(Up		,	41	,	40	);
				FIND_ITEM(Down		,	41	,	42	);
				FIND_ITEM(Left		,	41	,	31	);
				FIND_ITEM(Right		,	41	,	49	);
				FIND_ITEM(Home		,	41	,	0	);
				FIND_ITEM(End		,	41	,	49	);
				FIND_ITEM(PageUp	,	41	,	40	);
				FIND_ITEM(PageDown	,	41	,	49	);
				FIND_ITEM(PageLeft	,	41	,	-1	);
				FIND_ITEM(PageRight	,	41	,	-1	);

				FIND_ITEM(Up		,	49	,	48	);
				FIND_ITEM(Down		,	49	,	49	);
				FIND_ITEM(Left		,	49	,	39	);
				FIND_ITEM(Right		,	49	,	49	);
				FIND_ITEM(Home		,	49	,	0	);
				FIND_ITEM(End		,	49	,	49	);
				FIND_ITEM(PageUp	,	49	,	40	);
				FIND_ITEM(PageDown	,	49	,	49	);
				FIND_ITEM(PageLeft	,	49	,	-1	);
				FIND_ITEM(PageRight	,	49	,	-1	);
			});

			TEST_CASE(L"GetAdoptedSize")
			{
				{
					auto size = root->GetAdoptedSize({ 100,100 });
					TEST_ASSERT(size == Size(100, 100));
				}
				{
					auto size = root->GetAdoptedSize({ 100,300 });
					TEST_ASSERT(size == Size(100, 100));
				}
				{
					auto size = root->GetAdoptedSize({ 100,500 });
					TEST_ASSERT(size == Size(100, 100));
				}
			});

			auto testTotalSize = [&](bool useMinimumTotalSize)
			{
				// 5x10
				TEST_ASSERT(root->GetTotalSize() == Size(600, (useMinimumTotalSize ? 0 : 100)));

				// 3x20
				root->SetPreferredMinSize({ 150,200 });
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(600, (useMinimumTotalSize ? 0 : 200)));

				// 4x20
				for (vint i = 1; i <= 15; i++) xs.Add(i);
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(750, (useMinimumTotalSize ? 0 : 200)));

				// 7x10
				root->SetPreferredMinSize({ 100,100 });
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(800, (useMinimumTotalSize ? 0 : 100)));

				// 5x10
				xs.RemoveRange(50, 15);
				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->GetTotalSize() == Size(600, (useMinimumTotalSize ? 0 : 100)));
			};

			TEST_CASE(L"UseMinimumTotalSize == false")
			{
				TEST_ASSERT(root->GetUseMinimumTotalSize() == false);
				testTotalSize(false);
			});

			TEST_CASE(L"UseMinimumTotalSize == true")
			{
				root->SetUseMinimumTotalSize(true);
				TEST_ASSERT(root->GetUseMinimumTotalSize() == true);
				testTotalSize(true);
			});

			root->SetUseMinimumTotalSize(false);

			auto checkItems = [&](vint firstColumn, vint visibleColumns, vint x0, vint y0, vint w, vint h, bool horizontal)
			{
				vint cw = w > 0 ? w : -w;
				vint ch = h > 0 ? h : -h;
				if (w < 0) x0 += w;
				if (h < 0) y0 += h;

				root->ForceCalculateSizeImmediately();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(root->Children().Count() == visibleColumns * 10);
				TEST_ASSERT(root->GetTotalSize() == (horizontal ? Size(600, 100) : Size(100, 600)));

				for (vint c = 0; c < visibleColumns; c++)
				{
					for (vint r = 0; r < 10; r++)
					{
						vint i = (firstColumn + c) * 10 + r;
						auto style = root->GetVisibleStyle(i);
						TEST_ASSERT(root->GetVisibleIndex(style) == i);
						TEST_ASSERT(style->GetText() == itow(xs[i]));
						TEST_ASSERT(style->GetContext() == root->GetContext());

						auto actualBounds = style->GetCachedBounds();
						auto expectedBounds = horizontal
							? Rect({ x0 + c * w,y0 + r * h }, { cw,ch })
							: Rect({ x0 + r * w,y0 + c * h }, { cw,ch });
						TEST_ASSERT(actualBounds == expectedBounds);
					}
				}
			};

			auto testHorizontal = [&](vint vx0, vint vx1, vint vx2, vint w, vint h)
			{
				vint x0 = w > 0 ? 0 : 100;
				vint y0 = h > 0 ? 0 : 100;
				vint tw = w > 0 ? 500 : -500;

				checkItems(0, 3, x0, y0, w, h, true);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, 0));

				root->SetViewLocation({ (vx0 + vx1) / 2,10 });
				checkItems(0, 3, x0, y0 - 10, w, h, true);
				TEST_ASSERT(root->GetViewLocation() == Point((vx0 + vx1) / 2, 10));

				root->SetViewLocation({ vx1,20 });
				checkItems(1, 3, x0, y0 - 20, w, h, true);
				TEST_ASSERT(root->GetViewLocation() == Point(vx1, 20));

				root->SetViewLocation({ vx0 - tw,20 });
				checkItems(0, 3, x0, y0 - 20, w, h, true);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0 - tw, 20));

				root->SetViewLocation({ vx0 + tw,20 });
				checkItems(4, 1, x0, y0 - 20, w, h, true);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0 + tw, 20));

				TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
				TEST_ASSERT(root->EnsureItemVisible(50) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, 20));
				checkItems(0, 3, x0, y0 - 20, w, h, true);

				TEST_ASSERT(root->EnsureItemVisible(15) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, 20));
				checkItems(0, 3, x0, y0 - 20, w, h, true);

				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx1, 20));
				checkItems(1, 3, x0, y0 - 20, w, h, true);

				TEST_ASSERT(root->EnsureItemVisible(49) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(49) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx2, 20));
				checkItems(3, 2, x0, y0 - 20, w, h, true);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, 20));
				checkItems(0, 3, x0, y0 - 20, w, h, true);
			};

			auto testVertical = [&](vint vy0, vint vy1, vint vy2, vint w, vint h)
			{
				vint x0 = w > 0 ? 0 : 100;
				vint y0 = h > 0 ? 0 : 100;
				vint th = h > 0 ? 500 : -500;

				checkItems(0, 3, x0, y0, w, h, false);
				TEST_ASSERT(root->GetViewLocation() == Point(0, vy0));

				root->SetViewLocation({ 10,(vy0 + vy1) / 2 });
				checkItems(0, 3, x0 - 10, y0, w, h, false);
				TEST_ASSERT(root->GetViewLocation() == Point(10, (vy0 + vy1) / 2));

				root->SetViewLocation({ 20,vy1 });
				checkItems(1, 3, x0 - 20, y0, w, h, false);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy1));

				root->SetViewLocation({ 20,vy0 - th });
				checkItems(0, 3, x0 - 20, y0, w, h, false);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy0 - th));

				root->SetViewLocation({ 20,vy0 + th });
				checkItems(4, 1, x0 - 20, y0, w, h, false);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy0 + th));

				TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
				TEST_ASSERT(root->EnsureItemVisible(50) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy0));
				checkItems(0, 3, x0 - 20, y0, w, h, false);

				TEST_ASSERT(root->EnsureItemVisible(15) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy0));
				checkItems(0, 3, x0 - 20, y0, w, h, false);

				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy1));
				checkItems(1, 3, x0 - 20, y0, w, h, false);

				TEST_ASSERT(root->EnsureItemVisible(49) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(49) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy2));
				checkItems(3, 2, x0 - 20, y0, w, h, false);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy0));
				checkItems(0, 3, x0 - 20, y0, w, h, false);
			};

			TEST_CASE(L"RightDown")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
				testHorizontal(0, 100, 300, 40, 10);
			});

			TEST_CASE(L"LeftDown")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftDown)));
				testHorizontal(500, 400, 200, -40, 10);
			});

			TEST_CASE(L"RightUp")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightUp)));
				testHorizontal(0, 100, 300, 40, -10);
			});

			TEST_CASE(L"LeftUp")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
				testHorizontal(500, 400, 200, -40, -10);
			});

			root->SetItemTemplate([](const Value& value)
			{
				vint x = UnboxValue<vint>(value);
				auto style = new GuiTemplate;
				style->SetText(itow(UnboxValue<vint>(value)));
				style->SetPreferredMinSize({ 10,40 });
				return style;
			});

			TEST_CASE(L"DownRight")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownRight)));
				testVertical(0, 100, 300, 10, 40);
			});

			TEST_CASE(L"UpRight")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
				testVertical(500, 400, 200, 10, -40);
			});

			TEST_CASE(L"DownLeft")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
				testVertical(0, 100, 300, -10, 40);
			});

			TEST_CASE(L"UpLeft")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpLeft)));
				testVertical(500, 400, 200, -10, -40);
			});

			SafeDeleteComposition(root);
			root = nullptr;
		}
	});
}