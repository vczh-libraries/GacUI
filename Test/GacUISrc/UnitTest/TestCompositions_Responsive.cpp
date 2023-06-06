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

			auto fixedA = new GuiResponsiveFixedComposition;
			auto fixedB = new GuiResponsiveFixedComposition;
			auto fixedC = new GuiResponsiveFixedComposition;

			view->GetViews().Add(fixedA);
			view->GetViews().Add(fixedB);
			view->GetViews().Add(fixedC);

			TEST_ASSERT(view->GetLevelCount() == 3);
			TEST_ASSERT(view->GetCurrentLevel() == 2);
			TEST_ASSERT(view->GetCurrentView() == fixedA);

			TEST_ASSERT(view->LevelDown() == true);
			TEST_ASSERT(view->GetCurrentLevel() == 1);
			TEST_ASSERT(view->GetCurrentView() == fixedB);

			TEST_ASSERT(view->LevelDown() == true);
			TEST_ASSERT(view->GetCurrentLevel() == 0);
			TEST_ASSERT(view->GetCurrentView() == fixedC);

			TEST_ASSERT(view->LevelDown() == false);
			TEST_ASSERT(view->GetCurrentLevel() == 0);
			TEST_ASSERT(view->GetCurrentView() == fixedC);

			TEST_ASSERT(view->LevelUp() == true);
			TEST_ASSERT(view->GetCurrentLevel() == 1);
			TEST_ASSERT(view->GetCurrentView() == fixedB);

			TEST_ASSERT(view->LevelUp() == true);
			TEST_ASSERT(view->GetCurrentLevel() == 2);
			TEST_ASSERT(view->GetCurrentView() == fixedA);

			TEST_ASSERT(view->LevelUp() == false);
			TEST_ASSERT(view->GetCurrentLevel() == 2);
			TEST_ASSERT(view->GetCurrentView() == fixedA);

			SafeDeleteComposition(view);
		});

		TEST_CASE(L"Test <ResponsiveStack>")
		{
			auto stack = new GuiResponsiveStackComposition;

			auto viewA = new GuiResponsiveViewComposition;
			auto viewB = new GuiResponsiveViewComposition;
			auto viewC = new GuiResponsiveViewComposition;

			auto fixedAA = new GuiResponsiveFixedComposition;
			auto fixedAB = new GuiResponsiveFixedComposition;
			auto fixedAC = new GuiResponsiveFixedComposition;
			auto fixedBA = new GuiResponsiveFixedComposition;
			auto fixedBB = new GuiResponsiveFixedComposition;
			auto fixedCA = new GuiResponsiveFixedComposition;

			viewA->GetViews().Add(fixedAA);
			viewA->GetViews().Add(fixedAB);
			viewA->GetViews().Add(fixedAC);
			viewB->GetViews().Add(fixedBA);
			viewB->GetViews().Add(fixedBB);
			viewC->GetViews().Add(fixedCA);

			stack->AddChild(viewA);
			stack->AddChild(viewB);
			stack->AddChild(viewC);

			TEST_ASSERT(stack->GetLevelCount() == 4);
			TEST_ASSERT(stack->GetCurrentLevel() == 3);
			TEST_ASSERT(viewA->GetLevelCount() == 3);
			TEST_ASSERT(viewA->GetCurrentLevel() == 2);
			TEST_ASSERT(viewA->GetCurrentView() == fixedAA);
			TEST_ASSERT(viewB->GetLevelCount() == 2);
			TEST_ASSERT(viewB->GetCurrentLevel() == 1);
			TEST_ASSERT(viewB->GetCurrentView() == fixedBA);
			TEST_ASSERT(viewC->GetLevelCount() == 1);
			TEST_ASSERT(viewC->GetCurrentLevel() == 0);
			TEST_ASSERT(viewC->GetCurrentView() == fixedCA);

			SafeDeleteComposition(stack);
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