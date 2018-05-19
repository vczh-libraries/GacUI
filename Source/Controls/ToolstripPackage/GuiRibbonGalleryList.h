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
				class GroupedDataSource : public Description<GroupedDataSource>
				{
					using IValueEnumerable = reflection::description::IValueEnumerable;
				protected:
					compositions::GuiGraphicsComposition*				associatedComposition;
					Ptr<IValueEnumerable>								itemSource;
					ItemProperty<WString>								titleProperty;
					ItemProperty<Ptr<IValueEnumerable>>					childrenProperty;

					virtual void										OnJoinedItemSourceChanged(Ptr<IValueEnumerable> source) = 0;

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

					description::Value									GetGroupValue(vint groupIndex);
					description::Value									GetItemValue(GalleryPos pos);
				};

				/// <summary>Item arranger for the ribbon gallery control, which handle responsive layout.</summary>
				class GalleryItemArranger : public RangedItemArrangerBase, public Description<GalleryItemArranger>
				{
				private:
					vint										pim_itemWidth = 0;

				protected:
					void										BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)override;
					void										PlaceItem(bool forMoving, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)override;
					bool										IsItemOutOfViewBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)override;
					bool										EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)override;
					void										InvalidateItemSizeCache()override;
					Size										OnCalculateTotalSize()override;
				public:
					/// <summary>Create the arranger.</summary>
					GalleryItemArranger();
					~GalleryItemArranger();

					vint										FindItem(vint itemIndex, compositions::KeyDirection key)override;
					bool										EnsureItemVisible(vint itemIndex)override;
					Size										GetAdoptedSize(Size expectedSize)override;

					void										SetStartIndex(vint value);
				};
			}

			class GuiBindableRibbonGalleryList : public GuiRibbonGallery, public list::GroupedDataSource, public Description<GuiBindableRibbonGalleryList>
			{
				using IValueEnumerable = reflection::description::IValueEnumerable;
				using ItemStyleProperty = TemplateProperty<templates::GuiListItemTemplate>;

				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonGalleryListTemplate, GuiRibbonGallery)
			protected:
				ItemStyleProperty										itemStyle;
				GuiBindableTextList*									itemList;
				GuiRibbonToolstripMenu*									subMenu;

				void													OnJoinedItemSourceChanged(Ptr<IValueEnumerable> source)override;
				void													OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void													OnRequestedDropdown(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				GuiBindableRibbonGalleryList(theme::ThemeName themeName);
				~GuiBindableRibbonGalleryList();

				compositions::GuiNotifyEvent							ItemTemplateChanged;
				compositions::GuiNotifyEvent							SelectionChanged;

				/// <summary>Get the item style provider.</summary>
				/// <returns>The item style provider.</returns>
				ItemStyleProperty										GetItemTemplate();
				/// <summary>Set the item style provider</summary>
				/// <param name="value">The new item style provider</param>
				void													SetItemTemplate(ItemStyleProperty value);

				GalleryPos												GetSelection();
				void													SetSelection(GalleryPos value);

				GuiToolstripMenu*										GetSubMenu();
			};
		}
	}
}

#endif
