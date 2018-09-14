#include "GuiListViewControls.h"
#include "GuiListViewItemTemplates.h"
#include "../Templates/GuiThemeStyleFactory.h"
#include "../../GraphicsComposition/GuiGraphicsStackComposition.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace compositions;
			using namespace collections;
			using namespace reflection::description;

/***********************************************************************
GuiListViewColumnHeader
***********************************************************************/

			void GuiListViewColumnHeader::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiListViewColumnHeader::AfterControlTemplateInstalled_(bool initialize)
			{
				GetControlTemplateObject(true)->SetSortingState(columnSortingState);
			}
			
			GuiListViewColumnHeader::GuiListViewColumnHeader(theme::ThemeName themeName)
				:GuiMenuButton(themeName)
			{
			}

			GuiListViewColumnHeader::~GuiListViewColumnHeader()
			{
			}

			bool GuiListViewColumnHeader::IsAltAvailable()
			{
				return false;
			}

			ColumnSortingState GuiListViewColumnHeader::GetColumnSortingState()
			{
				return columnSortingState;
			}

			void GuiListViewColumnHeader::SetColumnSortingState(ColumnSortingState value)
			{
				if (columnSortingState != value)
				{
					columnSortingState = value;
					GetControlTemplateObject(true)->SetSortingState(columnSortingState);
				}
			}

/***********************************************************************
GuiListViewBase
***********************************************************************/

			void GuiListViewBase::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiListViewBase::AfterControlTemplateInstalled_(bool initialize)
			{
			}

			GuiListViewBase::GuiListViewBase(theme::ThemeName themeName, GuiListControl::IItemProvider* _itemProvider)
				:GuiSelectableListControl(themeName, _itemProvider)
			{
				ColumnClicked.SetAssociatedComposition(boundsComposition);
			}

			GuiListViewBase::~GuiListViewBase()
			{
			}

			namespace list
			{

				const wchar_t* const IListViewItemView::Identifier = L"vl::presentation::controls::list::IListViewItemView";
				
/***********************************************************************
ListViewColumnItemArranger::ColumnItemViewCallback
***********************************************************************/

				ListViewColumnItemArranger::ColumnItemViewCallback::ColumnItemViewCallback(ListViewColumnItemArranger* _arranger)
					:arranger(_arranger)
				{
				}

				ListViewColumnItemArranger::ColumnItemViewCallback::~ColumnItemViewCallback()
				{
				}

				void ListViewColumnItemArranger::ColumnItemViewCallback::OnColumnChanged()
				{
					arranger->RebuildColumns();
					FOREACH(ItemStyleRecord, style, arranger->visibleStyles)
					{
						if (auto callback = dynamic_cast<IColumnItemViewCallback*>(style.key))
						{
							callback->OnColumnChanged();
						}
					}
				}
				
/***********************************************************************
ListViewColumnItemArranger
***********************************************************************/

				const wchar_t* const ListViewColumnItemArranger::IColumnItemView::Identifier = L"vl::presentation::controls::list::ListViewColumnItemArranger::IColumnItemView";

				void ListViewColumnItemArranger::ColumnClicked(vint index, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					GuiItemEventArgs args(listView->ColumnClicked.GetAssociatedComposition());
					args.itemIndex=index;
					listView->ColumnClicked.Execute(args);
				}

				void ListViewColumnItemArranger::ColumnBoundsChanged(vint index, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					GuiBoundsComposition* buttonBounds=columnHeaderButtons[index]->GetBoundsComposition();
					vint size=buttonBounds->GetBounds().Width();
					if(size>columnItemView->GetColumnSize(index))
					{
						columnItemView->SetColumnSize(index, size);
					}
				}

				void ListViewColumnItemArranger::ColumnHeaderSplitterLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					if(listView->GetVisuallyEnabled())
					{
						arguments.handled=true;
						splitterDragging=true;
						splitterLatestX=arguments.x;
					}
				}

				void ListViewColumnItemArranger::ColumnHeaderSplitterLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					if(listView->GetVisuallyEnabled())
					{
						arguments.handled=true;
						splitterDragging=false;
						splitterLatestX=0;
					}
				}

				void ListViewColumnItemArranger::ColumnHeaderSplitterMouseMove(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					if(splitterDragging)
					{
						vint offset=arguments.x-splitterLatestX;
						vint index=columnHeaderSplitters.IndexOf(dynamic_cast<GuiBoundsComposition*>(sender));
						if(index!=-1)
						{
							GuiBoundsComposition* buttonBounds=columnHeaderButtons[index]->GetBoundsComposition();
							Rect bounds=buttonBounds->GetBounds();
							Rect newBounds(bounds.LeftTop(), Size(bounds.Width()+offset, bounds.Height()));
							buttonBounds->SetBounds(newBounds);

							vint finalSize=buttonBounds->GetBounds().Width();
							columnItemView->SetColumnSize(index, finalSize);
						}
					}
				}

				void ListViewColumnItemArranger::RearrangeItemBounds()
				{
					FixedHeightItemArranger::RearrangeItemBounds();
					vint count = columnHeaders->GetParent()->Children().Count();
					columnHeaders->GetParent()->MoveChild(columnHeaders, count - 1);
					columnHeaders->SetBounds(Rect(Point(-viewBounds.Left(), 0), Size(0, 0)));
				}

				vint ListViewColumnItemArranger::GetWidth()
				{
					vint width=columnHeaders->GetBounds().Width()-SplitterWidth;
					if(width<SplitterWidth)
					{
						width=SplitterWidth;
					}
					return width;
				}

				vint ListViewColumnItemArranger::GetYOffset()
				{
					return columnHeaders->GetBounds().Height();
				}

				Size ListViewColumnItemArranger::OnCalculateTotalSize()
				{
					Size size=FixedHeightItemArranger::OnCalculateTotalSize();
					size.x+=SplitterWidth;
					return size;
				}

				void ListViewColumnItemArranger::DeleteColumnButtons()
				{
					for(vint i=columnHeaders->GetStackItems().Count()-1;i>=0;i--)
					{
						GuiStackItemComposition* item=columnHeaders->GetStackItems().Get(i);
						columnHeaders->RemoveChild(item);

						GuiControl* button=item->Children().Get(0)->GetAssociatedControl();
						if(button)
						{
							item->RemoveChild(button->GetBoundsComposition());
							delete button;
						}
						delete item;
					}
					columnHeaderButtons.Clear();
					columnHeaderSplitters.Clear();
				}

				void ListViewColumnItemArranger::RebuildColumns()
				{
					if (columnItemView && columnHeaderButtons.Count() == listViewItemView->GetColumnCount())
					{
						for (vint i = 0; i < listViewItemView->GetColumnCount(); i++)
						{
							GuiListViewColumnHeader* button = columnHeaderButtons[i];
							button->SetText(listViewItemView->GetColumnText(i));
							button->SetSubMenu(columnItemView->GetDropdownPopup(i), false);
							button->SetColumnSortingState(columnItemView->GetSortingState(i));
							button->GetBoundsComposition()->SetBounds(Rect(Point(0, 0), Size(columnItemView->GetColumnSize(i), 0)));
						}
					}
					else
					{
						DeleteColumnButtons();
						if (columnItemView && listViewItemView)
						{
							for (vint i = 0; i < listViewItemView->GetColumnCount(); i++)
							{
								GuiBoundsComposition* splitterComposition = new GuiBoundsComposition;
								splitterComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
								splitterComposition->SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::SizeWE));
								splitterComposition->SetAlignmentToParent(Margin(0, 0, -1, 0));
								splitterComposition->SetPreferredMinSize(Size(SplitterWidth, 0));
								columnHeaderSplitters.Add(splitterComposition);

								splitterComposition->GetEventReceiver()->leftButtonDown.AttachMethod(this, &ListViewColumnItemArranger::ColumnHeaderSplitterLeftButtonDown);
								splitterComposition->GetEventReceiver()->leftButtonUp.AttachMethod(this, &ListViewColumnItemArranger::ColumnHeaderSplitterLeftButtonUp);
								splitterComposition->GetEventReceiver()->mouseMove.AttachMethod(this, &ListViewColumnItemArranger::ColumnHeaderSplitterMouseMove);
							}
							for (vint i = 0; i < listViewItemView->GetColumnCount(); i++)
							{
								GuiListViewColumnHeader* button = new GuiListViewColumnHeader(theme::ThemeName::Unknown);
								button->SetAutoFocus(false);
								button->SetControlTemplate(listView->GetControlTemplateObject(true)->GetColumnHeaderTemplate());
								button->SetText(listViewItemView->GetColumnText(i));
								button->SetSubMenu(columnItemView->GetDropdownPopup(i), false);
								button->SetColumnSortingState(columnItemView->GetSortingState(i));
								button->GetBoundsComposition()->SetBounds(Rect(Point(0, 0), Size(columnItemView->GetColumnSize(i), 0)));
								button->Clicked.AttachLambda(Curry(Func<void(vint, GuiGraphicsComposition*, GuiEventArgs&)>(this, &ListViewColumnItemArranger::ColumnClicked))(i));
								button->GetBoundsComposition()->BoundsChanged.AttachLambda(Curry(Func<void(vint, GuiGraphicsComposition*, GuiEventArgs&)>(this, &ListViewColumnItemArranger::ColumnBoundsChanged))(i));
								columnHeaderButtons.Add(button);
								if (i > 0)
								{
									button->GetContainerComposition()->AddChild(columnHeaderSplitters[i - 1]);
								}

								GuiStackItemComposition* item = new GuiStackItemComposition;
								item->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
								item->AddChild(button->GetBoundsComposition());
								columnHeaders->AddChild(item);
							}
							if (listViewItemView->GetColumnCount() > 0)
							{
								GuiBoundsComposition* splitterComposition = columnHeaderSplitters[listViewItemView->GetColumnCount() - 1];

								GuiStackItemComposition* item = new GuiStackItemComposition;
								item->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
								item->AddChild(splitterComposition);
								columnHeaders->AddChild(item);
							}
						}
					}
					callback->OnTotalSizeChanged();
				}

				ListViewColumnItemArranger::ListViewColumnItemArranger()
				{
					columnHeaders = new GuiStackComposition;
					columnHeaders->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					columnItemViewCallback = new ColumnItemViewCallback(this);
				}

				ListViewColumnItemArranger::~ListViewColumnItemArranger()
				{
					if(!columnHeaders->GetParent())
					{
						DeleteColumnButtons();
						delete columnHeaders;
					}
				}

				void ListViewColumnItemArranger::AttachListControl(GuiListControl* value)
				{
					FixedHeightItemArranger::AttachListControl(value);
					listView = dynamic_cast<GuiListViewBase*>(value);
					if (listView)
					{
						listViewItemView = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(IListViewItemView::Identifier));
						columnItemView = dynamic_cast<IColumnItemView*>(listView->GetItemProvider()->RequestView(IColumnItemView::Identifier));
						listView->GetContainerComposition()->AddChild(columnHeaders);
						if (columnItemView)
						{
							columnItemView->AttachCallback(columnItemViewCallback.Obj());
							RebuildColumns();
						}
					}
				}

				void ListViewColumnItemArranger::DetachListControl()
				{
					if (listView)
					{
						if (columnItemView)
						{
							columnItemView->DetachCallback(columnItemViewCallback.Obj());
							columnItemView = nullptr;
						}
						listViewItemView = nullptr;
						listView->GetContainerComposition()->RemoveChild(columnHeaders);
						listView = nullptr;
					}
					FixedHeightItemArranger::DetachListControl();
				}

/***********************************************************************
ListViewSubItems
***********************************************************************/

				void ListViewSubItems::NotifyUpdateInternal(vint start, vint count, vint newCount)
				{
					owner->NotifyUpdate();
				}

/***********************************************************************
ListViewItem
***********************************************************************/

				void ListViewItem::NotifyUpdate()
				{
					if (owner)
					{
						vint index = owner->IndexOf(this);
						owner->NotifyUpdateInternal(index, 1, 1);
					}
				}

				ListViewItem::ListViewItem()
					:owner(0)
				{
					subItems.owner = this;
				}

				ListViewSubItems& ListViewItem::GetSubItems()
				{
					return subItems;
				}

				Ptr<GuiImageData> ListViewItem::GetSmallImage()
				{
					return smallImage;
				}

				void ListViewItem::SetSmallImage(Ptr<GuiImageData> value)
				{
					smallImage = value;
					NotifyUpdate();
				}

				Ptr<GuiImageData> ListViewItem::GetLargeImage()
				{
					return largeImage;
				}
				
				void ListViewItem::SetLargeImage(Ptr<GuiImageData> value)
				{
					largeImage = value;
					NotifyUpdate();
				}

				const WString& ListViewItem::GetText()
				{
					return text;
				}

				void ListViewItem::SetText(const WString& value)
				{
					text = value;
					NotifyUpdate();
				}

				description::Value ListViewItem::GetTag()
				{
					return tag;
				}

				void ListViewItem::SetTag(const description::Value& value)
				{
					tag = value;
					NotifyUpdate();
				}

/***********************************************************************
ListViewColumn
***********************************************************************/

				void ListViewColumn::NotifyUpdate(bool affectItem)
				{
					if (owner)
					{
						vint index = owner->IndexOf(this);
						owner->NotifyColumnUpdated(index, affectItem);
					}
				}

				ListViewColumn::ListViewColumn(const WString& _text, vint _size)
					:text(_text)
					,size(_size)
				{
				}

				ListViewColumn::~ListViewColumn()
				{
					if (dropdownPopup && ownPopup)
					{
						SafeDeleteControl(dropdownPopup);
					}
				}

				const WString& ListViewColumn::GetText()
				{
					return text;
				}

				void ListViewColumn::SetText(const WString& value)
				{
					if (text != value)
					{
						text = value;
						NotifyUpdate(false);
					}
				}

				ItemProperty<WString> ListViewColumn::GetTextProperty()
				{
					return textProperty;
				}

				void ListViewColumn::SetTextProperty(const ItemProperty<WString>& value)
				{
					textProperty = value;
					NotifyUpdate(true);
				}

				vint ListViewColumn::GetSize()
				{
					return size;
				}

				void ListViewColumn::SetSize(vint value)
				{
					if (size != value)
					{
						size = value;
						NotifyUpdate(false);
					}
				}

				bool ListViewColumn::GetOwnPopup()
				{
					return ownPopup;
				}

				void ListViewColumn::SetOwnPopup(bool value)
				{
					ownPopup = value;
				}

				GuiMenu* ListViewColumn::GetDropdownPopup()
				{
					return dropdownPopup;
				}

				void ListViewColumn::SetDropdownPopup(GuiMenu* value)
				{
					if (dropdownPopup != value)
					{
						dropdownPopup = value;
						NotifyUpdate(false);
					}
				}

				ColumnSortingState ListViewColumn::GetSortingState()
				{
					return sortingState;
				}

				void ListViewColumn::SetSortingState(ColumnSortingState value)
				{
					if (sortingState != value)
					{
						sortingState = value;
						NotifyUpdate(false);
					}
				}

/***********************************************************************
ListViewDataColumns
***********************************************************************/

				void ListViewDataColumns::NotifyUpdateInternal(vint start, vint count, vint newCount)
				{
					itemProvider->NotifyAllItemsUpdate();
				}

				ListViewDataColumns::ListViewDataColumns(IListViewItemProvider* _itemProvider)
					:itemProvider(_itemProvider)
				{
				}

				ListViewDataColumns::~ListViewDataColumns()
				{
				}

/***********************************************************************
ListViewColumns
***********************************************************************/

				void ListViewColumns::NotifyColumnUpdated(vint column, bool affectItem)
				{
					affectItemFlag = affectItem;
					NotifyUpdate(column, 1);
					affectItemFlag = true;
				}

				void ListViewColumns::AfterInsert(vint index, const Ptr<ListViewColumn>& value)
				{
					collections::ObservableListBase<Ptr<ListViewColumn>>::AfterInsert(index, value);
					value->owner = this;
				}

				void ListViewColumns::BeforeRemove(vint index, const Ptr<ListViewColumn>& value)
				{
					value->owner = 0;
					collections::ObservableListBase<Ptr<ListViewColumn>>::BeforeRemove(index, value);
				}

				void ListViewColumns::NotifyUpdateInternal(vint start, vint count, vint newCount)
				{
					itemProvider->NotifyAllColumnsUpdate();
					if (affectItemFlag)
					{
						itemProvider->NotifyAllItemsUpdate();
					}
				}

				ListViewColumns::ListViewColumns(IListViewItemProvider* _itemProvider)
					:itemProvider(_itemProvider)
				{
				}

				ListViewColumns::~ListViewColumns()
				{
				}

/***********************************************************************
ListViewItemProvider
***********************************************************************/

				void ListViewItemProvider::AfterInsert(vint index, const Ptr<ListViewItem>& value)
				{
					ListProvider<Ptr<ListViewItem>>::AfterInsert(index, value);
					value->owner = this;
				}

				void ListViewItemProvider::BeforeRemove(vint index, const Ptr<ListViewItem>& value)
				{
					value->owner = 0;
					ListProvider<Ptr<ListViewItem>>::AfterInsert(index, value);
				}

				void ListViewItemProvider::NotifyAllItemsUpdate()
				{
					NotifyUpdate(0, Count());
				}

				void ListViewItemProvider::NotifyAllColumnsUpdate()
				{
					for (vint i = 0; i < columnItemViewCallbacks.Count(); i++)
					{
						columnItemViewCallbacks[i]->OnColumnChanged();
					}
				}

				Ptr<GuiImageData> ListViewItemProvider::GetSmallImage(vint itemIndex)
				{
					return Get(itemIndex)->smallImage;
				}

				Ptr<GuiImageData> ListViewItemProvider::GetLargeImage(vint itemIndex)
				{
					return Get(itemIndex)->largeImage;
				}

				WString ListViewItemProvider::GetText(vint itemIndex)
				{
					return Get(itemIndex)->text;
				}

				WString ListViewItemProvider::GetSubItem(vint itemIndex, vint index)
				{
					Ptr<ListViewItem> item=Get(itemIndex);
					if(index<0 || index>=item->GetSubItems().Count())
					{
						return L"";
					}
					else
					{
						return item->GetSubItems()[index];
					}
				}

				vint ListViewItemProvider::GetDataColumnCount()
				{
					return dataColumns.Count();
				}

				vint ListViewItemProvider::GetDataColumn(vint index)
				{
					return dataColumns[index];
				}

				vint ListViewItemProvider::GetColumnCount()
				{
					return columns.Count();
				}

				WString ListViewItemProvider::GetColumnText(vint index)
				{
					if (index<0 || index >= columns.Count())
					{
						return L"";
					}
					else
					{
						return columns[index]->GetText();
					}
				}

				bool ListViewItemProvider::AttachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)
				{
					if(columnItemViewCallbacks.Contains(value))
					{
						return false;
					}
					else
					{
						columnItemViewCallbacks.Add(value);
						return true;
					}
				}

				bool ListViewItemProvider::DetachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)
				{
					vint index=columnItemViewCallbacks.IndexOf(value);
					if(index==-1)
					{
						return false;
					}
					else
					{
						columnItemViewCallbacks.Remove(value);
						return true;
					}
				}

				vint ListViewItemProvider::GetColumnSize(vint index)
				{
					if(index<0 || index>=columns.Count())
					{
						return 0;
					}
					else
					{
						return columns[index]->GetSize();
					}
				}

				void ListViewItemProvider::SetColumnSize(vint index, vint value)
				{
					if(index>=0 && index<columns.Count())
					{
						columns[index]->SetSize(value);
					}
				}

				GuiMenu* ListViewItemProvider::GetDropdownPopup(vint index)
				{
					if(index<0 || index>=columns.Count())
					{
						return 0;
					}
					else
					{
						return columns[index]->GetDropdownPopup();
					}
				}

				ColumnSortingState ListViewItemProvider::GetSortingState(vint index)
				{
					if (index < 0 || index >= columns.Count())
					{
						return ColumnSortingState::NotSorted;
					}
					else
					{
						return columns[index]->GetSortingState();
					}
				}

				WString ListViewItemProvider::GetTextValue(vint itemIndex)
				{
					return GetText(itemIndex);
				}

				description::Value ListViewItemProvider::GetBindingValue(vint itemIndex)
				{
					return Value::From(Get(itemIndex));
				}

				ListViewItemProvider::ListViewItemProvider()
					:columns(this)
					, dataColumns(this)
				{
				}

				ListViewItemProvider::~ListViewItemProvider()
				{
				}

				IDescriptable* ListViewItemProvider::RequestView(const WString& identifier)
				{
					if (identifier == IListViewItemView::Identifier)
					{
						return (IListViewItemView*)this;
					}
					else if (identifier == ListViewColumnItemArranger::IColumnItemView::Identifier)
					{
						return (ListViewColumnItemArranger::IColumnItemView*)this;
					}
					else
					{
						return 0;
					}
				}

				ListViewDataColumns& ListViewItemProvider::GetDataColumns()
				{
					return dataColumns;
				}

				ListViewColumns& ListViewItemProvider::GetColumns()
				{
					return columns;
				}
			}

/***********************************************************************
GuiListView
***********************************************************************/

			void GuiVirtualListView::OnStyleInstalled(vint itemIndex, ItemStyle* style)
			{
				GuiListViewBase::OnStyleInstalled(itemIndex, style);
			}

			void GuiVirtualListView::OnItemTemplateChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				view = ListViewView::Unknown;
			}

			GuiVirtualListView::GuiVirtualListView(theme::ThemeName themeName, GuiListControl::IItemProvider* _itemProvider)
				:GuiListViewBase(themeName, _itemProvider)
			{
				SetView(ListViewView::Detail);
			}

			GuiVirtualListView::~GuiVirtualListView()
			{
			}

			ListViewView GuiVirtualListView::GetView()
			{
				return view;
			}

			void GuiVirtualListView::SetView(ListViewView _view)
			{
				switch (_view)
				{
				case ListViewView::BigIcon:
					SetStyleAndArranger(
						[](const Value&) { return new list::BigIconListViewItemTemplate; },
						new list::FixedSizeMultiColumnItemArranger
						);
					break;
				case ListViewView::SmallIcon:
					SetStyleAndArranger(
						[](const Value&) { return new list::SmallIconListViewItemTemplate; },
						new list::FixedSizeMultiColumnItemArranger
					);
					break;
				case ListViewView::List:
					SetStyleAndArranger(
						[](const Value&) { return new list::ListListViewItemTemplate; },
						new list::FixedHeightMultiColumnItemArranger
					);
					break;
				case ListViewView::Tile:
					SetStyleAndArranger(
						[](const Value&) { return new list::TileListViewItemTemplate; },
						new list::FixedSizeMultiColumnItemArranger
					);
					break;
				case ListViewView::Information:
					SetStyleAndArranger(
						[](const Value&) { return new list::InformationListViewItemTemplate; },
						new list::FixedHeightItemArranger
					);
					break;
				case ListViewView::Detail:
					SetStyleAndArranger(
						[](const Value&) { return new list::DetailListViewItemTemplate; },
						new list::ListViewColumnItemArranger
					);
					break;
				default:;
				}
				view = _view;
			}

/***********************************************************************
GuiListView
***********************************************************************/

			GuiListView::GuiListView(theme::ThemeName themeName)
				:GuiVirtualListView(themeName, new list::ListViewItemProvider)
			{
				items=dynamic_cast<list::ListViewItemProvider*>(itemProvider.Obj());
			}

			GuiListView::~GuiListView()
			{
			}

			list::ListViewItemProvider& GuiListView::GetItems()
			{
				return *items;
			}

			list::ListViewDataColumns& GuiListView::GetDataColumns()
			{
				return items->GetDataColumns();
			}

			list::ListViewColumns& GuiListView::GetColumns()
			{
				return items->GetColumns();
			}

			Ptr<list::ListViewItem> GuiListView::GetSelectedItem()
			{
				vint index = GetSelectedItemIndex();
				if (index == -1) return 0;
				return items->Get(index);
			}
		}
	}
}