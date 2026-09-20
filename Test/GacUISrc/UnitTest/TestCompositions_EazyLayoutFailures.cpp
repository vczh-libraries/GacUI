#include "TestCompositions.h"
#include <limits>

using namespace vl::collections;
using namespace vl::presentation::compositions::eazy_layout;
using namespace easy_layout_tests;

TEST_FILE
{
	TEST_CASE(L"Splitters reject missing predecessors, final boundaries, payloads, children and reuse")
	{
		for (vint scenario = 0; scenario < 8; scenario++)
		{
			auto root = new GuiEasyLayoutComposition;
			auto marker = Ptr(new GuiEasySplitterLayout);
			if (scenario != 0) root->GetLayouts().Add(Leaf<GuiEasyTopLayout>());
			root->GetLayouts().Add(marker);
			if (scenario == 2) root->GetLayouts().Add(Ptr(new GuiEasySplitterLayout));
			if (scenario == 3) marker->SetComposition(Payload());
			if (scenario == 4) marker->GetLayouts().Add(Leaf<GuiEasyFillLayout>());
			if (scenario == 5) marker->GetLayouts().Add(marker);
			if (scenario >= 2) root->GetLayouts().Add(Leaf<GuiEasyTopLayout>());
			if (scenario == 6) root->GetLayouts().Add(marker);
			if (scenario == 7) root->GetLayouts().Add(Leaf<GuiEasyLeftLayout>());
			TEST_ERROR(root->BuildLayout());
			marker->GetLayouts().Clear();
			SafeDeleteComposition(root);
		}
		for (vint transpose = 0; transpose < 2; transpose++)
		for (vint scenario = 0; scenario < 4; scenario++)
		{
			auto root = new GuiEasyLayoutComposition;
			auto outer = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout);
			root->GetLayouts().Add(outer);
			auto inner = transpose ? Ptr<GuiEasyCellLayout>(new GuiEasyRowLayout) : Ptr<GuiEasyCellLayout>(new GuiEasyColumnLayout);
			if (scenario != 0) outer->GetLayouts().Add(inner);
			outer->GetLayouts().Add(Ptr(new GuiEasySplitterLayout));
			if (scenario == 2) outer->GetLayouts().Add(Ptr(new GuiEasySplitterLayout));
			if (scenario == 3) inner->SetCellSpan(0);
			TEST_ERROR(root->BuildLayout());
			SafeDeleteComposition(root);
		}
	});

	TEST_CASE(L"Invalid marker mutations retain the old tree and marker ownership is released on rebuild")
	{
		auto root = new GuiEasyLayoutComposition;
		auto first = Leaf<GuiEasyFillLayout>();
		auto marker = Ptr(new GuiEasySplitterLayout);
		root->GetLayouts().Add(first);
		root->GetLayouts().Add(marker);
		root->GetLayouts().Add(Leaf<GuiEasyFillLayout>());
		root->BuildLayout();
		auto oldParent = first->GetComposition()->GetParent();
		marker->GetLayouts().Add(Leaf<GuiEasyFillLayout>());
		TEST_ERROR(root->BuildLayout());
		TEST_ASSERT(first->GetComposition()->GetParent() == oldParent);
		marker->GetLayouts().Clear();
		auto other = new GuiEasyLayoutComposition;
		other->GetLayouts().Add(Leaf<GuiEasyFillLayout>());
		other->GetLayouts().Add(marker);
		other->GetLayouts().Add(Leaf<GuiEasyFillLayout>());
		TEST_ERROR(other->BuildLayout());
		root->GetLayouts().RemoveAt(1);
		root->BuildLayout();
		other->BuildLayout();
		SafeDeleteComposition(other);
		SafeDeleteComposition(root);
	});

	TEST_CASE(L"Invalid sibling subsets reject every multiplicity and declaration order")
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
			if (!invalid) continue;
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
			TEST_ERROR(root->BuildLayout());
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
			TEST_ERROR(root->BuildLayout());
			TEST_ASSERT(a->GetComposition()->GetParent() == oldParent);
		}
		a->SetPercentage(1);
		a->SetDirection(GuiEasyLayoutDirection::Horizontal);
		b->SetDirection(GuiEasyLayoutDirection::Vertical);
		TEST_ERROR(root->BuildLayout());
		b->SetDirection(GuiEasyLayoutDirection::Inherited);
		a->SetDirection(static_cast<GuiEasyLayoutDirection>(-1));
		TEST_ERROR(root->BuildLayout());
		a->SetDirection(GuiEasyLayoutDirection::Horizontal);
		root->SetPadding(-1);
		TEST_ERROR(root->BuildLayout());
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
			TEST_ERROR(grid->BuildLayout());
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
			TEST_ERROR(grid->BuildLayout());
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
			TEST_ERROR(root->BuildLayout());
			SafeDeleteComposition(root);
		}
		for (vint kind = 0; kind < 2; kind++)
		{
			auto root = new GuiEasyLayoutComposition;
			root->SetComposition(kind ? static_cast<GuiGraphicsComposition*>(new GuiCellComposition) : new GuiStackItemComposition);
			TEST_ERROR(root->BuildLayout());
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
			TEST_ERROR(root->BuildLayout());
			SafeDeleteComposition(root);
		}
		auto root = new GuiEasyLayoutComposition;
		auto descriptor = Ptr(new GuiEasyTopLayout);
		root->GetLayouts().Add(descriptor);
		root->GetLayouts().Add(descriptor);
		TEST_ERROR(root->BuildLayout());
		root->GetLayouts().RemoveAt(1);
		descriptor->GetLayouts().Add(descriptor);
		TEST_ERROR(root->BuildLayout());
		descriptor->GetLayouts().Clear();
		SafeDeleteComposition(root);
	});

}
