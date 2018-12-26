/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIBINDABLELISTCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUIBINDABLELISTCONTROLS

#include "GuiTextListControls.h"
#include "GuiListViewControls.h"
#include "GuiTreeViewControls.h"

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
GuiBindableTextList
***********************************************************************/

			/// <summary>A bindable Text list control.</summary>
			class GuiBindableTextList : public GuiVirtualTextList, public Description<GuiBindableTextList>
			{
			protected:
				class ItemSource
					: public list::ItemProviderBase
					, protected list::ITextItemView
				{
				protected:
					Ptr<EventHandler>								itemChangedEventHandler;
					Ptr<description::IValueReadonlyList>			itemSource;

				public:
					ItemProperty<WString>							textProperty;
					WritableItemProperty<bool>						checkedProperty;

				public:
					ItemSource();
					~ItemSource();

					Ptr<description::IValueEnumerable>				GetItemSource();
					void											SetItemSource(Ptr<description::IValueEnumerable> _itemSource);

					description::Value								Get(vint index);
					void											UpdateBindingProperties();
					
					// ===================== GuiListControl::IItemProvider =====================

					WString											GetTextValue(vint itemIndex)override;
					description::Value								GetBindingValue(vint itemIndex)override;
					vint											Count()override;
					IDescriptable*									RequestView(const WString& identifier)override;
					
					// ===================== list::TextItemStyleProvider::ITextItemView =====================

					bool											GetChecked(vint itemIndex)override;
					void											SetChecked(vint itemIndex, bool value)override;
				};

			protected:
				ItemSource*											itemSource;

			public:
				/// <summary>Create a bindable Text list control.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiBindableTextList(theme::ThemeName themeName);
				~GuiBindableTextList();
				
				/// <summary>Text property name changed event.</summary>
				compositions::GuiNotifyEvent						TextPropertyChanged;
				/// <summary>Checked property name changed event.</summary>
				compositions::GuiNotifyEvent						CheckedPropertyChanged;

				/// <summary>Get the item source.</summary>
				/// <returns>The item source.</returns>
				Ptr<description::IValueEnumerable>					GetItemSource();
				/// <summary>Set the item source.</summary>
				/// <param name="_itemSource">The item source. Null is acceptable if you want to clear all data.</param>
				void												SetItemSource(Ptr<description::IValueEnumerable> _itemSource);
				
				/// <summary>Get the text property name to get the item text from an item.</summary>
				/// <returns>The text property name.</returns>
				ItemProperty<WString>								GetTextProperty();
				/// <summary>Set the text property name to get the item text from an item.</summary>
				/// <param name="value">The text property name.</param>
				void												SetTextProperty(const ItemProperty<WString>& value);
				
				/// <summary>Get the checked property name to get the check state from an item.</summary>
				/// <returns>The checked property name.</returns>
				WritableItemProperty<bool>							GetCheckedProperty();
				/// <summary>Set the checked property name to get the check state from an item.</summary>
				/// <param name="value">The checked property name.</param>
				void												SetCheckedProperty(const WritableItemProperty<bool>& value);

				/// <summary>Get the selected item.</summary>
				/// <returns>Returns the selected item. If there are multiple selected items, or there is no selected item, null will be returned.</returns>
				description::Value									GetSelectedItem();
			};

/***********************************************************************
GuiBindableListView
***********************************************************************/
			
			/// <summary>A bindable List view control.</summary>
			class GuiBindableListView : public GuiVirtualListView, public Description<GuiBindableListView>
			{
			protected:
				class ItemSource
					: public list::ItemProviderBase
					, protected virtual list::IListViewItemProvider
					, public virtual list::IListViewItemView
					, public virtual list::ListViewColumnItemArranger::IColumnItemView
				{
					typedef collections::List<list::ListViewColumnItemArranger::IColumnItemViewCallback*>		ColumnItemViewCallbackList;
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
					ItemSource();
					~ItemSource();

					Ptr<description::IValueEnumerable>				GetItemSource();
					void											SetItemSource(Ptr<description::IValueEnumerable> _itemSource);
					
					description::Value								Get(vint index);
					void											UpdateBindingProperties();
					bool											NotifyUpdate(vint start, vint count);
					list::ListViewDataColumns&						GetDataColumns();
					list::ListViewColumns&							GetColumns();
					
					// ===================== list::IListViewItemProvider =====================

					void											NotifyAllItemsUpdate()override;
					void											NotifyAllColumnsUpdate()override;
					
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
						
					bool											AttachCallback(list::ListViewColumnItemArranger::IColumnItemViewCallback* value)override;
					bool											DetachCallback(list::ListViewColumnItemArranger::IColumnItemViewCallback* value)override;
					vint											GetColumnSize(vint index)override;
					void											SetColumnSize(vint index, vint value)override;
					GuiMenu*										GetDropdownPopup(vint index)override;
					ColumnSortingState								GetSortingState(vint index)override;
				};

			protected:
				ItemSource*											itemSource;

			public:
				/// <summary>Create a bindable List view control.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiBindableListView(theme::ThemeName themeName);
				~GuiBindableListView();

				/// <summary>Get all data columns indices in columns.</summary>
				/// <returns>All data columns indices in columns.</returns>
				list::ListViewDataColumns&							GetDataColumns();
				/// <summary>Get all columns.</summary>
				/// <returns>All columns.</returns>
				list::ListViewColumns&								GetColumns();

				/// <summary>Get the item source.</summary>
				/// <returns>The item source.</returns>
				Ptr<description::IValueEnumerable>					GetItemSource();
				/// <summary>Set the item source.</summary>
				/// <param name="_itemSource">The item source. Null is acceptable if you want to clear all data.</param>
				void												SetItemSource(Ptr<description::IValueEnumerable> _itemSource);
				
				/// <summary>Large image property name changed event.</summary>
				compositions::GuiNotifyEvent						LargeImagePropertyChanged;
				/// <summary>Small image property name changed event.</summary>
				compositions::GuiNotifyEvent						SmallImagePropertyChanged;
				
				/// <summary>Get the large image property name to get the large image from an item.</summary>
				/// <returns>The large image property name.</returns>
				ItemProperty<Ptr<GuiImageData>>						GetLargeImageProperty();
				/// <summary>Set the large image property name to get the large image from an item.</summary>
				/// <param name="value">The large image property name.</param>
				void												SetLargeImageProperty(const ItemProperty<Ptr<GuiImageData>>& value);
				
				/// <summary>Get the small image property name to get the small image from an item.</summary>
				/// <returns>The small image property name.</returns>
				ItemProperty<Ptr<GuiImageData>>						GetSmallImageProperty();
				/// <summary>Set the small image property name to get the small image from an item.</summary>
				/// <param name="value">The small image property name.</param>
				void												SetSmallImageProperty(const ItemProperty<Ptr<GuiImageData>>& value);

				/// <summary>Get the selected item.</summary>
				/// <returns>Returns the selected item. If there are multiple selected items, or there is no selected item, null will be returned.</returns>
				description::Value									GetSelectedItem();
			};

/***********************************************************************
GuiBindableTreeView
***********************************************************************/
			
			/// <summary>A bindable Tree view control.</summary>
			class GuiBindableTreeView : public GuiVirtualTreeView, public Description<GuiBindableTreeView>
			{
				using IValueEnumerable = reflection::description::IValueEnumerable;
			protected:
				class ItemSource;

				class ItemSourceNode
					: public Object
					, public virtual tree::INodeProvider
				{
					friend class ItemSource;
					typedef collections::List<Ptr<ItemSourceNode>>	NodeList;
				protected:
					description::Value								itemSource;
					ItemSource*										rootProvider;
					ItemSourceNode*									parent;
					tree::INodeProviderCallback*					callback;
					bool											expanding = false;

					Ptr<EventHandler>								itemChangedEventHandler;
					Ptr<description::IValueReadonlyList>			childrenVirtualList;
					NodeList										children;

					Ptr<description::IValueReadonlyList>			PrepareValueList(const description::Value& inputItemSource);
					void											PrepareChildren(Ptr<description::IValueReadonlyList> newValueList);
					void											UnprepareChildren();
				public:
					ItemSourceNode(const description::Value& _itemSource, ItemSourceNode* _parent);
					ItemSourceNode(ItemSource* _rootProvider);
					~ItemSourceNode();

					description::Value								GetItemSource();
					void											SetItemSource(const description::Value& _itemSource);

					// ===================== tree::INodeProvider =====================

					bool											GetExpanding()override;
					void											SetExpanding(bool value)override;
					vint											CalculateTotalVisibleNodes()override;

					vint											GetChildCount()override;
					Ptr<tree::INodeProvider>						GetParent()override;
					Ptr<tree::INodeProvider>						GetChild(vint index)override;
				};

				class ItemSource
					: public tree::NodeRootProviderBase
					, public virtual tree::ITreeViewItemView
				{
					friend class ItemSourceNode;
				public:
					ItemProperty<WString>							textProperty;
					ItemProperty<Ptr<GuiImageData>>					imageProperty;
					ItemProperty<Ptr<IValueEnumerable>>				childrenProperty;
					Ptr<ItemSourceNode>								rootNode;

				public:
					ItemSource();
					~ItemSource();

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

			protected:
				ItemSource*											itemSource;

			public:
				/// <summary>Create a bindable Tree view control.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiBindableTreeView(theme::ThemeName themeName);
				~GuiBindableTreeView();
				
				/// <summary>Text property name changed event.</summary>
				compositions::GuiNotifyEvent						TextPropertyChanged;
				/// <summary>Image property name changed event.</summary>
				compositions::GuiNotifyEvent						ImagePropertyChanged;
				/// <summary>Children property name changed event.</summary>
				compositions::GuiNotifyEvent						ChildrenPropertyChanged;

				/// <summary>Get the item source.</summary>
				/// <returns>The item source.</returns>
				description::Value									GetItemSource();
				/// <summary>Set the item source.</summary>
				/// <param name="_itemSource">The item source. Null is acceptable if you want to clear all data.</param>
				void												SetItemSource(description::Value _itemSource);
				
				/// <summary>Get the text property name to get the item text from an item.</summary>
				/// <returns>The text property name.</returns>
				ItemProperty<WString>								GetTextProperty();
				/// <summary>Set the text property name to get the item text from an item.</summary>
				/// <param name="value">The text property name.</param>
				void												SetTextProperty(const ItemProperty<WString>& value);
				
				/// <summary>Get the image property name to get the image from an item.</summary>
				/// <returns>The image property name.</returns>
				ItemProperty<Ptr<GuiImageData>>						GetImageProperty();
				/// <summary>Set the image property name to get the image from an item.</summary>
				/// <param name="value">The image property name.</param>
				void												SetImageProperty(const ItemProperty<Ptr<GuiImageData>>& value);
				
				/// <summary>Get the children property name to get the children from an item.</summary>
				/// <returns>The children property name.</returns>
				ItemProperty<Ptr<IValueEnumerable>>					GetChildrenProperty();
				/// <summary>Set the children property name to get the children from an item.</summary>
				/// <param name="value">The children property name.</param>
				void												SetChildrenProperty(const ItemProperty<Ptr<IValueEnumerable>>& value);

				/// <summary>Get the selected item.</summary>
				/// <returns>Returns the selected item. If there are multiple selected items, or there is no selected item, null will be returned.</returns>
				description::Value									GetSelectedItem();
			};
		}
	}
}

#endif
