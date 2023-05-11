#include "../../../Source/GacUI.h"

using namespace vl;
using namespace vl::reflection;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::compositions;
using namespace vl::presentation::controls;
using namespace vl::presentation::templates;

TEST_FILE
{
	TEST_CASE(L"Test <Bounds> default properties")
	{
		auto bounds = new GuiBoundsComposition;

		TEST_ASSERT(bounds->GetParent() == nullptr);
		TEST_ASSERT(bounds->Children().Count() == 0);
		TEST_ASSERT(bounds->GetVisible() == true);
		TEST_ASSERT(bounds->GetMinSizeLimitation() == GuiGraphicsComposition::NoLimit);
		TEST_ASSERT(bounds->GetRenderTarget() == nullptr);
		TEST_ASSERT(bounds->HasEventReceiver() == false);
		TEST_ASSERT(bounds->GetEventReceiver() != nullptr);
		TEST_ASSERT(bounds->HasEventReceiver() == true);
		TEST_ASSERT(bounds->GetAssociatedHitTestResult() == INativeWindowListener::NoDecision);

		TEST_ASSERT(bounds->GetOwnedElement() == nullptr);
		TEST_ASSERT(bounds->GetAssociatedControl() == nullptr);
		TEST_ASSERT(bounds->GetAssociatedHost() == nullptr);
		TEST_ASSERT(bounds->GetAssociatedCursor() == nullptr);
		TEST_ASSERT(bounds->GetRelatedControl() == nullptr);
		TEST_ASSERT(bounds->GetRelatedGraphicsHost() == nullptr);
		TEST_ASSERT(bounds->GetRelatedControlHost() == nullptr);
		TEST_ASSERT(bounds->GetRelatedCursor() == nullptr);

		TEST_ASSERT(bounds->GetGlobalBounds() == Rect(0, 0, 0, 0));
		TEST_ASSERT(bounds->GetMargin() == Margin(0, 0, 0, 0));
		TEST_ASSERT(bounds->GetInternalMargin() == Margin(0, 0, 0, 0));
		TEST_ASSERT(bounds->GetPreferredMinSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetAlignmentToParent() == Margin(-1, -1, -1, -1));
		TEST_ASSERT(bounds->GetBounds() == Rect(0, 0, 0, 0));

		TEST_ASSERT(bounds->GetClientArea() == Rect(0, 0, 0, 0));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect(0, 0, 0, 0));

		SafeDeleteComposition(bounds);
	});

	TEST_CASE(L"Test single <Bounds> layout")
	{
		auto bounds = new GuiBoundsComposition();

		bounds->SetMargin(Margin(1, 2, 3, 4));
		TEST_ASSERT(bounds->GetMargin() == Margin(1, 2, 3, 4));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 1,2 }, { 0,0 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 0,0 }, { 4,6 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 0,0 }, { 4,6 }));

		bounds->SetInternalMargin(Margin(10, 20, 30, 40));
		TEST_ASSERT(bounds->GetInternalMargin() == Margin(10, 20, 30, 40));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 11,22 }, { 0,0 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 0,0 }, { 44,66 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 0,0 }, { 44,66 }));

		bounds->SetPreferredMinSize(Size(100, 200));
		TEST_ASSERT(bounds->GetPreferredMinSize() == Size(100, 200));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 11,22 }, { 100,200 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 0,0 }, { 144,266 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 0,0 }, { 144,266 }));

		bounds->SetBounds(Rect({ 300,400 }, { 10,20 }));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 311,422 }, { 100,200 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 300,400 }, { 144,266 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 300,400 }, { 144,266 }));

		bounds->SetBounds(Rect({ 300,400 }, { 100,200 }));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 311,422 }, { 100,200 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 300,400 }, { 144,266 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 300,400 }, { 144,266 }));

		bounds->SetBounds(Rect({ 300,400 }, { 1000,2000 }));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 311,422 }, { 1000-11-33,2000-22-44 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 300,400 }, { 1000,2000 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 300,400 }, { 1000,2000 }));

		SafeDeleteComposition(bounds);
	});

	TEST_CASE(L"Test <Bounds> children operations")
	{
	});

	TEST_CASE(L"Test multiple <Bounds> layout")
	{
	});

	TEST_CASE(L"Test multiple <Bounds> visibility")
	{
	});

	TEST_CASE(L"Test multiple <Bounds> hit test")
	{
	});

	TEST_CASE(L"Test multiple <Bounds> associated resources")
	{
	});
}