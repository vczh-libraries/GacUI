#include "GuiTextListControls.h"
#include "GuiListControlItemArrangers.h"
#include "../Styles/GuiThemeStyleFactory.h"

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

				DefaultTextListItemTemplate::BulletStyle* DefaultTextListItemTemplate::CreateBulletStyle()
				{
					return nullptr;
				}

				void DefaultTextListItemTemplate::OnInitialize()
				{
					templates::GuiListItemTemplate::OnInitialize();

					backgroundButton = new GuiSelectableButton(theme::GetCurrentTheme()->CreateListItemBackgroundStyle());
					backgroundButton->SetAutoSelection(false);
					backgroundButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					AddChild(backgroundButton->GetBoundsComposition());

					textElement = GuiSolidLabelElement::Create();
					textElement->SetAlignments(Alignment::Left, Alignment::Center);
					textElement->SetFont(backgroundButton->GetFont());

					GuiBoundsComposition* textComposition = new GuiBoundsComposition;
					textComposition->SetOwnedElement(textElement);
					textComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);

					if (auto bulletStyleController = CreateBulletStyle())
					{
						bulletButton = new GuiSelectableButton(bulletStyleController);
						bulletButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
						bulletButton->SelectedChanged.AttachMethod(this, &DefaultTextListItemTemplate::OnBulletSelectedChanged);

						GuiTableComposition* table = new GuiTableComposition;
						backgroundButton->GetContainerComposition()->AddChild(table);
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
						backgroundButton->GetContainerComposition()->AddChild(textComposition);
						textComposition->SetAlignmentToParent(Margin(5, 0, 0, 0));
					}

					SelectedChanged.AttachMethod(this, &DefaultTextListItemTemplate::OnSelectedChanged);
					IndexChanged.AttachMethod(this, &DefaultTextListItemTemplate::OnIndexChanged);
					TextColorChanged.AttachMethod(this, &DefaultTextListItemTemplate::OnTextColorChanged);

					SelectedChanged.Execute(compositions::GuiEventArgs(this));
					IndexChanged.Execute(compositions::GuiEventArgs(this));
					TextColorChanged.Execute(compositions::GuiEventArgs(this));
				}

				void DefaultTextListItemTemplate::OnSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					backgroundButton->SetSelected(GetSelected());
				}

				void DefaultTextListItemTemplate::OnIndexChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					textElement->SetText(listControl->GetItemProvider()->GetTextValue(GetIndex()));
					if (auto textItemView = dynamic_cast<ITextItemView*>(listControl->GetItemProvider()->RequestView(ITextItemView::Identifier)))
					{
						if (bulletButton)
						{
							bulletButton->SetSelected(textItemView->GetChecked(GetIndex()));
						}
					}
				}

				void DefaultTextListItemTemplate::OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					textElement->SetColor(GetTextColor());
				}

				void DefaultTextListItemTemplate::OnBulletSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					if (auto textItemView = dynamic_cast<ITextItemView*>(listControl->GetItemProvider()->RequestView(ITextItemView::Identifier)))
					{
						BeginEditListItem();
						textItemView->SetChecked(GetIndex(), bulletButton->GetSelected());
						EndEditListItem();
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

				DefaultTextListItemTemplate::BulletStyle* DefaultCheckTextListItemTemplate::CreateBulletStyle()
				{
					return theme::GetCurrentTheme()->CreateCheckTextListItemStyle();
				}

/***********************************************************************
DefaultRadioTextListItemTemplate
***********************************************************************/

				DefaultTextListItemTemplate::BulletStyle* DefaultRadioTextListItemTemplate::CreateBulletStyle()
				{
					return theme::GetCurrentTheme()->CreateRadioTextListItemStyle();
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

				void TextItemProvider::ReleaseView(IDescriptable* view)
				{
				}
			}

/***********************************************************************
GuiTextList
***********************************************************************/

			void GuiVirtualTextList::OnStyleInstalled(vint itemIndex, ItemStyle* style)
			{
				GuiSelectableListControl::OnStyleInstalled(itemIndex, style);
				if (auto textItemStyle = dynamic_cast<templates::GuiTextListItemTemplate*>(style))
				{
					textItemStyle->SetTextColor(styleProvider->GetTextColor());
				}
			}

			GuiVirtualTextList::GuiVirtualTextList(IStyleProvider* _styleProvider, GuiListControl::IItemProvider* _itemProvider)
				:GuiSelectableListControl(_styleProvider, _itemProvider)
			{
				ItemChecked.SetAssociatedComposition(boundsComposition);

				styleProvider = dynamic_cast<IStyleProvider*>(styleController->GetStyleProvider());
				SetArranger(new list::FixedHeightItemArranger);
			}

			GuiVirtualTextList::~GuiVirtualTextList()
			{
			}

			GuiVirtualTextList::IStyleProvider* GuiVirtualTextList::GetTextListStyleProvider()
			{
				return styleProvider;
			}

/***********************************************************************
GuiTextList
***********************************************************************/

			GuiTextList::GuiTextList(IStyleProvider* _styleProvider)
				:GuiVirtualTextList(_styleProvider, new list::TextItemProvider)
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