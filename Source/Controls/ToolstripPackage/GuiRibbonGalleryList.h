/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIRIBBONGALLERYLIST
#define VCZH_PRESENTATION_CONTROLS_GUIRIBBONGALLERYLIST

#include "GuiRibbonControls.h"

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

			class GuiBindableRibbonGalleryList : public GuiRibbonGallery, public Description<GuiBindableRibbonGalleryList>
			{
				using IValueEnumerable = reflection::description::IValueEnumerable;
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(RibbonGalleryListTemplate, GuiRibbonGallery)
			protected:
				GuiBindableTextList*									itemList;
				GuiRibbonToolstripMenu*									subMenu;
				ItemProperty<WString>									titleProperty;
				ItemProperty<Ptr<IValueEnumerable>>						childrenProperty;

				void													OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void													OnRequestedDropdown(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				GuiBindableRibbonGalleryList(theme::ThemeName themeName);
				~GuiBindableRibbonGalleryList();

				compositions::GuiNotifyEvent							GroupEnabledChanged;
				compositions::GuiNotifyEvent							GroupTitlePropertyChanged;
				compositions::GuiNotifyEvent							GroupChildrenPropertyChanged;
				compositions::GuiNotifyEvent							SelectionChanged;

				Ptr<IValueEnumerable>									GetItemSource();
				void													SetItemSource(Ptr<IValueEnumerable> value);

				bool													GetGroupEnabled();
				ItemProperty<WString>									GetGroupTitleProperty();
				void													SetGroupTitleProperty(const ItemProperty<WString>& value);
				ItemProperty<Ptr<IValueEnumerable>>						GetGroupChildrenProperty();
				void													SetGroupChildrenProperty(const ItemProperty<Ptr<IValueEnumerable>>& value);

				GalleryPos												GetSelection();
				void													SetSelection(GalleryPos value);

				description::Value										GetGroupValue(vint groupIndex);
				description::Value										GetItemValue(GalleryPos pos);

				GuiToolstripMenu*										GetSubMenu();
			};
		}
	}
}

#endif
