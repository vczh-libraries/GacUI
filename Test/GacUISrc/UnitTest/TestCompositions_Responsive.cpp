#include "TestCompositions.h"
using namespace vl::presentation::controls;
using namespace composition_bounds_tests;

void TestResponsiveGroupViewSizes(vint sizeCount)
{
	for (auto direction : { ResponsiveDirection::Horizontal,ResponsiveDirection::Vertical })
	{
		for (auto indirect : { false,true })
		{
			auto horizontal = direction == ResponsiveDirection::Horizontal;
			auto makeSize = [=](vint main, vint cross) { return horizontal ? Size(main, cross) : Size(cross, main); };
			auto container = new GuiResponsiveContainerComposition;
			auto group = new GuiResponsiveGroupComposition;
			auto view = new GuiResponsiveViewComposition;
			GuiBoundsComposition* parent = group;
			if (indirect)
			{
				parent = new GuiBoundsComposition;
				parent->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				group->AddChild(parent);
			}
			group->SetDirection(direction);
			view->SetDirection(direction);
			for (auto minimum : { 100,20,10 })
			{
				auto fixed = new GuiResponsiveFixedComposition;
				fixed->SetPreferredMinSize(makeSize(minimum, 1));
				view->GetViews().Add(fixed);
			}
			parent->AddChild(view);
			group->SetPreferredMinSize(makeSize(7, 13));
			collections::Array<vint> sizes(sizeCount);
			for (vint i = 0; i < sizeCount; i++) sizes[i] = (i + 1) * 40;
			group->SetViewSizes(sizes);
			container->SetResponsiveTarget(group);

			vint levelCount = sizeCount > 3 ? sizeCount : 3;
			vint thresholds[] = { sizeCount == 0 ? 10 : 40,sizeCount == 0 ? 20 : 80,sizeCount < 3 ? 100 : 120,160 };
			auto respond = [&](vint size, vint level, bool hasView = true)
			{
				container->SetExpectedBounds(Rect({ 0,0 }, makeSize(size, 200)));
				container->ForceCalculateSizeImmediately();
				TEST_ASSERT(group->GetLevelCount() == (hasView ? levelCount : sizeCount == 0 ? 1 : sizeCount));
				TEST_ASSERT(group->GetCurrentLevel() == level);
				if (hasView) TEST_ASSERT(view->GetCurrentLevel() == (level < 3 ? level : 2));
				auto preferred = sizeCount == 0 ? 7 : level < sizeCount ? (level + 1) * 40 : sizeCount * 40 + level - sizeCount + 1;
				TEST_ASSERT(group->GetPreferredMinSize() == makeSize(preferred, 13));
				TEST_ASSERT(group->GetViewSizes().Count() == sizeCount);
				for (vint i = 0; i < sizeCount; i++) TEST_ASSERT(group->GetViewSizes()[i] == (i + 1) * 40);
			};

			// Changing the input array must not mutate the group's owned values.
			if (sizeCount > 0) sizes[0] = 999;
			for (vint repeat = 0; repeat < 2; repeat++)
			{
				respond(200, levelCount - 1);
				for (vint level = levelCount - 1; level >= 0; level--)
				{
					respond(thresholds[level], level);
					respond(thresholds[level] - 1, level == 0 ? 0 : level - 1);
				}
				for (vint level = 1; level < levelCount; level++)
				{
					respond(thresholds[level] - 1, level - 1);
					respond(thresholds[level], level);
				}
			}

			// Configured levels survive removing every participating child and can still resize.
			respond(200, levelCount - 1);
			parent->RemoveChild(view);
			respond(200, sizeCount == 0 ? 0 : sizeCount - 1, false);
			for (vint level = sizeCount - 1; level >= 0; level--)
			{
				respond((level + 1) * 40, level, false);
				respond((level + 1) * 40 - 1, level == 0 ? 0 : level - 1, false);
			}
			for (vint level = 1; level < sizeCount; level++)
			{
				respond((level + 1) * 40 - 1, level - 1, false);
				respond((level + 1) * 40, level, false);
			}
			if (sizeCount > 0)
			{
				while (view->GetCurrentLevel() > group->GetCurrentLevel()) view->LevelDown();
			}
			parent->AddChild(view);
			respond(200, levelCount - 1);

			if (sizeCount > 0)
			{
				// Reload synchronously from the final group event, as the movie list does.
				vint reloadCount = 0;
				auto handler = group->CurrentLevelChanged.AttachLambda([&](auto self, auto& arguments)
				{
					auto level = group->GetCurrentLevel();
					if (level >= sizeCount) return;
					reloadCount++;
					parent->RemoveChild(view);
					TEST_ASSERT(group->GetLevelCount() == sizeCount);
					TEST_ASSERT(group->GetCurrentLevel() == level);
					while (view->LevelDown());
					while (view->GetCurrentLevel() < level && view->LevelUp());
					parent->AddChild(view);
					TEST_ASSERT(group->GetCurrentLevel() == level);
				});
				for (vint repeat = 0; repeat < 2; repeat++)
				{
					respond(39, 0);
					respond(200, levelCount - 1);
				}
				TEST_ASSERT(reloadCount == 4 * (sizeCount - 1) + (levelCount > sizeCount ? 2 : 0));
				TEST_ASSERT(group->CurrentLevelChanged.Detach(handler));

				group->SetPreferredMinSize(makeSize(999, 13));
				respond(200, levelCount - 1);
				auto preferred = group->GetPreferredMinSize();
				group->SetViewSizes(collections::Array<vint>());
				container->ForceCalculateSizeImmediately();
				TEST_ASSERT(group->GetLevelCount() == 3);
				TEST_ASSERT(group->GetCurrentLevel() == 2);
				TEST_ASSERT(group->GetPreferredMinSize() == preferred);
				TEST_ASSERT(group->GetViewSizes().Count() == 0);
				group->SetDirection(ResponsiveDirection::Both);
				TEST_ASSERT(group->GetPreferredMinSize() == preferred);
				sizes[0] = 40;
				group->SetViewSizes(sizes);
				vint largest = sizeCount < 3 ? 81 : sizeCount * 40;
				TEST_ASSERT(group->GetPreferredMinSize() == Size(largest, largest));
				group->SetDirection(horizontal ? ResponsiveDirection::Vertical : ResponsiveDirection::Horizontal);
				TEST_ASSERT(group->GetLevelCount() == sizeCount);
				TEST_ASSERT(group->GetCurrentLevel() == sizeCount - 1);
				TEST_ASSERT(group->GetPreferredMinSize() == makeSize(largest, sizeCount * 40));
				group->SetDirection(direction);
				TEST_ASSERT(group->GetLevelCount() == levelCount);
				TEST_ASSERT(group->GetCurrentLevel() == levelCount - 1);
				TEST_ASSERT(group->GetPreferredMinSize() == makeSize(largest, sizeCount * 40));

				// Clearing and assigning without children removes and restores the permanent contribution.
				parent->RemoveChild(view);
				group->SetViewSizes(collections::Array<vint>());
				TEST_ASSERT(group->GetLevelCount() == 1);
				TEST_ASSERT(group->GetCurrentLevel() == 0);
				group->SetViewSizes(sizes);
				TEST_ASSERT(group->GetLevelCount() == sizeCount);
				TEST_ASSERT(group->GetCurrentLevel() == sizeCount - 1);
				parent->AddChild(view);
			}
			SafeDeleteComposition(container);
		}
	}
}
TEST_FILE
{
	TEST_CASE(L"Responsive group with no view sizes")
	{
		TestResponsiveGroupViewSizes(0);
	});
	TEST_CASE(L"Responsive group with fewer view sizes than levels")
	{
		TestResponsiveGroupViewSizes(2);
	});
	TEST_CASE(L"Responsive group with one view size per level")
	{
		TestResponsiveGroupViewSizes(3);
	});
	TEST_CASE(L"Responsive group with more view sizes than levels")
	{
		TestResponsiveGroupViewSizes(4);
	});
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
