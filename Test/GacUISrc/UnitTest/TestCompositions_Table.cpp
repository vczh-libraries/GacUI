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

	TEST_CASE(L"Test <Table> with MinSize only")
	{
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