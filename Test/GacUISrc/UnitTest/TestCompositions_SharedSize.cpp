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
		TestBoundsWithTrivialChildren<GuiSharedSizeItemComposition>();
	});
}