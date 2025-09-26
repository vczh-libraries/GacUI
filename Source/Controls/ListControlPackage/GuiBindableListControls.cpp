#include "GuiBindableListControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace collections;
			using namespace list;
			using namespace tree;
			using namespace reflection::description;
			using namespace templates;

/***********************************************************************
GuiBindableTextList
***********************************************************************/

			GuiBindableTextList::GuiBindableTextList(theme::ThemeName themeName)
				:GuiVirtualTextList(themeName, new TextItemBindableProvider)
			{
				itemSource = dynamic_cast<TextItemBindableProvider*>(GetItemProvider());

				TextPropertyChanged.SetAssociatedComposition(boundsComposition);
				TextPropertyChanged.SetAssociatedComposition(boundsComposition);
			}

			GuiBindableTextList::~GuiBindableTextList()
			{
			}

			Ptr<description::IValueEnumerable> GuiBindableTextList::GetItemSource()
			{
				return itemSource->GetItemSource();
			}

			void GuiBindableTextList::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
			{
				itemSource->SetItemSource(_itemSource);
			}

			ItemProperty<WString> GuiBindableTextList::GetTextProperty()
			{
				return itemSource->textProperty;
			}

			void GuiBindableTextList::SetTextProperty(const ItemProperty<WString>& value)
			{
				if (itemSource->textProperty != value)
				{
					itemSource->textProperty = value;
					itemSource->UpdateBindingProperties();
					TextPropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			WritableItemProperty<bool> GuiBindableTextList::GetCheckedProperty()
			{
				return itemSource->checkedProperty;
			}

			void GuiBindableTextList::SetCheckedProperty(const WritableItemProperty<bool>& value)
			{
				if (itemSource->checkedProperty != value)
				{
					itemSource->checkedProperty = value;
					itemSource->UpdateBindingProperties();
					CheckedPropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			description::Value GuiBindableTextList::GetSelectedItem()
			{
				vint index = GetSelectedItemIndex();
				if (index == -1) return Value();
				return itemSource->Get(index);
			}

			bool GuiBindableTextList::NotifyItemDataModified(vint start, vint count)
			{
				return itemSource->NotifyUpdate(start, count, false);
			}

/***********************************************************************
GuiBindableListView
***********************************************************************/

			GuiBindableListView::GuiBindableListView(theme::ThemeName themeName)
				:GuiVirtualListView(themeName, new ListViewItemBindableProvider)
			{
				itemSource = dynamic_cast<ListViewItemBindableProvider*>(GetItemProvider());

				LargeImagePropertyChanged.SetAssociatedComposition(boundsComposition);
				SmallImagePropertyChanged.SetAssociatedComposition(boundsComposition);
			}

			GuiBindableListView::~GuiBindableListView()
			{
			}

			list::ListViewDataColumns& GuiBindableListView::GetDataColumns()
			{
				return itemSource->GetDataColumns();
			}

			list::ListViewColumns& GuiBindableListView::GetColumns()
			{
				return itemSource->GetColumns();
			}

			Ptr<description::IValueEnumerable> GuiBindableListView::GetItemSource()
			{
				return itemSource->GetItemSource();
			}

			void GuiBindableListView::SetItemSource(Ptr<description::IValueEnumerable> _itemSource)
			{
				itemSource->SetItemSource(_itemSource);
			}

			ItemProperty<Ptr<GuiImageData>> GuiBindableListView::GetLargeImageProperty()
			{
				return itemSource->largeImageProperty;
			}

			void GuiBindableListView::SetLargeImageProperty(const ItemProperty<Ptr<GuiImageData>>& value)
			{
				if (itemSource->largeImageProperty != value)
				{
					itemSource->largeImageProperty = value;
					itemSource->UpdateBindingProperties();
					LargeImagePropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			ItemProperty<Ptr<GuiImageData>> GuiBindableListView::GetSmallImageProperty()
			{
				return itemSource->smallImageProperty;
			}

			void GuiBindableListView::SetSmallImageProperty(const ItemProperty<Ptr<GuiImageData>>& value)
			{
				if (itemSource->smallImageProperty != value)
				{
					itemSource->smallImageProperty = value;
					itemSource->UpdateBindingProperties();
					SmallImagePropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			description::Value GuiBindableListView::GetSelectedItem()
			{
				vint index = GetSelectedItemIndex();
				if (index == -1) return Value();
				return itemSource->Get(index);
			}

			bool GuiBindableListView::NotifyItemDataModified(vint start, vint count)
			{
				return itemSource->NotifyUpdate(start, count, false);
			}

/***********************************************************************
GuiBindableTreeView
***********************************************************************/

			GuiBindableTreeView::GuiBindableTreeView(theme::ThemeName themeName, WritableItemProperty<description::Value> reverseMappingProperty)
				:GuiVirtualTreeView(themeName, Ptr(new TreeViewItemBindableRootProvider))
			{
				itemSource = dynamic_cast<TreeViewItemBindableRootProvider*>(GetNodeRootProvider());
				itemSource->reverseMappingProperty = reverseMappingProperty;

				TextPropertyChanged.SetAssociatedComposition(boundsComposition);
				ImagePropertyChanged.SetAssociatedComposition(boundsComposition);
				ChildrenPropertyChanged.SetAssociatedComposition(boundsComposition);
			}

			GuiBindableTreeView::~GuiBindableTreeView()
			{
			}

			description::Value GuiBindableTreeView::GetItemSource()
			{
				return itemSource->GetItemSource();
			}

			void GuiBindableTreeView::SetItemSource(description::Value _itemSource)
			{
				itemSource->SetItemSource(_itemSource);
			}
			
			WritableItemProperty<description::Value> GuiBindableTreeView::GetReverseMappingProperty()
			{
				return itemSource->reverseMappingProperty;
			}

			ItemProperty<WString> GuiBindableTreeView::GetTextProperty()
			{
				return itemSource->textProperty;
			}

			void GuiBindableTreeView::SetTextProperty(const ItemProperty<WString>& value)
			{
				if (itemSource->textProperty != value)
				{
					itemSource->textProperty = value;
					itemSource->UpdateBindingProperties(false);
					TextPropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			ItemProperty<Ptr<GuiImageData>> GuiBindableTreeView::GetImageProperty()
			{
				return itemSource->imageProperty;
			}

			void GuiBindableTreeView::SetImageProperty(const ItemProperty<Ptr<GuiImageData>>& value)
			{
				if (itemSource->imageProperty != value)
				{
					itemSource->imageProperty = value;
					itemSource->UpdateBindingProperties(false);
					ImagePropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			ItemProperty<Ptr<IValueEnumerable>> GuiBindableTreeView::GetChildrenProperty()
			{
				return itemSource->childrenProperty;
			}

			void GuiBindableTreeView::SetChildrenProperty(const ItemProperty<Ptr<IValueEnumerable>>& value)
			{
				if (itemSource->childrenProperty != value)
				{
					itemSource->childrenProperty = value;
					itemSource->UpdateBindingProperties(true);
					ChildrenPropertyChanged.Execute(GetNotifyEventArguments());
				}
			}

			description::Value GuiBindableTreeView::GetSelectedItem()
			{
				vint index = GetSelectedItemIndex();
				if (index == -1) return Value();

				Value result;
				if (auto node = nodeItemView->RequestNode(index))
				{
					if (auto itemSourceNode = node.Cast<TreeViewItemBindableNode>())
					{
						result = itemSourceNode->GetItemSource();
					}
				}
				return result;
			}

			void GuiBindableTreeView::NotifyNodeDataModified(description::Value value)
			{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::controls::GuiBindableTreeView::NotifyNodeDataModified(Value)#"

				CHECK_ERROR(itemSource->reverseMappingProperty, ERROR_MESSAGE_PREFIX L"This function can only be called when the ReverseMappingProperty is in use.");
				CHECK_ERROR(!value.IsNull(), ERROR_MESSAGE_PREFIX L"The item cannot be null.");
				auto mapping = ReadProperty(value, itemSource->reverseMappingProperty);
				auto node = dynamic_cast<tree::INodeProvider*>(mapping.GetRawPtr());
				CHECK_ERROR(node, ERROR_MESSAGE_PREFIX L"The item is not binded to a GuiBindableTreeView control or its reverse mapping property has been unexpectedly changed.");

				auto rootNode = node;
				while (rootNode->GetParent())
				{
					rootNode = rootNode->GetParent().Obj();
				}

				CHECK_ERROR(rootNode == itemSource->rootNode.Obj(), ERROR_MESSAGE_PREFIX L"The item is not binded to this control.");
				CHECK_ERROR(node != itemSource->rootNode.Obj(), ERROR_MESSAGE_PREFIX L"The item should not be the root item, which is the item source assigned to this control.");
				node->NotifyDataModified();

#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}
