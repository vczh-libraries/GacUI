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

			struct GalleryPos
			{
				vint			group;
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

					WString												GetName();
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

					compositions::GuiNotifyEvent						GroupEnabledChanged;
					compositions::GuiNotifyEvent						GroupTitlePropertyChanged;
					compositions::GuiNotifyEvent						GroupChildrenPropertyChanged;

					/// <summary>Get the item source.</summary>
					/// <returns>The item source.</returns>
					Ptr<IValueEnumerable>								GetItemSource();
					/// <summary>Set the item source.</summary>
					/// <param name="value">The item source. Null is acceptable if you want to clear all data.</param>
					void												SetItemSource(Ptr<IValueEnumerable> value);

					bool												GetGroupEnabled();
					ItemProperty<WString>								GetGroupTitleProperty();
					void												SetGroupTitleProperty(const ItemProperty<WString>& value);
					ItemProperty<Ptr<IValueEnumerable>>					GetGroupChildrenProperty();
					void												SetGroupChildrenProperty(const ItemProperty<Ptr<IValueEnumerable>>& value);

					const GalleryGroupList&								GetGroups();
				};
			}

			namespace ribbon_impl
			{
				class GalleryItemArranger;
				class GalleryResponsiveLayout;
			}

			class GuiBindableRibbonGalleryList : public GuiRibbonGallery, public list::GroupedDataSource, public Description<GuiBindableRibbonGalleryList>
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
			public:
				GuiBindableRibbonGalleryList(theme::ThemeName themeName);
				~GuiBindableRibbonGalleryList();

				compositions::GuiNotifyEvent							ItemTemplateChanged;
				compositions::GuiNotifyEvent							SelectionChanged;
				compositions::GuiItemNotifyEvent						PreviewStarted;
				compositions::GuiItemNotifyEvent						PreviewStopped;

				/// <summary>Get the item style provider.</summary>
				/// <returns>The item style provider.</returns>
				ItemStyleProperty										GetItemTemplate();
				/// <summary>Set the item style provider</summary>
				/// <param name="value">The new item style provider</param>
				void													SetItemTemplate(ItemStyleProperty value);

				GalleryPos												IndexToGalleryPos(vint index);
				vint													GalleryPosToIndex(GalleryPos pos);

				vint													GetMinCount();
				void													SetMinCount(vint value);
				vint													GetMaxCount();
				void													SetMaxCount(vint value);

				vint													GetSelectedIndex();
				void													SetSelectedIndex(vint value);

				GuiToolstripMenu*										GetSubMenu();
			};
		}
	}
}

#endif
