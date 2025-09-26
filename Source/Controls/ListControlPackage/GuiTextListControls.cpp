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

/***********************************************************************
GuiVirtualTextList
***********************************************************************/

			void GuiVirtualTextList::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiVirtualTextList::AfterControlTemplateInstalled_(bool initialize)
			{
			}

			void GuiVirtualTextList::OnStyleInstalled(vint itemIndex, ItemStyle* style, bool refreshPropertiesOnly)
			{
				GuiSelectableListControl::OnStyleInstalled(itemIndex, style, refreshPropertiesOnly);
				if (auto textItemStyle = dynamic_cast<templates::GuiTextListItemTemplate*>(style))
				{
					textItemStyle->SetTextColor(TypedControlTemplateObject(true)->GetTextColor());
					if (auto textItemView = dynamic_cast<list::ITextItemView*>(itemProvider->RequestView(WString::Unmanaged(list::ITextItemView::Identifier))))
					{
						textItemStyle->SetChecked(textItemView->GetChecked(itemIndex));
					}
				}
				if (refreshPropertiesOnly)
				{
					if (auto predefinedItemStyle = dynamic_cast<list::DefaultTextListItemTemplate*>(style))
					{
						predefinedItemStyle->RefreshItem();
					}
				}
			}

			void GuiVirtualTextList::OnItemTemplateChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				view = TextListView::Unknown;
			}

			GuiVirtualTextList::GuiVirtualTextList(theme::ThemeName themeName, list::IItemProvider* _itemProvider)
				:GuiSelectableListControl(themeName, _itemProvider)
			{
				ItemTemplateChanged.AttachMethod(this, &GuiVirtualTextList::OnItemTemplateChanged);
				ItemChecked.SetAssociatedComposition(boundsComposition);

				if (focusableComposition)
				{
					focusableComposition->GetEventReceiver()->keyDown.AttachMethod(this, &GuiVirtualTextList::OnKeyDown);
				}

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
						Ptr(new list::FixedHeightItemArranger)
					);
					break;
				case TextListView::Check:
					SetStyleAndArranger(
						[](const Value&) { return new list::DefaultCheckTextListItemTemplate; },
						Ptr(new list::FixedHeightItemArranger)
					);
					break;
				case TextListView::Radio:
					SetStyleAndArranger(
						[](const Value&) { return new list::DefaultRadioTextListItemTemplate; },
						Ptr(new list::FixedHeightItemArranger)
					);
					break;
				default:;
				}
				view = _view;
			}

			void GuiVirtualTextList::OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::GuiVirtualTextList::OnKeyDown(GuiGraphicsComposition*, GuiKeyEventArgs&)#"
				if (arguments.code == VKEY::KEY_SPACE && !arguments.ctrl && !arguments.shift && !arguments.alt)
				{
					const auto& selectedItems = GetSelectedItems();
					if (selectedItems.Count() > 0)
					{
						if (auto textItemView = dynamic_cast<list::ITextItemView*>(itemProvider->RequestView(WString::Unmanaged(list::ITextItemView::Identifier))))
						{
							bool hasUnchecked = false;
							for (vint i = 0; i < selectedItems.Count(); i++)
							{
								if (!textItemView->GetChecked(selectedItems[i]))
								{
									hasUnchecked = true;
									break;
								}
							}
							
							itemProvider->PushEditing();
							for (vint i = 0; i < selectedItems.Count(); i++)
							{
								vint itemIndex = selectedItems[i];
								if (textItemView->GetChecked(itemIndex) != hasUnchecked)
								{
									textItemView->SetChecked(itemIndex, hasUnchecked);
							
									GuiItemEventArgs eventArgs;
									eventArgs.itemIndex = itemIndex;
									ItemChecked.Execute(eventArgs);
								}
							}
							CHECK_ERROR(itemProvider->PopEditing(), ERROR_MESSAGE_PREFIX L"PushEditing and PopEditing calls are not paired.");
							
							arguments.handled = true;
						}
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
GuiTextList
***********************************************************************/

			void GuiTextList::OnItemCheckedChanged(vint itemIndex)
			{
				GuiItemEventArgs eventArgs;
				eventArgs.itemIndex = itemIndex;
				ItemChecked.Execute(eventArgs);
			}

			GuiTextList::GuiTextList(theme::ThemeName themeName)
				:GuiVirtualTextList(themeName, new list::TextItemProvider(this))
			{
				items=dynamic_cast<list::TextItemProvider*>(itemProvider.Obj());
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