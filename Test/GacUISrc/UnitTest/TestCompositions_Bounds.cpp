#include "../../../Source/GacUI.h"

using namespace vl;
using namespace vl::reflection;
using namespace vl::reflection::description;
using namespace vl::presentation;
using namespace vl::presentation::elements;
using namespace vl::presentation::compositions;

namespace composition_bounds_tests
{
	class FakeElement : public Object, public IGuiGraphicsElement, public IGuiGraphicsRenderer
	{
	protected:
		GuiGraphicsComposition*			ownerComposition = nullptr;
		Size							size;

		void SetOwnerComposition(compositions::GuiGraphicsComposition* composition)
		{
			ownerComposition = composition;
		}
	public:
		FakeElement(Size _size)
			: size(_size)
		{
		}

		IGuiGraphicsRenderer* GetRenderer() override							{ return this; }
		compositions::GuiGraphicsComposition* GetOwnerComposition() override	{ return ownerComposition; }
		IGuiGraphicsRendererFactory* GetFactory() override						{ return nullptr; }
		void Initialize(IGuiGraphicsElement* element) override					{}
		void Finalize() override												{}
		void SetRenderTarget(IGuiGraphicsRenderTarget* renderTarget) override	{}
		void Render(Rect bounds) override										{}
		void OnElementStateChanged() override									{}
		Size GetMinSize() override												{ return size; }
	};
}
using namespace composition_bounds_tests;

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

		// Margin
		bounds->SetMargin(Margin(1, 2, 3, 4));
		TEST_ASSERT(bounds->GetMargin() == Margin(1, 2, 3, 4));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 1,2 }, { 0,0 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 0,0 }, { 4,6 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 0,0 }, { 4,6 }));

		// Margin + InternalMargin
		bounds->SetInternalMargin(Margin(10, 20, 30, 40));
		TEST_ASSERT(bounds->GetInternalMargin() == Margin(10, 20, 30, 40));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 11,22 }, { 0,0 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 0,0 }, { 44,66 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 0,0 }, { 44,66 }));

		// Margin + InternalMargin + PreferredMinSize
		bounds->SetPreferredMinSize(Size(100, 200));
		TEST_ASSERT(bounds->GetPreferredMinSize() == Size(100, 200));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 11,22 }, { 100,200 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 0,0 }, { 144,266 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 0,0 }, { 144,266 }));

		// Margin + InternalMargin + PreferredMinSize + Element(disabled)
		auto element = Ptr(new FakeElement({ 300,400 }));
		bounds->SetOwnedElement(element);
		TEST_ASSERT(bounds->GetOwnedElement() == element);
		TEST_ASSERT(element->GetOwnerComposition() == bounds);
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 11,22 }, { 100,200 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 0,0 }, { 144,266 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 0,0 }, { 144,266 }));

		// Margin + InternalMargin + PreferredMinSize + Element
		bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
		TEST_ASSERT(bounds->GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElement);
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 11,22 }, { 300,400 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(300, 400));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 0,0 }, { 344,466 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 0,0 }, { 344,466 }));

		// Margin + InternalMargin + PreferredMinSize + Element
		bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		TEST_ASSERT(bounds->GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElementAndChildren);
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 11,22 }, { 300,400 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(300, 400));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 0,0 }, { 344,466 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 0,0 }, { 344,466 }));

		// Margin + InternalMargin + PreferredMinSize + Bounds(too small) + Element
		bounds->SetBounds(Rect({ 300,400 }, { 10,20 }));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 311,422 }, { 300,400 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(300, 400));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 300,400 }, { 344,466 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 300,400 }, { 344,466 }));

		// Margin + InternalMargin + PreferredMinSize + Bounds(too small) + Element(disabled)
		bounds->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
		TEST_ASSERT(bounds->GetMinSizeLimitation() == GuiGraphicsComposition::NoLimit);
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 311,422 }, { 100,200 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 300,400 }, { 144,266 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 300,400 }, { 144,266 }));

		// Margin + InternalMargin + PreferredMinSize + Bounds(just enough) + Element(disabled)
		bounds->SetBounds(Rect({ 300,400 }, { 100,200 }));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 311,422 }, { 100,200 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 300,400 }, { 144,266 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 300,400 }, { 144,266 }));

		// Margin + InternalMargin + PreferredMinSize + Bounds(just enough) + Element
		bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
		TEST_ASSERT(bounds->GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElement);
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 311,422 }, { 300,400 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(300, 400));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 300,400 }, { 344,466 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 300,400 }, { 344,466 }));

		// Margin + InternalMargin + PreferredMinSize + Bounds + Element
		bounds->SetBounds(Rect({ 300,400 }, { 1000,2000 }));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 311,422 }, { 1000 - 11 - 33,2000 - 22 - 44 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(300, 400));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 300,400 }, { 1000,2000 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 300,400 }, { 1000,2000 }));

		// Margin + InternalMargin + PreferredMinSize + Bounds + Element(disabled)
		bounds->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
		TEST_ASSERT(bounds->GetMinSizeLimitation() == GuiGraphicsComposition::NoLimit);
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 311,422 }, { 1000 - 11 - 33,2000 - 22 - 44 }));
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