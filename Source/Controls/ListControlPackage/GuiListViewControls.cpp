#include "GuiListViewControls.h"
#include "GuiListViewItemTemplates.h"
#include "../Templates/GuiThemeStyleFactory.h"

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
				TypedControlTemplateObject(true)->SetSortingState(columnSortingState);
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
					TypedControlTemplateObject(true)->SetSortingState(columnSortingState);
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

			GuiListViewBase::GuiListViewBase(theme::ThemeName themeName, list::IItemProvider* _itemProvider)
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

				void ListViewColumnItemArranger::ColumnItemViewCallback::OnColumnRebuilt()
				{
					arranger->RebuildColumns();
				}

				void ListViewColumnItemArranger::ColumnItemViewCallback::OnColumnChanged(bool needToRefreshItems)
				{
					arranger->RefreshColumns();
				}
				
/***********************************************************************
ListViewColumnItemArranger::ColumnItemArrangerRepeatComposition
***********************************************************************/

				void ListViewColumnItemArranger::ColumnItemArrangerRepeatComposition::Layout_EndLayout(bool totalSizeUpdated)
				{
					TBase::ArrangerRepeatComposition::Layout_EndLayout(totalSizeUpdated);
					arranger->FixColumnsAfterLayout();
				}

				void ListViewColumnItemArranger::ColumnItemArrangerRepeatComposition::Layout_CalculateTotalSize(Size& full, Size& minimum)
				{
					TBase::ArrangerRepeatComposition::Layout_CalculateTotalSize(full, minimum);
					full.x += arranger->SplitterWidth;
					minimum.x += arranger->SplitterWidth;
				}

				ListViewColumnItemArranger::ColumnItemArrangerRepeatComposition::ColumnItemArrangerRepeatComposition(ListViewColumnItemArranger* _arranger)
					: TBase::ArrangerRepeatComposition(_arranger)
					, arranger(_arranger)
				{
				}
				
/***********************************************************************
ListViewColumnItemArranger
***********************************************************************/

				const wchar_t* const ListViewColumnItemArranger::IColumnItemView::Identifier = L"vl::presentation::controls::list::ListViewColumnItemArranger::IColumnItemView";

				void ListViewColumnItemArranger::OnViewLocationChanged(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments)
				{
					FixColumnsAfterViewLocationChanged();
				}

				void ListViewColumnItemArranger::ColumnClicked(vint index, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					GuiItemEventArgs args(listView->ColumnClicked.GetAssociatedComposition());
					args.itemIndex=index;
					listView->ColumnClicked.Execute(args);
				}

				void ListViewColumnItemArranger::ColumnCachedBoundsChanged(vint index, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					GuiBoundsComposition* buttonBounds=columnHeaderButtons[index]->GetBoundsComposition();
					vint size=buttonBounds->GetCachedBounds().Width();
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
							Rect bounds=buttonBounds->GetCachedBounds();
							Rect newBounds(bounds.LeftTop(), Size(bounds.Width()+offset, bounds.Height()));
							buttonBounds->SetExpectedBounds(newBounds);

							vint finalSize=buttonBounds->GetCachedBounds().Width();
							columnItemView->SetColumnSize(index, finalSize);
						}
					}
				}

				void ListViewColumnItemArranger::ColumnHeadersCachedBoundsChanged(compositions::GuiGraphicsComposition* composition, compositions::GuiEventArgs& arguments)
				{
					UpdateRepeatConfig();
				}

				void ListViewColumnItemArranger::FixColumnsAfterViewLocationChanged()
				{
					vint x = GetRepeatComposition()->GetViewLocation().x;
					columnHeaders->SetExpectedBounds(Rect(Point(-x, 0), Size(0, 0)));
				}

				void ListViewColumnItemArranger::FixColumnsAfterLayout()
				{
					vint count = columnHeaders->GetParent()->Children().Count();
					columnHeaders->GetParent()->MoveChild(columnHeaders, count - 1);
					FixColumnsAfterViewLocationChanged();
				}

				vint ListViewColumnItemArranger::GetColumnsWidth()
				{
					vint width=columnHeaders->GetCachedBounds().Width()-SplitterWidth;
					if(width<SplitterWidth)
					{
						width=SplitterWidth;
					}
					return width;
				}

				vint ListViewColumnItemArranger::GetColumnsYOffset()
				{
					return columnHeaders->GetCachedBounds().Height();
				}

				void ListViewColumnItemArranger::DeleteColumnButtons()
				{
					// TODO: (enumerable) foreach:reversed
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
							button->GetBoundsComposition()->SetExpectedBounds(Rect(Point(0, 0), Size(columnItemView->GetColumnSize(i), 0)));
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
								button->SetControlTemplate(listView->TypedControlTemplateObject(true)->GetColumnHeaderTemplate());
								button->Clicked.AttachLambda([this, i](GuiGraphicsComposition* sender, GuiEventArgs& args) { ColumnClicked(i, sender, args); });
								button->GetBoundsComposition()->CachedBoundsChanged.AttachLambda([this, i](GuiGraphicsComposition* sender, GuiEventArgs& args) { ColumnCachedBoundsChanged(i, sender, args); });
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

					RefreshColumns();
					callback->OnTotalSizeChanged();
				}

				void ListViewColumnItemArranger::UpdateRepeatConfig()
				{
					GetRepeatComposition()->SetItemWidth(GetColumnsWidth());
					GetRepeatComposition()->SetItemYOffset(GetColumnsYOffset());
				}

				void ListViewColumnItemArranger::RefreshColumns()
				{
					if (columnItemView && listViewItemView)
					{
						for (vint i = 0; i < listViewItemView->GetColumnCount(); i++)
						{
							auto button = columnHeaderButtons[i];
							button->SetText(listViewItemView->GetColumnText(i));
							button->SetSubMenu(columnItemView->GetDropdownPopup(i), false);
							button->SetColumnSortingState(columnItemView->GetSortingState(i));
							button->GetBoundsComposition()->SetExpectedBounds(Rect(Point(0, 0), Size(columnItemView->GetColumnSize(i), 0)));
						}
						columnHeaders->ForceCalculateSizeImmediately();
						UpdateRepeatConfig();
					}
				}

				ListViewColumnItemArranger::ListViewColumnItemArranger()
					: TBase(new TBase::ArrangerRepeatComposition(this))
				{
					columnHeaders = new GuiStackComposition;
					columnHeaders->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					columnHeaders->CachedBoundsChanged.AttachMethod(this, &ListViewColumnItemArranger::ColumnHeadersCachedBoundsChanged);
					columnItemViewCallback = Ptr(new ColumnItemViewCallback(this));
					GetRepeatComposition()->ViewLocationChanged.AttachMethod(this, &ListViewColumnItemArranger::OnViewLocationChanged);
				}

				ListViewColumnItemArranger::~ListViewColumnItemArranger()
				{
					if(!columnHeaders->GetParent())
					{
						DeleteColumnButtons();
						delete columnHeaders;
					}
				}

				Size ListViewColumnItemArranger::GetTotalSize()
				{
					Size size = TBase::GetTotalSize();
					size.x += SplitterWidth;
					return size;
				}

				void ListViewColumnItemArranger::AttachListControl(GuiListControl* value)
				{
					TBase::AttachListControl(value);
					listView = dynamic_cast<GuiListViewBase*>(value);
					if (listView)
					{
						listViewItemView = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(WString::Unmanaged(IListViewItemView::Identifier)));
						columnItemView = dynamic_cast<IColumnItemView*>(listView->GetItemProvider()->RequestView(WString::Unmanaged(IColumnItemView::Identifier)));
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
					TBase::DetachListControl();
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
						owner->InvokeOnItemModified(index, 1, 1, false);
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

				void ListViewColumn::NotifyRebuilt()
				{
					if (owner)
					{
						vint index = owner->IndexOf(this);
						if (index != -1)
						{
							owner->NotifyColumnRebuilt(index);
						}
					}
				}

				void ListViewColumn::NotifyChanged(bool needToRefreshItems)
				{
					if (owner)
					{
						vint index = owner->IndexOf(this);
						if (index != -1)
						{
							owner->NotifyColumnChanged(index, needToRefreshItems);
						}
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
						NotifyChanged(false);
					}
				}

				ItemProperty<WString> ListViewColumn::GetTextProperty()
				{
					return textProperty;
				}

				void ListViewColumn::SetTextProperty(const ItemProperty<WString>& value)
				{
					textProperty = value;
					NotifyChanged(true);
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
						NotifyChanged(true);
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
						NotifyChanged(false);
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
						NotifyChanged(false);
					}
				}

/***********************************************************************
ListViewDataColumns
***********************************************************************/

				void ListViewDataColumns::NotifyUpdateInternal(vint start, vint count, vint newCount)
				{
					itemProvider->RefreshAllItems();
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

				void ListViewColumns::NotifyColumnRebuilt(vint column)
				{
					NotifyUpdate(column, 1);
				}

				void ListViewColumns::NotifyColumnChanged(vint column, bool needToRefreshItems)
				{
					itemProvider->NotifyColumnChanged();
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
					itemProvider->NotifyColumnRebuilt();
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

				void ListViewItemProvider::RebuildAllItems()
				{
					InvokeOnItemModified(0, Count(), Count(), true);
				}

				void ListViewItemProvider::RefreshAllItems()
				{
					InvokeOnItemModified(0, Count(), Count(), false);
				}

				void ListViewItemProvider::NotifyColumnRebuilt()
				{
					for (auto callback : columnItemViewCallbacks)
					{
						callback->OnColumnRebuilt();
					}
					RefreshAllItems();
				}

				void ListViewItemProvider::NotifyColumnChanged()
				{
					for (auto callback : columnItemViewCallbacks)
					{
						callback->OnColumnChanged(true);
					}
					RefreshAllItems();
				}

				ListViewItemProvider::ListViewItemProvider()
					:columns(this)
					, dataColumns(this)
				{
				}

				ListViewItemProvider::~ListViewItemProvider()
				{
				}

				WString ListViewItemProvider::GetTextValue(vint itemIndex)
				{
					return GetText(itemIndex);
				}

				description::Value ListViewItemProvider::GetBindingValue(vint itemIndex)
				{
					return Value::From(Get(itemIndex));
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

			void GuiVirtualListView::OnStyleInstalled(vint itemIndex, ItemStyle* style, bool refreshPropertiesOnly)
			{
				GuiListViewBase::OnStyleInstalled(itemIndex, style, refreshPropertiesOnly);
				if (auto textItemStyle = dynamic_cast<templates::GuiTextListItemTemplate*>(style))
				{
					textItemStyle->SetTextColor(TypedControlTemplateObject(true)->GetPrimaryTextColor());
				}
				if (refreshPropertiesOnly)
				{
					if (auto predefinedItemStyle = dynamic_cast<list::DefaultListViewItemTemplate*>(style))
					{
						predefinedItemStyle->RefreshItem();
					}
				}
			}

			void GuiVirtualListView::OnItemTemplateChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				view = ListViewView::Unknown;
			}

			GuiVirtualListView::GuiVirtualListView(theme::ThemeName themeName, list::IItemProvider* _itemProvider)
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
						Ptr(new list::FixedSizeMultiColumnItemArranger)
						);
					break;
				case ListViewView::SmallIcon:
					SetStyleAndArranger(
						[](const Value&) { return new list::SmallIconListViewItemTemplate; },
						Ptr(new list::FixedSizeMultiColumnItemArranger)
						);
					break;
				case ListViewView::List:
					SetStyleAndArranger(
						[](const Value&) { return new list::ListListViewItemTemplate; },
						Ptr(new list::FixedHeightMultiColumnItemArranger)
						);
					break;
				case ListViewView::Tile:
					SetStyleAndArranger(
						[](const Value&) { return new list::TileListViewItemTemplate; },
						Ptr(new list::FixedSizeMultiColumnItemArranger)
						);
					break;
				case ListViewView::Information:
					SetStyleAndArranger(
						[](const Value&) { return new list::InformationListViewItemTemplate; },
						Ptr(new list::FixedHeightItemArranger)
						);
					break;
				case ListViewView::Detail:
					SetStyleAndArranger(
						[](const Value&) { return new list::DetailListViewItemTemplate; },
						Ptr(new list::ListViewColumnItemArranger)
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