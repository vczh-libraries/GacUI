#include "ItemTemplate_ITextItemView.h"
#include "ItemProvider_ITextItemView.h"
#include "../GuiButtonControls.h"
#include "../../GraphicsComposition/GuiGraphicsTableComposition.h"

namespace vl::presentation::controls::list
{
	using namespace collections;
	using namespace elements;
	using namespace compositions;
	using namespace reflection::description;

/***********************************************************************
DefaultTextListItemTemplate
***********************************************************************/

	TemplateProperty<DefaultTextListItemTemplate::BulletStyle> DefaultTextListItemTemplate::CreateBulletStyle()
	{
		return {};
	}

	void DefaultTextListItemTemplate::OnInitialize()
	{
		SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

		textElement = GuiSolidLabelElement::Create();
		textElement->SetAlignments(Alignment::Left, Alignment::Center);

		GuiBoundsComposition* textComposition = new GuiBoundsComposition;
		textComposition->SetOwnedElement(Ptr(textElement));
		textComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);

		if (auto bulletStyleController = CreateBulletStyle())
		{
			bulletButton = new GuiSelectableButton(theme::ThemeName::Unknown);
			bulletButton->SetAutoFocus(false);
			bulletButton->SetControlTemplate(bulletStyleController);
			bulletButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
			bulletButton->SelectedChanged.AttachMethod(this, &DefaultTextListItemTemplate::OnBulletSelectedChanged);

			GuiTableComposition* table = new GuiTableComposition;
			AddChild(table);
			table->SetAlignmentToParent(Margin(0, 0, 0, 0));
			table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
			table->SetRowsAndColumns(1, 2);
			table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
			table->SetColumnOption(0, GuiCellOption::MinSizeOption());
			table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
			{
				GuiCellComposition* cell = new GuiCellComposition;
				table->AddChild(cell);
				cell->SetSite(0, 0, 1, 1);
				cell->AddChild(bulletButton->GetBoundsComposition());
			}
			{
				GuiCellComposition* cell = new GuiCellComposition;
				table->AddChild(cell);
				cell->SetSite(0, 1, 1, 1);
				cell->AddChild(textComposition);
				textComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
			}
		}
		else
		{
			AddChild(textComposition);
			textComposition->SetAlignmentToParent(Margin(5, 2, 0, 2));
		}

		FontChanged.AttachMethod(this, &DefaultTextListItemTemplate::OnFontChanged);
		TextChanged.AttachMethod(this, &DefaultTextListItemTemplate::OnTextChanged);
		TextColorChanged.AttachMethod(this, &DefaultTextListItemTemplate::OnTextColorChanged);
		CheckedChanged.AttachMethod(this, &DefaultTextListItemTemplate::OnCheckedChanged);

		FontChanged.Execute(compositions::GuiEventArgs(this));
		TextChanged.Execute(compositions::GuiEventArgs(this));
		TextColorChanged.Execute(compositions::GuiEventArgs(this));
		CheckedChanged.Execute(compositions::GuiEventArgs(this));
	}

	void DefaultTextListItemTemplate::OnRefresh()
	{
	}

	void DefaultTextListItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		textElement->SetFont(GetFont());
	}

	void DefaultTextListItemTemplate::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		textElement->SetText(GetText());
	}

	void DefaultTextListItemTemplate::OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		textElement->SetColor(GetTextColor());
	}

	void DefaultTextListItemTemplate::OnCheckedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
		if (bulletButton)
		{
			supressEdit = true;
			bulletButton->SetSelected(GetChecked());
			supressEdit = false;
		}
	}

	void DefaultTextListItemTemplate::OnBulletSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::list::DefaultTextListItemTemplate::OnBulletSelectedChanged(GuiGraphicsComposition*, GuiEventArgs&)#"
		if (!supressEdit)
		{
			if (auto textItemView = dynamic_cast<ITextItemView*>(listControl->GetItemProvider()->RequestView(WString::Unmanaged(ITextItemView::Identifier))))
			{
				listControl->GetItemProvider()->PushEditing();
				textItemView->SetChecked(GetIndex(), bulletButton->GetSelected());
				CHECK_ERROR(listControl->GetItemProvider()->PopEditing(), ERROR_MESSAGE_PREFIX L"BeginEditListItem and EndEditListItem calls are not paired.");
			}
		}
#undef ERROR_MESSAGE_PREFIX
	}

	DefaultTextListItemTemplate::DefaultTextListItemTemplate()
	{
	}

	DefaultTextListItemTemplate::~DefaultTextListItemTemplate()
	{
	}

/***********************************************************************
DefaultCheckTextListItemTemplate
***********************************************************************/

	TemplateProperty<DefaultTextListItemTemplate::BulletStyle> DefaultCheckTextListItemTemplate::CreateBulletStyle()
	{
		if (auto controlTemplate = dynamic_cast<templates::GuiTextListTemplate*>(listControl->TypedControlTemplateObject(true)))
		{
			auto style = controlTemplate->GetCheckBulletTemplate();
			if (style) return style;
		}
		return theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::CheckTextListItem);
	}

/***********************************************************************
DefaultRadioTextListItemTemplate
***********************************************************************/

	TemplateProperty<DefaultTextListItemTemplate::BulletStyle> DefaultRadioTextListItemTemplate::CreateBulletStyle()
	{
		if (auto controlTemplate = dynamic_cast<templates::GuiTextListTemplate*>(listControl->TypedControlTemplateObject(true)))
		{
			auto style = controlTemplate->GetRadioBulletTemplate();
			if (style) return style;
		}
		return theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::RadioTextListItem);
	}
}