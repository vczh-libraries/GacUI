#include "GuiTextListControls.h"
#include "GuiListControlItemArrangers.h"
#include "../GuiButtonControls.h"
#include "../Templates/GuiThemeStyleFactory.h"
#include "../../GraphicsComposition/GuiGraphicsTableComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace elements;
			using namespace compositions;
			using namespace reflection::description;

			namespace list
			{
				const wchar_t* const ITextItemView::Identifier = L"vl::presentation::controls::list::ITextItemView";

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
					textComposition->SetOwnedElement(textElement);
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
					if (!supressEdit)
					{
						if (auto textItemView = dynamic_cast<ITextItemView*>(listControl->GetItemProvider()->RequestView(ITextItemView::Identifier)))
						{
							BeginEditListItem();
							textItemView->SetChecked(GetIndex(), bulletButton->GetSelected());
							EndEditListItem();
						}
					}
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
					if (auto textList = dynamic_cast<GuiVirtualTextList*>(listControl))
					{
						auto style = textList->GetControlTemplateObject(true)->GetCheckBulletTemplate();
						if (style) return style;
					}
					return theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::CheckTextListItem);
				}

/***********************************************************************
DefaultRadioTextListItemTemplate
***********************************************************************/

				TemplateProperty<DefaultTextListItemTemplate::BulletStyle> DefaultRadioTextListItemTemplate::CreateBulletStyle()
				{
					if (auto textList = dynamic_cast<GuiVirtualTextList*>(listControl))
					{
						auto style = textList->GetControlTemplateObject(true)->GetRadioBulletTemplate();
						if (style) return style;
					}
					return theme::GetCurrentTheme()->CreateStyle(theme::ThemeName::RadioTextListItem);
				}

/***********************************************************************
TextItem
***********************************************************************/

				TextItem::TextItem()
					:owner(0)
					, checked(false)
				{
				}

				TextItem::TextItem(const WString& _text, bool _checked)
					:owner(0)
					, text(_text)
					, checked(_checked)
				{
				}

				TextItem::~TextItem()
				{
				}

				bool TextItem::operator==(const TextItem& value)const
				{
					return text==value.text;
				}

				bool TextItem::operator!=(const TextItem& value)const
				{
					return text!=value.text;
				}

				const WString& TextItem::GetText()
				{
					return text;
				}

				void TextItem::SetText(const WString& value)
				{
					if (text != value)
					{
						text = value;
						if (owner)
						{
							vint index = owner->IndexOf(this);
							owner->InvokeOnItemModified(index, 1, 1);
						}
					}
				}

				bool TextItem::GetChecked()
				{
					return checked;
				}

				void TextItem::SetChecked(bool value)
				{
					if (checked != value)
					{
						checked = value;
						if (owner)
						{
							vint index = owner->IndexOf(this);
							owner->InvokeOnItemModified(index, 1, 1);

							GuiItemEventArgs arguments;
							arguments.itemIndex = index;
							owner->listControl->ItemChecked.Execute(arguments);
						}
					}
				}

/***********************************************************************
TextItemProvider
***********************************************************************/

				void TextItemProvider::AfterInsert(vint item, const Ptr<TextItem>& value)
				{
					ListProvider<Ptr<TextItem>>::AfterInsert(item, value);
					value->owner = this;
				}

				void TextItemProvider::BeforeRemove(vint item, const Ptr<TextItem>& value)
				{
					value->owner = 0;
					ListProvider<Ptr<TextItem>>::BeforeRemove(item, value);
				}

				WString TextItemProvider::GetTextValue(vint itemIndex)
				{
					return Get(itemIndex)->GetText();
				}

				description::Value TextItemProvider::GetBindingValue(vint itemIndex)
				{
					return Value::From(Get(itemIndex));
				}

				bool TextItemProvider::GetChecked(vint itemIndex)
				{
					return Get(itemIndex)->GetChecked();
				}

				void TextItemProvider::SetChecked(vint itemIndex, bool value)
				{
					return Get(itemIndex)->SetChecked(value);
				}

				TextItemProvider::TextItemProvider()
					:listControl(0)
				{
				}

				TextItemProvider::~TextItemProvider()
				{
				}

				IDescriptable* TextItemProvider::RequestView(const WString& identifier)
				{
					if (identifier == ITextItemView::Identifier)
					{
						return (ITextItemView*)this;
					}
					else
					{
						return nullptr;
					}
				}
			}

/***********************************************************************
GuiTextList
***********************************************************************/

			void GuiVirtualTextList::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiVirtualTextList::AfterControlTemplateInstalled_(bool initialize)
			{
			}

			void GuiVirtualTextList::OnStyleInstalled(vint itemIndex, ItemStyle* style)
			{
				GuiSelectableListControl::OnStyleInstalled(itemIndex, style);
				if (auto textItemStyle = dynamic_cast<templates::GuiTextListItemTemplate*>(style))
				{
					textItemStyle->SetTextColor(GetControlTemplateObject(true)->GetTextColor());
					if (auto textItemView = dynamic_cast<list::ITextItemView*>(itemProvider->RequestView(list::ITextItemView::Identifier)))
					{
						textItemStyle->SetChecked(textItemView->GetChecked(itemIndex));
					}
				}
			}

			void GuiVirtualTextList::OnItemTemplateChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				view = TextListView::Unknown;
			}

			GuiVirtualTextList::GuiVirtualTextList(theme::ThemeName themeName, GuiListControl::IItemProvider* _itemProvider)
				:GuiSelectableListControl(themeName, _itemProvider)
			{
				ItemTemplateChanged.AttachMethod(this, &GuiVirtualTextList::OnItemTemplateChanged);
				ItemChecked.SetAssociatedComposition(boundsComposition);

				SetView(TextListView::Text);
			}

			GuiVirtualTextList::~GuiVirtualTextList()
			{
			}

			TextListView GuiVirtualTextList::GetView()
			{
				return view;
			}

			void GuiVirtualTextList::SetView(TextListView _view)
			{
				switch (_view)
				{
				case TextListView::Text:
					SetStyleAndArranger(
						[](const Value&) { return new list::DefaultTextListItemTemplate; },
						new list::FixedHeightItemArranger
					);
					break;
				case TextListView::Check:
					SetStyleAndArranger(
						[](const Value&) { return new list::DefaultCheckTextListItemTemplate; },
						new list::FixedHeightItemArranger
					);
					break;
				case TextListView::Radio:
					SetStyleAndArranger(
						[](const Value&) { return new list::DefaultRadioTextListItemTemplate; },
						new list::FixedHeightItemArranger
					);
					break;
				default:;
				}
				view = _view;
			}

/***********************************************************************
GuiTextList
***********************************************************************/

			GuiTextList::GuiTextList(theme::ThemeName themeName)
				:GuiVirtualTextList(themeName, new list::TextItemProvider)
			{
				items=dynamic_cast<list::TextItemProvider*>(itemProvider.Obj());
				items->listControl=this;
			}

			GuiTextList::~GuiTextList()
			{
			}

			list::TextItemProvider& GuiTextList::GetItems()
			{
				return *items;
			}

			Ptr<list::TextItem> GuiTextList::GetSelectedItem()
			{
				vint index = GetSelectedItemIndex();
				if (index == -1) return 0;
				return items->Get(index);
			}
		}
	}
}