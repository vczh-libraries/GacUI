#include "TestCompositions.h"
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test <Table> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiTableComposition>(
			[](GuiTableComposition* table)
			{
				table->SetBorderVisible(false);
				table->SetRowOption(0, GuiCellOption::AbsoluteOption(0));
				table->SetColumnOption(0, GuiCellOption::AbsoluteOption(0));
			});
	});

	TEST_CASE(L"Test <Table> default properties")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->ForceCalculateSizeImmediately();

		TEST_ASSERT(table->GetRows() == 1);
		TEST_ASSERT(table->GetRowOption(0) == GuiCellOption::AbsoluteOption(20));
		TEST_ASSERT(table->GetColumns() == 1);
		TEST_ASSERT(table->GetColumnOption(0) == GuiCellOption::AbsoluteOption(20));
		TEST_ASSERT(table->GetCellPadding() == 0);
		TEST_ASSERT(table->GetBorderVisible() == true);

		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 20,20 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test empty <Table> with MinSize only")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::MinSizeOption());
		table->SetRowOption(1, GuiCellOption::MinSizeOption());
		table->SetRowOption(2, GuiCellOption::MinSizeOption());
		table->SetColumnOption(0, GuiCellOption::MinSizeOption());
		table->SetColumnOption(1, GuiCellOption::MinSizeOption());
		table->SetColumnOption(2, GuiCellOption::MinSizeOption());
		table->ForceCalculateSizeImmediately();

		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		table->SetCellPadding(10);
		table->ForceCalculateSizeImmediately();
		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 40,40 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		table->SetBorderVisible(false);
		table->ForceCalculateSizeImmediately();
		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 20,20 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test empty <Table> with Absolute only")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::AbsoluteOption(1));
		table->SetRowOption(1, GuiCellOption::AbsoluteOption(2));
		table->SetRowOption(2, GuiCellOption::AbsoluteOption(3));
		table->SetColumnOption(0, GuiCellOption::AbsoluteOption(4));
		table->SetColumnOption(1, GuiCellOption::AbsoluteOption(5));
		table->SetColumnOption(2, GuiCellOption::AbsoluteOption(6));
		table->ForceCalculateSizeImmediately();

		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 15,6 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		table->SetCellPadding(10);
		table->ForceCalculateSizeImmediately();
		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 55,46 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		table->SetBorderVisible(false);
		table->ForceCalculateSizeImmediately();
		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 35,26 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test empty <Table> with Percentage only")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
		table->SetRowOption(1, GuiCellOption::PercentageOption(1.0));
		table->SetRowOption(2, GuiCellOption::PercentageOption(1.0));
		table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
		table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
		table->SetColumnOption(2, GuiCellOption::PercentageOption(1.0));
		table->ForceCalculateSizeImmediately();

		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		table->SetCellPadding(10);
		table->ForceCalculateSizeImmediately();
		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 40,40 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		table->SetBorderVisible(false);
		table->ForceCalculateSizeImmediately();
		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 20,20 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test empty <Table> with mixed GuiCellOption")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::MinSizeOption());
		table->SetRowOption(1, GuiCellOption::AbsoluteOption(1));
		table->SetRowOption(2, GuiCellOption::PercentageOption(1.0));
		table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
		table->SetColumnOption(1, GuiCellOption::AbsoluteOption(2));
		table->SetColumnOption(2, GuiCellOption::MinSizeOption());
		table->ForceCalculateSizeImmediately();

		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 2,1 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		table->SetCellPadding(10);
		table->ForceCalculateSizeImmediately();
		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 42,41 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		table->SetBorderVisible(false);
		table->ForceCalculateSizeImmediately();
		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, { 22,21 }));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		SafeDeleteComposition(table);
	});

	auto testUnmergedCells = []<vint ROWS, vint COLUMNS>(
		GuiTableComposition* table,
		Size tableSize,
		vint(&xs)[COLUMNS],
		vint(&ys)[ROWS],
		vint(&ws)[COLUMNS],
		vint(&hs)[ROWS]
		)
	{
		table->ForceCalculateSizeImmediately();
		TEST_ASSERT(table->GetCachedClientArea() == Rect({ 0,0 }, tableSize));
		TEST_ASSERT(table->GetCachedMinSize() == table->GetCachedClientArea().GetSize());
		TEST_ASSERT(table->GetCachedBounds() == table->GetCachedClientArea());

		for (vint r = 0; r < ROWS; r++)
		{
			for (vint c = 0; c < COLUMNS; c++)
			{
				auto cell = table->GetSitedCell(r, c);
				TEST_ASSERT(cell->GetCachedBounds() == Rect({ xs[c],ys[r] }, { ws[c],hs[r] }));
			}
		}
	};

	TEST_CASE(L"Test <Table> with MinSize only")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetCellPadding(10);
		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::MinSizeOption());
		table->SetRowOption(1, GuiCellOption::MinSizeOption());
		table->SetRowOption(2, GuiCellOption::MinSizeOption());
		table->SetColumnOption(0, GuiCellOption::MinSizeOption());
		table->SetColumnOption(1, GuiCellOption::MinSizeOption());
		table->SetColumnOption(2, GuiCellOption::MinSizeOption());

		for (vint r = 0; r < 3; r++)
		{
			for (vint c = 0; c < 3; c++)
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(r, c, 1, 1);
				cell->SetPreferredMinSize(Size(10 + r * 3 + c, 10 + c * 4 + r));
				table->AddChild(cell);
			}
		}

		{
			vint xs[3] = { 10,36,63 };
			vint ys[3] = { 10,38,67 };
			vint ws[3] = { 16,17,18 };
			vint hs[3] = { 18,19,20 };
			testUnmergedCells(table, { 91,97 }, xs, ys, ws, hs);
		}

		table->SetBorderVisible(false);
		table->SetPreferredMinSize(Size(100, 200));
		{
			vint xs[3] = { 0,26,53 };
			vint ys[3] = { 0,28,57 };
			vint ws[3] = { 16,17,47 };
			vint hs[3] = { 18,19,143 };
			testUnmergedCells(table, { 100,200 }, xs, ys, ws, hs);
		}

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test <Table> with Absolute only")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetCellPadding(10);
		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::AbsoluteOption(20));
		table->SetRowOption(1, GuiCellOption::AbsoluteOption(19));
		table->SetRowOption(2, GuiCellOption::AbsoluteOption(18));
		table->SetColumnOption(0, GuiCellOption::AbsoluteOption(18));
		table->SetColumnOption(1, GuiCellOption::AbsoluteOption(17));
		table->SetColumnOption(2, GuiCellOption::AbsoluteOption(16));

		for (vint r = 0; r < 3; r++)
		{
			for (vint c = 0; c < 3; c++)
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(r, c, 1, 1);
				cell->SetPreferredMinSize(Size(10 + r * 3 + c, 10 + c * 4 + r));
				table->AddChild(cell);
			}
		}

		{
			vint xs[3] = { 10,38,65 };
			vint ys[3] = { 10,40,69 };
			vint ws[3] = { 18,17,16 };
			vint hs[3] = { 20,19,18 };
			testUnmergedCells(table, { 91,97 }, xs, ys, ws, hs);
		}

		table->SetBorderVisible(false);
		table->SetPreferredMinSize(Size(100, 200));
		{
			vint xs[3] = { 0,28,55 };
			vint ys[3] = { 0,30,59 };
			vint ws[3] = { 18,17,45 };
			vint hs[3] = { 20,19,141 };
			testUnmergedCells(table, { 100,200 }, xs, ys, ws, hs);
		}

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test <Table> with Percentage only")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetPreferredMinSize(Size(100, 200));
		table->SetCellPadding(10);
		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
		table->SetRowOption(1, GuiCellOption::PercentageOption(0.3));
		table->SetRowOption(2, GuiCellOption::PercentageOption(0.2));
		table->SetColumnOption(0, GuiCellOption::PercentageOption(0.2));
		table->SetColumnOption(1, GuiCellOption::PercentageOption(0.3));
		table->SetColumnOption(2, GuiCellOption::PercentageOption(0.5));

		for (vint r = 0; r < 3; r++)
		{
			for (vint c = 0; c < 3; c++)
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(r, c, 1, 1);
				table->AddChild(cell);
			}
		}

		{
			vint xs[3] = { 10,32,60 };
			vint ys[3] = { 10,100,158 };
			vint ws[3] = { 12,18,30 };
			vint hs[3] = { 80,48,32 };
			testUnmergedCells(table, { 100,200 }, xs, ys, ws, hs);
		}

		table->SetBorderVisible(false);
		{
			vint xs[3] = { 0,26,60 };
			vint ys[3] = { 0,100,164 };
			vint ws[3] = { 16,24,40 };
			vint hs[3] = { 90,54,36 };
			testUnmergedCells(table, { 100,200 }, xs, ys, ws, hs);
		}

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test <Table> with mixed GuiCellOption")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetPreferredMinSize(Size(100, 200));
		table->SetCellPadding(10);
		table->SetRowsAndColumns(4, 4);
		table->SetRowOption(0, GuiCellOption::MinSizeOption());
		table->SetRowOption(1, GuiCellOption::AbsoluteOption(20));
		table->SetRowOption(2, GuiCellOption::PercentageOption(0.2));
		table->SetRowOption(3, GuiCellOption::PercentageOption(0.8));
		table->SetColumnOption(0, GuiCellOption::MinSizeOption());
		table->SetColumnOption(1, GuiCellOption::AbsoluteOption(30));
		table->SetColumnOption(2, GuiCellOption::PercentageOption(0.2));
		table->SetColumnOption(3, GuiCellOption::PercentageOption(0.2));

		for (vint r = 0; r < 4; r++)
		{
			for (vint c = 0; c < 4; c++)
			{
				auto cell = new GuiCellComposition;
				cell->SetSite(r, c, 1, 1);
				cell->SetPreferredMinSize(Size(25, 25));
				table->AddChild(cell);
			}
		}

		{
			vint xs[4] = { 10,45,85,120 };
			vint ys[4] = { 10,45,75,110 };
			vint ws[4] = { 25,30,25,25 };
			vint hs[4] = { 25,20,25,100 };
			testUnmergedCells(table, { 155,220 }, xs, ys, ws, hs);
		}

		table->SetBorderVisible(false);
		{
			vint xs[4] = { 0,35,75,110 };
			vint ys[4] = { 0,35,65,100 };
			vint ws[4] = { 25,30,25,25 };
			vint hs[4] = { 25,20,25,100 };
			testUnmergedCells(table, { 135,200 }, xs, ys, ws, hs);
		}

		SafeDeleteComposition(table);
	});

	TEST_CATEGORY(L"Test <Table> with merged cells and MinSize only")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetCellPadding(10);
		table->SetRowsAndColumns(2, 2);
		table->SetRowOption(0, GuiCellOption::MinSizeOption());
		table->SetRowOption(1, GuiCellOption::MinSizeOption());
		table->SetColumnOption(0, GuiCellOption::MinSizeOption());
		table->SetColumnOption(1, GuiCellOption::MinSizeOption());

		auto cell1 = new GuiCellComposition; cell1->SetPreferredMinSize(Size(50, 60));
		auto cell2 = new GuiCellComposition; cell2->SetPreferredMinSize(Size(70, 80));
		auto cell3 = new GuiCellComposition; cell3->SetPreferredMinSize(Size(1, 2));
		auto cell4 = new GuiCellComposition; cell4->SetPreferredMinSize(Size(3, 4));

		TEST_CASE(L"A1B2")
		{
			cell1->SetSite(0, 0, 2, 2);
			table->AddChild(cell1);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == cell1);
			TEST_ASSERT(table->GetSitedCell(1, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(1, 1) == cell1);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(30, 30));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 10,10 }));
		});

		TEST_CASE(L"A1A1, B1B2")
		{
			cell1->SetSite(0, 0, 1, 1);
			cell2->SetSite(0, 1, 2, 1);
			table->AddChild(cell2);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == cell2);
			TEST_ASSERT(table->GetSitedCell(1, 0) == nullptr);
			TEST_ASSERT(table->GetSitedCell(1, 1) == cell2);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(150, 90));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 50,60 }));
			TEST_ASSERT(cell2->GetCachedBounds() == Rect({ 70,10 }, { 70,70 }));
		});

		TEST_CASE(L"A1A1, A2B2")
		{
			cell2->SetSite(1, 0, 1, 2);
			cell2->SetPreferredMinSize(Size(70, 80));
			table->AddChild(cell2);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == nullptr);
			TEST_ASSERT(table->GetSitedCell(1, 0) == cell2);
			TEST_ASSERT(table->GetSitedCell(1, 1) == cell2);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(80, 170));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 50,60 }));
			TEST_ASSERT(cell2->GetCachedBounds() == Rect({ 10,80 }, { 60,80 }));
		});

		TEST_CASE(L"A1A1, B1B2, C2C2, B3B3")
		{
			table->SetRowsAndColumns(3, 3);
			table->SetRowOption(2, GuiCellOption::MinSizeOption());
			table->SetColumnOption(2, GuiCellOption::MinSizeOption());

			cell2->SetSite(0, 1, 2, 1);
			cell3->SetSite(1, 2, 1, 1);
			table->AddChild(cell3);
			cell4->SetSite(2, 1, 1, 1);
			table->AddChild(cell4);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == cell2);
			TEST_ASSERT(table->GetSitedCell(0, 2) == nullptr);
			TEST_ASSERT(table->GetSitedCell(1, 0) == nullptr);
			TEST_ASSERT(table->GetSitedCell(1, 1) == cell2);
			TEST_ASSERT(table->GetSitedCell(1, 2) == cell3);
			TEST_ASSERT(table->GetSitedCell(2, 0) == nullptr);
			TEST_ASSERT(table->GetSitedCell(2, 1) == cell4);
			TEST_ASSERT(table->GetSitedCell(2, 2) == nullptr);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(161, 106));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 50,60 }));
			TEST_ASSERT(cell2->GetCachedBounds() == Rect({ 70,10 }, { 70,72 }));
			TEST_ASSERT(cell3->GetCachedBounds() == Rect({ 150,80 }, { 1,2 }));
			TEST_ASSERT(cell4->GetCachedBounds() == Rect({ 70,92 }, { 70,4 }));
		});

		TEST_CASE(L"A1A1, A2B2, C2C2, B3B3")
		{
			table->SetRowsAndColumns(3, 3);
			table->SetRowOption(2, GuiCellOption::MinSizeOption());
			table->SetColumnOption(2, GuiCellOption::MinSizeOption());

			cell2->SetSite(1, 0, 1, 2);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == nullptr);
			TEST_ASSERT(table->GetSitedCell(0, 2) == nullptr);
			TEST_ASSERT(table->GetSitedCell(1, 0) == cell2);
			TEST_ASSERT(table->GetSitedCell(1, 1) == cell2);
			TEST_ASSERT(table->GetSitedCell(1, 2) == cell3);
			TEST_ASSERT(table->GetSitedCell(2, 0) == nullptr);
			TEST_ASSERT(table->GetSitedCell(2, 1) == cell4);
			TEST_ASSERT(table->GetSitedCell(2, 2) == nullptr);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(94, 184));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 50,60 }));
			TEST_ASSERT(cell2->GetCachedBounds() == Rect({ 10,80 }, { 63,80 }));
			TEST_ASSERT(cell3->GetCachedBounds() == Rect({ 83,80 }, { 1,80 }));
			TEST_ASSERT(cell4->GetCachedBounds() == Rect({ 70,170 }, { 3,4 }));
		});

		SafeDeleteComposition(table);
	});

	TEST_CATEGORY(L"Test <Table> with merged cells and Absolute only")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetCellPadding(10);
		table->SetRowsAndColumns(2, 2);
		table->SetRowOption(0, GuiCellOption::AbsoluteOption(1));
		table->SetRowOption(1, GuiCellOption::AbsoluteOption(2));
		table->SetColumnOption(0, GuiCellOption::AbsoluteOption(3));
		table->SetColumnOption(1, GuiCellOption::AbsoluteOption(4));

		auto cell1 = new GuiCellComposition; cell1->SetPreferredMinSize(Size(50, 60));
		auto cell2 = new GuiCellComposition; cell2->SetPreferredMinSize(Size(70, 80));

		TEST_CASE(L"A1B2")
		{
			cell1->SetSite(0, 0, 2, 2);
			table->AddChild(cell1);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == cell1);
			TEST_ASSERT(table->GetSitedCell(1, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(1, 1) == cell1);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(37, 33));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 17,13 }));
		});

		TEST_CASE(L"A1A1, B1B2")
		{
			cell1->SetSite(0, 0, 1, 1);
			cell2->SetSite(0, 1, 2, 1);
			table->AddChild(cell2);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == cell2);
			TEST_ASSERT(table->GetSitedCell(1, 0) == nullptr);
			TEST_ASSERT(table->GetSitedCell(1, 1) == cell2);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(37, 33));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 3,1 }));
			TEST_ASSERT(cell2->GetCachedBounds() == Rect({ 23,10 }, { 4,13 }));
		});

		TEST_CASE(L"A1A1, A2B2")
		{
			cell2->SetSite(1, 0, 1, 2);
			table->AddChild(cell2);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == nullptr);
			TEST_ASSERT(table->GetSitedCell(1, 0) == cell2);
			TEST_ASSERT(table->GetSitedCell(1, 1) == cell2);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(37, 33));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 3,1 }));
			TEST_ASSERT(cell2->GetCachedBounds() == Rect({ 10,21 }, { 17,2 }));
		});

		SafeDeleteComposition(table);
	});

	TEST_CATEGORY(L"Test <Table> with merged cells and Percentage only")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetCellPadding(10);
		table->SetRowsAndColumns(2, 2);
		table->SetRowOption(0, GuiCellOption::PercentageOption(1));
		table->SetRowOption(1, GuiCellOption::PercentageOption(2));
		table->SetColumnOption(0, GuiCellOption::PercentageOption(3));
		table->SetColumnOption(1, GuiCellOption::PercentageOption(2));

		auto cell1 = new GuiCellComposition; cell1->SetPreferredMinSize(Size(60, 70));
		auto cell2 = new GuiCellComposition; cell2->SetPreferredMinSize(Size(100, 100));

		TEST_CASE(L"A1B2")
		{
			cell1->SetSite(0, 0, 2, 2);
			table->AddChild(cell1);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == cell1);
			TEST_ASSERT(table->GetSitedCell(1, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(1, 1) == cell1);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(80, 90));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 60,70 }));
		});

		TEST_CASE(L"A1A1")
		{
			cell1->SetSite(0, 0, 1, 1);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == nullptr);
			TEST_ASSERT(table->GetSitedCell(1, 0) == nullptr);
			TEST_ASSERT(table->GetSitedCell(1, 1) == nullptr);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(130, 240));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 60,70 }));
		});

		TEST_CASE(L"A1A1, B1B2")
		{
			cell2->SetSite(0, 1, 2, 1);
			table->AddChild(cell2);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == cell2);
			TEST_ASSERT(table->GetSitedCell(1, 0) == nullptr);
			TEST_ASSERT(table->GetSitedCell(1, 1) == cell2);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(280, 240));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 150,70 }));
			TEST_ASSERT(cell2->GetCachedBounds() == Rect({ 170,10 }, { 100,220 }));
		});
		
		TEST_CASE(L"A1A1, A2B2")
		{
			cell2->SetSite(1, 0, 1, 2);
			table->AddChild(cell2);
			TEST_ASSERT(table->GetSitedCell(0, 0) == cell1);
			TEST_ASSERT(table->GetSitedCell(0, 1) == nullptr);
			TEST_ASSERT(table->GetSitedCell(1, 0) == cell2);
			TEST_ASSERT(table->GetSitedCell(1, 1) == cell2);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(130, 240));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 10,10 }, { 60,70 }));
			TEST_ASSERT(cell2->GetCachedBounds() == Rect({ 10,90 }, { 110,140 }));
		});

		SafeDeleteComposition(table);
	});

	TEST_CATEGORY(L"Test <Table> with merged cells and mixed GuiCellOption")
	{
		auto table = new GuiTableComposition;

		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetBorderVisible(false);
		table->SetCellPadding(10);
		table->SetRowsAndColumns(5, 5);
		table->SetRowOption(0, GuiCellOption::PercentageOption(1));
		table->SetRowOption(1, GuiCellOption::PercentageOption(2));
		table->SetRowOption(2, GuiCellOption::MinSizeOption());
		table->SetRowOption(3, GuiCellOption::AbsoluteOption(20));
		table->SetRowOption(4, GuiCellOption::PercentageOption(3));
		table->SetColumnOption(0, GuiCellOption::PercentageOption(1));
		table->SetColumnOption(1, GuiCellOption::PercentageOption(2));
		table->SetColumnOption(2, GuiCellOption::MinSizeOption());
		table->SetColumnOption(3, GuiCellOption::AbsoluteOption(20));
		table->SetColumnOption(4, GuiCellOption::PercentageOption(3));

		auto cell1 = new GuiCellComposition; cell1->SetPreferredMinSize(Size(150, 150));
		auto cell2 = new GuiCellComposition; cell2->SetPreferredMinSize(Size(50, 50));
		cell2->SetSite(2, 2, 1, 1);
		table->AddChild(cell2);

		TEST_CASE(L"A2E2: P1[P2;M;A20;P3]")
		{
			cell1->SetSite(0, 1, 1, 4);
			table->AddChild(cell1);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(170, 1010));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 20,0 }, { 150,150 }));
			TEST_ASSERT(cell2->GetCachedBounds() == Rect({ 50,470 }, { 50,50 }));
		});

		TEST_CASE(L"A2A5: P1[P2;M;A20;P3]")
		{
			cell1->SetSite(1, 0, 4, 1);

			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetCachedBounds().GetSize() == Size(1010, 170));
			TEST_ASSERT(cell1->GetCachedBounds() == Rect({ 0,20 }, { 150,150 }));
			TEST_ASSERT(cell2->GetCachedBounds() == Rect({ 470,50 }, { 50,50 }));
		});

		SafeDeleteComposition(table);
	});

	auto dragSplitter = [](GuiTableSplitterCompositionBase* splitter, Size offset)
	{
		GuiMouseEventArgs prototype(splitter);
		prototype.ctrl = false;
		prototype.shift = false;
		prototype.left = true;
		prototype.middle = false;
		prototype.right = true;
		prototype.x = 2;
		prototype.y = 2;
		prototype.wheel = 0;
		prototype.nonClient = false;

		{
			auto args = prototype;
			splitter->GetEventReceiver()->leftButtonDown.Execute(args);
		}
		{
			auto args = prototype;
			args.x += offset.x;
			args.y += offset.y;
			splitter->GetEventReceiver()->mouseMove.Execute(args);
		}
		{
			auto args = prototype;
			args.x += offset.x;
			args.y += offset.y;
			splitter->GetEventReceiver()->leftButtonUp.Execute(args);
		}
	};

	TEST_CATEGORY(L"Test <RowSplitter>")
	{
		TEST_CASE(L"PA|AP")
		{
			auto table = new GuiTableComposition;

			table->SetPreferredMinSize(Size(33, 100));
			table->SetBorderVisible(false);
			table->SetCellPadding(10);
			table->SetRowsAndColumns(4, 1);
			table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
			table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
			table->SetRowOption(1, GuiCellOption::AbsoluteOption(20));
			table->SetRowOption(2, GuiCellOption::AbsoluteOption(20));
			table->SetRowOption(3, GuiCellOption::PercentageOption(0.5));

			auto splitter = new GuiRowSplitterComposition;
			splitter->SetRowsToTheTop(2);
			table->AddChild(splitter);
			table->ForceCalculateSizeImmediately();

			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,45 }, { 33,10 }));

			dragSplitter(splitter, { 0,-10 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(1).absolute == 10);
			TEST_ASSERT(table->GetRowOption(2).absolute == 30);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,35 }, { 33,10 }));

			dragSplitter(splitter, { 0,-10 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(1).absolute == 1);
			TEST_ASSERT(table->GetRowOption(2).absolute == 39);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,26 }, { 33,10 }));

			dragSplitter(splitter, { 0,20 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(1).absolute == 21);
			TEST_ASSERT(table->GetRowOption(2).absolute == 19);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,46 }, { 33,10 }));

			dragSplitter(splitter, { 0,20 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(1).absolute == 39);
			TEST_ASSERT(table->GetRowOption(2).absolute == 1);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,64 }, { 33,10 }));

			SafeDeleteComposition(table);
		});

		TEST_CASE(L"P|P")
		{
			auto table = new GuiTableComposition;

			table->SetPreferredMinSize(Size(33, 100));
			table->SetBorderVisible(false);
			table->SetCellPadding(10);
			table->SetRowsAndColumns(2, 1);
			table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
			table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
			table->SetRowOption(1, GuiCellOption::PercentageOption(0.5));

			auto splitter = new GuiRowSplitterComposition;
			splitter->SetRowsToTheTop(1);
			table->AddChild(splitter);
			table->ForceCalculateSizeImmediately();

			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,45 }, { 33,10 }));

			dragSplitter(splitter, { 0,-10 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,45 }, { 33,10 }));

			dragSplitter(splitter, { 0,10 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,45 }, { 33,10 }));

			SafeDeleteComposition(table);
		});

		TEST_CASE(L"PP|A")
		{
			auto table = new GuiTableComposition;

			table->SetPreferredMinSize(Size(33, 100));
			table->SetBorderVisible(false);
			table->SetCellPadding(10);
			table->SetRowsAndColumns(3, 1);
			table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
			table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
			table->SetRowOption(1, GuiCellOption::PercentageOption(0.5));
			table->SetRowOption(2, GuiCellOption::AbsoluteOption(20));

			auto splitter = new GuiRowSplitterComposition;
			splitter->SetRowsToTheTop(2);
			table->AddChild(splitter);
			table->ForceCalculateSizeImmediately();

			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,70 }, { 33,10 }));

			dragSplitter(splitter, { 0,-50 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(2).absolute == 70);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,20 }, { 33,10 }));

			dragSplitter(splitter, { 0,-50 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(2).absolute == 78);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,12 }, { 33,10 }));

			dragSplitter(splitter, { 0,50 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(2).absolute == 28);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,62 }, { 33,10 }));

			dragSplitter(splitter, { 0,50 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(2).absolute == 1);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,88 }, { 33,10 }));

			SafeDeleteComposition(table);
		});

		TEST_CASE(L"A|PP")
		{
			auto table = new GuiTableComposition;

			table->SetPreferredMinSize(Size(33, 100));
			table->SetBorderVisible(false);
			table->SetCellPadding(10);
			table->SetRowsAndColumns(3, 1);
			table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
			table->SetRowOption(0, GuiCellOption::AbsoluteOption(20));
			table->SetRowOption(1, GuiCellOption::PercentageOption(0.5));
			table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));

			auto splitter = new GuiRowSplitterComposition;
			splitter->SetRowsToTheTop(1);
			table->AddChild(splitter);
			table->ForceCalculateSizeImmediately();

			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,20 }, { 33,10 }));

			dragSplitter(splitter, { 0,50 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(0).absolute == 70);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,70 }, { 33,10 }));

			dragSplitter(splitter, { 0,50 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(0).absolute == 78);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,78 }, { 33,10 }));

			dragSplitter(splitter, { 0,-50 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(0).absolute == 28);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,28 }, { 33,10 }));

			dragSplitter(splitter, { 0,-50 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetRowOption(0).absolute == 1);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 0,1 }, { 33,10 }));

			SafeDeleteComposition(table);
		});
	});

	TEST_CATEGORY(L"Test <ColumnSplitter>")
	{
		TEST_CASE(L"PA|AP")
		{
			auto table = new GuiTableComposition;

			table->SetPreferredMinSize(Size(100, 33));
			table->SetBorderVisible(false);
			table->SetCellPadding(10);
			table->SetRowsAndColumns(1, 4);
			table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
			table->SetColumnOption(0, GuiCellOption::PercentageOption(0.5));
			table->SetColumnOption(1, GuiCellOption::AbsoluteOption(20));
			table->SetColumnOption(2, GuiCellOption::AbsoluteOption(20));
			table->SetColumnOption(3, GuiCellOption::PercentageOption(0.5));

			auto splitter = new GuiColumnSplitterComposition;
			splitter->SetColumnsToTheLeft(2);
			table->AddChild(splitter);
			table->ForceCalculateSizeImmediately();

			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 45,0 }, { 10,33 }));

			dragSplitter(splitter, { -10,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(1).absolute == 10);
			TEST_ASSERT(table->GetColumnOption(2).absolute == 30);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 35,0 }, { 10,33 }));

			dragSplitter(splitter, { -10,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(1).absolute == 1);
			TEST_ASSERT(table->GetColumnOption(2).absolute == 39);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 26,0 }, { 10,33 }));

			dragSplitter(splitter, { 20,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(1).absolute == 21);
			TEST_ASSERT(table->GetColumnOption(2).absolute == 19);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 46,0 }, { 10,33 }));

			dragSplitter(splitter, { 20,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(1).absolute == 39);
			TEST_ASSERT(table->GetColumnOption(2).absolute == 1);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 64,0 }, { 10,33 }));

			SafeDeleteComposition(table);
		});

		TEST_CASE(L"P|P")
		{
			auto table = new GuiTableComposition;

			table->SetPreferredMinSize(Size(100, 33));
			table->SetBorderVisible(false);
			table->SetCellPadding(10);
			table->SetRowsAndColumns(1, 2);
			table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
			table->SetColumnOption(0, GuiCellOption::PercentageOption(0.5));
			table->SetColumnOption(1, GuiCellOption::PercentageOption(0.5));

			auto splitter = new GuiColumnSplitterComposition;
			splitter->SetColumnsToTheLeft(1);
			table->AddChild(splitter);
			table->ForceCalculateSizeImmediately();

			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 45,0 }, { 10,33 }));

			dragSplitter(splitter, { -10,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 45,0 }, { 10,33 }));

			dragSplitter(splitter, { 10,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 45,0 }, { 10,33 }));

			SafeDeleteComposition(table);
		});

		TEST_CASE(L"PP|A")
		{
			auto table = new GuiTableComposition;

			table->SetPreferredMinSize(Size(100, 33));
			table->SetBorderVisible(false);
			table->SetCellPadding(10);
			table->SetRowsAndColumns(1, 3);
			table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
			table->SetColumnOption(0, GuiCellOption::PercentageOption(0.5));
			table->SetColumnOption(1, GuiCellOption::PercentageOption(0.5));
			table->SetColumnOption(2, GuiCellOption::AbsoluteOption(20));

			auto splitter = new GuiColumnSplitterComposition;
			splitter->SetColumnsToTheLeft(2);
			table->AddChild(splitter);
			table->ForceCalculateSizeImmediately();

			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 70,0 }, { 10,33 }));

			dragSplitter(splitter, { -50,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(2).absolute == 70);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 20,0 }, { 10,33 }));

			dragSplitter(splitter, { -50,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(2).absolute == 78);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 12,0 }, { 10,33 }));

			dragSplitter(splitter, { 50,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(2).absolute == 28);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 62,0 }, { 10,33 }));

			dragSplitter(splitter, { 50,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(2).absolute == 1);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 88,0 }, { 10,33 }));

			SafeDeleteComposition(table);
		});

		TEST_CASE(L"A|PP")
		{
			auto table = new GuiTableComposition;

			table->SetPreferredMinSize(Size(100, 33));
			table->SetBorderVisible(false);
			table->SetCellPadding(10);
			table->SetRowsAndColumns(1, 3);
			table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
			table->SetColumnOption(0, GuiCellOption::AbsoluteOption(20));
			table->SetColumnOption(1, GuiCellOption::PercentageOption(0.5));
			table->SetColumnOption(2, GuiCellOption::PercentageOption(0.5));

			auto splitter = new GuiColumnSplitterComposition;
			splitter->SetColumnsToTheLeft(1);
			table->AddChild(splitter);
			table->ForceCalculateSizeImmediately();

			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 20,0 }, { 10,33 }));

			dragSplitter(splitter, { 50,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(0).absolute == 70);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 70,0 }, { 10,33 }));

			dragSplitter(splitter, { 50,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(0).absolute == 78);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 78,0 }, { 10,33 }));

			dragSplitter(splitter, { -50,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(0).absolute == 28);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 28,0 }, { 10,33 }));

			dragSplitter(splitter, { -50,0 });
			table->ForceCalculateSizeImmediately();
			TEST_ASSERT(table->GetColumnOption(0).absolute == 1);
			TEST_ASSERT(splitter->GetCachedBounds() == Rect({ 1,0 }, { 10,33 }));

			SafeDeleteComposition(table);
		});
	});
}