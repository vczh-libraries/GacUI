#include "TestCompositions.h"
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test <Bounds> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiBoundsComposition>();
	});

	TEST_CASE(L"Test nested <Bounds> visibility")
	{
	});

	TEST_CASE(L"Test nested <Bounds> hit test")
	{
	});

	TEST_CASE(L"Test nested <Bounds> associated resources")
	{
	});
}