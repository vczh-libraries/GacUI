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
			{
				GroupEnabledChanged.SetAssociatedComposition(boundsComposition);
				GroupTitlePropertyChanged.SetAssociatedComposition(boundsComposition);
				GroupChildrenPropertyChanged.SetAssociatedComposition(boundsComposition);
				SelectionChanged.SetAssociatedComposition(boundsComposition);

				itemList = new GuiBindableTextList(theme::ThemeName::RibbonGalleryItemList);
				itemList->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				containerComposition->AddChild(itemList->GetBoundsComposition());

				subMenu = new GuiRibbonToolstripMenu(theme::ThemeName::RibbonToolstripMenu, this);

				RequestedDropdown.AttachMethod(this, &GuiBindableRibbonGalleryList::OnRequestedDropdown);
				boundsComposition->BoundsChanged.AttachMethod(this, &GuiBindableRibbonGalleryList::OnBoundsChanged);
			}

			GuiBindableRibbonGalleryList::~GuiBindableRibbonGalleryList()
			{
				delete subMenu;
			}

			Ptr<IValueEnumerable> GuiBindableRibbonGalleryList::GetItemSource()
			{
				throw 0;
			}

			void GuiBindableRibbonGalleryList::SetItemSource(Ptr<IValueEnumerable> value)
			{
				throw 0;
			}

			bool GuiBindableRibbonGalleryList::GetGroupEnabled()
			{
				return titleProperty && childrenProperty;
			}

			ItemProperty<WString> GuiBindableRibbonGalleryList::GetGroupTitleProperty()
			{
				return titleProperty;
			}

			void GuiBindableRibbonGalleryList::SetGroupTitleProperty(const ItemProperty<WString>& value)
			{
				if (titleProperty != value)
				{
					titleProperty = value;
					GroupTitlePropertyChanged.Execute(GetNotifyEventArguments());
					GroupEnabledChanged.Execute(GetNotifyEventArguments());
				}
			}

			ItemProperty<Ptr<IValueEnumerable>> GuiBindableRibbonGalleryList::GetGroupChildrenProperty()
			{
				return childrenProperty;
			}

			void GuiBindableRibbonGalleryList::SetGroupChildrenProperty(const ItemProperty<Ptr<IValueEnumerable>>& value)
			{
				if (childrenProperty != value)
				{
					childrenProperty = value;
					GroupChildrenPropertyChanged.Execute(GetNotifyEventArguments());
					GroupEnabledChanged.Execute(GetNotifyEventArguments());
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

			description::Value GuiBindableRibbonGalleryList::GetGroupValue(vint groupIndex)
			{
				throw 0;
			}

			description::Value GuiBindableRibbonGalleryList::GetItemValue(GalleryPos pos)
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