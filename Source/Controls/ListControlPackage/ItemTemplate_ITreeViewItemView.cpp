#include "ItemTemplate_ITreeViewItemView.h"
#include "ItemProvider_ITreeViewItemView.h"
#include "../GuiButtonControls.h"
#include "../../GraphicsComposition/GuiGraphicsTableComposition.h"

namespace vl::presentation::controls::tree
{
	using namespace elements;
	using namespace compositions;
	using namespace reflection::description;

/***********************************************************************
DefaultTreeItemTemplate
***********************************************************************/

	void DefaultTreeItemTemplate::OnInitialize()
	{
		SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

		table = new GuiTableComposition;
		AddChild(table);
		table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
		table->SetRowsAndColumns(3, 4);
		table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
		table->SetRowOption(1, GuiCellOption::MinSizeOption());
		table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
		table->SetColumnOption(0, GuiCellOption::AbsoluteOption(0));
		table->SetColumnOption(1, GuiCellOption::MinSizeOption());
		table->SetColumnOption(2, GuiCellOption::MinSizeOption());
		table->SetColumnOption(3, GuiCellOption::MinSizeOption());
		table->SetAlignmentToParent(Margin(0, 0, 0, 0));
		table->SetCellPadding(2);
		{
			GuiCellComposition* cell = new GuiCellComposition;
			table->AddChild(cell);
			cell->SetSite(0, 1, 3, 1);
			cell->SetPreferredMinSize(Size(16, 16));

			expandingButton = new GuiSelectableButton(theme::ThemeName::TreeItemExpander);
			if (auto controlTemplate = dynamic_cast<templates::GuiTreeViewTemplate*>(listControl->TypedControlTemplateObject(true)))
			{
				if (auto expanderStyle = controlTemplate->GetExpandingDecoratorTemplate())
				{
					expandingButton->SetControlTemplate(expanderStyle);
				}
			}
			expandingButton->SetAutoFocus(false);
			expandingButton->SetAutoSelection(false);
			expandingButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
			expandingButton->GetBoundsComposition()->GetEventReceiver()->leftButtonDoubleClick.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandingButtonDoubleClick);
			expandingButton->Clicked.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandingButtonClicked);
			cell->AddChild(expandingButton->GetBoundsComposition());
		}
		{
			GuiCellComposition* cell = new GuiCellComposition;
			table->AddChild(cell);
			cell->SetSite(1, 2, 1, 1);
			cell->SetPreferredMinSize(Size(16, 16));

			imageElement = GuiImageFrameElement::Create();
			imageElement->SetStretch(true);
			cell->SetOwnedElement(Ptr(imageElement));
		}
		{
			GuiCellComposition* cell = new GuiCellComposition;
			table->AddChild(cell);
			cell->SetSite(0, 3, 3, 1);
			cell->SetPreferredMinSize(Size(192, 0));

			textElement = GuiSolidLabelElement::Create();
			textElement->SetAlignments(Alignment::Left, Alignment::Center);
			textElement->SetEllipse(true);
			cell->SetOwnedElement(Ptr(textElement));
		}

		FontChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnFontChanged);
		TextChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnTextChanged);
		TextColorChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnTextColorChanged);
		ExpandingChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandingChanged);
		ExpandableChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnExpandableChanged);
		LevelChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnLevelChanged);
		ImageChanged.AttachMethod(this, &DefaultTreeItemTemplate::OnImageChanged);

		FontChanged.Execute(compositions::GuiEventArgs(this));
		TextChanged.Execute(compositions::GuiEventArgs(this));
		TextColorChanged.Execute(compositions::GuiEventArgs(this));
		ExpandingChanged.Execute(compositions::GuiEventArgs(this));
		ExpandableChanged.Execute(compositions::GuiEventArgs(this));
		LevelChanged.Execute(compositions::GuiEventArgs(this));
		ImageChanged.Execute(compositions::GuiEventArgs(this));
	}

	void DefaultTreeItemTemplate::OnRefresh()
	{
	}

	void DefaultTreeItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		textElement->SetFont(GetFont());
	}

	void DefaultTreeItemTemplate::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		textElement->SetText(GetText());
	}

	void DefaultTreeItemTemplate::OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		textElement->SetColor(GetTextColor());
	}

	void DefaultTreeItemTemplate::OnExpandingChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		expandingButton->SetSelected(GetExpanding());
	}

	void DefaultTreeItemTemplate::OnExpandableChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		expandingButton->SetVisible(GetExpandable());
	}

	void DefaultTreeItemTemplate::OnLevelChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		table->SetColumnOption(0, GuiCellOption::AbsoluteOption(GetLevel() * 12));
	}

	void DefaultTreeItemTemplate::OnImageChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		if (auto imageData = GetImage())
		{
			imageElement->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
		}
		else
		{
			imageElement->SetImage(nullptr);
		}
	}

	void DefaultTreeItemTemplate::OnExpandingButtonDoubleClick(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
	{
		arguments.handled = true;
	}

	void DefaultTreeItemTemplate::OnExpandingButtonClicked(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		if (expandingButton->GetVisuallyEnabled())
		{
			if (auto view = dynamic_cast<INodeItemView*>(listControl->GetItemProvider()->RequestView(WString::Unmanaged(INodeItemView::Identifier))))
			{
				vint index = listControl->GetArranger()->GetVisibleIndex(this);
				if (index != -1)
				{
					if (auto node = view->RequestNode(index))
					{
						bool expanding = node->GetExpanding();
						node->SetExpanding(!expanding);
					}
				}
			}
		}
	}

	DefaultTreeItemTemplate::DefaultTreeItemTemplate()
	{
	}

	DefaultTreeItemTemplate::~DefaultTreeItemTemplate()
	{
	}
}