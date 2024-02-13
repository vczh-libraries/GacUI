#include "TestCompositions.h"
using namespace composition_bounds_tests;

TEST_FILE
{
	TEST_CATEGORY(L"Test <Bounds> with common <Bounds> operations")
	{
		TestBoundsWithTrivialChildren<GuiBoundsComposition>();
	});

	TEST_CATEGORY(L"Test <Bounds> nested properties")
	{
		auto a = new GuiBoundsComposition;
		auto b = new GuiBoundsComposition;
		auto c = new GuiBoundsComposition;
		auto d = new GuiBoundsComposition;

		a->SetExpectedBounds(Rect({ 0,0 }, { 100,100 }));
		b->SetExpectedBounds(Rect({ 10,10 }, { 50,50 }));
		c->SetExpectedBounds(Rect({ 20,20 }, { 40,40 }));
		d->SetExpectedBounds(Rect({ 50,50 }, { 40,40 }));

		a->AddChild(b);
		a->AddChild(d);
		b->AddChild(c);

		TEST_CASE(L"Ensure layout")
		{
			a->ForceCalculateSizeImmediately();
			TEST_ASSERT(a->GetCachedBounds() == Rect({ 0,0 }, { 100,100 }));
			TEST_ASSERT(b->GetCachedBounds() == Rect({ 10,10 }, { 50,50 }));
			TEST_ASSERT(c->GetCachedBounds() == Rect({ 20,20 }, { 40,40 }));
			TEST_ASSERT(d->GetCachedBounds() == Rect({ 50,50 }, { 40,40 }));

			TEST_ASSERT(a->GetGlobalBounds() == Rect({ 0,0 }, { 100,100 }));
			TEST_ASSERT(b->GetGlobalBounds() == Rect({ 10,10 }, { 50,50 }));
			TEST_ASSERT(c->GetGlobalBounds() == Rect({ 30,30 }, { 40,40 }));
			TEST_ASSERT(d->GetGlobalBounds() == Rect({ 50,50 }, { 40,40 }));
		});

		TEST_CASE(L"Test nested <Bounds> visibility")
		{
			TEST_ASSERT(a->GetVisible() == true);
			TEST_ASSERT(b->GetVisible() == true);
			TEST_ASSERT(c->GetVisible() == true);
			TEST_ASSERT(d->GetVisible() == true);
			TEST_ASSERT(a->GetEventuallyVisible() == true);
			TEST_ASSERT(b->GetEventuallyVisible() == true);
			TEST_ASSERT(c->GetEventuallyVisible() == true);
			TEST_ASSERT(d->GetEventuallyVisible() == true);

			b->SetVisible(false);
			TEST_ASSERT(a->GetVisible() == true);
			TEST_ASSERT(b->GetVisible() == false);
			TEST_ASSERT(c->GetVisible() == true);
			TEST_ASSERT(d->GetVisible() == true);
			TEST_ASSERT(a->GetEventuallyVisible() == true);
			TEST_ASSERT(b->GetEventuallyVisible() == false);
			TEST_ASSERT(c->GetEventuallyVisible() == false);
			TEST_ASSERT(d->GetEventuallyVisible() == true);

			c->SetVisible(false);
			TEST_ASSERT(a->GetVisible() == true);
			TEST_ASSERT(b->GetVisible() == false);
			TEST_ASSERT(c->GetVisible() == false);
			TEST_ASSERT(d->GetVisible() == true);
			TEST_ASSERT(a->GetEventuallyVisible() == true);
			TEST_ASSERT(b->GetEventuallyVisible() == false);
			TEST_ASSERT(c->GetEventuallyVisible() == false);
			TEST_ASSERT(d->GetEventuallyVisible() == true);

			b->SetVisible(true);
			TEST_ASSERT(a->GetVisible() == true);
			TEST_ASSERT(b->GetVisible() == true);
			TEST_ASSERT(c->GetVisible() == false);
			TEST_ASSERT(d->GetVisible() == true);
			TEST_ASSERT(a->GetEventuallyVisible() == true);
			TEST_ASSERT(b->GetEventuallyVisible() == true);
			TEST_ASSERT(c->GetEventuallyVisible() == false);
			TEST_ASSERT(d->GetEventuallyVisible() == true);

			c->SetVisible(true);
			TEST_ASSERT(a->GetVisible() == true);
			TEST_ASSERT(b->GetVisible() == true);
			TEST_ASSERT(c->GetVisible() == true);
			TEST_ASSERT(d->GetVisible() == true);
			TEST_ASSERT(a->GetEventuallyVisible() == true);
			TEST_ASSERT(b->GetEventuallyVisible() == true);
			TEST_ASSERT(c->GetEventuallyVisible() == true);
			TEST_ASSERT(d->GetEventuallyVisible() == true);
		});

		TEST_CASE(L"Test nested <Bounds> hit test")
		{
			Point pa = { 0,0 };
			Point pb = { 15,15 };
			Point pc = { 30,30 };
			Point pd = { 50,50 };
		});

		TEST_CASE(L"Test nested <Bounds> cursor")
		{
		});

		TEST_CASE(L"Test nested <Bounds> associated resources")
		{
		});
	});
}