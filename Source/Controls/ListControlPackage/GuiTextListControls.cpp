#include "GuiTextListControls.h"
#include "GuiListControlItemArrangers.h"

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

/***********************************************************************
TextItemStyleProvider::TextItemStyleController
***********************************************************************/

				void TextItemStyleProvider::TextItemStyleController::OnBulletSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					textItemStyleProvider->OnStyleCheckedChanged(this);
				}

				TextItemStyleProvider::TextItemStyleController::TextItemStyleController(TextItemStyleProvider* provider)
					:ItemStyleControllerBase(provider, 0)
					,backgroundButton(0)
					,bulletButton(0)
					,textElement(0)
					,textItemStyleProvider(provider)
				{
					auto styleProvider = textItemStyleProvider->listControl->GetTextListStyleProvider();

					backgroundButton=new GuiSelectableButton(styleProvider->CreateItemBackground());
					backgroundButton->SetAutoSelection(false);
					
					textElement=GuiSolidLabelElement::Create();
					textElement->SetAlignments(Alignment::Left, Alignment::Center);
					textElement->SetFont(backgroundButton->GetFont());
					textElement->SetColor(styleProvider->GetTextColor());

					GuiBoundsComposition* textComposition=new GuiBoundsComposition;
					textComposition->SetOwnedElement(textElement);
					textComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);

					GuiSelectableButton::IStyleController* bulletStyleController=textItemStyleProvider->bulletFactory->CreateBulletStyleController();
					if(bulletStyleController)
					{
						bulletButton=new GuiSelectableButton(bulletStyleController);
						bulletButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
						bulletButton->SelectedChanged.AttachMethod(this, &TextItemStyleController::OnBulletSelectedChanged);

						GuiTableComposition* table=new GuiTableComposition;
						backgroundButton->GetContainerComposition()->AddChild(table);
						table->SetAlignmentToParent(Margin(0, 0, 0, 0));
						table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						table->SetRowsAndColumns(1, 2);
						table->SetRowOption(0, GuiCellOption::PercentageOption(1.0));
						table->SetColumnOption(0, GuiCellOption::MinSizeOption());
						table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
						{
							GuiCellComposition* cell=new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 0, 1, 1);
							cell->AddChild(bulletButton->GetBoundsComposition());
						}
						{
							GuiCellComposition* cell=new GuiCellComposition;
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
					Initialize(backgroundButton->GetBoundsComposition(), backgroundButton);
				}

				TextItemStyleProvider::TextItemStyleController::~TextItemStyleController()
				{
				}

				bool TextItemStyleProvider::TextItemStyleController::GetSelected()
				{
					return backgroundButton->GetSelected();
				}

				void TextItemStyleProvider::TextItemStyleController::SetSelected(bool value)
				{
					backgroundButton->SetSelected(value);
				}

				bool TextItemStyleProvider::TextItemStyleController::GetChecked()
				{
					return bulletButton?bulletButton->GetSelected():false;
				}

				void TextItemStyleProvider::TextItemStyleController::SetChecked(bool value)
				{
					if(bulletButton) bulletButton->SetSelected(value);
				}
				
				const WString& TextItemStyleProvider::TextItemStyleController::GetText()
				{
					return textElement->GetText();
				}

				void TextItemStyleProvider::TextItemStyleController::SetText(const WString& value)
				{
					textElement->SetText(value);
				}

/***********************************************************************
TextItemStyleProvider
***********************************************************************/

				const wchar_t* const TextItemStyleProvider::ITextItemView::Identifier = L"vl::presentation::controls::list::TextItemStyleProvider::ITextItemView";

				void TextItemStyleProvider::OnStyleCheckedChanged(TextItemStyleController* style)
				{
					vint index=listControl->GetArranger()->GetVisibleIndex(style);
					if(index!=-1)
					{
						textItemView->SetCheckedSilently(index, style->GetChecked());

						GuiItemEventArgs arguments(style->GetBoundsComposition());
						arguments.itemIndex=index;
						listControl->ItemChecked.Execute(arguments);
					}
				}

				TextItemStyleProvider::TextItemStyleProvider(IBulletFactory* _bulletFactory)
					:bulletFactory(_bulletFactory)
				{
				}

				TextItemStyleProvider::~TextItemStyleProvider()
				{
				}

				void TextItemStyleProvider::AttachListControl(GuiListControl* value)
				{
					listControl = dynamic_cast<GuiVirtualTextList*>(value);
					textItemView = dynamic_cast<ITextItemView*>(value->GetItemProvider()->RequestView(ITextItemView::Identifier));
				}

				void TextItemStyleProvider::DetachListControl()
				{
					listControl->GetItemProvider()->ReleaseView(textItemView);
					textItemView=0;
					listControl=0;
				}

				GuiListControl::IItemStyleController* TextItemStyleProvider::CreateItemStyle()
				{
					return new TextItemStyleController(this);
				}

				void TextItemStyleProvider::DestroyItemStyle(GuiListControl::IItemStyleController* style)
				{
					delete dynamic_cast<TextItemStyleController*>(style);
				}

				void TextItemStyleProvider::Install(GuiListControl::IItemStyleController* style, vint itemIndex)
				{
					TextItemStyleController* textStyle = dynamic_cast<TextItemStyleController*>(style);
					textStyle->SetText(listControl->GetItemProvider()->GetTextValue(itemIndex));
					textStyle->SetChecked(textItemView->GetChecked(itemIndex));
				}

				void TextItemStyleProvider::SetStyleIndex(GuiListControl::IItemStyleController* style, vint value)
				{
				}

				void TextItemStyleProvider::SetStyleSelected(GuiListControl::IItemStyleController* style, bool value)
				{
					TextItemStyleController* textStyle=dynamic_cast<TextItemStyleController*>(style);
					textStyle->SetSelected(value);
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
					text = value;
					if (owner)
					{
						vint index = owner->IndexOf(this);
						owner->InvokeOnItemModified(index, 1, 1);
					}
				}

				bool TextItem::GetChecked()
				{
					return checked;
				}

				void TextItem::SetChecked(bool value)
				{
					checked = value;
					if (owner)
					{
						vint index = owner->IndexOf(this);
						owner->InvokeOnItemModified(index, 1, 1);

						GuiItemEventArgs arguments;
						arguments.itemIndex=index;
						owner->listControl->ItemChecked.Execute(arguments);
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

				void TextItemProvider::SetCheckedSilently(vint itemIndex, bool value)
				{
					items[itemIndex]->checked=value;
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
					if(identifier==TextItemStyleProvider::ITextItemView::Identifier)
					{
						return (TextItemStyleProvider::ITextItemView*)this;
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

			GuiVirtualTextList::GuiVirtualTextList(IStyleProvider* _styleProvider, list::TextItemStyleProvider::IBulletFactory* _bulletFactory, GuiListControl::IItemProvider* _itemProvider)
				:GuiSelectableListControl(_styleProvider, _itemProvider)
			{
				ItemChecked.SetAssociatedComposition(boundsComposition);

				styleProvider = dynamic_cast<IStyleProvider*>(styleController->GetStyleProvider());
				ChangeItemStyle(_bulletFactory);
				SetArranger(new list::FixedHeightItemArranger);
			}

			GuiVirtualTextList::~GuiVirtualTextList()
			{
			}

			GuiVirtualTextList::IStyleProvider* GuiVirtualTextList::GetTextListStyleProvider()
			{
				return styleProvider;
			}

			Ptr<GuiListControl::IItemStyleProvider> GuiVirtualTextList::ChangeItemStyle(list::TextItemStyleProvider::IBulletFactory* bulletFactory)
			{
				if(bulletFactory)
				{
					return SetStyleProvider(new list::TextItemStyleProvider(bulletFactory));
				}
				else
				{
					return 0;
				}
			}

/***********************************************************************
GuiTextList
***********************************************************************/

			GuiTextList::GuiTextList(IStyleProvider* _styleProvider, list::TextItemStyleProvider::IBulletFactory* _bulletFactory)
				:GuiVirtualTextList(_styleProvider, _bulletFactory, new list::TextItemProvider)
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