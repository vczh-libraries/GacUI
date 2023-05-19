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
				cell->SetPreferredMinSize(Size(10 + r * 3 + c, 10 + c * 3 + r));
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

		TEST_ASSERT(table->GetClientArea() == Rect({ 0,0 }, { 73,73 }));
		TEST_ASSERT(table->GetMinPreferredClientSize() == table->GetClientArea().GetSize());
		TEST_ASSERT(table->GetPreferredBounds() == table->GetClientArea());
		TEST_ASSERT(table->GetBounds() == table->GetClientArea());

		TEST_ASSERT(table->GetSitedCell(0, 0)->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetSitedCell(0, 1)->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetSitedCell(0, 2)->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetSitedCell(1, 0)->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetSitedCell(1, 1)->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetSitedCell(1, 2)->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetSitedCell(2, 0)->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetSitedCell(2, 1)->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(table->GetSitedCell(2, 2)->GetBounds() == Rect({ 0,0 }, { 0,0 }));

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