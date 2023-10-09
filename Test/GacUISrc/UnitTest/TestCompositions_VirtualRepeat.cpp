#include "TestCompositions.h"

using namespace vl::collections;
using namespace vl::presentation::templates;
using namespace composition_bounds_tests;

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

		auto testDown = [&]()
		{
			checkItemsDown(0, 12, 0, 0);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 114 + 8));

			root->SetViewLocation({ 10,100 });
			checkItemsDown(11, 6, -10, -100);
			TEST_ASSERT(root->GetViewLocation() == Point(10, 100));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 204 + 3));

			root->SetViewLocation({ 20,200 });
			checkItemsDown(16, 4, -20, -200);
			TEST_ASSERT(root->GetViewLocation() == Point(20, 200));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 270));

			TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
			TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(20, 0));
			checkItemsDown(0, 12, -20, 0);
			TEST_ASSERT(root->GetTotalSize() == Size(100, 270));

			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(20, 170));
			checkItemsDown(15, 5, -20, -170);
			TEST_ASSERT(root->GetTotalSize() == Size(100, 270));
		};

		TEST_CASE(L"RightDown")
		{
			root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
			testDown();
		});

		TEST_CASE(L"LeftDown")
		{
			root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftDown)));
			testDown();
		});

		auto testUp = [&]()
		{
			checkItemsUp(0, 12, 0, 100);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 22));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 114 + 8));

			root->SetViewLocation({ 10,-78 });
			checkItemsUp(11, 6, -10, 200);
			TEST_ASSERT(root->GetViewLocation() == Point(10, 7));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 204 + 3));

			root->SetViewLocation({ 20,-93 });
			checkItemsUp(16, 4, -20, 300);
			TEST_ASSERT(root->GetViewLocation() == Point(20, -30));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 270));

			TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
			TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(20, 170));
			checkItemsUp(0, 12, -20, 100);
			TEST_ASSERT(root->GetTotalSize() == Size(100, 270));

			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(20, 0));
			checkItemsUp(15, 5, -20, 270);
			TEST_ASSERT(root->GetTotalSize() == Size(100, 270));
		};

		TEST_CASE(L"RightUp")
		{
			root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightUp)));
			testUp();
		});

		TEST_CASE(L"LeftUp")
		{
			root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
			testUp();
		});

		auto testRight = [&]()
		{
			checkItemsRight(0, 12, 0, 0);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(102 + 8, 100));

			root->SetViewLocation({ 100,10 });
			checkItemsRight(11, 7, -100, -10);
			TEST_ASSERT(root->GetViewLocation() == Point(100, 10));
			TEST_ASSERT(root->GetTotalSize() == Size(207 + 2, 100));

			root->SetViewLocation({ 200,20 });
			checkItemsRight(17, 3, -200, -20);
			TEST_ASSERT(root->GetViewLocation() == Point(200, 20));
			TEST_ASSERT(root->GetTotalSize() == Size(250, 100));

			TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
			TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 20));
			checkItemsRight(0, 12, 0, -20);
			TEST_ASSERT(root->GetTotalSize() == Size(250, 100));

			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(150, 20));
			checkItemsRight(15, 5, -150, -20);
			TEST_ASSERT(root->GetTotalSize() == Size(250, 100));
		};

		TEST_CASE(L"DownRight")
		{
			root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownRight)));
			testRight();
		});

		TEST_CASE(L"UpRight")
		{
			root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
			testRight();
		});

		auto testLeft = [&]()
		{
			checkItemsLeft(0, 12, 100, 0);
			TEST_ASSERT(root->GetViewLocation() == Point(10, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(102 + 8, 100));

			root->SetViewLocation({ -90,10 });
			checkItemsLeft(11, 7, 200, -10);
			TEST_ASSERT(root->GetViewLocation() == Point(9, 10));
			TEST_ASSERT(root->GetTotalSize() == Size(207 + 2, 100));

			root->SetViewLocation({ -91,20 });
			checkItemsLeft(17, 3, 300, -20);
			TEST_ASSERT(root->GetViewLocation() == Point(-50, 20));
			TEST_ASSERT(root->GetTotalSize() == Size(250, 100));

			TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
			TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(150, 20));
			checkItemsLeft(0, 12, 100, -20);
			TEST_ASSERT(root->GetTotalSize() == Size(250, 100));

			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
			TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 20));
			checkItemsLeft(15, 5, 250, -20);
			TEST_ASSERT(root->GetTotalSize() == Size(250, 100));
		};

		TEST_CASE(L"DownLeft")
		{
			root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
			testLeft();
		});

		TEST_CASE(L"UpLeft")
		{
			root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpLeft)));
			testLeft();
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

		auto checkItems = [&](vint first, vint count, vint x0, vint y0, vint w, vint h)
		{
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->Children().Count() == count);

			if (w < 0) x0 += w;
			if (h < 0) y0 += h;

			for (vint i = 0; i < count; i++)
			{
				auto style = root->GetVisibleStyle(first + i);
				TEST_ASSERT(root->GetVisibleIndex(style) == first + i);
				TEST_ASSERT(style->GetText() == itow(xs[first + i]));
				TEST_ASSERT(style->GetContext() == root->GetContext());

				auto actualBounds = style->GetCachedBounds();
				auto expectedBounds = Rect({
					x0 + (i + first) * w,
					y0 + (i + first) * h
				}, {
					w == 0 ? root->GetCachedBounds().Width() : (w > 0 ? w : -w),
					h == 0 ? root->GetCachedBounds().Height() : (h > 0 ? h : -h)
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
			checkItems(0, 7, 0, 0, 0, 17);
			TEST_ASSERT(root->GetViewLocation() == Point(0, 0));
			TEST_ASSERT(root->GetTotalSize() == Size(100, 340));

			root->EnsureItemVisible(4);
			checkItems(0, 7, 0, 0, 0, 17);
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

			auto testHorizontal = [&](vint y0, vint y1, vint y2, vint vy0, vint vy1, vint vy2, vint h)
			{
				checkItems(0, 7, 0, y0, 0, h);
				TEST_ASSERT(root->GetViewLocation() == Point(0, vy0));
				TEST_ASSERT(root->GetTotalSize() == Size(100, 300));

				root->SetViewLocation({ 10,vy1 });
				checkItems(6, 8, 0, y1, 0, h);
				TEST_ASSERT(root->GetViewLocation() == Point(10, vy1));
				TEST_ASSERT(root->GetTotalSize() == Size(100, 300));

				root->SetViewLocation({ 20,vy2 });
				checkItems(13, 7, 0, y2, 0, h);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy2));
				TEST_ASSERT(root->GetTotalSize() == Size(100, 300));

				TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
				TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy0));
				checkItems(0, 7, 0, y0, 0, h);
				TEST_ASSERT(root->GetTotalSize() == Size(100, 300));

				TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(20, vy2));
				checkItems(13, 7, 0, y2, 0, h);
				TEST_ASSERT(root->GetTotalSize() == Size(100, 300));
			};

			auto testVertical = [&](vint x0, vint x1, vint x2, vint vx0, vint vx1, vint vx2, vint w)
			{
				checkItems(0, 7, x0, 0, w, 0);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, 0));
				TEST_ASSERT(root->GetTotalSize() == Size(300, 100));

				root->SetViewLocation({ vx1,10 });
				checkItems(6, 8, x1, 0, w, 0);
				TEST_ASSERT(root->GetViewLocation() == Point(vx1, 10));
				TEST_ASSERT(root->GetTotalSize() == Size(300, 100));

				root->SetViewLocation({ vx2,20 });
				checkItems(13, 7, x2, 0, w, 0);
				TEST_ASSERT(root->GetViewLocation() == Point(vx2, 20));
				TEST_ASSERT(root->GetTotalSize() == Size(300, 100));

				TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
				TEST_ASSERT(root->EnsureItemVisible(20) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, 20));
				checkItems(0, 7, x0, 0, w, 0);
				TEST_ASSERT(root->GetTotalSize() == Size(300, 100));

				TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(19) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx2, 20));
				checkItems(13, 7, x2, 0, w, 0);
				TEST_ASSERT(root->GetTotalSize() == Size(300, 100));
			};

			auto testDown = [&]()
			{
				testHorizontal(
					0, -100, -200,
					0, 100, 200,
					15);
			};

			TEST_CASE(L"RightDown")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
				testDown();
			});

			TEST_CASE(L"LeftDown")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftDown)));
				testDown();
			});

			auto testUp = [&]()
			{
				testHorizontal(
					100, 200, 300,
					200, 100, 0,
					-15);
			};

			TEST_CASE(L"RightUp")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightUp)));
				testUp();
			});

			TEST_CASE(L"LeftUp")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
				testUp();
			});

			auto testRight = [&]()
			{
				testVertical(
					0, -100, -200,
					0, 100, 200,
					15);
			};

			TEST_CASE(L"DownRight")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownRight)));
				testRight();
			});

			TEST_CASE(L"UpRight")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
				testRight();
			});

			auto testLeft = [&]()
			{
				testVertical(
					100, 200, 300,
					200, 100, 0,
					-15);
			};

			TEST_CASE(L"DownLeft")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
				testLeft();
			});

			TEST_CASE(L"UpLeft")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpLeft)));
				testLeft();
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

			auto testHorizontal = [&](vint x0, vint y0, vint y1, vint y2, vint vx0, vint vy0, vint vy1, vint vy2, vint w, vint h)
			{
				checkItems(0, 12, x0, y0, w, h, true);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, vy0));
				TEST_ASSERT(root->GetTotalSize() == Size(90, 300));

				root->SetViewLocation({ vx0 + 10,vy1 });
				checkItems(9, 12, x0 - 10, y1, w, h, true);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0 + 10, vy1));
				TEST_ASSERT(root->GetTotalSize() == Size(90, 300));

				root->SetViewLocation({ vx0 + 20,vy2 });
				checkItems(18, 12, x0 - 20, y2, w, h, true);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0 + 20, vy2));
				TEST_ASSERT(root->GetTotalSize() == Size(90, 300));

				TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
				TEST_ASSERT(root->EnsureItemVisible(30) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0 + 20, vy0));
				checkItems(0, 12, x0 - 20, y0, w, h, true);
				TEST_ASSERT(root->GetTotalSize() == Size(90, 300));

				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0 + 20, vy2));
				checkItems(18, 12, x0 - 20, y2, w, h, true);
				TEST_ASSERT(root->GetTotalSize() == Size(90, 300));
			};

			auto testVertical = [&](vint y0, vint x0, vint x1, vint x2, vint vy0, vint vx0, vint vx1, vint vx2, vint w, vint h)
			{
				checkItems(0, 12, x0, y0, w, h, false);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, vy0));
				TEST_ASSERT(root->GetTotalSize() == Size(300, 90));

				root->SetViewLocation({ vx1,vy0 + 10 });
				checkItems(9, 12, x1, y0 - 10, w, h, false);
				TEST_ASSERT(root->GetViewLocation() == Point(vx1, vy0 + 10));
				TEST_ASSERT(root->GetTotalSize() == Size(300, 90));

				root->SetViewLocation({ vx2,vy0 + 20 });
				checkItems(18, 12, x2, y0 - 20, w, h, false);
				TEST_ASSERT(root->GetViewLocation() == Point(vx2, vy0 + 20));
				TEST_ASSERT(root->GetTotalSize() == Size(300, 90));

				TEST_ASSERT(root->EnsureItemVisible(-1) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);
				TEST_ASSERT(root->EnsureItemVisible(30) == VirtualRepeatEnsureItemVisibleResult::ItemNotExists);

				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(0) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx0, vy0 + 20));
				checkItems(0, 12, x0, y0 - 20, w, h, false);
				TEST_ASSERT(root->GetTotalSize() == Size(300, 90));

				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::Moved);
				TEST_ASSERT(root->EnsureItemVisible(29) == VirtualRepeatEnsureItemVisibleResult::NotMoved);
				TEST_ASSERT(root->GetViewLocation() == Point(vx2, vy0 + 20));
				checkItems(18, 12, x2, y0 - 20, w, h, false);
				TEST_ASSERT(root->GetTotalSize() == Size(300, 90));
			};

			TEST_CASE(L"RightDown")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
				testHorizontal(
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
					100,
					100, 200, 300,
					-10,
					200, 100, 0,
					-30, -30);
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

		auto checkItemsCommon = [&]<typename TGetWidth>(vint first, vint count, vint h, TGetWidth&& getWidth)
		{
			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			vint itemsInColumn = root->GetCachedBounds().Height() / h;
			TEST_ASSERT(first % itemsInColumn == 0);
			TEST_ASSERT(root->Children().Count() == count);

			vint cx = 0;
			vint cf = first;
			while (true)
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

					vint w = getWidth(cf + i);
					if (cw < w) cw = w;
					auto actualBounds = style->GetCachedBounds();
					auto expectedBounds = Rect({ cx,i * h }, { w,h });
					TEST_ASSERT(actualBounds == expectedBounds);
				}

				cx += cw;
				cf += stepCount;
			}
		};

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
				style->SetPreferredMinSize({ 10 + x,30 + x});
				return style;
			});

			root->ForceCalculateSizeImmediately();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->Children().Count() == 0);
			TEST_ASSERT(root->GetTotalSize() == Size(0, 0));

			xs.Clear();
			SafeDeleteComposition(root);
			root = nullptr;
		});

		{
			root = new GuiRepeatFixedHeightMultiColumnItemComposition;
			root->SetPreferredMinSize({ 100,100 });
			root->SetItemSource(UnboxValue<Ptr<IValueObservableList>>(BoxParameter(xs)));

			TEST_CASE(L"RightDown")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightDown)));
			});

			TEST_CASE(L"LeftDown")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftDown)));
			});

			TEST_CASE(L"RightUp")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::RightUp)));
			});

			TEST_CASE(L"LeftUp")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::LeftUp)));
			});

			TEST_CASE(L"DownRight")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownRight)));
			});

			TEST_CASE(L"UpRight")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpRight)));
			});

			TEST_CASE(L"DownLeft")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::DownLeft)));
			});

			TEST_CASE(L"UpLeft")
			{
				root->SetAxis(Ptr(new GuiAxis(AxisDirection::UpLeft)));
			});

			SafeDeleteComposition(root);
			root = nullptr;
		}
	});
}