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
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(100, 200));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 0,0 }, { 144,266 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 0,0 }, { 144,266 }));

		// Margin + InternalMargin + PreferredMinSize + Element(disabled)
		auto element = Ptr(new FakeElement({ 300,400 }));
		bounds->SetOwnedElement(element);
		TEST_ASSERT(bounds->GetOwnedElement() == element);
		TEST_ASSERT(element->GetOwnerComposition() == bounds);
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 11,22 }, { 100,200 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(100, 200));
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
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(100, 200));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 300,400 }, { 144,266 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 300,400 }, { 144,266 }));

		// Margin + InternalMargin + PreferredMinSize + Bounds(just enough) + Element(disabled)
		bounds->SetBounds(Rect({ 300,400 }, { 100,200 }));
		TEST_ASSERT(bounds->GetClientArea() == Rect({ 311,422 }, { 100,200 }));
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(100, 200));
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
		TEST_ASSERT(bounds->GetMinPreferredClientSize() == Size(100, 200));
		TEST_ASSERT(bounds->GetPreferredBounds() == Rect({ 300,400 }, { 1000,2000 }));
		TEST_ASSERT(bounds->GetBounds() == Rect({ 300,400 }, { 1000,2000 }));

		SafeDeleteComposition(bounds);
	});

	TEST_CASE(L"Test <Bounds> children operations")
	{
		auto root = new GuiBoundsComposition();
		auto childA = new GuiBoundsComposition();
		auto childAA = new GuiBoundsComposition();
		auto childAB = new GuiBoundsComposition();
		auto childB = new GuiBoundsComposition();

		TEST_ASSERT(root->GetParent() == nullptr);
		TEST_ASSERT(childA->GetParent() == nullptr);
		TEST_ASSERT(childAA->GetParent() == nullptr);
		TEST_ASSERT(childAB->GetParent() == nullptr);
		TEST_ASSERT(childB->GetParent() == nullptr);

		// root{ A{ AA, AB }, B }
		TEST_ASSERT(root->AddChild(childA) == true);
		TEST_ASSERT(root->AddChild(childB) == true);
		TEST_ASSERT(childA->InsertChild(0, childAB) == true);
		TEST_ASSERT(childA->InsertChild(0, childAA) == true);

		TEST_ASSERT(root->GetParent() == nullptr);
		TEST_ASSERT(childA->GetParent() == root);
		TEST_ASSERT(childAA->GetParent() == childA);
		TEST_ASSERT(childAB->GetParent() == childA);
		TEST_ASSERT(childB->GetParent() == root);

		TEST_ASSERT(root->Children().Count() == 2);
		TEST_ASSERT(root->Children()[0] == childA);
		TEST_ASSERT(root->Children()[1] == childB);
		TEST_ASSERT(childA->Children().Count() == 2);
		TEST_ASSERT(childA->Children()[0] == childAA);
		TEST_ASSERT(childA->Children()[1] == childAB);

		// root{ A{ AB, AA }, B }
		TEST_ASSERT(childA->MoveChild(childAB, 0) == true);
		TEST_ASSERT(childAA->GetParent() == childA);
		TEST_ASSERT(childAB->GetParent() == childA);
		TEST_ASSERT(childA->Children().Count() == 2);
		TEST_ASSERT(childA->Children()[0] == childAB);
		TEST_ASSERT(childA->Children()[1] == childAA);

		// no-op
		TEST_ASSERT(root->AddChild(nullptr) == false);
		TEST_ASSERT(root->InsertChild(0, nullptr) == false);
		TEST_ASSERT(root->MoveChild(nullptr, 0) == false);
		TEST_ASSERT(root->AddChild(childAA) == false);
		TEST_ASSERT(root->InsertChild(0, childAA) == false);
		TEST_ASSERT(root->MoveChild(childAA, 0) == false);
		TEST_ASSERT(childA->AddChild(childAA) == false);
		TEST_ASSERT(childA->InsertChild(0, childAA) == false);

		TEST_ASSERT(root->GetParent() == nullptr);
		TEST_ASSERT(childA->GetParent() == root);
		TEST_ASSERT(childAA->GetParent() == childA);
		TEST_ASSERT(childAB->GetParent() == childA);
		TEST_ASSERT(childB->GetParent() == root);

		TEST_ASSERT(root->Children().Count() == 2);
		TEST_ASSERT(root->Children()[0] == childA);
		TEST_ASSERT(root->Children()[1] == childB);
		TEST_ASSERT(childA->Children().Count() == 2);
		TEST_ASSERT(childA->Children()[0] == childAB);
		TEST_ASSERT(childA->Children()[1] == childAA);

		// root{ B }
		// A{ AB, AA }
		TEST_ASSERT(root->RemoveChild(childA) == true);

		TEST_ASSERT(root->GetParent() == nullptr);
		TEST_ASSERT(childA->GetParent() == nullptr);
		TEST_ASSERT(childAA->GetParent() == childA);
		TEST_ASSERT(childAB->GetParent() == childA);
		TEST_ASSERT(childB->GetParent() == root);

		TEST_ASSERT(root->Children().Count() == 1);
		TEST_ASSERT(root->Children()[0] == childB);
		TEST_ASSERT(childA->Children().Count() == 2);
		TEST_ASSERT(childA->Children()[0] == childAB);
		TEST_ASSERT(childA->Children()[1] == childAA);

		TEST_ASSERT(childAA->Children().Count() == 0);
		TEST_ASSERT(childAB->Children().Count() == 0);

		SafeDeleteComposition(root);
		SafeDeleteComposition(childA);
	});

	TEST_CASE(L"Test child <Bounds> layout")
	{
		auto root = new GuiBoundsComposition();
		auto childA = new GuiBoundsComposition();
		auto childB = new GuiBoundsComposition();
		root->AddChild(childA);
		root->AddChild(childB);

		childA->SetPreferredMinSize(Size(100, 200));
		childB->SetPreferredMinSize(Size(300, 400));

		// NoLimit
		TEST_ASSERT(root->GetClientArea() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(root->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childA->GetBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 0,0 }, { 300,400 }));
		TEST_ASSERT(childB->GetBounds() == Rect({ 0,0 }, { 300,400 }));

		// LimitToElement
		root->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
		TEST_ASSERT(root->GetClientArea() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(root->GetMinPreferredClientSize() == Size(0, 0));
		TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 0,0 }));
		TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childA->GetBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 0,0 }, { 300,400 }));
		TEST_ASSERT(childB->GetBounds() == Rect({ 0,0 }, { 300,400 }));

		// LimitToElementAndChildren
		root->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		TEST_ASSERT(root->GetClientArea() == Rect({ 0,0 }, { 300,400 }));
		TEST_ASSERT(root->GetMinPreferredClientSize() == Size(300, 400));
		TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 300,400 }));
		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 300,400 }));
		TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childA->GetBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 0,0 }, { 300,400 }));
		TEST_ASSERT(childB->GetBounds() == Rect({ 0,0 }, { 300,400 }));

		// Element
		root->SetOwnedElement(Ptr(new FakeElement(Size(200, 500))));
		TEST_ASSERT(root->GetClientArea() == Rect({ 0,0 }, { 300,500 }));
		TEST_ASSERT(root->GetMinPreferredClientSize() == Size(300, 500));
		TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 300,500 }));
		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 300,500 }));
		TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childA->GetBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 0,0 }, { 300,400 }));
		TEST_ASSERT(childB->GetBounds() == Rect({ 0,0 }, { 300,400 }));

		// Margin + Element
		root->SetMargin(Margin(1, 2, 3, 4));
		TEST_ASSERT(root->GetClientArea() == Rect({ 1,2 }, { 300,500 }));
		TEST_ASSERT(root->GetMinPreferredClientSize() == Size(300, 500));
		TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 304,506 }));
		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 304,506 }));
		TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childA->GetBounds() == Rect({ 1,2 }, { 100,200 }));
		TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 0,0 }, { 300,400 }));
		TEST_ASSERT(childB->GetBounds() == Rect({ 1,2 }, { 300,400 }));

		// Margin + Element + InternalMargin
		root->SetInternalMargin(Margin(10, 20, 30, 40));
		TEST_ASSERT(root->GetClientArea() == Rect({ 11,22 }, { 300,500 }));
		TEST_ASSERT(root->GetMinPreferredClientSize() == Size(300,500));
		TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 344,566 }));
		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 344,566 }));
		TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childA->GetBounds() == Rect({ 11,22 }, { 100,200 }));
		TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 0,0 }, { 300,400 }));
		TEST_ASSERT(childB->GetBounds() == Rect({ 11,22 }, { 300,400 }));

		// childA:Bounds
		childA->SetBounds(Rect(Point(222, 222), {}));
		TEST_ASSERT(root->GetClientArea() == Rect({ 11,22 }, { 322,500 }));
		TEST_ASSERT(root->GetMinPreferredClientSize() == Size(322, 500));
		TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 366,566 }));
		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 366,566 }));
		TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 222,222 }, { 100,200 }));
		TEST_ASSERT(childA->GetBounds() == Rect({ 233,244 }, { 100,200 }));
		TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 0,0 }, { 300,400 }));
		TEST_ASSERT(childB->GetBounds() == Rect({ 11,22 }, { 300,400 }));

		// childB:AlignmentToParent(left)
		childA->SetBounds({});
		childB->SetAlignmentToParent(Margin(222, -1, -1, -1));
		TEST_ASSERT(root->GetClientArea() == Rect({ 11,22 }, { 522,500 }));
		TEST_ASSERT(root->GetMinPreferredClientSize() == Size(522, 500));
		TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 566,566 }));
		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 566,566 }));
		TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childA->GetBounds() == Rect({ 11,22 }, { 100,200 }));
		TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 222,0 }, { 300,400 }));
		TEST_ASSERT(childB->GetBounds() == Rect({ 222,22 }, { 300,400 }));

		// childB:AlignmentToParent(top)
		childB->SetAlignmentToParent(Margin(-1, 222, -1, -1));
		TEST_ASSERT(root->GetClientArea() == Rect({ 11,22 }, { 300,622 }));
		TEST_ASSERT(root->GetMinPreferredClientSize() == Size(300, 622));
		TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 344,688 }));
		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 344,688 }));
		TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childA->GetBounds() == Rect({ 11,22 }, { 100,200 }));
		TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 0,222 }, { 300,400 }));
		TEST_ASSERT(childB->GetBounds() == Rect({ 11,222 }, { 300,400 }));

		// childB:AlignmentToParent(right)
		//childB->SetAlignmentToParent(Margin(-1, -1, 222, -1));
		//TEST_ASSERT(root->GetClientArea() == Rect({ 11,22 }, { 300,500 }));
		//TEST_ASSERT(root->GetMinPreferredClientSize() == Size(300, 500));
		//TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 344,566 }));
		//TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 344,566 }));
		//TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 0,0 }, { 100,200 }));
		//TEST_ASSERT(childA->GetBounds() == Rect({ 11,22 }, { 100,200 }));
		//TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 0,0 }, { 300,400 }));
		//TEST_ASSERT(childB->GetBounds() == Rect({ 11,22 }, { 300,400 }));

		// childB:AlignmentToParent(bottom)
		//childB->SetAlignmentToParent(Margin(-1, -1, -1, 222));
		//TEST_ASSERT(root->GetClientArea() == Rect({ 11,22 }, { 300,500 }));
		//TEST_ASSERT(root->GetMinPreferredClientSize() == Size(300, 500));
		//TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 344,566 }));
		//TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 344,566 }));
		//TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 0,0 }, { 100,200 }));
		//TEST_ASSERT(childA->GetBounds() == Rect({ 11,22 }, { 100,200 }));
		//TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 0,0 }, { 300,400 }));
		//TEST_ASSERT(childB->GetBounds() == Rect({ 11,22 }, { 300,400 }));

		// childB:AlignmentToParent(all)
		childB->SetAlignmentToParent(Margin(222, 222, 222, 222));
		TEST_ASSERT(root->GetClientArea() == Rect({ 11,22 }, { 744,844 }));
		TEST_ASSERT(root->GetMinPreferredClientSize() == Size(744, 844));
		TEST_ASSERT(root->GetPreferredBounds() == Rect({ 0,0 }, { 788,910 }));
		TEST_ASSERT(root->GetBounds() == Rect({ 0,0 }, { 788,910 }));
		TEST_ASSERT(childA->GetPreferredBounds() == Rect({ 0,0 }, { 100,200 }));
		TEST_ASSERT(childA->GetBounds() == Rect({ 11,22 }, { 100,200 }));
		// TODO: unknown bug, size not the same
		TEST_ASSERT(childB->GetPreferredBounds() == Rect({ 222,222 }, { 522,622 }));
		TEST_ASSERT(childB->GetBounds() == Rect({ 222,222 }, { 300,400 }));

		SafeDeleteComposition(root);
	});

	TEST_CASE(L"Test nested <Bounds> layout")
	{
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