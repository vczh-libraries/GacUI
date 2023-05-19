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

		TEST_ASSERT(table->GetRows() == 1);
		TEST_ASSERT(table->GetRowOption(0) == GuiCellOption::AbsoluteOption(20));
		TEST_ASSERT(table->GetColumns() == 1);
		TEST_ASSERT(table->GetColumnOption(0) == GuiCellOption::AbsoluteOption(20));
		TEST_ASSERT(table->GetCellPadding() == 0);
		TEST_ASSERT(table->GetBorderVisible() == true);

		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 20,20 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test empty <Table> with MinSize only")
	{
		auto table = new GuiTableComposition;

		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::MinSizeOption());
		table->SetRowOption(1, GuiCellOption::MinSizeOption());
		table->SetRowOption(2, GuiCellOption::MinSizeOption());
		table->SetColumnOption(0, GuiCellOption::MinSizeOption());
		table->SetColumnOption(1, GuiCellOption::MinSizeOption());
		table->SetColumnOption(2, GuiCellOption::MinSizeOption());

		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		table->SetCellPadding(10);
		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 40,40 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		table->SetBorderVisible(false);
		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 20,20 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test empty <Table> with Absolute only")
	{
		auto table = new GuiTableComposition;

		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::AbsoluteOption(1));
		table->SetRowOption(1, GuiCellOption::AbsoluteOption(2));
		table->SetRowOption(2, GuiCellOption::AbsoluteOption(3));
		table->SetColumnOption(0, GuiCellOption::AbsoluteOption(4));
		table->SetColumnOption(1, GuiCellOption::AbsoluteOption(5));
		table->SetColumnOption(2, GuiCellOption::AbsoluteOption(6));

		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 15,6 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		table->SetCellPadding(10);
		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 55,46 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		table->SetBorderVisible(false);
		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 35,26 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test empty <Table> with Percentage only")
	{
		auto table = new GuiTableComposition;

		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
		table->SetRowOption(1, GuiCellOption::PercentageOption(1.0));
		table->SetRowOption(2, GuiCellOption::PercentageOption(1.0));
		table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
		table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
		table->SetColumnOption(2, GuiCellOption::PercentageOption(1.0));

		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		table->SetCellPadding(10);
		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 40,40 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		table->SetBorderVisible(false);
		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 20,20 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test empty <Table> with mixed GuiCellOption")
	{
		auto table = new GuiTableComposition;

		table->SetRowsAndColumns(3, 3);
		table->SetRowOption(0, GuiCellOption::MinSizeOption());
		table->SetRowOption(1, GuiCellOption::AbsoluteOption(1));
		table->SetRowOption(2, GuiCellOption::PercentageOption(1.0));
		table->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
		table->SetColumnOption(1, GuiCellOption::AbsoluteOption(2));
		table->SetColumnOption(2, GuiCellOption::MinSizeOption());

		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 2,1 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		table->SetCellPadding(10);
		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 42,41 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		table->SetBorderVisible(false);
		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 22,21 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		SafeDeleteComposition(table);
	});

	auto test3x3Cells = [](GuiTableComposition* table, vint(&xs)[3], vint(&ys)[3], vint(&ws)[3], vint(&hs)[3])
	{
		TEST_ASSERT(table->GetSitedCell(0, 0)->GetBounds() == Rect({ xs[0],ys[0] }, { ws[0],hs[0] }));
		TEST_ASSERT(table->GetSitedCell(0, 1)->GetBounds() == Rect({ xs[1],ys[0] }, { ws[1],hs[0] }));
		TEST_ASSERT(table->GetSitedCell(0, 2)->GetBounds() == Rect({ xs[2],ys[0] }, { ws[2],hs[0] }));
		TEST_ASSERT(table->GetSitedCell(1, 0)->GetBounds() == Rect({ xs[0],ys[1] }, { ws[0],hs[1] }));
		TEST_ASSERT(table->GetSitedCell(1, 1)->GetBounds() == Rect({ xs[1],ys[1] }, { ws[1],hs[1] }));
		TEST_ASSERT(table->GetSitedCell(1, 2)->GetBounds() == Rect({ xs[2],ys[1] }, { ws[2],hs[1] }));
		TEST_ASSERT(table->GetSitedCell(2, 0)->GetBounds() == Rect({ xs[0],ys[2] }, { ws[0],hs[2] }));
		TEST_ASSERT(table->GetSitedCell(2, 1)->GetBounds() == Rect({ xs[1],ys[2] }, { ws[1],hs[2] }));
		TEST_ASSERT(table->GetSitedCell(2, 2)->GetBounds() == Rect({ xs[2],ys[2] }, { ws[2],hs[2] }));
	};

	TEST_CASE(L"Test <Table> with MinSize only")
	{
		auto table = new GuiTableComposition;

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

		for (vint r = 0; r < 3; r++)
		{
			for (vint c = 0; c < 3; c++)
			{
				auto cell = table->GetSitedCell(r, c);
				TEST_ASSERT(table->Children()[r * 3 + c] == cell);
			}
		}

		{
			TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 91,97 }));
			TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
			TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
			TEST_ASSERT(table->GetBounds() == table->GetClientArea());

			vint xs[3] = { 10,36,63 };
			vint ys[3] = { 10,38,67 };
			vint ws[3] = { 16,17,18 };
			vint hs[3] = { 18,19,20 };
			test3x3Cells(table, xs, ys, ws, hs);
		}

		table->SetBorderVisible(false);
		table->SetPreferredMinSize(Size(100, 200));
		{
			TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 100,200 }));
			TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
			TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
			TEST_ASSERT(table->GetBounds() == table->GetClientArea());

			vint xs[3] = { 0,26,53 };
			vint ys[3] = { 0,28,57 };
			vint ws[3] = { 16,17,47 };
			vint hs[3] = { 18,19,143 };
			test3x3Cells(table, xs, ys, ws, hs);
		}

		SafeDeleteComposition(table);
	});

	TEST_CASE(L"Test <Table> with Absolute only")
	{
	});

	TEST_CASE(L"Test <Table> with Percentage only")
	{
	});

	TEST_CASE(L"Test <Table> with mixed GuiCellOption")
	{
	});

	TEST_CASE(L"Test <Table> with merged cells and MinSize only")
	{
	});

	TEST_CASE(L"Test <Table> with merged cells and Absolute only")
	{
	});

	TEST_CASE(L"Test <Table> with merged cells and Percentage only")
	{
	});

	TEST_CASE(L"Test <Table> with merged cells and mixed GuiCellOption")
	{
	});
}