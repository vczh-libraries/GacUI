#include "GuiEasyLayout.h"
#include "../GuiGraphicsStackComposition.h"
#include <cmath>

namespace vl::presentation::compositions::eazy_layout
{
	using namespace collections;

	const wchar_t* const EasyPayloadProperty = L"vl::presentation::compositions::eazy_layout::Payload";

	class GuiEasyPayloadState : public Object
	{
	public:
		GuiGraphicsComposition*		composition = nullptr;
		GuiEasyLayout*				descriptor = nullptr;
		bool						attached = false;
	};

	class GuiEasyPayloadLifetime : public Object
	{
	public:
		Ptr<GuiEasyPayloadState>		state;

		GuiEasyPayloadLifetime(Ptr<GuiEasyPayloadState> _state)
			: state(_state)
		{
		}

		~GuiEasyPayloadLifetime()
		{
			// The composition may have been deleted before its easy-layout owner.
			state->composition = nullptr;
		}
	};

/***********************************************************************
GuiEasyLayout
***********************************************************************/

	GuiEasyLayout::GuiEasyLayout(Kind _kind)
		: kind(_kind)
	{
	}

	GuiEasyLayout::~GuiEasyLayout()
	{
		if (payload)
		{
			payload->descriptor = nullptr;
			if (!payload->attached)
			{
				SafeDeleteComposition(payload->composition);
			}
		}
	}

	List<Ptr<GuiEasyLayout>>& GuiEasyLayout::GetLayouts()
	{
		return layouts;
	}

	GuiGraphicsComposition* GuiEasyLayout::GetComposition()
	{
		return payload ? payload->composition : nullptr;
	}

	void GuiEasyLayout::SetComposition(GuiGraphicsComposition* value)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::compositions::eazy_layout::GuiEasyLayout::SetComposition#"
		if (GetComposition() == value) return;
		Ptr<GuiEasyPayloadState> next;
		if (value)
		{
			auto lifetime = value->GetInternalProperty(EasyPayloadProperty).Cast<GuiEasyPayloadLifetime>();
			if (lifetime)
			{
				CHECK_ERROR(!lifetime->state->descriptor, ERROR_MESSAGE_PREFIX L"The payload already belongs to another descriptor.");
				next = lifetime->state;
			}
			else
			{
				CHECK_ERROR(!value->GetParent(), ERROR_MESSAGE_PREFIX L"A new payload must be unattached.");
				next = Ptr(new GuiEasyPayloadState);
				next->composition = value;
				value->SetInternalProperty(EasyPayloadProperty, Ptr(new GuiEasyPayloadLifetime(next)));
			}
			next->descriptor = this;
		}
		if (payload)
		{
			payload->descriptor = nullptr;
			if (!payload->attached) SafeDeleteComposition(payload->composition);
		}
		payload = next;
#undef ERROR_MESSAGE_PREFIX
	}

/***********************************************************************
Descriptor properties
***********************************************************************/

	GuiEasyTopLayout::GuiEasyTopLayout() : GuiEasyLayout(Kind::Top) {}
	GuiEasyBottomLayout::GuiEasyBottomLayout() : GuiEasyLayout(Kind::Bottom) {}
	GuiEasyLeftLayout::GuiEasyLeftLayout() : GuiEasyLayout(Kind::Left) {}
	GuiEasyRightLayout::GuiEasyRightLayout() : GuiEasyLayout(Kind::Right) {}
	GuiEasyCellLayout::GuiEasyCellLayout(Kind _kind) : GuiEasyLayout(_kind) {}
	GuiCellOption GuiEasyCellLayout::GetCellOption() { return cellOption; }
	void GuiEasyCellLayout::SetCellOption(GuiCellOption value) { cellOption = value; }
	vint GuiEasyCellLayout::GetCellSpan() { return cellSpan; }
	void GuiEasyCellLayout::SetCellSpan(vint value) { cellSpan = value; }
	GuiEasyRowLayout::GuiEasyRowLayout() : GuiEasyCellLayout(Kind::Row) {}
	GuiEasyColumnLayout::GuiEasyColumnLayout() : GuiEasyCellLayout(Kind::Column) {}
	GuiEasyFillLayout::GuiEasyFillLayout() : GuiEasyLayout(Kind::Fill) {}
	double GuiEasyFillLayout::GetPercentage() { return percentage; }
	void GuiEasyFillLayout::SetPercentage(double value) { percentage = value; }
	GuiEasyLayoutDirection GuiEasyFillLayout::GetDirection() { return direction; }
	void GuiEasyFillLayout::SetDirection(GuiEasyLayoutDirection value) { direction = value; }

/***********************************************************************
GuiEasyLayoutBuilder
***********************************************************************/

	class GuiEasyLayoutBuilder
	{
		using Kind = GuiEasyLayout::Kind;

	public:
		struct Plan : Object
		{
			Ptr<GuiEasyPayloadState>		payload;
			bool						vertical = false;
			bool						stack = false;
			bool						reversed = false;
			bool						spacer = false;
			List<GuiCellOption>			tracks;
			List<GuiCellOption>			crossTracks;
			List<Ptr<Plan>>				children;
			List<Rect>					sites;
			List<bool>					insets;
			List<bool>					fullWidth;
		};

		GuiEasyLayoutComposition*		root;
		List<Ptr<GuiEasyLayout>>			descriptors;
		List<Ptr<GuiEasyPayloadState>>	payloads;

		GuiEasyLayoutBuilder(GuiEasyLayoutComposition* _root) : root(_root) {}

		void Register(Ptr<GuiEasyLayout> layout)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::compositions::eazy_layout::GuiEasyLayoutComposition::BuildLayout#"
			CHECK_ERROR(layout, ERROR_MESSAGE_PREFIX L"A descriptor cannot be null.");
			CHECK_ERROR(!descriptors.Contains(layout.Obj()), ERROR_MESSAGE_PREFIX L"Descriptor trees cannot be cyclic or shared.");
			CHECK_ERROR(!layout->owner || layout->owner == root, ERROR_MESSAGE_PREFIX L"The descriptor belongs to another layout.");
			descriptors.Add(layout);
			if (layout->payload && layout->payload->composition)
			{
				CHECK_ERROR(layout->layouts.Count() == 0, ERROR_MESSAGE_PREFIX L"Cannot mix layouts and a payload.");
				auto composition = layout->payload->composition;
				CHECK_ERROR(dynamic_cast<GuiBoundsComposition*>(composition), ERROR_MESSAGE_PREFIX L"Payloads must have independent bounds; retain the parent of a cell or stack item.");
				CHECK_ERROR(!payloads.Contains(layout->payload.Obj()), ERROR_MESSAGE_PREFIX L"Payloads cannot be shared.");
				CHECK_ERROR(!composition->GetParent() || root->builtPayloads.Contains(layout->payload.Obj()), ERROR_MESSAGE_PREFIX L"The payload belongs to another composition tree.");
				for (GuiGraphicsComposition* current = root; current; current = current->GetParent())
				{
					CHECK_ERROR(composition != current, ERROR_MESSAGE_PREFIX L"A layout cannot contain itself.");
				}
				payloads.Add(layout->payload);
			}
#undef ERROR_MESSAGE_PREFIX
		}

		static void ValidateOption(GuiCellOption option)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::compositions::eazy_layout::GuiEasyLayoutComposition::BuildLayout#"
			switch (option.composeType)
			{
			case GuiCellOption::MinSize:
				break;
			case GuiCellOption::Absolute:
				CHECK_ERROR(option.absolute >= 0, ERROR_MESSAGE_PREFIX L"Absolute sizes must be nonnegative.");
				break;
			case GuiCellOption::Percentage:
				CHECK_ERROR(std::isfinite(option.percentage) && option.percentage > 0, ERROR_MESSAGE_PREFIX L"Percentage weights must be finite and positive.");
				break;
			default:
				CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unknown cell option.");
			}
#undef ERROR_MESSAGE_PREFIX
		}

		static void Normalize(List<GuiCellOption>& options)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::compositions::eazy_layout::GuiEasyLayoutComposition::BuildLayout#"
			double maximum = 0;
			for (auto option : options)
			{
				ValidateOption(option);
				if (option.composeType == GuiCellOption::Percentage && maximum < option.percentage) maximum = option.percentage;
			}
			int exponent = 0;
			if (maximum > 0) std::frexp(maximum, &exponent);
			for (vint i = 0; i < options.Count(); i++)
			{
				auto option = options[i];
				if (option.composeType == GuiCellOption::Percentage)
				{
					CHECK_ERROR(option.percentage / maximum >= 0.001, ERROR_MESSAGE_PREFIX L"Percentage weight ratios below 0.001 are unsupported by the table.");
					// Binary scaling preserves exact ratios such as 1:3, avoiding an extra
					// truncation pixel from normalizing them to 1/3:1. The largest weight
					// becomes [1,2), so every supported weight stays above native coercion.
					option.percentage = std::scalbn(option.percentage, 1 - exponent);
					options.Set(i, option);
				}
			}
#undef ERROR_MESSAGE_PREFIX
		}

		static GuiCellOption MergeOption(GuiCellOption first, GuiCellOption second)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::compositions::eazy_layout::GuiEasyLayoutComposition::BuildLayout#"
			ValidateOption(first);
			ValidateOption(second);
			if (first.composeType == GuiCellOption::MinSize) return second;
			if (second.composeType == GuiCellOption::MinSize) return first;
			CHECK_ERROR(first.composeType == second.composeType, ERROR_MESSAGE_PREFIX L"Conflicting shared-track option kinds.");
			CHECK_ERROR(first.composeType == GuiCellOption::Absolute ? first.absolute == second.absolute : first.percentage == second.percentage,
				ERROR_MESSAGE_PREFIX L"Conflicting shared-track option values.");
			return first;
#undef ERROR_MESSAGE_PREFIX
		}

		Ptr<Plan> Prepare(Ptr<GuiEasyLayout> layout, bool inheritedVertical)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::compositions::eazy_layout::GuiEasyLayoutComposition::BuildLayout#"
			Register(layout);
			auto plan = Ptr(new Plan);
			if (layout->payload && layout->payload->composition)
			{
				plan->payload = layout->payload;
				return plan;
			}
			if (layout->layouts.Count() == 0) return plan;

			bool vertical = false, horizontal = false, fills = false, cells = false;
			Nullable<bool> explicitVertical;
			for (auto child : layout->layouts)
			{
				CHECK_ERROR(child, ERROR_MESSAGE_PREFIX L"A descriptor cannot be null.");
				switch (child->kind)
				{
				case Kind::Top: case Kind::Bottom: vertical = true; break;
				case Kind::Left: case Kind::Right: horizontal = true; break;
				case Kind::Row: vertical = true; cells = true; break;
				case Kind::Column: horizontal = true; cells = true; break;
				case Kind::Fill:
					{
						fills = true;
						auto fill = static_cast<GuiEasyFillLayout*>(child.Obj());
						ValidateOption(GuiCellOption::PercentageOption(fill->GetPercentage()));
						auto direction = fill->GetDirection();
						CHECK_ERROR(direction == GuiEasyLayoutDirection::Inherited || direction == GuiEasyLayoutDirection::Horizontal || direction == GuiEasyLayoutDirection::Vertical,
							ERROR_MESSAGE_PREFIX L"Unknown fill direction.");
						if (direction != GuiEasyLayoutDirection::Inherited)
						{
							bool value = direction == GuiEasyLayoutDirection::Vertical;
							CHECK_ERROR(!explicitVertical || explicitVertical.Value() == value, ERROR_MESSAGE_PREFIX L"Conflicting fill directions.");
							explicitVertical = value;
						}
					}
					break;
				default: CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Invalid child descriptor.");
				}
			}
			CHECK_ERROR(!(vertical && horizontal), ERROR_MESSAGE_PREFIX L"Cannot mix horizontal and vertical docking or grid tracks.");
			CHECK_ERROR(!(fills && cells), ERROR_MESSAGE_PREFIX L"Cannot mix fills and grid tracks.");
			plan->vertical = vertical || (!horizontal && (explicitVertical ? explicitVertical.Value() : inheritedVertical));
			CHECK_ERROR(!explicitVertical || explicitVertical.Value() == plan->vertical, ERROR_MESSAGE_PREFIX L"Fill direction conflicts with its arrangement.");

			auto leading = plan->vertical ? Kind::Top : Kind::Left;
			auto trailing = plan->vertical ? Kind::Bottom : Kind::Right;
			auto outer = plan->vertical ? Kind::Row : Kind::Column;
			auto inner = plan->vertical ? Kind::Column : Kind::Row;
			List<Ptr<GuiEasyLayout>> ordered;
			for (auto child : layout->layouts) if (child->kind == leading) ordered.Add(child);
			vint leadingCount = ordered.Count();
			for (auto child : layout->layouts) if (child->kind == Kind::Fill || child->kind == outer) ordered.Add(child);
			for (auto child : layout->layouts) if (child->kind == trailing) ordered.Add(child);
			plan->stack = !fills && !cells && (leadingCount == 0 || leadingCount == ordered.Count());
			plan->reversed = plan->stack && leadingCount == 0;
			plan->spacer = !fills && !cells && !plan->stack;

			Dictionary<vint, GuiCellOption> crossOptions;
			vint width = 0;
			for (auto [child, index] : indexed(ordered))
			{
				vint track = index + (plan->spacer && index >= leadingCount ? 1 : 0);
				if (plan->spacer && index == leadingCount) plan->tracks.Add(GuiCellOption::PercentageOption(1));
				if (child->kind == outer)
				{
					Register(child);
					CHECK_ERROR(!child->GetComposition(), ERROR_MESSAGE_PREFIX L"Outer rows/columns require opposite-axis track children.");
					auto outerCell = static_cast<GuiEasyCellLayout*>(child.Obj());
					plan->tracks.Add(outerCell->GetCellOption());
					vint column = 0;
					for (auto nested : child->layouts)
					{
						CHECK_ERROR(nested && nested->kind == inner, ERROR_MESSAGE_PREFIX L"Outer rows/columns only accept opposite-axis tracks.");
						auto innerCell = static_cast<GuiEasyCellLayout*>(nested.Obj());
						vint span = innerCell->GetCellSpan();
						if (span == 1 && column >= 0)
						{
							auto option = innerCell->GetCellOption();
							ValidateOption(option);
							vint found = crossOptions.Keys().IndexOf(column);
							crossOptions.Set(column, found == -1 ? option : MergeOption(crossOptions.Values()[found], option));
						}
						plan->children.Add(Prepare(nested, !plan->vertical));
						plan->sites.Add(Rect(column, track, column + span, track + outerCell->GetCellSpan()));
						plan->insets.Add(false);
						plan->fullWidth.Add(false);
						column += span;
						if (width < column) width = column;
					}
				}
				else
				{
					auto option = child->kind == Kind::Fill
						? GuiCellOption::PercentageOption(static_cast<GuiEasyFillLayout*>(child.Obj())->GetPercentage())
						: GuiCellOption::MinSizeOption();
					plan->tracks.Add(option);
					plan->children.Add(Prepare(child, plan->vertical));
					plan->sites.Add(Rect(0, track, -1, track + 1));
					plan->insets.Add(plan->spacer && index > 0);
					plan->fullWidth.Add(true);
				}
			}
			if (cells && width > 0)
			{
				CHECK_ERROR(crossOptions.Count() == width, ERROR_MESSAGE_PREFIX L"Each shared track needs a single-span declaration.");
				for (vint i = 0; i < width; i++)
				{
					CHECK_ERROR(crossOptions.Keys().Contains(i), ERROR_MESSAGE_PREFIX L"Each shared track needs a single-span declaration.");
					plan->crossTracks.Add(crossOptions[i]);
				}
			}
			else
			{
				plan->crossTracks.Add(GuiCellOption::PercentageOption(1));
			}
			Normalize(plan->tracks);
			Normalize(plan->crossTracks);
			return plan;
#undef ERROR_MESSAGE_PREFIX
		}

		void Build(Ptr<Plan> plan, GuiGraphicsComposition* parent)
		{
			if (plan->payload)
			{
				auto composition = static_cast<GuiBoundsComposition*>(plan->payload->composition);
				// A separately built easy-layout root owns its effective border alignment.
				if (!dynamic_cast<GuiEasyLayoutComposition*>(composition)) composition->SetAlignmentToParent({ 0,0,0,0 });
				parent->AddChild(composition);
				plan->payload->attached = true;
				return;
			}
			if (plan->tracks.Count() == 0) return;
			if (plan->stack)
			{
				auto stack = new GuiStackComposition;
				stack->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				stack->SetAlignmentToParent({ 0,0,0,0 });
				stack->SetExtraMargin({ 0,0,0,0 });
				stack->SetPadding(root->padding);
				stack->SetDirection(plan->vertical
					? (plan->reversed ? GuiStackComposition::ReversedVertical : GuiStackComposition::Vertical)
					: (plan->reversed ? GuiStackComposition::ReversedHorizontal : GuiStackComposition::Horizontal));
				parent->AddChild(stack);
				for (vint i = 0; i < plan->children.Count(); i++)
				{
					auto item = new GuiStackItemComposition;
					stack->AddChild(item);
					auto bounds = new GuiBoundsComposition;
					bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					bounds->SetAlignmentToParent({ 0,0,0,0 });
					item->AddChild(bounds);
					Build(plan->children[plan->reversed ? plan->children.Count() - i - 1 : i], bounds);
				}
			}
			else
			{
				auto table = new GuiTableComposition;
				table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				table->SetAlignmentToParent({ 0,0,0,0 });
				table->SetBorderVisible(false);
				table->SetCellPadding(plan->spacer ? 0 : root->padding);
				table->SetRowsAndColumns(plan->vertical ? plan->tracks.Count() : plan->crossTracks.Count(), plan->vertical ? plan->crossTracks.Count() : plan->tracks.Count());
				for (auto [option, index] : indexed(plan->tracks))
				{
					if (plan->vertical) table->SetRowOption(index, option); else table->SetColumnOption(index, option);
				}
				for (auto [option, index] : indexed(plan->crossTracks))
				{
					if (plan->vertical) table->SetColumnOption(index, option); else table->SetRowOption(index, option);
				}
				parent->AddChild(table);
				for (auto [child, index] : indexed(plan->children))
				{
					auto site = plan->sites[index];
					if (plan->fullWidth[index]) site.x2 = plan->crossTracks.Count();
					auto cell = new GuiCellComposition;
					table->AddChild(cell);
					if (plan->vertical) cell->SetSite(site.y1, site.x1, site.Height(), site.Width());
					else cell->SetSite(site.x1, site.y1, site.Width(), site.Height());
					auto bounds = new GuiBoundsComposition;
					bounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					bounds->SetAlignmentToParent(plan->insets[index]
						? (plan->vertical ? Margin(0, root->padding, 0, 0) : Margin(root->padding, 0, 0, 0))
						: Margin(0, 0, 0, 0));
					cell->AddChild(bounds);
					Build(child, bounds);
				}
			}
		}
	};

/***********************************************************************
GuiEasyLayoutComposition
***********************************************************************/

	GuiEasyLayoutComposition::GuiEasyLayoutComposition()
		: content(new GuiEasyLayout(GuiEasyLayout::Kind::Root))
	{
		SetMinSizeLimitation(LimitToElementAndChildren);
		SetAlignmentToParent({ 0,0,0,0 });
	}

	GuiEasyLayoutComposition::~GuiEasyLayoutComposition()
	{
		List<Ptr<GuiEasyLayout>> pending;
		pending.Add(content);
		for (vint i = 0; i < pending.Count(); i++)
		{
			auto layout = pending[i];
			for (auto child : layout->layouts)
			{
				if (child && !pending.Contains(child.Obj())) pending.Add(child);
			}
			if (layout->payload && !layout->payload->attached) SafeDeleteComposition(layout->payload->composition);
		}
		for (auto layout : builtLayouts) layout->owner = nullptr;
	}

	vint GuiEasyLayoutComposition::GetPadding() { return padding; }
	void GuiEasyLayoutComposition::SetPadding(vint value) { padding = value; }
	bool GuiEasyLayoutComposition::GetBorder() { return border; }
	void GuiEasyLayoutComposition::SetBorder(bool value) { border = value; }
	List<Ptr<GuiEasyLayout>>& GuiEasyLayoutComposition::GetLayouts() { return content->GetLayouts(); }
	GuiGraphicsComposition* GuiEasyLayoutComposition::GetComposition() { return content->GetComposition(); }
	void GuiEasyLayoutComposition::SetComposition(GuiGraphicsComposition* value) { content->SetComposition(value); }

	void GuiEasyLayoutComposition::BuildLayout()
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::compositions::eazy_layout::GuiEasyLayoutComposition::BuildLayout#"
		CHECK_ERROR(padding >= 0, ERROR_MESSAGE_PREFIX L"Padding must be nonnegative.");
		GuiEasyLayoutBuilder builder(this);
		auto plan = builder.Prepare(content, false);

		for (auto payload : builder.payloads)
		{
			auto composition = payload->composition;
			if (composition->GetParent()) composition->GetParent()->RemoveChild(composition);
			payload->attached = false;
		}
		if (generated) SafeDeleteComposition(generated);
		for (auto payload : builtPayloads)
		{
			if (!builder.payloads.Contains(payload.Obj()) && payload->composition) SafeDeleteComposition(payload->composition);
		}
		for (auto layout : builtLayouts) layout->owner = nullptr;
		builtLayouts = std::move(builder.descriptors);
		builtPayloads = std::move(builder.payloads);
		for (auto layout : builtLayouts) layout->owner = this;

		vint inset = border ? padding : 0;
		SetAlignmentToParent({ inset,inset,inset,inset });
		generated = nullptr;
		if (plan->payload)
		{
			builder.Build(plan, this);
		}
		else if (plan->tracks.Count() > 0)
		{
			auto bounds = new GuiBoundsComposition;
			bounds->SetMinSizeLimitation(LimitToElementAndChildren);
			bounds->SetAlignmentToParent({ 0,0,0,0 });
			AddChild(bounds);
			generated = bounds;
			builder.Build(plan, bounds);
		}
#undef ERROR_MESSAGE_PREFIX
	}
}
