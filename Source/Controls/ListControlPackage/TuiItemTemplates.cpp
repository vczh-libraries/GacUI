#include "TuiItemTemplates.h"
#include "GuiDataGridControls.h"
#include "ItemTemplate_IListViewItemView.h"
#include "../../PlatformProviders/TUI/TuiApplication.h"

namespace vl::presentation::controls::list
{
	using namespace templates;
	using namespace compositions;
	using namespace elements;

	TuiListItemBackgroundTemplate* TuiGetItemBackground(GuiGraphicsComposition* item)
	{
		if (!GetTuiApplication()) return nullptr;
		for (auto parent = item->GetParent(); parent; parent = parent->GetParent())
		{
			if (auto background = dynamic_cast<TuiListItemBackgroundTemplate*>(parent)) return background;
		}
		return nullptr;
	}

	Color TuiGetItemTextColor(GuiGraphicsComposition* item, Color fallback)
	{
		if (auto background = TuiGetItemBackground(item)) return background->GetTextColor();
		return fallback;
	}

	void TuiUpdateGridCellColors(GuiGridVisualizerTemplate* cell)
	{
		if (auto background = TuiGetItemBackground(cell))
		{
			auto color = cell->GetSelected() ? background->GetSelectedTextColor() : background->GetTextColor();
			cell->SetPrimaryTextColor(color);
			cell->SetSecondaryTextColor(color);
			if (!cell->GetOwnedElement()) cell->SetOwnedElement(Ptr(GuiSolidBackgroundElement::Create()));
			if (auto element = cell->GetOwnedElement().Cast<GuiSolidBackgroundElement>())
			{
				element->SetColor(cell->GetSelected() ? background->GetSelectedBackgroundColor() : Color(0, 0, 0, 0));
			}
		}
	}

	void TuiRefreshGridColors(GuiGraphicsComposition* composition)
	{
		if (auto cell = dynamic_cast<GuiGridVisualizerTemplate*>(composition)) TuiUpdateGridCellColors(cell);
		for (auto child : composition->Children()) TuiRefreshGridColors(child);
	}

	void TuiInitializeItemBackground(GuiListItemTemplate* item, GuiSelectableButton* button)
	{
		if (auto background = dynamic_cast<TuiListItemBackgroundTemplate*>(button->GetControlTemplateObject()))
		{
			background->SetGridRow(dynamic_cast<GuiVirtualDataGrid*>(item->GetAssociatedListControl()) != nullptr);
			button->SetEnabled(item->GetVisuallyEnabled());
			item->VisuallyEnabledChanged.AttachLambda([=](GuiGraphicsComposition*, GuiEventArgs&)
			{
				button->SetEnabled(item->GetVisuallyEnabled());
			});
			auto refresh = [=](GuiGraphicsComposition*, GuiEventArgs&)
			{
				if (auto text = dynamic_cast<GuiTextListItemTemplate*>(item)) text->SetTextColor(background->GetTextColor());
				if (auto tree = dynamic_cast<GuiTreeItemTemplate*>(item)) tree->SetTextColor(background->GetTextColor());
				if (auto detail = dynamic_cast<DetailListViewItemTemplate*>(item)) detail->RefreshItem();
				if (background->GetGridRow()) TuiRefreshGridColors(item);
			};
			background->TextColorChanged.AttachLambda(refresh);
			background->SelectedTextColorChanged.AttachLambda(refresh);
			background->SelectedBackgroundColorChanged.AttachLambda(refresh);
			if (auto text = dynamic_cast<GuiTextListItemTemplate*>(item)) text->TextColorChanged.AttachLambda(refresh);
			if (auto tree = dynamic_cast<GuiTreeItemTemplate*>(item)) tree->TextColorChanged.AttachLambda(refresh);
			GuiEventArgs arguments(item);
			refresh(item, arguments);
		}
	}
}
