/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIRIBBONGALLERYLIST
#define VCZH_PRESENTATION_CONTROLS_GUIRIBBONGALLERYLIST

#include "GuiRibbonControls.h"
#include "../ListControlPackage/GuiListControlItemArrangers.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
Ribbon Gallery List
***********************************************************************/

			/// <summary>Represents the position of an item in the gallery list.</summary>
			struct GalleryPos
			{
				/// <summary>Group index.</summary>
				vint			group;
				/// <summary>Item index.</summary>
				vint			item;

				GalleryPos()
					:group(-1), item(-1)
				{
				}

				GalleryPos(vint _group, vint _item)
					:group(_group), item(_item)
				{
				}

				vint Compare(GalleryPos value)const
				{
					vint result = group - value.group;
					if (result != 0) return result;
					return item - value.item;
				}

				bool operator==(const GalleryPos& value)const { return Compare(value) == 0; }
				bool operator!=(const GalleryPos& value)const { return Compare(value) != 0; }
				bool operator<(const GalleryPos& value)const { return Compare(value)<0; }
				bool operator<=(const GalleryPos& value)const { return Compare(value) <= 0; }
				bool operator>(const GalleryPos& value)const { return Compare(value)>0; }
				bool operator>=(const GalleryPos& value)const { return Compare(value) >= 0; }
			};

			namespace list
			{
				class GroupedDataSource;

				/// <summary>A gallery group.</summary>
				class GalleryGroup : public Description<GalleryGroup>
				{
					friend class GroupedDataSource;
					using IValueList = reflection::description::IValueList;
				protected:
					Ptr<EventHandler>									eventHandler;
					WString												name;
					Ptr<IValueList>										itemValues;

				public:
					GalleryGroup();
					~GalleryGroup();

					/// <summary>Get the title of this group.</summary>
					/// <returns>The title of this group.</returns>
					WString												GetName();
					/// <summary>Get the all items of this group, could be null.</summary>
					/// <returns>All items of this group.</returns>
					Ptr<IValueList>										GetItemValues();
				};

				class GroupedDataSource : public Description<GroupedDataSource>
				{
					using IValueEnumerable = reflection::description::IValueEnumerable;
					using IValueList = reflection::description::IValueList;
					using IValueObservableList = reflection::description::IValueObservableList;
					using GalleryItemList = collections::ObservableList<reflection::description::Value>;
					using GalleryGroupList = collections::ObservableList<Ptr<GalleryGroup>>;

				protected:
					compositions::GuiGraphicsComposition*				associatedComposition;
					Ptr<IValueEnumerable>								itemSource;
					ItemProperty<WString>								titleProperty;
					ItemProperty<Ptr<IValueEnumerable>>					childrenProperty;

					GalleryItemList										joinedItemSource;
					GalleryGroupList									groupedItemSource;
					collections::List<vint>								cachedGroupItemCounts;
					Ptr<EventHandler>									groupChangedHandler;
					bool												ignoreGroupChanged = false;

					void												RebuildItemSource();
					Ptr<IValueList>										GetChildren(Ptr<IValueEnumerable> children);
					void												AttachGroupChanged(Ptr<GalleryGroup> group, vint index);
					void												OnGroupChanged(vint start, vint oldCount, vint newCount);
					void												OnGroupItemChanged(vint index, vint start, vint oldCount, vint newCount);
					vint												GetCountBeforeGroup(vint index);
					void												InsertGroupToJoined(vint index);
					void												RemoveGroupFromJoined(vint index);

				public:
					GroupedDataSource(compositions::GuiGraphicsComposition* _associatedComposition);
					~GroupedDataSource();

					/// <summary>Group enabled event.</summary>
					compositions::GuiNotifyEvent						GroupEnabledChanged;
					/// <summary>Group title property changed event.</summary>
					compositions::GuiNotifyEvent						GroupTitlePropertyChanged;
					/// <summary>Group children property changed event.</summary>
					compositions::GuiNotifyEvent						GroupChildrenPropertyChanged;

					/// <summary>Get the item source.</summary>
					/// <returns>The item source.</returns>
					Ptr<IValueEnumerable>								GetItemSource();
					/// <summary>Set the item source.</summary>
					/// <param name="value">The item source. Null is acceptable if you want to clear all data.</param>
					void												SetItemSource(Ptr<IValueEnumerable> value);

					/// <summary>Test if grouping is enabled. Enabled means there is really both GroupTitleProperty and GroupChildrenProperty is not empty.</summary>
					/// <returns>Returns true if grouping is enabled.</returns>
					bool												GetGroupEnabled();

					/// <summary>Get the group title property.</summary>
					/// <returns>The group title property.</returns>
					ItemProperty<WString>								GetGroupTitleProperty();
					/// <summary>Get the group title property.</summary>
					/// <param name="value">The group title property.</param>
					void												SetGroupTitleProperty(const ItemProperty<WString>& value);

					/// <summary>Get the group children property.</summary>
					/// <returns>The group children property.</returns>
					ItemProperty<Ptr<IValueEnumerable>>					GetGroupChildrenProperty();
					/// <summary>Get the group children property.</summary>
					/// <param name="value">The children title property.</param>
					void												SetGroupChildrenProperty(const ItemProperty<Ptr<IValueEnumerable>>& value);

					/// <summary>Get all groups.</summary>
					/// <returns>All groups.</returns>
					const GalleryGroupList&								GetGroups();
				};
			}

			namespace ribbon_impl
			{
				class GalleryItemArranger;
				class GalleryResponsiveLayout;
			}

			/// <summary>Auto resizable ribbon gallyer list.</summary>
			class GuiBindableRibbonGalleryList : public GuiRibbonGallery, public list::GroupedDataSource, private IGuiMenuDropdownProvider, public Description<GuiBindableRibbonGalleryList>
			{
				friend class ribbon_impl::GalleryItemArranger;

				using IValueEnumerable = reflection::description::IValueEnumerable;
				using IValueObservableList = reflection::description::IValueObservableList;
				using ItemStyleProperty = TemplateProperty<templates::GuiListItemTemplate>;

				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonGalleryListTemplate, GuiRibbonGallery)
			protected:
				ItemStyleProperty										itemStyle;
				GuiBindableTextList*									itemList;
				GuiRibbonToolstripMenu*									subMenu;
				vint													visibleItemCount = 1;
				bool													skipItemAppliedEvent = false;

				ribbon_impl::GalleryItemArranger*						itemListArranger;
				ribbon_impl::GalleryResponsiveLayout*					layout;
				GuiScrollContainer*										groupContainer;
				compositions::GuiRepeatStackComposition*				groupStack;

				void													UpdateLayoutSizeOffset();
				void													OnItemListSelectionChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void													OnItemListItemMouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments);
				void													OnItemListItemMouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiItemEventArgs& arguments);
				void													OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void													OnRequestedDropdown(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void													OnRequestedScrollUp(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void													OnRequestedScrollDown(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

				void													MenuResetGroupTemplate();
				GuiControl*												MenuGetGroupHeader(vint groupIndex);
				compositions::GuiRepeatFlowComposition*					MenuGetGroupFlow(vint groupIndex);
				GuiSelectableButton*									MenuGetGroupItemBackground(vint groupIndex, vint itemIndex);

				void													StartPreview(vint index);
				void													StopPreview(vint index);

			private:
				GuiMenu*												ProvideDropdownMenu()override;

			public:
				/// <summary>Create a control with a specified default theme.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiBindableRibbonGalleryList(theme::ThemeName themeName);
				~GuiBindableRibbonGalleryList();

				/// <summary>Item template changed event.</summary>
				compositions::GuiNotifyEvent							ItemTemplateChanged;
				/// <summary>Selection changed event.</summary>
				compositions::GuiNotifyEvent							SelectionChanged;
				/// <summary>Preview started event.</summary>
				compositions::GuiItemNotifyEvent						PreviewStarted;
				/// <summary>Preview stopped event.</summary>
				compositions::GuiItemNotifyEvent						PreviewStopped;
				/// <summary>Item applied event.</summary>
				compositions::GuiItemNotifyEvent						ItemApplied;

				/// <summary>Get the item style provider.</summary>
				/// <returns>The item style provider.</returns>
				ItemStyleProperty										GetItemTemplate();
				/// <summary>Set the item style provider</summary>
				/// <param name="value">The new item style provider</param>
				void													SetItemTemplate(ItemStyleProperty value);

				/// <summary>Convert an item index to a gallery item position.</summary>
				/// <returns>The gallery item position.</returns>
				/// <param name="index">The item index.</param>
				GalleryPos												IndexToGalleryPos(vint index);
				/// <summary>Convert a gallery item position to an item index.</summary>
				/// <returns>The item index.</returns>
				/// <param name="pos">The gallery item position.</param>
				vint													GalleryPosToIndex(GalleryPos pos);

				/// <summary>Get the minimum number of items should be displayed.</summary>
				/// <returns>The minimum number of items should be displayed.</returns>
				vint													GetMinCount();
				/// <summary>Set the minimum number of items should be displayed.</summary>
				/// <param name="value">The minimum number of items should be displayed.</param>
				void													SetMinCount(vint value);

				/// <summary>Get the maximum number of items should be displayed.</summary>
				/// <returns>The maximum number of items should be displayed.</returns>
				vint													GetMaxCount();
				/// <summary>Set the maximum number of items should be displayed.</summary>
				/// <param name="value">The maximum number of items should be displayed.</param>
				void													SetMaxCount(vint value);

				/// <summary>Get the selected item index.</summary>
				/// <returns>The index of the selected item. If there are multiple selected items, or there is no selected item, -1 will be returned.</returns>
				vint													GetSelectedIndex();
				/// <summary>Get the selected item.</summary>
				/// <returns>Returns the selected item. If there are multiple selected items, or there is no selected item, null will be returned.</returns>
				description::Value										GetSelectedItem();
				/// <summary>Select an item with <see cref="ItemApplied"/> event raised.</summary>
				/// <param name="index">The index of the item to select. Set to -1 to clear the selection.</param>
				void													ApplyItem(vint index);
				/// <summary>Select an item without <see cref="ItemApplied"/> event raised.</summary>
				/// <param name="index">The index of the item to select. Set to -1 to clear the selection.</param>
				void													SelectItem(vint index);

				/// <summary>Get the minimum items visible in the drop down menu.</summary>
				/// <returns>The minimum items visible in the drop down menu.</returns>
				vint													GetVisibleItemCount();
				/// <summary>Set minimum items visible in the drop down menu.</summary>
				/// <param name="value">The minimum items visible in the drop down menu.</param>
				void													SetVisibleItemCount(vint value);

				/// <summary>Get the dropdown menu.</summary>
				/// <returns>The dropdown menu.</returns>
				GuiToolstripMenu*										GetSubMenu();

				IDescriptable*											QueryService(const WString& identifier)override;
			};
		}
	}
}

#endif
