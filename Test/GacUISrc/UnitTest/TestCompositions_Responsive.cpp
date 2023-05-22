#include "TestCompositions.h"
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test <ResponsiveShared> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiResponsiveSharedComposition>(
			[](GuiResponsiveSharedComposition* responsive)
			{
				responsive->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
			});
	});

	TEST_CATEGORY(L"Test <ResponsiveView> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiResponsiveViewComposition>(
			[](GuiResponsiveViewComposition* responsive)
			{
				responsive->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
				responsive->SetPreferredMinSize({ 0,0 });
			});
	});

	TEST_CATEGORY(L"Test <ResponsiveFixed> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiResponsiveFixedComposition>(
			[](GuiResponsiveFixedComposition* responsive)
			{
				responsive->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
				responsive->SetPreferredMinSize({ 0,0 });
			});
	});

	TEST_CATEGORY(L"Test <ResponsiveStack> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiResponsiveStackComposition>(
			[](GuiResponsiveStackComposition* responsive)
			{
				responsive->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
				responsive->SetPreferredMinSize({ 0,0 });
			});
	});

	TEST_CATEGORY(L"Test <ResponsiveGroup> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiResponsiveGroupComposition>(
			[](GuiResponsiveGroupComposition* responsive)
			{
				responsive->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
				responsive->SetPreferredMinSize({ 0,0 });
			});
	});

	TEST_CATEGORY(L"Test <ResponsiveContainer> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiResponsiveContainerComposition>();
	});
}