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

	TEST_CATEGORY(L"Test <RepeatFreeHeightItem> binding without item template")
	{
	});

	TEST_CATEGORY(L"Test <RepeatFreeHeightItem> binding in default direction")
	{
	});

	TEST_CATEGORY(L"Test <RepeatFreeHeightItem> layout in different direction")
	{
	});
}