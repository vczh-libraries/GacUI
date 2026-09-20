#include "TestCompositions.h"

using namespace vl::collections;
using namespace vl::presentation::controls;
using namespace vl::presentation::compositions::eazy_layout;

namespace easy_layout_tests
{
	GuiBoundsComposition* Payload(vint width, vint height)
	{
		auto result = new GuiBoundsComposition;
		result->SetPreferredMinSize({ width,height });
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

	TEST_CASE(L"Accepted sibling subsets, multiplicities and declaration orders have the specified minimum geometry")
	{
		for (vint mask = 0; mask < 128; mask++)
		for (vint repeat = 1; repeat <= 2; repeat++)
		for (vint reverse = 0; reverse < 2; reverse++)
		{
			bool vertical = (mask & ((1 << 0) | (1 << 1) | (1 << 4))) != 0;
			bool horizontal = (mask & ((1 << 2) | (1 << 3) | (1 << 5))) != 0;
			bool grid = (mask & ((1 << 4) | (1 << 5))) != 0;
			bool fill = (mask & (1 << 6)) != 0;
			bool invalid = (vertical && horizontal) || (grid && fill);
			if (invalid) continue;
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
			root->BuildLayout();
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

	TEST_CASE(L"Splitters preserve one-sided and mixed docking geometry without extra gutters")
	{
		for (vint transpose = 0; transpose < 2; transpose++)
		for (vint arrangement = 0; arrangement < 3; arrangement++)
		for (auto padding : { 0,5,7 })
		for (vint border = 0; border < 2; border++)
		{
			auto host = new GuiBoundsComposition;
			host->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			auto root = new GuiEasyLayoutComposition;
			host->AddChild(root);
			root->SetPadding(padding);
			root->SetBorder(border != 0);
			auto first = Descriptor((transpose ? 2 : 0) + (arrangement == 1 ? 1 : 0));
			auto last = Descriptor((transpose ? 2 : 0) + (arrangement == 0 ? 0 : 1));
			if (arrangement == 2) root->GetLayouts().Add(last);
			root->GetLayouts().Add(first);
			root->GetLayouts().Add(Ptr(new GuiEasySplitterLayout));
			if (arrangement != 2) root->GetLayouts().Add(last);
			root->BuildLayout();
			auto table = dynamic_cast<GuiTableComposition*>(root->Children()[0]->Children()[0]);
			TEST_ASSERT(table && !table->GetBorderVisible() && table->GetCellPadding() == padding);
			TEST_ASSERT(table->GetRows() == (transpose ? 1 : 2) && table->GetColumns() == (transpose ? 2 : 1));
			auto splitter = dynamic_cast<GuiTableSplitterCompositionBase*>(table->Children()[2]);
			TEST_ASSERT(splitter && splitter->GetTableParent() == table);
			if (transpose) TEST_ASSERT(dynamic_cast<GuiColumnSplitterComposition*>(splitter)->GetColumnsToTheLeft() == 1);
			else TEST_ASSERT(dynamic_cast<GuiRowSplitterComposition*>(splitter)->GetRowsToTheTop() == 1);
			vint inset = border ? padding : 0;
			host->ForceCalculateSizeImmediately();
			TEST_ASSERT(host->GetCachedMinSize() == (transpose ? Size(20 + padding + 2 * inset, 10 + 2 * inset) : Size(10 + 2 * inset, 20 + padding + 2 * inset)));
			for (vint enlarged = 0; enlarged < 2; enlarged++)
			{
				if (enlarged) host->SetExpectedBounds(Rect(0, 0, 120 + 2 * inset, 120 + 2 * inset));
				host->ForceCalculateSizeImmediately();
				vint cross = enlarged ? 120 : 10;
				vint length = enlarged ? 120 : 20 + padding;
				vint begin = arrangement == 1 ? length - 20 - padding : 0;
				vint end = arrangement == 0 ? 10 + padding : length - 10;
				TEST_ASSERT(first->GetComposition()->GetGlobalBounds() == (transpose ? Rect(inset + begin, inset, inset + begin + 10, inset + cross) : Rect(inset, inset + begin, inset + cross, inset + begin + 10)));
				TEST_ASSERT(last->GetComposition()->GetGlobalBounds() == (transpose ? Rect(inset + end, inset, inset + end + 10, inset + cross) : Rect(inset, inset + end, inset + cross, inset + end + 10)));
				TEST_ASSERT(splitter->GetGlobalBounds() == (transpose ? Rect(inset + begin + 10, inset, inset + begin + 10 + padding, inset + cross) : Rect(inset, inset + begin + 10, inset + cross, inset + begin + 10 + padding)));
			}
			SafeDeleteComposition(host);
		}
	});

	TEST_CASE(L"Splitter direction follows explicit, inherited and fallback directions without consuming empty tracks")
	{
		for (vint direction = 0; direction < 3; direction++)
		{
			auto host = new GuiBoundsComposition;
			host->SetExpectedBounds(Rect(0, 0, 406, direction == 2 ? 419 : 406));
			auto root = new GuiEasyLayoutComposition;
			root->SetBorder(false);
			root->SetPadding(3);
			host->AddChild(root);
			if (direction == 2) root->GetLayouts().Add(Leaf<GuiEasyTopLayout>());
			auto group = Ptr(new GuiEasyFillLayout);
			root->GetLayouts().Add(group);
			List<Ptr<GuiEasyFillLayout>> tracks;
			for (vint i = 0; i < 3; i++)
			{
				auto fill = Ptr(new GuiEasyFillLayout);
				if (direction == 1) fill->SetDirection(GuiEasyLayoutDirection::Vertical);
				fill->SetPercentage(i == 1 ? 2 : 1);
				if (i != 1) fill->SetComposition(Payload(10, 10));
				tracks.Add(fill);
				group->GetLayouts().Add(fill);
				if (i < 2) group->GetLayouts().Add(Ptr(new GuiEasySplitterLayout));
			}
			root->BuildLayout();
			host->ForceCalculateSizeImmediately();
			auto table = dynamic_cast<GuiTableComposition*>(CellOf(tracks[0])->GetParent());
			TEST_ASSERT(table && table->Children().Count() == 5);
			TEST_ASSERT(table->GetRows() == (direction ? 3 : 1) && table->GetColumns() == (direction ? 1 : 3));
			vint start = direction == 2 ? 13 : 0;
			TEST_ASSERT(tracks[0]->GetComposition()->GetGlobalBounds() == (direction ? Rect(0, start, 406, start + 100) : Rect(0, 0, 100, 406)));
			TEST_ASSERT(tracks[2]->GetComposition()->GetGlobalBounds() == (direction ? Rect(0, start + 306, 406, start + 406) : Rect(306, 0, 406, 406)));
			for (vint i = 0; i < 2; i++)
			{
				auto splitter = dynamic_cast<GuiTableSplitterCompositionBase*>(table->Children()[3 + i]);
				TEST_ASSERT(splitter && splitter->GetTableParent() == table);
				if (direction) TEST_ASSERT(dynamic_cast<GuiRowSplitterComposition*>(splitter)->GetRowsToTheTop() == i + 1);
				else TEST_ASSERT(dynamic_cast<GuiColumnSplitterComposition*>(splitter)->GetColumnsToTheLeft() == i + 1);
				vint offset = i == 0 ? 100 : 303;
				TEST_ASSERT(splitter->GetGlobalBounds() == (direction ? Rect(0, start + offset, 406, start + offset + 3) : Rect(offset, 0, offset + 3, 406)));
			}
			SafeDeleteComposition(host);
		}
	});

	TEST_CASE(L"Reordered trailing markers keep their predecessors and force tables only in their own group")
	{
		for (vint transpose = 0; transpose < 2; transpose++)
		{
			auto host = new GuiBoundsComposition;
			host->SetExpectedBounds(Rect(0, 0, 120, 120));
			auto root = new GuiEasyLayoutComposition;
			root->SetBorder(false);
			host->AddChild(root);
			auto group = transpose ? Ptr<GuiEasyLayout>(new GuiEasyLeftLayout) : Ptr<GuiEasyLayout>(new GuiEasyTopLayout);
			root->GetLayouts().Add(group);
			auto leading = Descriptor(transpose ? 2 : 0);
			auto trailing1 = Descriptor(transpose ? 3 : 1);
			auto trailing2 = Descriptor(transpose ? 3 : 1);
			group->GetLayouts().Add(trailing1);
			group->GetLayouts().Add(Ptr(new GuiEasySplitterLayout));
			group->GetLayouts().Add(trailing2);
			group->GetLayouts().Add(leading);
			group->GetLayouts().Add(Ptr(new GuiEasySplitterLayout));
			root->BuildLayout();
			host->ForceCalculateSizeImmediately();
			TEST_ASSERT(dynamic_cast<GuiStackComposition*>(root->Children()[0]->Children()[0]));
			auto table = dynamic_cast<GuiTableComposition*>(CellOf(leading)->GetParent());
			TEST_ASSERT(table && table->Children().Count() == 5);
			TEST_ASSERT((transpose ? table->GetColumnOption(1) : table->GetRowOption(1)).composeType == GuiCellOption::Percentage);
			for (vint i = 0; i < 3; i++)
			{
				auto leaf = i == 0 ? leading : i == 1 ? trailing1 : trailing2;
				TEST_ASSERT(leaf->GetComposition()->GetGlobalBounds() == (transpose ? Rect(i * 15, 0, i * 15 + 10, 120) : Rect(0, i * 15, 120, i * 15 + 10)));
			}
			for (vint i = 0; i < 2; i++)
			{
				auto splitter = dynamic_cast<GuiTableSplitterCompositionBase*>(table->Children()[3 + i]);
				TEST_ASSERT(splitter && splitter->GetTableParent() == table);
				TEST_ASSERT(splitter->GetGlobalBounds() == (transpose ? Rect(10 + i * 15, 0, 15 + i * 15, 120) : Rect(0, 10 + i * 15, 120, 15 + i * 15)));
			}
			SafeDeleteComposition(host);
		}
	});

	TEST_CASE(L"Shared splitter boundaries follow inner spans and outer starting tracks and coalesce")
	{
		for (vint transpose = 0; transpose < 2; transpose++)
		{
			auto root = new GuiEasyLayoutComposition;
			root->SetBorder(false);
			List<Ptr<GuiEasyCellLayout>> leaves;
			for (vint i = 0; i < 3; i++)
			{
				auto outer = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout);
				outer->SetCellOption(GuiCellOption::AbsoluteOption(30));
				if (i == 0) outer->SetCellSpan(2);
				root->GetLayouts().Add(outer);
				if (i < 2) root->GetLayouts().Add(Ptr(new GuiEasySplitterLayout));
				for (vint j = 0; j < 3; j++)
				{
					auto inner = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout);
					inner->SetComposition(Payload(10, 10));
					if (i == 2) inner->SetCellOption(GuiCellOption::AbsoluteOption(40));
					if (i == 0 && j == 0) { inner->SetCellSpan(2); j++; }
					outer->GetLayouts().Add(inner);
					leaves.Add(inner);
					if (j == 1) outer->GetLayouts().Add(Ptr(new GuiEasySplitterLayout));
				}
			}
			root->BuildLayout();
			root->ForceCalculateSizeImmediately();
			auto table = dynamic_cast<GuiTableComposition*>(root->Children()[0]->Children()[0]);
			TEST_ASSERT(table && table->GetRows() == 3 && table->GetColumns() == 3);
			TEST_ASSERT(table->Children().Count() == 11);
			TEST_ASSERT(root->GetCachedMinSize() == (transpose ? Size(100, 130) : Size(130, 100)));
			for (vint i = 0; i < 3; i++)
			{
				auto splitter = dynamic_cast<GuiTableSplitterCompositionBase*>(table->Children()[8 + i]);
				TEST_ASSERT(splitter && splitter->GetTableParent() == table);
				vint boundary = i == 0 ? 1 : 2;
				bool row = (transpose == 0) == (i < 2);
				if (row) TEST_ASSERT(dynamic_cast<GuiRowSplitterComposition*>(splitter)->GetRowsToTheTop() == boundary);
				else TEST_ASSERT(dynamic_cast<GuiColumnSplitterComposition*>(splitter)->GetColumnsToTheLeft() == boundary);
				vint offset = i < 2 ? boundary * 35 - 5 : 85;
				vint cross = i < 2 ? 130 : 100;
				TEST_ASSERT(splitter->GetGlobalBounds() == (row ? Rect(0, offset, cross, offset + 5) : Rect(offset, 0, offset + 5, cross)));
			}
			TEST_ASSERT(leaves[0]->GetComposition()->GetGlobalBounds() == Rect(0, 0, transpose ? 65 : 85, transpose ? 85 : 65));
			TEST_ASSERT(CellOf(leaves[2])->GetRow() == -1);
			TEST_ASSERT(leaves[7]->GetComposition()->GetGlobalBounds() == (transpose ? Rect(70, 90, 100, 130) : Rect(90, 70, 130, 100)));
			SafeDeleteComposition(root);
		}
	});

	TEST_CASE(L"Distinct shared markers at the same boundary coalesce even around a rejected zero-span cell")
	{
		for (vint transpose = 0; transpose < 2; transpose++)
		{
			auto root = new GuiEasyLayoutComposition;
			auto outer = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout);
			root->GetLayouts().Add(outer);
			for (vint i = 0; i < 3; i++)
			{
				auto inner = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout);
				if (i == 1) inner->SetCellSpan(0);
				outer->GetLayouts().Add(inner);
				if (i < 2) outer->GetLayouts().Add(Ptr(new GuiEasySplitterLayout));
			}
			root->BuildLayout();
			auto table = dynamic_cast<GuiTableComposition*>(root->Children()[0]->Children()[0]);
			TEST_ASSERT(table && table->Children().Count() == 4);
			TEST_ASSERT(dynamic_cast<GuiCellComposition*>(table->Children()[1])->GetRow() == -1);
			if (transpose) TEST_ASSERT(dynamic_cast<GuiRowSplitterComposition*>(table->Children()[3])->GetRowsToTheTop() == 1);
			else TEST_ASSERT(dynamic_cast<GuiColumnSplitterComposition*>(table->Children()[3])->GetColumnsToTheLeft() == 1);
			SafeDeleteComposition(root);
		}
	});

	TEST_CASE(L"Native splitter dragging preserves configured options until rebuilding and retains payload identity")
	{
		for (vint transpose = 0; transpose < 2; transpose++)
		for (vint options = 0; options < 6; options++)
		{
			vint deleted = 0;
			auto host = new GuiBoundsComposition;
			host->SetExpectedBounds(Rect(0, 0, 205, 205));
			auto root = new GuiEasyLayoutComposition;
			root->SetBorder(false);
			host->AddChild(root);
			List<Ptr<GuiEasyCellLayout>> tracks;
			for (vint i = 0; i < 2; i++)
			{
				auto outer = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout);
				outer->SetCellOption(options < 3 && ((options + 1) & (vint(1) << i)) ? GuiCellOption::AbsoluteOption(100)
					: options == 4 || (options == 5 && i == 1) ? GuiCellOption::MinSizeOption() : GuiCellOption::PercentageOption(1));
				auto inner = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout);
				if (i == 0) inner->SetCellOption(GuiCellOption::PercentageOption(1));
				auto payload = new CountBounds(deleted);
				if (outer->GetCellOption().composeType == GuiCellOption::MinSize) payload->SetPreferredMinSize(transpose ? Size(100, 0) : Size(0, 100));
				inner->SetComposition(payload);
				outer->GetLayouts().Add(inner);
				root->GetLayouts().Add(outer);
				tracks.Add(outer);
				if (i == 0) root->GetLayouts().Add(Ptr(new GuiEasySplitterLayout));
			}
			root->BuildLayout();
			host->ForceCalculateSizeImmediately();
			auto table = dynamic_cast<GuiTableComposition*>(root->Children()[0]->Children()[0]);
			auto splitter = dynamic_cast<GuiTableSplitterCompositionBase*>(table->Children()[2]);
			auto drag = [=](vint offset)
			{
				GuiMouseEventArgs args(splitter);
				args.button = NativeMouseButton::Left;
				args.left = true;
				args.ctrl = args.shift = args.middle = args.right = args.nonClient = false;
				args.wheel = 0;
				args.x = 2;
				args.y = 2;
				splitter->GetEventReceiver()->mouseDown.Execute(args);
				if (transpose) args.x += offset; else args.y += offset;
				splitter->GetEventReceiver()->mouseMove.Execute(args);
				splitter->GetEventReceiver()->mouseUp.Execute(args);
			};
			vint accumulated = 0;
			for (auto offset : { 20,-10,-1000,1000,-20 })
			{
				vint limited = offset;
				if (limited < 1 - (100 + accumulated)) limited = 1 - (100 + accumulated);
				if (limited > 100 - accumulated - 1) limited = 100 - accumulated - 1;
				if (options < 3) accumulated += limited;
				drag(offset);
				for (vint i = 0; i < 2; i++)
				{
					auto option = transpose ? table->GetColumnOption(i) : table->GetRowOption(i);
					if (tracks[i]->GetCellOption().composeType == GuiCellOption::Absolute)
					{
						TEST_ASSERT(option.absolute == 100 + (i ? -accumulated : accumulated));
						TEST_ASSERT(tracks[i]->GetCellOption().absolute == 100);
					}
					else TEST_ASSERT(option == tracks[i]->GetCellOption());
				}
			}
			auto payload = tracks[0]->GetLayouts()[0]->GetComposition();
			auto current = transpose ? table->GetColumnOption(0) : table->GetRowOption(0);
			host->SetExpectedBounds(Rect(0, 0, 305, 305));
			host->ForceCalculateSizeImmediately();
			TEST_ASSERT((transpose ? table->GetColumnOption(0) : table->GetRowOption(0)) == current);
			root->BuildLayout();
			host->ForceCalculateSizeImmediately();
			TEST_ASSERT(tracks[0]->GetLayouts()[0]->GetComposition() == payload && deleted == 0);
			table = dynamic_cast<GuiTableComposition*>(root->Children()[0]->Children()[0]);
			for (vint i = 0; i < 2; i++) TEST_ASSERT((transpose ? table->GetColumnOption(i) : table->GetRowOption(i)) == tracks[i]->GetCellOption());
			root->GetLayouts().RemoveAt(1);
			root->BuildLayout();
			TEST_ASSERT(deleted == 0 && tracks[0]->GetLayouts()[0]->GetComposition() == payload);
			SafeDeleteComposition(host);
			TEST_ASSERT(deleted == 2);
		}
	});

	TEST_CASE(L"Removing zero-aligned wrappers and folding matching insets into Border preserves visible bounds")
	{
		for (vint inset : { 0,5 })
		for (vint wrapped = 0; wrapped < 2; wrapped++)
		{
			auto host = new GuiBoundsComposition;
			host->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			auto root = new GuiEasyLayoutComposition;
			root->SetBorder(!wrapped || inset == 0);
			if (wrapped)
			{
				auto bounds = new GuiBoundsComposition;
				bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				bounds->SetAlignmentToParent({ inset,inset,inset,inset });
				host->AddChild(bounds);
				bounds->AddChild(root);
			}
			else host->AddChild(root);
			auto first = Leaf<GuiEasyFillLayout>();
			auto last = Leaf<GuiEasyFillLayout>();
			root->GetLayouts().Add(first);
			root->GetLayouts().Add(last);
			root->BuildLayout();
			host->ForceCalculateSizeImmediately();
			TEST_ASSERT(host->GetCachedMinSize() == Size(35, 20));
			TEST_ASSERT(first->GetComposition()->GetGlobalBounds() == Rect(5, 5, 15, 15));
			TEST_ASSERT(last->GetComposition()->GetGlobalBounds() == Rect(20, 5, 30, 15));
			host->SetExpectedBounds(Rect(0, 0, 115, 60));
			host->ForceCalculateSizeImmediately();
			TEST_ASSERT(first->GetComposition()->GetGlobalBounds() == Rect(5, 5, 55, 55));
			TEST_ASSERT(last->GetComposition()->GetGlobalBounds() == Rect(60, 5, 110, 55));
			SafeDeleteComposition(host);
		}
	});

	TEST_CASE(L"Tiny equal fill weights normalize to usable tracks")
	{
		auto root = new GuiEasyLayoutComposition;
		auto a = Leaf<GuiEasyFillLayout>();
		auto b = Leaf<GuiEasyFillLayout>();
		root->GetLayouts().Add(a);
		root->GetLayouts().Add(b);
		a->SetPercentage(1e-20);
		b->SetPercentage(1e-20);
		root->BuildLayout();
		root->ForceCalculateSizeImmediately();
		TEST_ASSERT(a->GetComposition()->GetCachedBounds().Width() >= 10);
		TEST_ASSERT(b->GetComposition()->GetCachedBounds().Width() >= 10);
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
