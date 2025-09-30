/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_ITEMPROVIDER_BINDING
#define VCZH_PRESENTATION_CONTROLS_ITEMPROVIDER_BINDING

#include "ItemProvider_ITextItemView.h"
#include "ItemProvider_IListViewItemView.h"
#include "ItemProvider_ITreeViewItemView.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			template<typename T>
			struct DefaultValueOf
			{
				static T Get()
				{
					return reflection::description::TypedValueSerializerProvider<T>::GetDefaultValue();
				}
			};

			template<typename T>
			struct DefaultValueOf<Ptr<T>>
			{
				static Ptr<T> Get()
				{
					return nullptr;
				}
			};

			template<>
			struct DefaultValueOf<description::Value>
			{
				static description::Value Get()
				{
					return description::Value();
				}
			};

			template<typename T>
			T ReadProperty(const description::Value& thisObject, const ItemProperty<T>& propertyName)
			{
				if (!thisObject.IsNull() && propertyName)
				{
					return propertyName(thisObject);
				}
				else
				{
					return DefaultValueOf<T>::Get();
				}
			}

			template<typename T>
			T ReadProperty(const description::Value& thisObject, const WritableItemProperty<T>& propertyName)
			{
				auto defaultValue = DefaultValueOf<T>::Get();
				if (!thisObject.IsNull() && propertyName)
				{
					return propertyName(thisObject, defaultValue, false);
				}
				else
				{
					return defaultValue;
				}
			}

			template<typename T>
			void WriteProperty(const description::Value& thisObject, const WritableItemProperty<T>& propertyName, const T& value)
			{
				if (!thisObject.IsNull() && propertyName)
				{
					propertyName(thisObject, value, true);
				}
			}

/***********************************************************************
TextItemBindableProvider
***********************************************************************/

			class TextItemBindableProvider
				: public list::ItemProviderBase
				, public virtual list::ITextItemView
				, public Description<TextItemBindableProvider>
			{
			protected:
				Ptr<EventHandler>								itemChangedEventHandler;
				Ptr<description::IValueReadonlyList>			itemSource;

			public:
				ItemProperty<WString>							textProperty;
				WritableItemProperty<bool>						checkedProperty;

			public:
				TextItemBindableProvider();
				~TextItemBindableProvider();

				Ptr<description::IValueEnumerable>				GetItemSource();
				void											SetItemSource(Ptr<description::IValueEnumerable> _itemSource);

				description::Value								Get(vint index);
				void											UpdateBindingProperties();
				bool											NotifyUpdate(vint start, vint count, bool itemReferenceUpdated);
				
				// ===================== GuiListControl::IItemProvider =====================

				WString											GetTextValue(vint itemIndex)override;
				description::Value								GetBindingValue(vint itemIndex)override;
				vint											Count()override;
				IDescriptable*									RequestView(const WString& identifier)override;
				
				// ===================== list::TextItemStyleProvider::ITextItemView =====================

				bool											GetChecked(vint itemIndex)override;
				void											SetChecked(vint itemIndex, bool value)override;
			};

/***********************************************************************
ListViewItemBindableProvider
***********************************************************************/

			class ListViewItemBindableProvider
				: public list::ItemProviderBase
				, protected virtual list::IListViewItemProvider
				, public virtual list::IListViewItemView
				, public virtual list::IColumnItemView
				, public Description<ListViewItemBindableProvider>
			{
				typedef collections::List<list::IColumnItemViewCallback*>		ColumnItemViewCallbackList;
			protected:
				list::ListViewDataColumns						dataColumns;
				list::ListViewColumns							columns;
				ColumnItemViewCallbackList						columnItemViewCallbacks;
				Ptr<EventHandler>								itemChangedEventHandler;
				Ptr<description::IValueReadonlyList>			itemSource;

			public:
				ItemProperty<Ptr<GuiImageData>>					largeImageProperty;
				ItemProperty<Ptr<GuiImageData>>					smallImageProperty;

			public:
				ListViewItemBindableProvider();
				~ListViewItemBindableProvider();

				Ptr<description::IValueEnumerable>				GetItemSource();
				void											SetItemSource(Ptr<description::IValueEnumerable> _itemSource);
				
				description::Value								Get(vint index);
				void											UpdateBindingProperties();
				bool											NotifyUpdate(vint start, vint count, bool itemReferenceUpdated);
				list::ListViewDataColumns&						GetDataColumns();
				list::ListViewColumns&							GetColumns();
				
				// ===================== list::IListViewItemProvider =====================

				void											RebuildAllItems() override;
				void											RefreshAllItems() override;
				void											NotifyColumnRebuilt() override;
				void											NotifyColumnChanged() override;
				
				// ===================== GuiListControl::IItemProvider =====================

				WString											GetTextValue(vint itemIndex)override;
				description::Value								GetBindingValue(vint itemIndex)override;
				vint											Count()override;
				IDescriptable*									RequestView(const WString& identifier)override;

				// ===================== list::ListViewItemStyleProvider::IListViewItemView =====================

				Ptr<GuiImageData>								GetSmallImage(vint itemIndex)override;
				Ptr<GuiImageData>								GetLargeImage(vint itemIndex)override;
				WString											GetText(vint itemIndex)override;
				WString											GetSubItem(vint itemIndex, vint index)override;
				vint											GetDataColumnCount()override;
				vint											GetDataColumn(vint index)override;
				vint											GetColumnCount()override;
				WString											GetColumnText(vint index)override;

				// ===================== list::ListViewColumnItemArranger::IColumnItemView =====================
					
				bool											AttachCallback(list::IColumnItemViewCallback* value)override;
				bool											DetachCallback(list::IColumnItemViewCallback* value)override;
				vint											GetColumnSize(vint index)override;
				void											SetColumnSize(vint index, vint value)override;
				GuiMenu*										GetDropdownPopup(vint index)override;
				ColumnSortingState								GetSortingState(vint index)override;
			};

/***********************************************************************
TreeViewItemBindableRootProvider
***********************************************************************/
			
			class TreeViewItemBindableRootProvider;

			class TreeViewItemBindableNode
				: public Object
				, public virtual tree::INodeProvider
			{
				friend class TreeViewItemBindableRootProvider;
				typedef collections::List<Ptr<TreeViewItemBindableNode>>	NodeList;
			protected:
				description::Value								itemSource;
				TreeViewItemBindableRootProvider*				rootProvider;
				TreeViewItemBindableNode*						parent;
				tree::INodeProviderCallback*					callback;
				bool											expanding = false;

				Ptr<EventHandler>								itemChangedEventHandler;
				Ptr<description::IValueReadonlyList>			childrenVirtualList;
				NodeList										children;

				Ptr<description::IValueReadonlyList>			PrepareValueList(const description::Value& inputItemSource);
				void											PrepareChildren(Ptr<description::IValueReadonlyList> newValueList);
				void											UnprepareChildren();
				void											PrepareReverseMapping();
				void											UnprepareReverseMapping();
			public:
				TreeViewItemBindableNode(const description::Value& _itemSource, TreeViewItemBindableNode* _parent);
				TreeViewItemBindableNode(TreeViewItemBindableRootProvider* _rootProvider);
				~TreeViewItemBindableNode();

				description::Value								GetItemSource();
				void											SetItemSource(const description::Value& _itemSource);

				// ===================== tree::INodeProvider =====================

				bool											GetExpanding()override;
				void											SetExpanding(bool value)override;
				vint											CalculateTotalVisibleNodes()override;
				void											NotifyDataModified()override;

				vint											GetChildCount()override;
				Ptr<tree::INodeProvider>						GetParent()override;
				Ptr<tree::INodeProvider>						GetChild(vint index)override;
			};

			class TreeViewItemBindableRootProvider
				: public tree::NodeRootProviderBase
				, public virtual tree::ITreeViewItemView
				, public Description<TreeViewItemBindableRootProvider>
			{
				friend class TreeViewItemBindableNode;
				using IValueEnumerable = reflection::description::IValueEnumerable;
			public:
				WritableItemProperty<description::Value>		reverseMappingProperty;
				ItemProperty<WString>							textProperty;
				ItemProperty<Ptr<GuiImageData>>					imageProperty;
				ItemProperty<Ptr<IValueEnumerable>>				childrenProperty;
				Ptr<TreeViewItemBindableNode>					rootNode;

			public:
				TreeViewItemBindableRootProvider();
				~TreeViewItemBindableRootProvider();

				description::Value								GetItemSource();
				void											SetItemSource(const description::Value& _itemSource);

				void											UpdateBindingProperties(bool updateChildrenProperty);

				// ===================== tree::INodeRootProvider =====================

				Ptr<tree::INodeProvider>						GetRootNode()override;
				WString											GetTextValue(tree::INodeProvider* node)override;
				description::Value								GetBindingValue(tree::INodeProvider* node)override;
				IDescriptable*									RequestView(const WString& identifier)override;

				// ===================== tree::ITreeViewItemView =====================

				Ptr<GuiImageData>								GetNodeImage(tree::INodeProvider* node)override;
			};
		}
	}
}

#endif
