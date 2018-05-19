#include "GuiRibbonGalleryList.h"
#include "../Templates/GuiThemeStyleFactory.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace reflection::description;
			using namespace collections;
			using namespace compositions;

/***********************************************************************
list::GroupedDataSource
***********************************************************************/

			namespace list
			{
				GroupedDataSource::GroupedDataSource(compositions::GuiGraphicsComposition* _associatedComposition)
					:associatedComposition(_associatedComposition)
				{
					GroupEnabledChanged.SetAssociatedComposition(associatedComposition);
					GroupTitlePropertyChanged.SetAssociatedComposition(associatedComposition);
					GroupChildrenPropertyChanged.SetAssociatedComposition(associatedComposition);
				}

				GroupedDataSource::~GroupedDataSource()
				{
				}

				Ptr<IValueEnumerable> GroupedDataSource::GetItemSource()
				{
					return itemSource;
				}

				void GroupedDataSource::SetItemSource(Ptr<IValueEnumerable> value)
				{
					itemSource = value;
					OnJoinedItemSourceChanged(itemSource);
				}

				bool GroupedDataSource::GetGroupEnabled()
				{
					return titleProperty && childrenProperty;
				}

				ItemProperty<WString> GroupedDataSource::GetGroupTitleProperty()
				{
					return titleProperty;
				}

				void GroupedDataSource::SetGroupTitleProperty(const ItemProperty<WString>& value)
				{
					if (titleProperty != value)
					{
						titleProperty = value;
						GroupTitlePropertyChanged.Execute(GuiEventArgs(associatedComposition));
						GroupEnabledChanged.Execute(GuiEventArgs(associatedComposition));
					}
				}

				ItemProperty<Ptr<IValueEnumerable>> GroupedDataSource::GetGroupChildrenProperty()
				{
					return childrenProperty;
				}

				void GroupedDataSource::SetGroupChildrenProperty(const ItemProperty<Ptr<IValueEnumerable>>& value)
				{
					if (childrenProperty != value)
					{
						childrenProperty = value;
						GroupChildrenPropertyChanged.Execute(GuiEventArgs(associatedComposition));
						GroupEnabledChanged.Execute(GuiEventArgs(associatedComposition));
					}
				}

				description::Value GroupedDataSource::GetGroupValue(vint groupIndex)
				{
					throw 0;
				}

				description::Value GroupedDataSource::GetItemValue(GalleryPos pos)
				{
					throw 0;
				}

/***********************************************************************
list::GalleryItemArranger
***********************************************************************/

				void GalleryItemArranger::BeginPlaceItem(bool forMoving, Rect newBounds, vint& newStartIndex)
				{
				}

				void GalleryItemArranger::PlaceItem(bool forMoving, vint index, ItemStyleRecord style, Rect viewBounds, Rect& bounds, Margin& alignmentToParent)
				{
				}

				bool GalleryItemArranger::IsItemOutOfViewBounds(vint index, ItemStyleRecord style, Rect bounds, Rect viewBounds)
				{
				}

				bool GalleryItemArranger::EndPlaceItem(bool forMoving, Rect newBounds, vint newStartIndex)
				{
				}

				void GalleryItemArranger::InvalidateItemSizeCache()
				{
				}

				Size GalleryItemArranger::OnCalculateTotalSize()
				{
				}

				GalleryItemArranger::GalleryItemArranger()
				{
				}

				GalleryItemArranger::~GalleryItemArranger()
				{
				}

				vint GalleryItemArranger::FindItem(vint itemIndex, compositions::KeyDirection key)
				{
				}

				bool GalleryItemArranger::EnsureItemVisible(vint itemIndex)
				{
				}

				Size GalleryItemArranger::GetAdoptedSize(Size expectedSize)
				{
				}

				void GalleryItemArranger::SetStartIndex(vint value)
				{
				}
			}

/***********************************************************************
GuiBindableRibbonGalleryList
***********************************************************************/

			void GuiBindableRibbonGalleryList::BeforeControlTemplateUninstalled_()
			{
			}

			void GuiBindableRibbonGalleryList::AfterControlTemplateInstalled_(bool initialize)
			{
				auto ct = GetControlTemplateObject();
				itemList->SetControlTemplate(ct->GetItemListTemplate());
				subMenu->SetControlTemplate(ct->GetMenuTemplate());
			}

			void GuiBindableRibbonGalleryList::OnJoinedItemSourceChanged(Ptr<IValueEnumerable> source)
			{
				itemList->SetItemSource(source);
			}

			void GuiBindableRibbonGalleryList::OnBoundsChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				subMenu->GetBoundsComposition()->SetPreferredMinSize(Size(boundsComposition->GetBounds().Width(), 1));
			}

			void GuiBindableRibbonGalleryList::OnRequestedDropdown(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				subMenu->ShowPopup(this, Point(0, 0));
			}

			GuiBindableRibbonGalleryList::GuiBindableRibbonGalleryList(theme::ThemeName themeName)
				:GuiRibbonGallery(themeName)
				, GroupedDataSource(boundsComposition)
			{
				ItemTemplateChanged.SetAssociatedComposition(boundsComposition);
				SelectionChanged.SetAssociatedComposition(boundsComposition);

				itemList = new GuiBindableTextList(theme::ThemeName::RibbonGalleryItemList);
				itemList->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				itemList->SetArranger(new list::GalleryItemArranger);
				containerComposition->AddChild(itemList->GetBoundsComposition());

				subMenu = new GuiRibbonToolstripMenu(theme::ThemeName::RibbonToolstripMenu, this);

				RequestedDropdown.AttachMethod(this, &GuiBindableRibbonGalleryList::OnRequestedDropdown);
				boundsComposition->BoundsChanged.AttachMethod(this, &GuiBindableRibbonGalleryList::OnBoundsChanged);
			}

			GuiBindableRibbonGalleryList::~GuiBindableRibbonGalleryList()
			{
				delete subMenu;
			}

			GuiBindableRibbonGalleryList::ItemStyleProperty GuiBindableRibbonGalleryList::GetItemTemplate()
			{
				return itemStyle;
			}

			void GuiBindableRibbonGalleryList::SetItemTemplate(ItemStyleProperty value)
			{
				if (itemStyle != value)
				{
					itemStyle = value;
					itemList->SetItemTemplate(value);
					ItemTemplateChanged.Execute(GetNotifyEventArguments());
				}
			}

			GalleryPos GuiBindableRibbonGalleryList::GetSelection()
			{
				throw 0;
			}

			void GuiBindableRibbonGalleryList::SetSelection(GalleryPos value)
			{
				throw 0;
			}

			GuiToolstripMenu* GuiBindableRibbonGalleryList::GetSubMenu()
			{
				return subMenu;
			}
		}
	}
}