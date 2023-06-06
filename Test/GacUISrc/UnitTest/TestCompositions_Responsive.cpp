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

	TEST_CATEGORY(L"Test level management")
	{
		TEST_CASE(L"Test <ResponsiveFixed>")
		{
			auto fixed = new GuiResponsiveFixedComposition;

			TEST_ASSERT(fixed->GetLevelCount() == 1);
			TEST_ASSERT(fixed->GetCurrentLevel() == 0);

			TEST_ASSERT(fixed->LevelUp() == false);
			TEST_ASSERT(fixed->GetCurrentLevel() == 0);

			TEST_ASSERT(fixed->LevelDown() == false);
			TEST_ASSERT(fixed->GetCurrentLevel() == 0);

			SafeDeleteComposition(fixed);
		});

		TEST_CASE(L"Test <ResponsiveView>")
		{
			auto view = new GuiResponsiveViewComposition;

			auto fixed1 = new GuiResponsiveFixedComposition;
			auto fixed2 = new GuiResponsiveFixedComposition;
			auto fixed3 = new GuiResponsiveFixedComposition;

			view->GetViews().Add(fixed1);
			view->GetViews().Add(fixed2);
			view->GetViews().Add(fixed3);

			TEST_ASSERT(view->GetLevelCount() == 3);
			TEST_ASSERT(view->GetCurrentLevel() == 2);
			TEST_ASSERT(view->GetCurrentView() == fixed1);

			TEST_ASSERT(view->LevelDown() == true);
			TEST_ASSERT(view->GetCurrentLevel() == 1);
			TEST_ASSERT(view->GetCurrentView() == fixed2);

			TEST_ASSERT(view->LevelDown() == true);
			TEST_ASSERT(view->GetCurrentLevel() == 0);
			TEST_ASSERT(view->GetCurrentView() == fixed3);

			TEST_ASSERT(view->LevelDown() == false);
			TEST_ASSERT(view->GetCurrentLevel() == 0);
			TEST_ASSERT(view->GetCurrentView() == fixed3);

			TEST_ASSERT(view->LevelUp() == true);
			TEST_ASSERT(view->GetCurrentLevel() == 1);
			TEST_ASSERT(view->GetCurrentView() == fixed2);

			TEST_ASSERT(view->LevelUp() == true);
			TEST_ASSERT(view->GetCurrentLevel() == 2);
			TEST_ASSERT(view->GetCurrentView() == fixed1);

			TEST_ASSERT(view->LevelUp() == false);
			TEST_ASSERT(view->GetCurrentLevel() == 2);
			TEST_ASSERT(view->GetCurrentView() == fixed1);

			SafeDeleteComposition(view);
		});

		TEST_CASE(L"Test <ResponsiveStack>")
		{
		});

		TEST_CASE(L"Test <ResponsiveGroup>")
		{
		});

		TEST_CASE(L"Test Stack -> Group -> View -> Fixed")
		{
		});

		TEST_CASE(L"Test Stack -> Shared -> View -> Fixed")
		{
		});
	});

	TEST_CATEGORY(L"Test shared controls management")
	{
	});
}