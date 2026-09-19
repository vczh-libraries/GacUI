#include "TestCompositions.h"
#include <limits>

using namespace vl::collections;
using namespace vl::presentation::controls;
using namespace vl::presentation::compositions::eazy_layout;

namespace easy_layout_tests
{
	GuiBoundsComposition* Payload(vint width = 10, vint height = 10)
	{
		auto result = new GuiBoundsComposition;
		result->SetPreferredMinSize({ width,height });
		return result;
	}

	template<typename T>
	Ptr<T> Leaf(vint width = 10, vint height = 10)
	{
		auto result = Ptr(new T);
		result->SetComposition(Payload(width, height));
		return result;
	}

	Ptr<GuiEasyLayout> Descriptor(vint kind)
	{
		switch (kind)
		{
		case 0: return Leaf<GuiEasyTopLayout>();
		case 1: return Leaf<GuiEasyBottomLayout>();
		case 2: return Leaf<GuiEasyLeftLayout>();
		case 3: return Leaf<GuiEasyRightLayout>();
		case 4:
			{
				auto result = Ptr(new GuiEasyRowLayout);
				result->GetLayouts().Add(Leaf<GuiEasyColumnLayout>());
				return result;
			}
		case 5:
			{
				auto result = Ptr(new GuiEasyColumnLayout);
				result->GetLayouts().Add(Leaf<GuiEasyRowLayout>());
				return result;
			}
		default: return Leaf<GuiEasyFillLayout>();
		}
	}

	bool BuildFails(GuiEasyLayoutComposition* root)
	{
		try { root->BuildLayout(); }
		catch (const Error&) { return true; }
		return false;
	}

	GuiCellComposition* CellOf(Ptr<GuiEasyLayout> descriptor)
	{
		return dynamic_cast<GuiCellComposition*>(descriptor->GetComposition()->GetParent()->GetParent());
	}

	class CountBounds : public GuiBoundsComposition
	{
		vint& count;
	public:
		CountBounds(vint& _count) : count(_count) {}
		~CountBounds() { count++; }
	};

	class CountControl : public GuiControl
	{
		vint& count;
	public:
		CountControl(vint& _count) : GuiControl(theme::ThemeName::CustomControl), count(_count)
		{
			SetControlTemplate([](const Value&) { return new templates::GuiControlTemplate; });
		}
		~CountControl() { count++; }
	};
}

using namespace easy_layout_tests;

TEST_FILE
{
	TEST_CASE(L"An empty root and a direct payload build explicitly and react to payload minimum size")
	{
		vint deleted = 0;
		auto host = new GuiBoundsComposition;
		host->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		auto root = new GuiEasyLayoutComposition;
		host->AddChild(root);
		root->BuildLayout();
		TEST_ASSERT(root->Children().Count() == 0);
		auto payload = new CountBounds(deleted);
		payload->SetPreferredMinSize(Size(20, 10));
		root->SetComposition(payload);
		TEST_ASSERT(root->Children().Count() == 0);
		root->BuildLayout();
		host->ForceCalculateSizeImmediately();
		TEST_ASSERT(host->GetCachedMinSize() == Size(30, 20));
		TEST_ASSERT(payload->GetGlobalBounds() == Rect(5, 5, 25, 15));
		payload->SetPreferredMinSize(Size(40, 30));
		host->ForceCalculateSizeImmediately();
		TEST_ASSERT(host->GetCachedMinSize() == Size(50, 40));
		TEST_ASSERT(payload->GetGlobalBounds() == Rect(5, 5, 45, 35));
		root->BuildLayout();
		TEST_ASSERT(root->Children().Count() == 1 && root->Children()[0] == payload);
		SafeDeleteComposition(host);
		TEST_ASSERT(deleted == 1);
	});

	TEST_CASE(L"Every sibling subset, multiplicity and declaration order has the specified grammar and minimum geometry")
	{
		for (vint mask = 0; mask < 128; mask++)
		for (vint repeat = 1; repeat <= 2; repeat++)
		for (vint reverse = 0; reverse < 2; reverse++)
		{
			auto root = new GuiEasyLayoutComposition;
			root->SetBorder(false);
			root->SetPadding(3);
			for (vint i = 0; i < 7; i++)
			{
				vint kind = reverse ? 6 - i : i;
				if (mask & (vint(1) << kind))
				{
					for (vint j = 0; j < repeat; j++) root->GetLayouts().Add(Descriptor(kind));
				}
			}
			bool vertical = (mask & ((1 << 0) | (1 << 1) | (1 << 4))) != 0;
			bool horizontal = (mask & ((1 << 2) | (1 << 3) | (1 << 5))) != 0;
			bool grid = (mask & ((1 << 4) | (1 << 5))) != 0;
			bool fill = (mask & (1 << 6)) != 0;
			bool invalid = (vertical && horizontal) || (grid && fill);
			TEST_ASSERT(BuildFails(root) == invalid);
			if (!invalid)
			{
				root->ForceCalculateSizeImmediately();
				vint count = root->GetLayouts().Count();
				vint length = count ? count * 10 + (count - 1) * 3 : 0;
				Size expected = !count ? Size() : (vertical ? Size(10, length) : Size(length, 10));
				if (root->GetCachedMinSize() != expected)
				{
					auto actual = root->GetCachedMinSize();
					TEST_PRINT(L"mask=" + itow(mask) + L" repeat=" + itow(repeat) + L" reverse=" + itow(reverse)
						+ L" expected=" + itow(expected.x) + L"," + itow(expected.y) + L" actual=" + itow(actual.x) + L"," + itow(actual.y));
				}
				TEST_ASSERT(root->GetCachedMinSize() == expected);
				List<Ptr<GuiEasyLayout>> visual;
				for (vint group = 0; group < 3; group++)
				for (auto descriptor : root->GetLayouts())
				{
					bool leading = descriptor.Cast<GuiEasyTopLayout>() || descriptor.Cast<GuiEasyLeftLayout>();
					bool trailing = descriptor.Cast<GuiEasyBottomLayout>() || descriptor.Cast<GuiEasyRightLayout>();
					if ((leading ? 0 : trailing ? 2 : 1) == group) visual.Add(descriptor);
				}
				for (auto [descriptor, index] : indexed(visual))
				{
					auto payload = descriptor->GetComposition();
					if (!payload) payload = descriptor->GetLayouts()[0]->GetComposition();
					Point origin = vertical ? Point(0, index * 13) : Point(index * 13, 0);
					TEST_ASSERT(payload->GetGlobalBounds() == Rect(origin, { 10,10 }));
				}
			}
			SafeDeleteComposition(root);
		}
	});

	TEST_CASE(L"Docking preserves visual declaration order and a single spacer gutter at minimum and enlarged sizes")
	{
		for (vint transpose = 0; transpose < 2; transpose++)
		{
			auto root = new GuiEasyLayoutComposition;
			root->SetBorder(false);
			root->SetPadding(7);
			auto first = Descriptor(transpose ? 2 : 0);
			auto last = Descriptor(transpose ? 3 : 1);
			root->GetLayouts().Add(last);
			root->GetLayouts().Add(first);
			root->BuildLayout();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->GetCachedMinSize() == (transpose ? Size(27, 10) : Size(10, 27)));
			root->SetExpectedBounds(Rect({}, transpose ? Size(100, 30) : Size(30, 100)));
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(first->GetComposition()->GetGlobalBounds() == Rect({}, transpose ? Size(10, 30) : Size(30, 10)));
			TEST_ASSERT(last->GetComposition()->GetGlobalBounds() == (transpose ? Rect(90, 0, 100, 30) : Rect(0, 90, 30, 100)));
			SafeDeleteComposition(root);
		}
	});

	TEST_CASE(L"Fill weights, explicit direction, border and padding are stored until rebuild")
	{
		auto host = new GuiBoundsComposition;
		host->SetExpectedBounds(Rect(0, 0, 205, 105));
		auto root = new GuiEasyLayoutComposition;
		host->AddChild(root);
		auto a = Leaf<GuiEasyFillLayout>(0, 0);
		auto b = Leaf<GuiEasyFillLayout>(0, 0);
		b->SetPercentage(3);
		root->GetLayouts().Add(a);
		root->GetLayouts().Add(b);
		TEST_ASSERT(root->Children().Count() == 0);
		TEST_ASSERT(root->GetAlignmentToParent() == Margin(0, 0, 0, 0));
		root->BuildLayout();
		host->ForceCalculateSizeImmediately();
		TEST_ASSERT(a->GetComposition()->GetGlobalBounds() == Rect(5, 5, 52, 100));
		TEST_ASSERT(b->GetComposition()->GetGlobalBounds() == Rect(57, 5, 200, 100));
		a->SetDirection(GuiEasyLayoutDirection::Vertical);
		root->SetBorder(false);
		root->SetPadding(1);
		b->SetPercentage(1);
		host->ForceCalculateSizeImmediately();
		TEST_ASSERT(a->GetComposition()->GetGlobalBounds() == Rect(5, 5, 52, 100));
		root->BuildLayout();
		host->ForceCalculateSizeImmediately();
		TEST_ASSERT(a->GetComposition()->GetGlobalBounds() == Rect(0, 0, 205, 52));
		TEST_ASSERT(b->GetComposition()->GetGlobalBounds() == Rect(0, 53, 205, 105));
		root->BuildLayout();
		host->ForceCalculateSizeImmediately();
		TEST_ASSERT(b->GetComposition()->GetGlobalBounds() == Rect(0, 53, 205, 105));
		SafeDeleteComposition(host);
	});

	TEST_CASE(L"Fills inherit the nearest resolved direction through descriptor nesting")
	{
		for (vint transpose = 0; transpose < 2; transpose++)
		{
			auto root = new GuiEasyLayoutComposition;
			root->SetBorder(false);
			root->SetPadding(0);
			root->SetExpectedBounds(Rect(0, 0, 120, 120));
			auto outer = transpose ? Ptr<GuiEasyLayout>(new GuiEasyLeftLayout) : Ptr<GuiEasyLayout>(new GuiEasyTopLayout);
			auto middle = Ptr(new GuiEasyFillLayout);
			auto a = Leaf<GuiEasyFillLayout>(10, 10);
			auto b = Leaf<GuiEasyFillLayout>(10, 10);
			middle->GetLayouts().Add(a);
			middle->GetLayouts().Add(b);
			outer->GetLayouts().Add(middle);
			root->GetLayouts().Add(outer);
			root->BuildLayout();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->GetCachedMinSize() == Size(120, 120));
			TEST_ASSERT(b->GetComposition()->GetGlobalBounds() == (transpose ? Rect(10, 0, 20, 120) : Rect(0, 10, 120, 20)));
			SafeDeleteComposition(root);
		}
	});

	TEST_CASE(L"Grid option changes wait for rebuild while parent resizing uses the built tracks")
	{
		auto host = new GuiBoundsComposition;
		host->SetExpectedBounds(Rect(0, 0, 200, 100));
		auto root = new GuiEasyLayoutComposition;
		root->SetBorder(false);
		root->SetPadding(0);
		host->AddChild(root);
		List<Ptr<GuiEasyRowLayout>> rows;
		List<Ptr<GuiEasyColumnLayout>> columns;
		for (vint i = 0; i < 2; i++)
		{
			auto row = Ptr(new GuiEasyRowLayout);
			row->SetCellOption(GuiCellOption::PercentageOption(1));
			root->GetLayouts().Add(row);
			rows.Add(row);
			for (vint j = 0; j < 2; j++)
			{
				auto column = Leaf<GuiEasyColumnLayout>(0, 0);
				column->SetCellOption(GuiCellOption::PercentageOption(j ? 3 : 1));
				row->GetLayouts().Add(column);
				columns.Add(column);
			}
		}
		root->BuildLayout();
		host->ForceCalculateSizeImmediately();
		TEST_ASSERT(columns[3]->GetComposition()->GetGlobalBounds() == Rect(50, 50, 200, 100));
		rows[0]->SetCellOption(GuiCellOption::AbsoluteOption(30));
		columns[0]->SetCellOption(GuiCellOption::PercentageOption(2));
		columns[2]->SetCellOption(GuiCellOption::PercentageOption(2));
		host->SetExpectedBounds(Rect(0, 0, 240, 120));
		host->ForceCalculateSizeImmediately();
		TEST_ASSERT(columns[3]->GetComposition()->GetGlobalBounds() == Rect(60, 60, 240, 120));
		root->BuildLayout();
		host->ForceCalculateSizeImmediately();
		TEST_ASSERT(columns[3]->GetComposition()->GetGlobalBounds() == Rect(96, 30, 240, 120));
		rows[0]->SetCellSpan(2);
		host->ForceCalculateSizeImmediately();
		TEST_ASSERT(CellOf(columns[3])->GetRow() == 1);
		root->BuildLayout();
		host->ForceCalculateSizeImmediately();
		TEST_ASSERT(CellOf(columns[0])->GetRowSpan() == 2);
		TEST_ASSERT(CellOf(columns[3])->GetRow() == -1);
		SafeDeleteComposition(host);
	});

	TEST_CASE(L"Grid shared options, ragged rows and native site rejection transpose correctly")
	{
		for (vint transpose = 0; transpose < 2; transpose++)
		{
			auto root = new GuiEasyLayoutComposition;
			root->SetBorder(false);
			root->SetPadding(0);
			List<Ptr<GuiEasyCellLayout>> outer;
			List<Ptr<GuiEasyCellLayout>> leaves;
			for (vint i = 0; i < 3; i++)
			{
				auto row = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout);
				row->SetCellOption(GuiCellOption::AbsoluteOption(20));
				root->GetLayouts().Add(row);
				outer.Add(row);
				for (vint j = 0; j < (i == 2 ? 1 : 2); j++)
				{
					auto cell = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout);
					cell->SetComposition(Payload());
					if (i == 0) cell->SetCellOption(GuiCellOption::AbsoluteOption(30 + j * 10));
					row->GetLayouts().Add(cell);
					leaves.Add(cell);
				}
			}
			root->BuildLayout();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(root->GetCachedMinSize() == (transpose ? Size(60, 70) : Size(70, 60)));
			TEST_ASSERT(leaves[3]->GetComposition()->GetGlobalBounds() == (transpose ? Rect(20, 30, 40, 70) : Rect(30, 20, 70, 40)));
			outer[0]->SetCellSpan(2);
			root->BuildLayout();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(CellOf(leaves[0])->GetRow() == 0);
			TEST_ASSERT(CellOf(leaves[2])->GetRow() == -1);
			TEST_ASSERT(CellOf(leaves[2])->GetCachedBounds() == Rect());
			TEST_ASSERT(CellOf(leaves[4])->GetRow() == (transpose ? 0 : 2));
			outer[0]->SetCellSpan(1);
			leaves[4]->SetCellSpan(2);
			leaves[4]->SetCellOption(GuiCellOption::AbsoluteOption(-100));
			root->BuildLayout();
			root->ForceCalculateSizeImmediately();
			TEST_ASSERT(leaves[4]->GetComposition()->GetGlobalBounds() == (transpose ? Rect(40, 0, 60, 70) : Rect(0, 40, 70, 60)));
			for (auto span : { 0,-1,4 })
			{
				outer[2]->SetCellSpan(span);
				root->BuildLayout();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(CellOf(leaves[4])->GetRow() == -1);
				TEST_ASSERT(CellOf(leaves[4])->GetCachedBounds() == Rect());
			}
			outer[2]->SetCellSpan(1);
			for (auto span : { 0,-1 })
			{
				leaves[4]->SetCellSpan(span);
				root->BuildLayout();
				root->ForceCalculateSizeImmediately();
				TEST_ASSERT(CellOf(leaves[4])->GetRow() == -1);
				TEST_ASSERT(CellOf(leaves[4])->GetCachedBounds() == Rect());
			}
			SafeDeleteComposition(root);
		}
	});

	TEST_CASE(L"Invalid values and shared option inference fail before replacing an existing layout")
	{
		auto root = new GuiEasyLayoutComposition;
		auto a = Leaf<GuiEasyFillLayout>();
		auto b = Leaf<GuiEasyFillLayout>();
		root->GetLayouts().Add(a);
		root->GetLayouts().Add(b);
		root->BuildLayout();
		auto oldParent = a->GetComposition()->GetParent();
		for (auto weight : { 0.0,-1.0,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN(),1e-20 })
		{
			a->SetPercentage(weight);
			TEST_ASSERT(BuildFails(root));
			TEST_ASSERT(a->GetComposition()->GetParent() == oldParent);
		}
		a->SetPercentage(1e-20);
		b->SetPercentage(1e-20);
		TEST_ASSERT(!BuildFails(root));
		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(a->GetComposition()->GetCachedBounds().Width() >= 10);
		TEST_ASSERT(b->GetComposition()->GetCachedBounds().Width() >= 10);
		a->SetDirection(GuiEasyLayoutDirection::Horizontal);
		b->SetDirection(GuiEasyLayoutDirection::Vertical);
		TEST_ASSERT(BuildFails(root));
		b->SetDirection(GuiEasyLayoutDirection::Inherited);
		a->SetDirection(static_cast<GuiEasyLayoutDirection>(-1));
		TEST_ASSERT(BuildFails(root));
		a->SetDirection(GuiEasyLayoutDirection::Horizontal);
		root->SetPadding(-1);
		TEST_ASSERT(BuildFails(root));
		SafeDeleteComposition(root);

		for (vint kind = 0; kind < 4; kind++)
		{
			auto grid = new GuiEasyLayoutComposition;
			auto row = Ptr(new GuiEasyRowLayout);
			auto cell = Leaf<GuiEasyColumnLayout>();
			row->GetLayouts().Add(cell);
			grid->GetLayouts().Add(row);
			if (kind == 0) cell->SetCellSpan(2);
			if (kind == 1) cell->SetCellOption(GuiCellOption::AbsoluteOption(-1));
			if (kind >= 2)
			{
				cell->SetCellOption(GuiCellOption::AbsoluteOption(10));
				auto row2 = Ptr(new GuiEasyRowLayout);
				auto cell2 = Leaf<GuiEasyColumnLayout>();
				cell2->SetCellOption(kind == 2 ? GuiCellOption::AbsoluteOption(20) : GuiCellOption::PercentageOption(1));
				row2->GetLayouts().Add(cell2);
				grid->GetLayouts().Add(row2);
			}
			TEST_ASSERT(BuildFails(grid));
			SafeDeleteComposition(grid);
		}
		for (auto weight : { 0.0,-1.0,std::numeric_limits<double>::infinity(),std::numeric_limits<double>::quiet_NaN(),1e-20 })
		for (vint transpose = 0; transpose < 2; transpose++)
		{
			auto grid = new GuiEasyLayoutComposition;
			auto outer = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout);
			grid->GetLayouts().Add(outer);
			for (vint i = 0; i < 2; i++)
			{
				auto inner = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout);
				inner->SetCellOption(GuiCellOption::PercentageOption(i ? 1 : weight));
				outer->GetLayouts().Add(inner);
			}
			TEST_ASSERT(BuildFails(grid));
			SafeDeleteComposition(grid);
		}
	});

	TEST_CASE(L"Mixed, parent-controlled, shared, cyclic and incorrectly nested content is rejected")
	{
		for (vint kind = 0; kind < 7; kind++)
		{
			auto root = new GuiEasyLayoutComposition;
			auto descriptor = Descriptor(kind);
			root->GetLayouts().Add(descriptor);
			root->SetComposition(Payload());
			TEST_ASSERT(BuildFails(root));
			SafeDeleteComposition(root);
		}
		for (vint kind = 0; kind < 2; kind++)
		{
			auto root = new GuiEasyLayoutComposition;
			root->SetComposition(kind ? static_cast<GuiGraphicsComposition*>(new GuiCellComposition) : new GuiStackItemComposition);
			TEST_ASSERT(BuildFails(root));
			SafeDeleteComposition(root);
		}
		for (vint parent = 4; parent <= 5; parent++)
		for (vint child = 0; child < 7; child++)
		{
			if (child == 9 - parent) continue;
			auto root = new GuiEasyLayoutComposition;
			auto descriptor = Descriptor(parent);
			descriptor->GetLayouts().Clear();
			descriptor->GetLayouts().Add(Descriptor(child));
			root->GetLayouts().Add(descriptor);
			TEST_ASSERT(BuildFails(root));
			SafeDeleteComposition(root);
		}
		auto root = new GuiEasyLayoutComposition;
		auto descriptor = Ptr(new GuiEasyTopLayout);
		root->GetLayouts().Add(descriptor);
		root->GetLayouts().Add(descriptor);
		TEST_ASSERT(BuildFails(root));
		root->GetLayouts().RemoveAt(1);
		descriptor->GetLayouts().Add(descriptor);
		TEST_ASSERT(BuildFails(root));
		descriptor->GetLayouts().Clear();
		SafeDeleteComposition(root);
	});

	TEST_CASE(L"Pending, attached and rebuilt payloads have exactly one deleting owner")
	{
		for (vint built = 0; built < 2; built++)
		for (vint controlled = 0; controlled < 2; controlled++)
		{
			vint boundsDeleted = 0, controlsDeleted = 0;
			auto root = new GuiEasyLayoutComposition;
			auto descriptor = Ptr(new GuiEasyTopLayout);
			auto bounds = new CountBounds(boundsDeleted);
			auto child = new CountControl(controlsDeleted);
			auto grandchild = new CountControl(controlsDeleted);
			child->GetContainerComposition()->AddChild(grandchild->GetBoundsComposition());
			bounds->AddChild(child->GetBoundsComposition());
			descriptor->SetComposition(bounds);
			root->GetLayouts().Add(descriptor);
			if (built) root->BuildLayout();
			if (controlled)
			{
				auto owner = new GuiControl(theme::ThemeName::CustomControl);
				owner->SetControlTemplate([](const Value&) { return new templates::GuiControlTemplate; });
				owner->GetContainerComposition()->AddChild(root);
				delete owner;
			}
			else
			{
				auto owner = new GuiBoundsComposition;
				owner->AddChild(root);
				SafeDeleteComposition(owner);
			}
			TEST_ASSERT(boundsDeleted == 1);
			TEST_ASSERT(controlsDeleted == 2);
			TEST_ASSERT(descriptor->GetComposition() == nullptr);
			descriptor = nullptr;
			TEST_ASSERT(boundsDeleted == 1 && controlsDeleted == 2);
		}

		vint deleted = 0;
		auto root = new GuiEasyLayoutComposition;
		auto descriptor = Ptr(new GuiEasyTopLayout);
		auto control = new CountControl(deleted);
		control->SetText(L"Retained state");
		descriptor->SetComposition(control->GetBoundsComposition());
		root->GetLayouts().Add(descriptor);
		root->BuildLayout();
		auto fill = Ptr(new GuiEasyFillLayout);
		fill->GetLayouts().Add(descriptor);
		root->GetLayouts().Clear();
		root->GetLayouts().Add(fill);
		root->BuildLayout();
		TEST_ASSERT(deleted == 0);
		TEST_ASSERT(control->GetText() == L"Retained state");
		root->BuildLayout();
		TEST_ASSERT(deleted == 0);
		root->GetLayouts().Clear();
		root->BuildLayout();
		TEST_ASSERT(deleted == 1);
		TEST_ASSERT(descriptor->GetComposition() == nullptr);
		SafeDeleteComposition(root);
	});
}
