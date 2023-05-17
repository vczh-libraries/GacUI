#include "TestCompositions.h"
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test <Stack> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiBoundsComposition>();
	});
}