#include "GuiTextAutoComplete.h"
#include "../GuiTextCommonInterface.h"
#include "../../ListControlPackage/GuiTextListControls.h"
#include "../../Templates/GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;

/***********************************************************************
GuiTextBoxAutoCompleteBase::TextListControlProvider
***********************************************************************/

			GuiTextBoxAutoCompleteBase::TextListControlProvider::TextListControlProvider(TemplateProperty<templates::GuiTextListTemplate> controlTemplate)
			{
				autoCompleteList = new GuiTextList(theme::ThemeName::TextList);
				if (controlTemplate)
				{
					autoCompleteList->SetControlTemplate(controlTemplate);
				}
				autoCompleteList->SetHorizontalAlwaysVisible(false);
				autoCompleteList->SetVerticalAlwaysVisible(false);
			}

			GuiTextBoxAutoCompleteBase::TextListControlProvider::~TextListControlProvider()
			{
			}

			GuiControl* GuiTextBoxAutoCompleteBase::TextListControlProvider::GetAutoCompleteControl()
			{
				return autoCompleteList;
			}

			GuiSelectableListControl* GuiTextBoxAutoCompleteBase::TextListControlProvider::GetListControl()
			{
				return autoCompleteList;
			}

			void GuiTextBoxAutoCompleteBase::TextListControlProvider::SetSortedContent(const collections::List<AutoCompleteItem>& items)
			{
				autoCompleteList->GetItems().Clear();
				FOREACH(AutoCompleteItem, item, items)
				{
					autoCompleteList->GetItems().Add(new list::TextItem(item.text));
				}
			}

			vint GuiTextBoxAutoCompleteBase::TextListControlProvider::GetItemCount()
			{
				return autoCompleteList->GetItems().Count();
			}

			WString GuiTextBoxAutoCompleteBase::TextListControlProvider::GetItemText(vint index)
			{
				return autoCompleteList->GetItems()[index]->GetText();
			}

/***********************************************************************
GuiTextBoxAutoCompleteBase
***********************************************************************/

			bool GuiTextBoxAutoCompleteBase::IsPrefix(const WString& prefix, const WString& candidate)
			{
				if(candidate.Length()>=prefix.Length())
				{
					if(INVLOC.Compare(prefix, candidate.Sub(0, prefix.Length()), Locale::IgnoreCase)==0)
					{
						return true;
					}
				}
				return false;
			}

			GuiTextBoxAutoCompleteBase::GuiTextBoxAutoCompleteBase(Ptr<IAutoCompleteControlProvider> _autoCompleteControlProvider)
				:element(0)
				, elementModifyLock(0)
				, ownerComposition(0)
				, autoCompleteControlProvider(_autoCompleteControlProvider)
			{
				if (!autoCompleteControlProvider)
				{
					autoCompleteControlProvider = new TextListControlProvider;
				}
				autoCompleteControlProvider->GetAutoCompleteControl()->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));

				autoCompletePopup = new GuiPopup(theme::ThemeName::Menu);
				autoCompletePopup->AddChild(autoCompleteControlProvider->GetAutoCompleteControl());
			}

			GuiTextBoxAutoCompleteBase::~GuiTextBoxAutoCompleteBase()
			{
				delete autoCompletePopup;
			}

			void GuiTextBoxAutoCompleteBase::Attach(elements::GuiColorizedTextElement* _element, SpinLock& _elementModifyLock, compositions::GuiGraphicsComposition* _ownerComposition, vuint editVersion)
			{
				if(_element)
				{
					SPIN_LOCK(_elementModifyLock)
					{
						element=_element;
						elementModifyLock=&_elementModifyLock;
						ownerComposition=_ownerComposition;
					}
				}
			}

			void GuiTextBoxAutoCompleteBase::Detach()
			{
				if(element && elementModifyLock)
				{
					SPIN_LOCK(*elementModifyLock)
					{
						element=0;
						elementModifyLock=0;
					}
				}
			}

			void GuiTextBoxAutoCompleteBase::TextEditPreview(TextEditPreviewStruct& arguments)
			{
			}

			void GuiTextBoxAutoCompleteBase::TextEditNotify(const TextEditNotifyStruct& arguments)
			{
				if(element && elementModifyLock)
				{
					if(IsListOpening())
					{
						TextPos begin=GetListStartPosition();
						TextPos end=arguments.inputEnd;
						WString editingText=element->GetLines().GetText(begin, end);
						HighlightList(editingText);
					}
				}
			}

			void GuiTextBoxAutoCompleteBase::TextCaretChanged(const TextCaretChangedStruct& arguments)
			{
			}

			void GuiTextBoxAutoCompleteBase::TextEditFinished(vuint editVersion)
			{
			}

			bool GuiTextBoxAutoCompleteBase::IsListOpening()
			{
				return autoCompletePopup->GetOpening();
			}

			void GuiTextBoxAutoCompleteBase::OpenList(TextPos startPosition)
			{
				if(element && elementModifyLock)
				{
					autoCompleteStartPosition=startPosition;
					Rect bounds=element->GetLines().GetRectFromTextPos(startPosition);
					Point viewPosition=element->GetViewPosition();
					GuiControl* ownerControl=ownerComposition->GetRelatedControl();
					Rect compositionBounds=ownerComposition->GetGlobalBounds();
					Rect controlBounds=ownerControl->GetBoundsComposition()->GetGlobalBounds();
					vint px=compositionBounds.x1-controlBounds.x1-viewPosition.x;
					vint py=compositionBounds.y1-controlBounds.y1-viewPosition.y;
					bounds.x1+=px;
					bounds.x2+=px;
					bounds.y1+=py+5;
					bounds.y2+=py+5;
					autoCompletePopup->ShowPopup(ownerControl, bounds, true);
				}
			}

			void GuiTextBoxAutoCompleteBase::CloseList()
			{
				autoCompletePopup->Close();
			}

			void GuiTextBoxAutoCompleteBase::SetListContent(const collections::List<AutoCompleteItem>& items)
			{
				if(items.Count()==0)
				{
					CloseList();
				}

				List<AutoCompleteItem> sortedItems;
				CopyFrom(
					sortedItems,
					From(items)
						.OrderBy([](const AutoCompleteItem& a, const AutoCompleteItem& b)
						{
							return INVLOC.Compare(a.text, b.text, Locale::IgnoreCase);
						})
					);

				autoCompleteControlProvider->SetSortedContent(sortedItems);
				autoCompleteControlProvider->GetAutoCompleteControl()->GetBoundsComposition()->SetPreferredMinSize(Size(200, 200));
			}

			TextPos GuiTextBoxAutoCompleteBase::GetListStartPosition()
			{
				return autoCompleteStartPosition;
			}

			bool GuiTextBoxAutoCompleteBase::SelectPreviousListItem()
			{
				if(!IsListOpening()) return false;
				if(autoCompleteControlProvider->GetListControl()->GetSelectedItems().Count()==0)
				{
					autoCompleteControlProvider->GetListControl()->SetSelected(0, true);
				}
				else
				{
					vint index=autoCompleteControlProvider->GetListControl()->GetSelectedItems()[0];
					if (index > 0) index--;
					autoCompleteControlProvider->GetListControl()->SetSelected(index, true);
					autoCompleteControlProvider->GetListControl()->EnsureItemVisible(index);
				}
				return true;
			}

			bool GuiTextBoxAutoCompleteBase::SelectNextListItem()
			{
				if(!IsListOpening()) return false;
				if (autoCompleteControlProvider->GetListControl()->GetSelectedItems().Count() == 0)
				{
					autoCompleteControlProvider->GetListControl()->SetSelected(0, true);
				}
				else
				{
					vint index = autoCompleteControlProvider->GetListControl()->GetSelectedItems()[0];
					if (index < autoCompleteControlProvider->GetItemCount() - 1) index++;
					autoCompleteControlProvider->GetListControl()->SetSelected(index, true);
					autoCompleteControlProvider->GetListControl()->EnsureItemVisible(index);
				}
				return true;
			}

			bool GuiTextBoxAutoCompleteBase::ApplySelectedListItem()
			{
				if(!IsListOpening()) return false;
				if(!ownerComposition) return false;
				const auto& selectedItems = autoCompleteControlProvider->GetListControl()->GetSelectedItems();
				if (selectedItems.Count() == 0) return false;
				GuiTextBoxCommonInterface* ci=dynamic_cast<GuiTextBoxCommonInterface*>(ownerComposition->GetRelatedControl());
				if(!ci) return false;

				vint index = selectedItems[0];
				WString selectedItem = autoCompleteControlProvider->GetItemText(index);
				TextPos begin = autoCompleteStartPosition;
				TextPos end = ci->GetCaretEnd();
				ci->Select(begin, end);
				ci->SetSelectionText(selectedItem);
				CloseList();
				return true;
			}

			WString GuiTextBoxAutoCompleteBase::GetSelectedListItem()
			{
				if(!IsListOpening()) return L"";
				const auto& selectedItems = autoCompleteControlProvider->GetListControl()->GetSelectedItems();
				if (selectedItems.Count() == 0) return L"";
				vint index = selectedItems[0];
				return autoCompleteControlProvider->GetItemText(index);
			}

			void GuiTextBoxAutoCompleteBase::HighlightList(const WString& editingText)
			{
				if(IsListOpening())
				{
					vint first=0;
					vint last = autoCompleteControlProvider->GetItemCount() - 1;
					vint selected=-1;

					while (first <= last)
					{
						vint middle = (first + last) / 2;
						WString text = autoCompleteControlProvider->GetItemText(middle);
						if (IsPrefix(editingText, text))
						{
							selected = middle;
							break;
						}

						vint result = INVLOC.Compare(editingText, text, Locale::IgnoreCase);
						if (result <= 0)
						{
							last = middle - 1;
						}
						else
						{
							first = middle + 1;
						}
					}

					while(selected>0)
					{
						WString text = autoCompleteControlProvider->GetItemText(selected - 1);
						if (IsPrefix(editingText, text))
						{
							selected--;
						}
						else
						{
							break;
						}
					}

					if(selected!=-1)
					{
						autoCompleteControlProvider->GetListControl()->SetSelected(selected, true);
						autoCompleteControlProvider->GetListControl()->EnsureItemVisible(selected);
					}
				}
			}
		}
	}
}