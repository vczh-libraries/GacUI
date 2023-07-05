#include "TestCompositions.h"
using namespace vl::presentation::controls;
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

#define TEST_ASSERT_RESPONSIVE(L, LA, VA, LB, VB, LC, VC)			\
			TEST_ASSERT(stack->GetLevelCount() == 4);				\
			TEST_ASSERT(stack->GetCurrentLevel() == L);				\
			TEST_ASSERT(viewA->GetLevelCount() == 3);				\
			TEST_ASSERT(viewA->GetCurrentLevel() == LA);			\
			TEST_ASSERT(viewA->GetCurrentView() == fixed ## VA);	\
			TEST_ASSERT(viewB->GetLevelCount() == 2);				\
			TEST_ASSERT(viewB->GetCurrentLevel() == LB);			\
			TEST_ASSERT(viewB->GetCurrentView() == fixed ## VB);	\
			TEST_ASSERT(viewC->GetLevelCount() == 1);				\
			TEST_ASSERT(viewC->GetCurrentLevel() == LC);			\
			TEST_ASSERT(viewC->GetCurrentView() == fixed ## VC)	\

			TEST_ASSERT_RESPONSIVE(3, 2, AA, 1, BA, 0, CA);

			TEST_ASSERT(stack->LevelDown() == true);
			TEST_ASSERT_RESPONSIVE(2, 1, AB, 1, BA, 0, CA);

			TEST_ASSERT(stack->LevelDown() == true);
			TEST_ASSERT_RESPONSIVE(1, 0, AC, 1, BA, 0, CA);

			TEST_ASSERT(stack->LevelDown() == true);
			TEST_ASSERT_RESPONSIVE(0, 0, AC, 0, BB, 0, CA);

			TEST_ASSERT(stack->LevelDown() == false);
			TEST_ASSERT_RESPONSIVE(0, 0, AC, 0, BB, 0, CA);

			TEST_ASSERT(stack->LevelUp() == true);
			TEST_ASSERT_RESPONSIVE(1, 1, AB, 0, BB, 0, CA);

			TEST_ASSERT(stack->LevelUp() == true);
			TEST_ASSERT_RESPONSIVE(2, 2, AA, 0, BB, 0, CA);

			TEST_ASSERT(stack->LevelUp() == true);
			TEST_ASSERT_RESPONSIVE(3, 2, AA, 1, BA, 0, CA);

			TEST_ASSERT(stack->LevelUp() == false);
			TEST_ASSERT_RESPONSIVE(3, 2, AA, 1, BA, 0, CA);

#undef TEST_ASSERT_RESPONSIVE
			SafeDeleteComposition(stack);
		});

		TEST_CASE(L"Test <ResponsiveGroup>")
		{
			auto group = new GuiResponsiveGroupComposition;

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

			group->AddChild(viewA);
			group->AddChild(viewB);
			group->AddChild(viewC);

#define TEST_ASSERT_RESPONSIVE(L, LA, VA, LB, VB, LC, VC)			\
			TEST_ASSERT(group->GetLevelCount() == 3);				\
			TEST_ASSERT(group->GetCurrentLevel() == L);				\
			TEST_ASSERT(viewA->GetLevelCount() == 3);				\
			TEST_ASSERT(viewA->GetCurrentLevel() == LA);			\
			TEST_ASSERT(viewA->GetCurrentView() == fixed ## VA);	\
			TEST_ASSERT(viewB->GetLevelCount() == 2);				\
			TEST_ASSERT(viewB->GetCurrentLevel() == LB);			\
			TEST_ASSERT(viewB->GetCurrentView() == fixed ## VB);	\
			TEST_ASSERT(viewC->GetLevelCount() == 1);				\
			TEST_ASSERT(viewC->GetCurrentLevel() == LC);			\
			TEST_ASSERT(viewC->GetCurrentView() == fixed ## VC)	\

			TEST_ASSERT_RESPONSIVE(2, 2, AA, 1, BA, 0, CA);

			TEST_ASSERT(group->LevelDown() == true);
			TEST_ASSERT_RESPONSIVE(1, 1, AB, 1, BA, 0, CA);

			TEST_ASSERT(group->LevelDown() == true);
			TEST_ASSERT_RESPONSIVE(0, 0, AC, 0, BB, 0, CA);

			TEST_ASSERT(group->LevelDown() == false);
			TEST_ASSERT_RESPONSIVE(0, 0, AC, 0, BB, 0, CA);

			TEST_ASSERT(group->LevelUp() == true);
			TEST_ASSERT_RESPONSIVE(1, 1, AB, 1, BA, 0, CA);

			TEST_ASSERT(group->LevelUp() == true);
			TEST_ASSERT_RESPONSIVE(2, 2, AA, 1, BA, 0, CA);

			TEST_ASSERT(group->LevelUp() == false);
			TEST_ASSERT_RESPONSIVE(2, 2, AA, 1, BA, 0, CA);

#undef TEST_ASSERT_RESPONSIVE
			SafeDeleteComposition(group);
		});
	});

	TEST_CASE(L"Test shared controls management")
	{
		auto view = new GuiResponsiveViewComposition;

		auto control1 = new GuiControl(theme::ThemeName::CustomControl);
		auto control2 = new GuiControl(theme::ThemeName::CustomControl);

		auto controlTemplate = [](const Value&)
		{
			return new templates::GuiControlTemplate;
		};
		control1->SetControlTemplate(controlTemplate);
		control2->SetControlTemplate(controlTemplate);

		view->GetSharedControls().Add(control1);
		view->GetSharedControls().Add(control2);

		auto fixedA = new GuiResponsiveFixedComposition;
		auto fixedB = new GuiResponsiveFixedComposition;
		auto fixedC = new GuiResponsiveFixedComposition;

		auto sharedA1 = new GuiResponsiveSharedComposition;
		auto sharedB1 = new GuiResponsiveSharedComposition;
		auto sharedC1 = new GuiResponsiveSharedComposition;

		auto sharedA2 = new GuiResponsiveSharedComposition;
		auto sharedB2 = new GuiResponsiveSharedComposition;
		auto sharedC2 = new GuiResponsiveSharedComposition;

		sharedA1->SetShared(control1);
		sharedB1->SetShared(control1);
		sharedC1->SetShared(control1);
		sharedA2->SetShared(control2);
		sharedB2->SetShared(control2);
		sharedC2->SetShared(control2);

		fixedA->AddChild(sharedA1);
		fixedA->AddChild(sharedA2);
		fixedB->AddChild(sharedB1);
		fixedB->AddChild(sharedB2);
		fixedC->AddChild(sharedC1);
		fixedC->AddChild(sharedC2);

		view->GetViews().Add(fixedA);
		view->GetViews().Add(fixedB);
		view->GetViews().Add(fixedC);

		TEST_ASSERT(view->GetLevelCount() == 3);
		TEST_ASSERT(control1->GetBoundsComposition()->GetParent() == sharedA1);
		TEST_ASSERT(control2->GetBoundsComposition()->GetParent() == sharedA2);

		TEST_ASSERT(view->LevelDown() == true);
		TEST_ASSERT(control1->GetBoundsComposition()->GetParent() == sharedB1);
		TEST_ASSERT(control2->GetBoundsComposition()->GetParent() == sharedB2);

		TEST_ASSERT(view->LevelDown() == true);
		TEST_ASSERT(control1->GetBoundsComposition()->GetParent() == sharedC1);
		TEST_ASSERT(control2->GetBoundsComposition()->GetParent() == sharedC2);

		TEST_ASSERT(view->LevelDown() == false);
		TEST_ASSERT(control1->GetBoundsComposition()->GetParent() == sharedC1);
		TEST_ASSERT(control2->GetBoundsComposition()->GetParent() == sharedC2);

		TEST_ASSERT(view->LevelUp() == true);
		TEST_ASSERT(control1->GetBoundsComposition()->GetParent() == sharedB1);
		TEST_ASSERT(control2->GetBoundsComposition()->GetParent() == sharedB2);

		TEST_ASSERT(view->LevelUp() == true);
		TEST_ASSERT(control1->GetBoundsComposition()->GetParent() == sharedA1);
		TEST_ASSERT(control2->GetBoundsComposition()->GetParent() == sharedA2);

		TEST_ASSERT(view->LevelUp() == false);
		TEST_ASSERT(control1->GetBoundsComposition()->GetParent() == sharedA1);
		TEST_ASSERT(control2->GetBoundsComposition()->GetParent() == sharedA2);

		SafeDeleteComposition(view);
	});

	TEST_CASE(L"Test <ResponsiveContainer>")
	{
		auto responsive = new GuiResponsiveContainerComposition;
		auto fake = new FakeResponsiveComposition;
		responsive->SetResponsiveTarget(fake);

#define RESPOND(W, H, L)\
		responsive->SetExpectedBounds(Rect({ 0,0 }, { W,H }));\
		responsive->ForceCalculateSizeImmediately();\
		TEST_ASSERT(fake->GetCurrentLevel() == L)\

		RESPOND(100, 100, 2);
		RESPOND(30, 30, 2);
		RESPOND(25, 25, 1);
		RESPOND(20, 20, 1);
		RESPOND(15, 15, 0);
		RESPOND(10, 10, 0);
		RESPOND(5, 5, 0);
		RESPOND(10, 10, 0);
		RESPOND(15, 15, 0);
		RESPOND(20, 20, 1);
		RESPOND(25, 25, 1);
		RESPOND(30, 30, 2);
		RESPOND(100, 100, 2);

#undef RESPOND

		SafeDeleteComposition(responsive);
	});
}