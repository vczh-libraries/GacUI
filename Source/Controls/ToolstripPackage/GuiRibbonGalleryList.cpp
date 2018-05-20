#include "GuiRibbonGalleryList.h"
#include "GuiRibbonImpl.h"
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

				auto cSize = ct->GetContainerComposition()->GetBounds();
				auto bSize = ct->GetBounds();
				layout->SetSizeOffset(Size(bSize.Width() - cSize.Width(), bSize.Height() - cSize.Height()));
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

			void GuiBindableRibbonGalleryList::OnRequestedScrollUp(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				itemListArranger->ScrollUp();
			}

			void GuiBindableRibbonGalleryList::OnRequestedScrollDown(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
			{
				itemListArranger->ScrollDown();
			}

			GuiBindableRibbonGalleryList::GuiBindableRibbonGalleryList(theme::ThemeName themeName)
				:GuiRibbonGallery(themeName)
				, GroupedDataSource(boundsComposition)
			{
				ItemTemplateChanged.SetAssociatedComposition(boundsComposition);
				SelectionChanged.SetAssociatedComposition(boundsComposition);

				layout = new ribbon_impl::GalleryResponsiveLayout;
				layout->SetAlignmentToParent(Margin(0, 0, 0, 0));
				containerComposition->AddChild(layout);

				itemListArranger = new ribbon_impl::GalleryItemArranger(this);
				itemList = new GuiBindableTextList(theme::ThemeName::RibbonGalleryItemList);
				itemList->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
				itemList->SetArranger(itemListArranger);
				layout->AddChild(itemList->GetBoundsComposition());

				subMenu = new GuiRibbonToolstripMenu(theme::ThemeName::RibbonToolstripMenu, this);

				RequestedScrollUp.AttachMethod(this, &GuiBindableRibbonGalleryList::OnRequestedScrollUp);
				RequestedScrollDown.AttachMethod(this, &GuiBindableRibbonGalleryList::OnRequestedScrollDown);
				RequestedDropdown.AttachMethod(this, &GuiBindableRibbonGalleryList::OnRequestedDropdown);
				boundsComposition->BoundsChanged.AttachMethod(this, &GuiBindableRibbonGalleryList::OnBoundsChanged);

				itemListArranger->UnblockScrollUpdate();
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

			vint GuiBindableRibbonGalleryList::GetMinCount()
			{
				return layout->GetMinCount();
			}

			void GuiBindableRibbonGalleryList::SetMinCount(vint value)
			{
				layout->SetMinCount(value);
			}

			vint GuiBindableRibbonGalleryList::GetMaxCount()
			{
				return layout->GetMaxCount();
			}

			void GuiBindableRibbonGalleryList::SetMaxCount(vint value)
			{
				layout->SetMaxCount(value);
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