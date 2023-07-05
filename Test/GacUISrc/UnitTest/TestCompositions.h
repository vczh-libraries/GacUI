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

		void SetOwnerComposition(compositions::GuiGraphicsComposition* composition) override
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

	class FakeResponsiveComposition : public GuiResponsiveCompositionBase
	{
	protected:
		vint								level = 2;

		void UpdateMinSize()
		{
			SetPreferredMinSize(Size(10 + level * 10, 10 + level * 10));
		}
	public:
		FakeResponsiveComposition()
		{
			UpdateMinSize();
		}

		vint GetLevelCount() override		{ return 3; }
		vint GetCurrentLevel() override		{ return level; }

		bool LevelDown() override
		{
			if (level == 0) return false;
			level--;
			UpdateMinSize();
			return true;
		}

		bool LevelUp() override
		{
			if (level == 2) return false;
			level++;
			UpdateMinSize();
			return true;
		}
	};

	template<typename T>
	void TestBoundsWithTrivialChildren(void(*init)(T*) = nullptr)
	{
		TEST_CASE(L"Test <Bounds> default properties")
		{
			auto bounds = new T;
			if (init) init(bounds);

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
			TEST_ASSERT(bounds->GetInternalMargin() == Margin(0, 0, 0, 0));
			TEST_ASSERT(bounds->GetPreferredMinSize() == Size(0, 0));
			TEST_ASSERT(bounds->GetAlignmentToParent() == Margin(-1, -1, -1, -1));

			TEST_ASSERT(bounds->GetCachedBounds() == Rect(0, 0, 0, 0));
			TEST_ASSERT(bounds->GetCachedClientArea() == Rect(0, 0, 0, 0));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(0, 0));
			TEST_ASSERT(bounds->GetCachedMinClientSize() == Size(0, 0));

			SafeDeleteComposition(bounds);
		});

		TEST_CASE(L"Test single <Bounds> layout")
		{
			auto bounds = new T;
			if (init) init(bounds);

			// InternalMargin
			bounds->SetInternalMargin(Margin(11, 22, 33, 44));
			bounds->ForceCalculateSizeImmediately();

			TEST_ASSERT(bounds->GetInternalMargin() == Margin(11, 22, 33, 44));
			TEST_ASSERT(bounds->GetCachedClientArea() == Rect({ 11,22 }, { 0,0 }));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(44, 66));
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 0,0 }, { 44,66 }));

			// InternalMargin + PreferredMinSize
			bounds->SetPreferredMinSize(Size(100, 200));
			TEST_ASSERT(bounds->GetPreferredMinSize() == Size(100, 200));
			bounds->ForceCalculateSizeImmediately();

			TEST_ASSERT(bounds->GetCachedClientArea() == Rect({ 11,22 }, { 100,200 }));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(144, 266));
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 0,0 }, { 144,266 }));

			// InternalMargin + PreferredMinSize + Element(disabled)
			auto element = Ptr(new FakeElement({ 300,400 }));
			bounds->SetOwnedElement(element);
			TEST_ASSERT(bounds->GetOwnedElement() == element);
			TEST_ASSERT(element->GetOwnerComposition() == bounds);
			bounds->ForceCalculateSizeImmediately();

			TEST_ASSERT(bounds->GetCachedClientArea() == Rect({ 11,22 }, { 100,200 }));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(144, 266));
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 0,0 }, { 144,266 }));

			// InternalMargin + PreferredMinSize + Element
			bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
			TEST_ASSERT(bounds->GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElement);
			bounds->ForceCalculateSizeImmediately();

			TEST_ASSERT(bounds->GetCachedClientArea() == Rect({ 11,22 }, { 300,400 }));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(344, 466));
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 0,0 }, { 344,466 }));

			// InternalMargin + PreferredMinSize + Element
			bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			TEST_ASSERT(bounds->GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElementAndChildren);
			bounds->ForceCalculateSizeImmediately();

			TEST_ASSERT(bounds->GetCachedClientArea() == Rect({ 11,22 }, { 300,400 }));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(344, 466));
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 0,0 }, { 344,466 }));

			// InternalMargin + PreferredMinSize + Bounds(too small) + Element
			bounds->SetExpectedBounds(Rect({ 300,400 }, { 10,20 }));
			bounds->ForceCalculateSizeImmediately();

			TEST_ASSERT(bounds->GetCachedClientArea() == Rect({ 311,422 }, { 300,400 }));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(344, 466));
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 300,400 }, { 344,466 }));

			// InternalMargin + PreferredMinSize + Bounds(too small) + Element(disabled)
			bounds->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
			TEST_ASSERT(bounds->GetMinSizeLimitation() == GuiGraphicsComposition::NoLimit);
			bounds->ForceCalculateSizeImmediately();

			TEST_ASSERT(bounds->GetCachedClientArea() == Rect({ 311,422 }, { 100,200 }));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(144, 266));
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 300,400 }, { 144,266 }));

			// InternalMargin + PreferredMinSize + Bounds(just enough) + Element(disabled)
			bounds->SetExpectedBounds(Rect({ 300,400 }, { 100,200 }));
			bounds->ForceCalculateSizeImmediately();

			TEST_ASSERT(bounds->GetCachedClientArea() == Rect({ 311,422 }, { 100,200 }));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(144, 266));
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 300,400 }, { 144,266 }));

			// InternalMargin + PreferredMinSize + Bounds(just enough) + Element
			bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
			TEST_ASSERT(bounds->GetMinSizeLimitation() == GuiGraphicsComposition::LimitToElement);
			bounds->ForceCalculateSizeImmediately();

			TEST_ASSERT(bounds->GetCachedClientArea() == Rect({ 311,422 }, { 300,400 }));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(344, 466));
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 300,400 }, { 344,466 }));

			// InternalMargin + PreferredMinSize + Bounds + Element
			bounds->SetExpectedBounds(Rect({ 300,400 }, { 1000,2000 }));
			bounds->ForceCalculateSizeImmediately();

			TEST_ASSERT(bounds->GetCachedClientArea() == Rect({ 311,422 }, { 1000 - 11 - 33,2000 - 22 - 44 }));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(1000, 2000));
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 300,400 }, { 1000,2000 }));

			// InternalMargin + PreferredMinSize + Bounds + Element(disabled)
			bounds->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
			TEST_ASSERT(bounds->GetMinSizeLimitation() == GuiGraphicsComposition::NoLimit);
			bounds->ForceCalculateSizeImmediately();

			TEST_ASSERT(bounds->GetCachedClientArea() == Rect({ 311,422 }, { 1000 - 11 - 33,2000 - 22 - 44 }));
			TEST_ASSERT(bounds->GetCachedMinSize() == Size(1000, 2000));
			TEST_ASSERT(bounds->GetCachedBounds() == Rect({ 300,400 }, { 1000,2000 }));

			SafeDeleteComposition(bounds);
		});

		TEST_CASE(L"Test <Bounds> children operations")
		{
			auto root = new T;
			if (init) init(root);

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
			auto root = new T;
			if (init) init(root);

			auto childA = new GuiBoundsComposition();
			auto childB = new GuiBoundsComposition();
			root->AddChild(childA);
			root->AddChild(childB);

			childA->SetPreferredMinSize(Size(100, 200));
			childB->SetPreferredMinSize(Size(300, 400));

			// NoLimit
			root->ForceCalculateSizeImmediately();

			TEST_ASSERT(root->GetCachedClientArea() == Rect({ 0,0 }, { 0,0 }));
			TEST_ASSERT(root->GetCachedMinSize() == Size(0, 0));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 0,0 }));
			TEST_ASSERT(childA->GetCachedMinSize() == Size(100, 200));
			TEST_ASSERT(childA->GetCachedBounds() == Rect({ 0,0 }, { 100,200 }));
			TEST_ASSERT(childB->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(childB->GetCachedBounds() == Rect({ 0,0 }, { 300,400 }));

			// LimitToElement
			root->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
			root->ForceCalculateSizeImmediately();

			TEST_ASSERT(root->GetCachedClientArea() == Rect({ 0,0 }, { 0,0 }));
			TEST_ASSERT(root->GetCachedMinSize() == Size(0, 0));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 0,0 }));
			TEST_ASSERT(childA->GetCachedMinSize() == Size(100, 200));
			TEST_ASSERT(childA->GetCachedBounds() == Rect({ 0,0 }, { 100,200 }));
			TEST_ASSERT(childB->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(childB->GetCachedBounds() == Rect({ 0,0 }, { 300,400 }));

			// LimitToElementAndChildren
			root->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			root->ForceCalculateSizeImmediately();

			TEST_ASSERT(root->GetCachedClientArea() == Rect({ 0,0 }, { 300,400 }));
			TEST_ASSERT(root->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 300,400 }));
			TEST_ASSERT(childA->GetCachedMinSize() == Size(100, 200));
			TEST_ASSERT(childA->GetCachedBounds() == Rect({ 0,0 }, { 100,200 }));
			TEST_ASSERT(childB->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(childB->GetCachedBounds() == Rect({ 0,0 }, { 300,400 }));

			// Element
			root->SetOwnedElement(Ptr(new FakeElement(Size(200, 500))));
			root->ForceCalculateSizeImmediately();

			TEST_ASSERT(root->GetCachedClientArea() == Rect({ 0,0 }, { 300,500 }));
			TEST_ASSERT(root->GetCachedMinSize() == Size(300, 500));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 300,500 }));
			TEST_ASSERT(childA->GetCachedMinSize() == Size(100, 200));
			TEST_ASSERT(childA->GetCachedBounds() == Rect({ 0,0 }, { 100,200 }));
			TEST_ASSERT(childB->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(childB->GetCachedBounds() == Rect({ 0,0 }, { 300,400 }));

			// Element + InternalMargin
			root->SetInternalMargin(Margin(11, 22, 33, 44));
			root->ForceCalculateSizeImmediately();

			TEST_ASSERT(root->GetCachedClientArea() == Rect({ 11,22 }, { 300,500 }));
			TEST_ASSERT(root->GetCachedMinSize() == Size(344, 566));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 344,566 }));
			TEST_ASSERT(childA->GetCachedMinSize() == Size(100, 200));
			TEST_ASSERT(childA->GetCachedBounds() == Rect({ 11,22 }, { 100,200 }));
			TEST_ASSERT(childB->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(childB->GetCachedBounds() == Rect({ 11,22 }, { 300,400 }));

			// childA:Bounds
			childA->SetExpectedBounds(Rect(Point(222, 222), {}));
			root->ForceCalculateSizeImmediately();

			TEST_ASSERT(root->GetCachedClientArea() == Rect({ 11,22 }, { 322,500 }));
			TEST_ASSERT(root->GetCachedMinSize() == Size(366, 566));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 366,566 }));
			TEST_ASSERT(childA->GetCachedMinSize() == Size(100, 200));
			TEST_ASSERT(childA->GetCachedBounds() == Rect({ 233,244 }, { 100,200 }));
			TEST_ASSERT(childB->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(childB->GetCachedBounds() == Rect({ 11,22 }, { 300,400 }));

			// childB:AlignmentToParent(left)
			childA->SetExpectedBounds({});
			childB->SetAlignmentToParent(Margin(222, -1, -1, -1));
			root->ForceCalculateSizeImmediately();

			TEST_ASSERT(root->GetCachedClientArea() == Rect({ 11,22 }, { 511,500 }));
			TEST_ASSERT(root->GetCachedMinSize() == Size(555, 566));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 555,566 }));
			TEST_ASSERT(childA->GetCachedMinSize() == Size(100, 200));
			TEST_ASSERT(childA->GetCachedBounds() == Rect({ 11,22 }, { 100,200 }));
			TEST_ASSERT(childB->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(childB->GetCachedBounds() == Rect({ 222,22 }, { 300,400 }));

			// childB:AlignmentToParent(top)
			childB->SetAlignmentToParent(Margin(-1, 222, -1, -1));
			root->ForceCalculateSizeImmediately();

			TEST_ASSERT(root->GetCachedClientArea() == Rect({ 11,22 }, { 300,600 }));
			TEST_ASSERT(root->GetCachedMinSize() == Size(344, 666));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 344,666 }));
			TEST_ASSERT(childA->GetCachedMinSize() == Size(100, 200));
			TEST_ASSERT(childA->GetCachedBounds() == Rect({ 11,22 }, { 100,200 }));
			TEST_ASSERT(childB->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(childB->GetCachedBounds() == Rect({ 11,222 }, { 300,400 }));

			// childB:AlignmentToParent(right)
			childB->SetAlignmentToParent(Margin(-1, -1, 222, -1));
			root->ForceCalculateSizeImmediately();

			TEST_ASSERT(root->GetCachedClientArea() == Rect({ 11,22 }, { 489,500 }));
			TEST_ASSERT(root->GetCachedMinSize() == Size(533, 566));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 533,566 }));
			TEST_ASSERT(childA->GetCachedMinSize() == Size(100, 200));
			TEST_ASSERT(childA->GetCachedBounds() == Rect({ 11,22 }, { 100,200 }));
			TEST_ASSERT(childB->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(childB->GetCachedBounds() == Rect({ 11,22 }, { 300,400 }));

			// childB:AlignmentToParent(bottom)
			childB->SetAlignmentToParent(Margin(-1, -1, -1, 222));
			root->ForceCalculateSizeImmediately();

			TEST_ASSERT(root->GetCachedClientArea() == Rect({ 11,22 }, { 300,578 }));
			TEST_ASSERT(root->GetCachedMinSize() == Size(344, 644));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 344,644 }));
			TEST_ASSERT(childA->GetCachedMinSize() == Size(100, 200));
			TEST_ASSERT(childA->GetCachedBounds() == Rect({ 11,22 }, { 100,200 }));
			TEST_ASSERT(childB->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(childB->GetCachedBounds() == Rect({ 11,22 }, { 300,400 }));

			// childB:AlignmentToParent(all)
			childB->SetAlignmentToParent(Margin(222, 222, 222, 222));
			root->ForceCalculateSizeImmediately();

			TEST_ASSERT(root->GetCachedClientArea() == Rect({ 11,22 }, { 700,778 }));
			TEST_ASSERT(root->GetCachedMinSize() == Size(744, 844));
			TEST_ASSERT(root->GetCachedBounds() == Rect({ 0,0 }, { 744,844 }));
			TEST_ASSERT(childA->GetCachedMinSize() == Size(100, 200));
			TEST_ASSERT(childA->GetCachedBounds() == Rect({ 11,22 }, { 100,200 }));
			TEST_ASSERT(childB->GetCachedMinSize() == Size(300, 400));
			TEST_ASSERT(childB->GetCachedBounds() == Rect({ 222,222 }, { 300,400 }));

			SafeDeleteComposition(root);
		});

		TEST_CASE(L"Test nested <Bounds> layout")
		{
			auto root = new T;
			if (init) init(root);

			SafeDeleteComposition(root);
		});
	}
}