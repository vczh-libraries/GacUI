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
#include "ItemProvider_Binding.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiBindableTextList
***********************************************************************/

			/// <summary>A bindable Text list control.</summary>
			class GuiBindableTextList : public GuiVirtualTextList, public Description<GuiBindableTextList>
			{
			protected:
				TextItemBindableProvider*							itemSource = nullptr;

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

				/// <summary>Notify the control that data in some items are modified.</summary>
				/// <param name="start">The index of the first item.</param>
				/// <param name="count">The number of items</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				bool												NotifyItemDataModified(vint start, vint count);
			};

/***********************************************************************
GuiBindableListView
***********************************************************************/

			/// <summary>A bindable List view control.</summary>
			class GuiBindableListView : public GuiVirtualListView, public Description<GuiBindableListView>
			{
			protected:
				ListViewItemBindableProvider*						itemSource = nullptr;

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

				/// <summary>Notify the control that data in some items are modified.</summary>
				/// <param name="start">The index of the first item.</param>
				/// <param name="count">The number of items</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				bool												NotifyItemDataModified(vint start, vint count);
			};

/***********************************************************************
GuiBindableTreeView
***********************************************************************/
			
			/// <summary>A bindable Tree view control.</summary>
			class GuiBindableTreeView : public GuiVirtualTreeView, public Description<GuiBindableTreeView>
			{
				using IValueEnumerable = reflection::description::IValueEnumerable;
			protected:
				TreeViewItemBindableRootProvider*					itemSource = nullptr;

			public:
				/// <summary>Create a bindable Tree view control.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="reverseMappingProperty">(Optional): The value of <see cref="GuiBindableTreeView::GetReverseMappingProperty"/>.</param>
				GuiBindableTreeView(theme::ThemeName themeName, WritableItemProperty<description::Value> reverseMappingProperty = {});
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

				/// <summary>
				/// Get the reverse mapping property name to store the internal tree view node for an item.
				/// The value is set in the constructor.
				/// Using this property makes items in item source exclusive to a treeview control.
				/// Sharing such item in different treeview controls causes exceptions.
				/// </summary>
				/// <returns>The reverse mapping property name.</returns>
				WritableItemProperty<description::Value>			GetReverseMappingProperty();
				
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

				/// <summary>Notify the control that data in an item is modified. Child nodes are not notified.</summary>
				/// <param name="value">The item from the item source.</param>
				/// <returns>Returns true if this operation succeeded.</returns>
				void												NotifyNodeDataModified(description::Value value);
			};
		}
	}
}

#endif
