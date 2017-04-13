#include "GuiListViewControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			using namespace elements;
			using namespace compositions;
			using namespace collections;
			using namespace reflection::description;

			namespace list
			{

/***********************************************************************
ListViewItemStyleProviderBase::TextItemStyleController
***********************************************************************/

				ListViewItemStyleProviderBase::ListViewItemStyleController::ListViewItemStyleController(ListViewItemStyleProviderBase* provider)
					:ItemStyleControllerBase(provider, 0)
					,backgroundButton(0)
					,listViewItemStyleProvider(provider)
				{
					backgroundButton=new GuiSelectableButton(listViewItemStyleProvider->listControl->GetListViewStyleProvider()->CreateItemBackground());
					backgroundButton->SetAutoSelection(false);
					Initialize(backgroundButton->GetBoundsComposition(), backgroundButton);
				}

				ListViewItemStyleProviderBase::ListViewItemStyleController::~ListViewItemStyleController()
				{
				}

				bool ListViewItemStyleProviderBase::ListViewItemStyleController::GetSelected()
				{
					return backgroundButton->GetSelected();
				}

				void ListViewItemStyleProviderBase::ListViewItemStyleController::SetSelected(bool value)
				{
					backgroundButton->SetSelected(value);
				}

/***********************************************************************
ListViewItemStyleProviderBase
***********************************************************************/

				ListViewItemStyleProviderBase::ListViewItemStyleProviderBase()
					:listControl(0)
				{
				}

				ListViewItemStyleProviderBase::~ListViewItemStyleProviderBase()
				{
				}

				void ListViewItemStyleProviderBase::AttachListControl(GuiListControl* value)
				{
					listControl=dynamic_cast<GuiListViewBase*>(value);
				}

				void ListViewItemStyleProviderBase::DetachListControl()
				{
					listControl=0;
				}

				void ListViewItemStyleProviderBase::SetStyleSelected(GuiListControl::IItemStyleController* style, bool value)
				{
					ListViewItemStyleController* textStyle=dynamic_cast<ListViewItemStyleController*>(style);
					textStyle->SetSelected(value);
				}
			}

/***********************************************************************
GuiListViewColumnHeader
***********************************************************************/
			
			GuiListViewColumnHeader::GuiListViewColumnHeader(IStyleController* _styleController)
				:GuiMenuButton(_styleController)
				,columnSortingState(NotSorted)
				,styleController(_styleController)
			{
				styleController->SetColumnSortingState(columnSortingState);
			}

			GuiListViewColumnHeader::~GuiListViewColumnHeader()
			{
			}

			bool GuiListViewColumnHeader::IsAltAvailable()
			{
				return false;
			}

			GuiListViewColumnHeader::ColumnSortingState GuiListViewColumnHeader::GetColumnSortingState()
			{
				return columnSortingState;
			}

			void GuiListViewColumnHeader::SetColumnSortingState(ColumnSortingState value)
			{
				if(columnSortingState!=value)
				{
					columnSortingState=value;
					styleController->SetColumnSortingState(columnSortingState);
				}
			}

/***********************************************************************
GuiListViewBase
***********************************************************************/

			GuiListViewBase::GuiListViewBase(IStyleProvider* _styleProvider, GuiListControl::IItemProvider* _itemProvider)
				:GuiSelectableListControl(_styleProvider, _itemProvider)
				,styleProvider(0)
			{
				styleProvider=dynamic_cast<IStyleProvider*>(styleController->GetStyleProvider());
				ColumnClicked.SetAssociatedComposition(boundsComposition);
			}

			GuiListViewBase::~GuiListViewBase()
			{
			}

			GuiListViewBase::IStyleProvider* GuiListViewBase::GetListViewStyleProvider()
			{
				return styleProvider;
			}

			Ptr<GuiListControl::IItemStyleProvider> GuiListViewBase::SetStyleProvider(Ptr<GuiListControl::IItemStyleProvider> value)
			{
				if(value.Cast<list::ListViewItemStyleProvider>())
				{
					return GuiSelectableListControl::SetStyleProvider(value);
				}
				else
				{
					return 0;
				}
			}

			namespace list
			{

/***********************************************************************
ListViewItemStyleProvider::ListViewContentItemStyleController
***********************************************************************/

				ListViewItemStyleProvider::ListViewContentItemStyleController::ListViewContentItemStyleController(ListViewItemStyleProvider* provider)
					:ListViewItemStyleController(provider)
					,listViewItemStyleProvider(provider)
				{
					content=listViewItemStyleProvider->listViewItemContentProvider->CreateItemContent(backgroundButton->GetFont());
					GuiBoundsComposition* composition=content->GetContentComposition();
					composition->SetAlignmentToParent(Margin(0, 0, 0, 0));
					backgroundButton->GetContainerComposition()->AddChild(composition);

					GuiBoundsComposition* decorator=content->GetBackgroundDecorator();
					if(decorator)
					{
						backgroundButton->GetBoundsComposition()->AddChild(decorator);
						backgroundButton->GetBoundsComposition()->MoveChild(decorator, 0);
					}
				}

				ListViewItemStyleProvider::ListViewContentItemStyleController::~ListViewContentItemStyleController()
				{
				}

				void ListViewItemStyleProvider::ListViewContentItemStyleController::OnUninstalled()
				{
					ListViewItemStyleController::OnUninstalled();
					content->Uninstall();
				}

				ListViewItemStyleProvider::IListViewItemContent* ListViewItemStyleProvider::ListViewContentItemStyleController::GetItemContent()
				{
					return content.Obj();
				}

				void ListViewItemStyleProvider::ListViewContentItemStyleController::Install(IListViewItemView* view, vint itemIndex)
				{
					content->Install(listViewItemStyleProvider->listControl->GetListViewStyleProvider(), view, itemIndex);
				}

/***********************************************************************
ListViewItemStyleProvider
***********************************************************************/

				const wchar_t* const ListViewItemStyleProvider::IListViewItemView::Identifier = L"vl::presentation::controls::list::ListViewItemStyleProvider::IListViewItemView";

				ListViewItemStyleProvider::ListViewItemStyleProvider(Ptr<IListViewItemContentProvider> itemContentProvider)
					:listViewItemView(0)
					,listViewItemContentProvider(itemContentProvider)
				{
				}

				ListViewItemStyleProvider::~ListViewItemStyleProvider()
				{
				}

				void ListViewItemStyleProvider::AttachListControl(GuiListControl* value)
				{
					ListViewItemStyleProviderBase::AttachListControl(value);
					listViewItemView=dynamic_cast<IListViewItemView*>(value->GetItemProvider()->RequestView(IListViewItemView::Identifier));
					listViewItemContentProvider->AttachListControl(value);
				}

				void ListViewItemStyleProvider::DetachListControl()
				{
					listViewItemContentProvider->DetachListControl();
					listControl->GetItemProvider()->ReleaseView(listViewItemView);
					listViewItemView=0;
					ListViewItemStyleProviderBase::DetachListControl();
				}

				GuiListControl::IItemStyleController* ListViewItemStyleProvider::CreateItemStyle()
				{
					ListViewContentItemStyleController* itemStyle=new ListViewContentItemStyleController(this);
					itemStyles.Add(itemStyle);
					return itemStyle;
				}

				void ListViewItemStyleProvider::DestroyItemStyle(GuiListControl::IItemStyleController* style)
				{
					ListViewContentItemStyleController* itemStyle=dynamic_cast<ListViewContentItemStyleController*>(style);
					if(itemStyle)
					{
						itemStyles.Remove(itemStyle);
						delete itemStyle;
					}
				}

				void ListViewItemStyleProvider::Install(GuiListControl::IItemStyleController* style, vint itemIndex)
				{
					ListViewContentItemStyleController* itemStyle=dynamic_cast<ListViewContentItemStyleController*>(style);
					itemStyle->Install(listViewItemView, itemIndex);
				}

				void ListViewItemStyleProvider::SetStyleIndex(GuiListControl::IItemStyleController* style, vint value)
				{
				}

				ListViewItemStyleProvider::IListViewItemContentProvider* ListViewItemStyleProvider::GetItemContentProvider()
				{
					return listViewItemContentProvider.Obj();
				}

				const ListViewItemStyleProvider::ItemStyleList& ListViewItemStyleProvider::GetCreatedItemStyles()
				{
					return itemStyles;
				}

				bool ListViewItemStyleProvider::IsItemStyleAttachedToListView(GuiListControl::IItemStyleController* itemStyle)
				{
					return itemStyle && itemStyle->GetBoundsComposition()->GetParent();
				}

				ListViewItemStyleProvider::IListViewItemContent* ListViewItemStyleProvider::GetItemContentFromItemStyleController(GuiListControl::IItemStyleController* itemStyleController)
				{
					if(itemStyleController)
					{
						ListViewContentItemStyleController* item=dynamic_cast<ListViewContentItemStyleController*>(itemStyleController);
						if(item)
						{
							return item->GetItemContent();
						}
					}
					return 0;
				}

				GuiListControl::IItemStyleController* ListViewItemStyleProvider::GetItemStyleControllerFromItemContent(IListViewItemContent* itemContent)
				{
					FOREACH(GuiListControl::IItemStyleController*, style, itemStyles)
					{
						if(GetItemContentFromItemStyleController(style)==itemContent)
						{
							return style;
						}
					}
					return 0;
				}
				
/***********************************************************************
ListViewBigIconContentProvider
***********************************************************************/

				ListViewBigIconContentProvider::ItemContent::ItemContent(Size minIconSize, bool fitImage, const FontProperties& font)
					:contentComposition(0)
				{
					contentComposition=new GuiBoundsComposition;
					contentComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					GuiTableComposition* table=new GuiTableComposition;
					contentComposition->AddChild(table);
					table->SetRowsAndColumns(2, 3);
					table->SetRowOption(0, GuiCellOption::MinSizeOption());
					table->SetRowOption(1, GuiCellOption::MinSizeOption());
					table->SetColumnOption(0, GuiCellOption::PercentageOption(0.5));
					table->SetColumnOption(1, GuiCellOption::MinSizeOption());
					table->SetColumnOption(2, GuiCellOption::PercentageOption(0.5));
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetCellPadding(5);
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 1, 1);
						cell->SetPreferredMinSize(minIconSize);
						if (!fitImage)
						{
							cell->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
						}

						image=GuiImageFrameElement::Create();
						image->SetStretch(true);
						cell->SetOwnedElement(image);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
						cell->SetSite(1, 0, 1, 3);
						cell->SetPreferredMinSize(Size(64, 40));

						text=GuiSolidLabelElement::Create();
						text->SetAlignments(Alignment::Center, Alignment::Top);
						text->SetFont(font);
						text->SetWrapLine(true);
						text->SetEllipse(true);
						cell->SetOwnedElement(text);
					}
				}

				ListViewBigIconContentProvider::ItemContent::~ItemContent()
				{
				}

				compositions::GuiBoundsComposition* ListViewBigIconContentProvider::ItemContent::GetContentComposition()
				{
					return contentComposition;
				}

				compositions::GuiBoundsComposition* ListViewBigIconContentProvider::ItemContent::GetBackgroundDecorator()
				{
					return 0;
				}

				void ListViewBigIconContentProvider::ItemContent::Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)
				{
					Ptr<GuiImageData> imageData=view->GetLargeImage(itemIndex);
					if(imageData)
					{
						image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
					}
					else
					{
						image->SetImage(0);
					}
					text->SetText(view->GetText(itemIndex));
					text->SetColor(styleProvider->GetPrimaryTextColor());
				}

				void ListViewBigIconContentProvider::ItemContent::Uninstall()
				{
				}

				ListViewBigIconContentProvider::ListViewBigIconContentProvider(Size _minIconSize, bool _fitImage)
					:minIconSize(_minIconSize)
					,fitImage(_fitImage)
				{
				}

				ListViewBigIconContentProvider::~ListViewBigIconContentProvider()
				{
				}

				compositions::IGuiAxis* ListViewBigIconContentProvider::CreatePreferredAxis()
				{
					return new GuiDefaultAxis;
				}

				GuiListControl::IItemArranger* ListViewBigIconContentProvider::CreatePreferredArranger()
				{
					return new FixedSizeMultiColumnItemArranger;
				}

				ListViewItemStyleProvider::IListViewItemContent* ListViewBigIconContentProvider::CreateItemContent(const FontProperties& font)
				{
					return new ItemContent(minIconSize, fitImage, font);
				}

				void ListViewBigIconContentProvider::AttachListControl(GuiListControl* value)
				{
				}

				void ListViewBigIconContentProvider::DetachListControl()
				{
				}
				
/***********************************************************************
ListViewSmallIconContentProvider
***********************************************************************/

				ListViewSmallIconContentProvider::ItemContent::ItemContent(Size minIconSize, bool fitImage, const FontProperties& font)
					:contentComposition(0)
				{
					contentComposition=new GuiBoundsComposition;
					contentComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					GuiTableComposition* table=new GuiTableComposition;
					contentComposition->AddChild(table);
					table->SetRowsAndColumns(3, 2);
					table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
					table->SetRowOption(1, GuiCellOption::MinSizeOption());
					table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());
					table->SetColumnOption(1, GuiCellOption::MinSizeOption());
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetCellPadding(2);
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(1, 0, 1, 1);
						cell->SetPreferredMinSize(minIconSize);
						if (!fitImage)
						{
							cell->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
						}

						image=GuiImageFrameElement::Create();
						image->SetStretch(true);
						cell->SetOwnedElement(image);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 3, 1);
						cell->SetPreferredMinSize(Size(192, 0));

						text=GuiSolidLabelElement::Create();
						text->SetAlignments(Alignment::Left, Alignment::Center);
						text->SetFont(font);
						text->SetEllipse(true);
						cell->SetOwnedElement(text);
					}
				}

				ListViewSmallIconContentProvider::ItemContent::~ItemContent()
				{
				}

				compositions::GuiBoundsComposition* ListViewSmallIconContentProvider::ItemContent::GetContentComposition()
				{
					return contentComposition;
				}

				compositions::GuiBoundsComposition* ListViewSmallIconContentProvider::ItemContent::GetBackgroundDecorator()
				{
					return 0;
				}

				void ListViewSmallIconContentProvider::ItemContent::Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)
				{
					Ptr<GuiImageData> imageData=view->GetSmallImage(itemIndex);
					if(imageData)
					{
						image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
					}
					else
					{
						image->SetImage(0);
					}
					text->SetText(view->GetText(itemIndex));
					text->SetColor(styleProvider->GetPrimaryTextColor());
				}

				void ListViewSmallIconContentProvider::ItemContent::Uninstall()
				{
				}

				ListViewSmallIconContentProvider::ListViewSmallIconContentProvider(Size _minIconSize, bool _fitImage)
					:minIconSize(_minIconSize)
					,fitImage(_fitImage)
				{
				}

				ListViewSmallIconContentProvider::~ListViewSmallIconContentProvider()
				{
				}

				compositions::IGuiAxis* ListViewSmallIconContentProvider::CreatePreferredAxis()
				{
					return new GuiDefaultAxis;
				}

				GuiListControl::IItemArranger* ListViewSmallIconContentProvider::CreatePreferredArranger()
				{
					return new FixedSizeMultiColumnItemArranger;
				}

				ListViewItemStyleProvider::IListViewItemContent* ListViewSmallIconContentProvider::CreateItemContent(const FontProperties& font)
				{
					return new ItemContent(minIconSize, fitImage, font);
				}

				void ListViewSmallIconContentProvider::AttachListControl(GuiListControl* value)
				{
				}

				void ListViewSmallIconContentProvider::DetachListControl()
				{
				}
				
/***********************************************************************
ListViewListContentProvider
***********************************************************************/

				ListViewListContentProvider::ItemContent::ItemContent(Size minIconSize, bool fitImage, const FontProperties& font)
					:contentComposition(0)
				{
					contentComposition=new GuiBoundsComposition;
					contentComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					GuiTableComposition* table=new GuiTableComposition;
					contentComposition->AddChild(table);
					table->SetRowsAndColumns(3, 2);
					table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
					table->SetRowOption(1, GuiCellOption::MinSizeOption());
					table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());
					table->SetColumnOption(1, GuiCellOption::MinSizeOption());
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetCellPadding(2);
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(1, 0, 1, 1);
						cell->SetPreferredMinSize(minIconSize);
						if (!fitImage)
						{
							cell->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
						}

						image=GuiImageFrameElement::Create();
						image->SetStretch(true);
						cell->SetOwnedElement(image);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 3, 1);
						cell->SetMargin(Margin(0, 0, 16, 0));

						text=GuiSolidLabelElement::Create();
						text->SetAlignments(Alignment::Left, Alignment::Center);
						text->SetFont(font);
						cell->SetOwnedElement(text);
					}
				}

				ListViewListContentProvider::ItemContent::~ItemContent()
				{
				}

				compositions::GuiBoundsComposition* ListViewListContentProvider::ItemContent::GetContentComposition()
				{
					return contentComposition;
				}

				compositions::GuiBoundsComposition* ListViewListContentProvider::ItemContent::GetBackgroundDecorator()
				{
					return 0;
				}

				void ListViewListContentProvider::ItemContent::Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)
				{
					Ptr<GuiImageData> imageData=view->GetSmallImage(itemIndex);
					if(imageData)
					{
						image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
					}
					else
					{
						image->SetImage(0);
					}
					text->SetText(view->GetText(itemIndex));
					text->SetColor(styleProvider->GetPrimaryTextColor());
				}

				void ListViewListContentProvider::ItemContent::Uninstall()
				{
				}

				ListViewListContentProvider::ListViewListContentProvider(Size _minIconSize, bool _fitImage)
					:minIconSize(_minIconSize)
					,fitImage(_fitImage)
				{
				}

				ListViewListContentProvider::~ListViewListContentProvider()
				{
				}

				compositions::IGuiAxis* ListViewListContentProvider::CreatePreferredAxis()
				{
					return new GuiDefaultAxis;
				}

				GuiListControl::IItemArranger* ListViewListContentProvider::CreatePreferredArranger()
				{
					return new FixedHeightMultiColumnItemArranger;
				}

				ListViewItemStyleProvider::IListViewItemContent* ListViewListContentProvider::CreateItemContent(const FontProperties& font)
				{
					return new ItemContent(minIconSize, fitImage, font);
				}

				void ListViewListContentProvider::AttachListControl(GuiListControl* value)
				{
				}

				void ListViewListContentProvider::DetachListControl()
				{
				}
				
/***********************************************************************
ListViewTileContentProvider
***********************************************************************/

				void ListViewTileContentProvider::ItemContent::RemoveTextElement(vint textRow)
				{
					GuiCellComposition* cell=textTable->GetSitedCell(textRow+1, 0);
					textTable->RemoveChild(cell);
					delete cell;
				}

				elements::GuiSolidLabelElement* ListViewTileContentProvider::ItemContent::CreateTextElement(vint textRow, const FontProperties& font)
				{
					GuiCellComposition* cell=new GuiCellComposition;
					textTable->AddChild(cell);
					cell->SetSite(textRow+1, 0, 1, 1);

					elements::GuiSolidLabelElement* textElement=GuiSolidLabelElement::Create();
					textElement->SetAlignments(Alignment::Left, Alignment::Center);
					textElement->SetFont(font);
					textElement->SetEllipse(true);
					cell->SetOwnedElement(textElement);
					return textElement;
				}

				void ListViewTileContentProvider::ItemContent::ResetTextTable(vint textRows)
				{
					textTable->SetRowsAndColumns(textRows+2, 1);
					textTable->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
					for(vint i=0;i<textRows;i++)
					{
						textTable->SetRowOption(i+1, GuiCellOption::MinSizeOption());
					}
					textTable->SetRowOption(textRows+1, GuiCellOption::PercentageOption(0.5));
					textTable->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
				}

				ListViewTileContentProvider::ItemContent::ItemContent(Size minIconSize, bool fitImage, const FontProperties& font)
					:contentComposition(0)
				{
					contentComposition=new GuiBoundsComposition;
					contentComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					GuiTableComposition* table=new GuiTableComposition;
					contentComposition->AddChild(table);
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					table->SetRowsAndColumns(3, 2);
					table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
					table->SetRowOption(1, GuiCellOption::MinSizeOption());
					table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());
					table->SetColumnOption(1, GuiCellOption::MinSizeOption());
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetCellPadding(4);
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(1, 0, 1, 1);
						cell->SetPreferredMinSize(minIconSize);
						if (!fitImage)
						{
							cell->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
						}

						image=GuiImageFrameElement::Create();
						image->SetStretch(true);
						cell->SetOwnedElement(image);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 3, 1);
						cell->SetPreferredMinSize(Size(224, 0));

						textTable=new GuiTableComposition;
						textTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						textTable->SetCellPadding(1);
						ResetTextTable(1);
						textTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
						cell->AddChild(textTable);
						{
							text=CreateTextElement(0, font);
						}
					}
				}

				ListViewTileContentProvider::ItemContent::~ItemContent()
				{
				}

				compositions::GuiBoundsComposition* ListViewTileContentProvider::ItemContent::GetContentComposition()
				{
					return contentComposition;
				}

				compositions::GuiBoundsComposition* ListViewTileContentProvider::ItemContent::GetBackgroundDecorator()
				{
					return 0;
				}

				void ListViewTileContentProvider::ItemContent::Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)
				{
					Ptr<GuiImageData> imageData=view->GetLargeImage(itemIndex);
					if(imageData)
					{
						image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
					}
					else
					{
						image->SetImage(0);
					}
					text->SetText(view->GetText(itemIndex));
					text->SetColor(styleProvider->GetPrimaryTextColor());

					for(vint i=0;i<dataTexts.Count();i++)
					{
						RemoveTextElement(i+1);
					}
					vint dataColumnCount=view->GetDataColumnCount();
					ResetTextTable(dataColumnCount+1);
					dataTexts.Resize(dataColumnCount);
					for(vint i=0;i<dataColumnCount;i++)
					{
						dataTexts[i]=CreateTextElement(i+1, text->GetFont());
						dataTexts[i]->SetText(view->GetSubItem(itemIndex, view->GetDataColumn(i)));
						dataTexts[i]->SetColor(styleProvider->GetSecondaryTextColor());
					}
				}

				void ListViewTileContentProvider::ItemContent::Uninstall()
				{
				}

				ListViewTileContentProvider::ListViewTileContentProvider(Size _minIconSize, bool _fitImage)
					:minIconSize(_minIconSize)
					,fitImage(_fitImage)
				{
				}

				ListViewTileContentProvider::~ListViewTileContentProvider()
				{
				}

				compositions::IGuiAxis* ListViewTileContentProvider::CreatePreferredAxis()
				{
					return new GuiDefaultAxis;
				}

				GuiListControl::IItemArranger* ListViewTileContentProvider::CreatePreferredArranger()
				{
					return new FixedSizeMultiColumnItemArranger;
				}

				ListViewItemStyleProvider::IListViewItemContent* ListViewTileContentProvider::CreateItemContent(const FontProperties& font)
				{
					return new ItemContent(minIconSize, fitImage, font);
				}

				void ListViewTileContentProvider::AttachListControl(GuiListControl* value)
				{
				}

				void ListViewTileContentProvider::DetachListControl()
				{
				}
				
/***********************************************************************
ListViewInformationContentProvider
***********************************************************************/

				ListViewInformationContentProvider::ItemContent::ItemContent(Size minIconSize, bool fitImage, const FontProperties& font)
					:contentComposition(0)
					,baselineFont(font)
				{
					contentComposition=new GuiBoundsComposition;
					contentComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					{
						bottomLine=GuiSolidBackgroundElement::Create();
						bottomLineComposition=new GuiBoundsComposition;
						bottomLineComposition->SetOwnedElement(bottomLine);
						bottomLineComposition->SetAlignmentToParent(Margin(8, -1, 8, 0));
						bottomLineComposition->SetPreferredMinSize(Size(0, 1));
					}

					GuiTableComposition* table=new GuiTableComposition;
					contentComposition->AddChild(table);
					table->SetRowsAndColumns(3, 3);
					table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
					table->SetRowOption(1, GuiCellOption::MinSizeOption());
					table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());
					table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
					table->SetColumnOption(2, GuiCellOption::MinSizeOption());
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));
					table->SetCellPadding(4);
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(1, 0, 1, 1);
						cell->SetPreferredMinSize(minIconSize);
						if (!fitImage)
						{
							cell->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
						}

						image=GuiImageFrameElement::Create();
						image->SetStretch(true);
						cell->SetOwnedElement(image);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 3, 1);

						FontProperties textFont=font;
						textFont.size=(vint)(textFont.size*1.2);

						text=GuiSolidLabelElement::Create();
						text->SetFont(textFont);
						text->SetEllipse(true);
						cell->SetOwnedElement(text);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 2, 3, 1);
						cell->SetPreferredMinSize(Size(224, 0));

						textTable=new GuiTableComposition;
						textTable->SetCellPadding(4);
						textTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						textTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
						cell->AddChild(textTable);
					}
				}

				ListViewInformationContentProvider::ItemContent::~ItemContent()
				{
				}

				compositions::GuiBoundsComposition* ListViewInformationContentProvider::ItemContent::GetContentComposition()
				{
					return contentComposition;
				}

				compositions::GuiBoundsComposition* ListViewInformationContentProvider::ItemContent::GetBackgroundDecorator()
				{
					return bottomLineComposition;
				}

				void ListViewInformationContentProvider::ItemContent::Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)
				{
					Ptr<GuiImageData> imageData = view->GetLargeImage(itemIndex);
					if (imageData)
					{
						image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
					}
					else
					{
						image->SetImage(0);
					}
					text->SetText(view->GetText(itemIndex));
					text->SetColor(styleProvider->GetPrimaryTextColor());
					bottomLine->SetColor(styleProvider->GetItemSeparatorColor());

					for (vint i = 0; i < dataTexts.Count(); i++)
					{
						GuiCellComposition* cell = textTable->GetSitedCell(i, 0);
						textTable->RemoveChild(cell);
						delete cell;
					}

					vint dataColumnCount = view->GetDataColumnCount();
					dataTexts.Resize(dataColumnCount);
					textTable->SetRowsAndColumns(dataColumnCount + 2, 1);
					textTable->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
					for (vint i = 0; i < dataColumnCount; i++)
					{
						textTable->SetRowOption(i + 1, GuiCellOption::MinSizeOption());
					}
					textTable->SetRowOption(dataColumnCount + 1, GuiCellOption::PercentageOption(0.5));
					textTable->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
					
					for (vint i = 0; i < dataColumnCount; i++)
					{
						GuiCellComposition* cell = new GuiCellComposition;
						textTable->AddChild(cell);
						cell->SetSite(i + 1, 0, 1, 1);

						GuiTableComposition* dataTable = new GuiTableComposition;
						dataTable->SetRowsAndColumns(1, 2);
						dataTable->SetRowOption(0, GuiCellOption::MinSizeOption());
						dataTable->SetColumnOption(0, GuiCellOption::MinSizeOption());
						dataTable->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
						dataTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						dataTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
						cell->AddChild(dataTable);
						{
							GuiCellComposition* cell = new GuiCellComposition;
							dataTable->AddChild(cell);
							cell->SetSite(0, 0, 1, 1);

							GuiSolidLabelElement* textColumn = GuiSolidLabelElement::Create();
							textColumn->SetFont(baselineFont);
							textColumn->SetText(view->GetColumnText(view->GetDataColumn(i) + 1) + L": ");
							textColumn->SetColor(styleProvider->GetSecondaryTextColor());
							cell->SetOwnedElement(textColumn);
						}
						{
							GuiCellComposition* cell = new GuiCellComposition;
							dataTable->AddChild(cell);
							cell->SetSite(0, 1, 1, 1);

							GuiSolidLabelElement* textData = GuiSolidLabelElement::Create();
							textData->SetFont(baselineFont);
							textData->SetEllipse(true);
							textData->SetText(view->GetSubItem(itemIndex, view->GetDataColumn(i)));
							textData->SetColor(styleProvider->GetPrimaryTextColor());
							cell->SetOwnedElement(textData);
						}
					}
				}

				void ListViewInformationContentProvider::ItemContent::Uninstall()
				{
				}

				ListViewInformationContentProvider::ListViewInformationContentProvider(Size _minIconSize, bool _fitImage)
					:minIconSize(_minIconSize)
					,fitImage(_fitImage)
				{
				}

				ListViewInformationContentProvider::~ListViewInformationContentProvider()
				{
				}

				compositions::IGuiAxis* ListViewInformationContentProvider::CreatePreferredAxis()
				{
					return new GuiDefaultAxis;
				}

				GuiListControl::IItemArranger* ListViewInformationContentProvider::CreatePreferredArranger()
				{
					return new FixedHeightItemArranger;
				}

				ListViewItemStyleProvider::IListViewItemContent* ListViewInformationContentProvider::CreateItemContent(const FontProperties& font)
				{
					return new ItemContent(minIconSize, fitImage, font);
				}

				void ListViewInformationContentProvider::AttachListControl(GuiListControl* value)
				{
				}

				void ListViewInformationContentProvider::DetachListControl()
				{
				}
				
/***********************************************************************
ListViewColumnItemArranger::ColumnItemViewCallback
***********************************************************************/

				ListViewColumnItemArranger::ColumnItemViewCallback::ColumnItemViewCallback(ListViewColumnItemArranger* _arranger)
					:arranger(_arranger)
				{
				}

				ListViewColumnItemArranger::ColumnItemViewCallback::~ColumnItemViewCallback()
				{
				}

				void ListViewColumnItemArranger::ColumnItemViewCallback::OnColumnChanged()
				{
					arranger->RebuildColumns();
				}
				
/***********************************************************************
ListViewColumnItemArranger
***********************************************************************/

				const wchar_t* const ListViewColumnItemArranger::IColumnItemView::Identifier = L"vl::presentation::controls::list::ListViewColumnItemArranger::IColumnItemView";

				void ListViewColumnItemArranger::ColumnClicked(vint index, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					GuiItemEventArgs args(listView->ColumnClicked.GetAssociatedComposition());
					args.itemIndex=index;
					listView->ColumnClicked.Execute(args);
				}

				void ListViewColumnItemArranger::ColumnBoundsChanged(vint index, compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					GuiBoundsComposition* buttonBounds=columnHeaderButtons[index]->GetBoundsComposition();
					vint size=buttonBounds->GetBounds().Width();
					if(size>columnItemView->GetColumnSize(index))
					{
						columnItemView->SetColumnSize(index, size);
					}
				}

				void ListViewColumnItemArranger::ColumnHeaderSplitterLeftButtonDown(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					if(listView->GetVisuallyEnabled())
					{
						arguments.handled=true;
						splitterDragging=true;
						splitterLatestX=arguments.x;
					}
				}

				void ListViewColumnItemArranger::ColumnHeaderSplitterLeftButtonUp(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					if(listView->GetVisuallyEnabled())
					{
						arguments.handled=true;
						splitterDragging=false;
						splitterLatestX=0;
					}
				}

				void ListViewColumnItemArranger::ColumnHeaderSplitterMouseMove(compositions::GuiGraphicsComposition* sender, compositions::GuiMouseEventArgs& arguments)
				{
					if(splitterDragging)
					{
						vint offset=arguments.x-splitterLatestX;
						vint index=columnHeaderSplitters.IndexOf(dynamic_cast<GuiBoundsComposition*>(sender));
						if(index!=-1)
						{
							GuiBoundsComposition* buttonBounds=columnHeaderButtons[index]->GetBoundsComposition();
							Rect bounds=buttonBounds->GetBounds();
							Rect newBounds(bounds.LeftTop(), Size(bounds.Width()+offset, bounds.Height()));
							buttonBounds->SetBounds(newBounds);

							vint finalSize=buttonBounds->GetBounds().Width();
							columnItemView->SetColumnSize(index, finalSize);
						}
					}
				}

				void ListViewColumnItemArranger::RearrangeItemBounds()
				{
					FixedHeightItemArranger::RearrangeItemBounds();
					vint count = columnHeaders->GetParent()->Children().Count();
					columnHeaders->GetParent()->MoveChild(columnHeaders, count - 1);
					columnHeaders->SetBounds(Rect(Point(-viewBounds.Left(), 0), Size(0, 0)));
				}

				vint ListViewColumnItemArranger::GetWidth()
				{
					vint width=columnHeaders->GetBounds().Width()-SplitterWidth;
					if(width<SplitterWidth)
					{
						width=SplitterWidth;
					}
					return width;
				}

				vint ListViewColumnItemArranger::GetYOffset()
				{
					return columnHeaders->GetBounds().Height();
				}

				Size ListViewColumnItemArranger::OnCalculateTotalSize()
				{
					Size size=FixedHeightItemArranger::OnCalculateTotalSize();
					size.x+=SplitterWidth;
					return size;
				}

				void ListViewColumnItemArranger::DeleteColumnButtons()
				{
					for(vint i=columnHeaders->GetStackItems().Count()-1;i>=0;i--)
					{
						GuiStackItemComposition* item=columnHeaders->GetStackItems().Get(i);
						columnHeaders->RemoveChild(item);

						GuiControl* button=item->Children().Get(0)->GetAssociatedControl();
						if(button)
						{
							item->RemoveChild(button->GetBoundsComposition());
							delete button;
						}
						delete item;
					}
					columnHeaderButtons.Clear();
					columnHeaderSplitters.Clear();
				}

				void ListViewColumnItemArranger::RebuildColumns()
				{
					if(columnItemView && columnHeaderButtons.Count()==columnItemView->GetColumnCount())
					{
						for(vint i=0;i<columnItemView->GetColumnCount();i++)
						{
							GuiListViewColumnHeader* button=columnHeaderButtons[i];
							button->SetText(columnItemView->GetColumnText(i));
							button->SetSubMenu(columnItemView->GetDropdownPopup(i), false);
							button->SetColumnSortingState(columnItemView->GetSortingState(i));
							button->GetBoundsComposition()->SetBounds(Rect(Point(0, 0), Size(columnItemView->GetColumnSize(i), 0)));
						}
					}
					else
					{
						DeleteColumnButtons();
						if(columnItemView)
						{
							for(vint i=0;i<columnItemView->GetColumnCount();i++)
							{
								GuiBoundsComposition* splitterComposition=new GuiBoundsComposition;
								splitterComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
								splitterComposition->SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::SizeWE));
								splitterComposition->SetAlignmentToParent(Margin(0, 0, -1, 0));
								splitterComposition->SetPreferredMinSize(Size(SplitterWidth, 0));
								columnHeaderSplitters.Add(splitterComposition);

								splitterComposition->GetEventReceiver()->leftButtonDown.AttachMethod(this, &ListViewColumnItemArranger::ColumnHeaderSplitterLeftButtonDown);
								splitterComposition->GetEventReceiver()->leftButtonUp.AttachMethod(this, &ListViewColumnItemArranger::ColumnHeaderSplitterLeftButtonUp);
								splitterComposition->GetEventReceiver()->mouseMove.AttachMethod(this, &ListViewColumnItemArranger::ColumnHeaderSplitterMouseMove);
							}
							for(vint i=0;i<columnItemView->GetColumnCount();i++)
							{
								GuiListViewColumnHeader* button=new GuiListViewColumnHeader(styleProvider->CreateColumnStyle());
								button->SetText(columnItemView->GetColumnText(i));
								button->SetSubMenu(columnItemView->GetDropdownPopup(i), false);
								button->SetColumnSortingState(columnItemView->GetSortingState(i));
								button->GetBoundsComposition()->SetBounds(Rect(Point(0, 0), Size(columnItemView->GetColumnSize(i), 0)));
								button->Clicked.AttachLambda(Curry(Func<void(vint, GuiGraphicsComposition*, GuiEventArgs&)>(this, &ListViewColumnItemArranger::ColumnClicked))(i));
								button->GetBoundsComposition()->BoundsChanged.AttachLambda(Curry(Func<void(vint, GuiGraphicsComposition*, GuiEventArgs&)>(this, &ListViewColumnItemArranger::ColumnBoundsChanged))(i));
								columnHeaderButtons.Add(button);
								if(i>0)
								{
									button->GetContainerComposition()->AddChild(columnHeaderSplitters[i-1]);
								}

								GuiStackItemComposition* item=new GuiStackItemComposition;
								item->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
								item->AddChild(button->GetBoundsComposition());
								columnHeaders->AddChild(item);
							}
							if(columnItemView->GetColumnCount()>0)
							{
								GuiBoundsComposition* splitterComposition=columnHeaderSplitters[columnItemView->GetColumnCount()-1];

								GuiStackItemComposition* item=new GuiStackItemComposition;
								item->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
								item->AddChild(splitterComposition);
								columnHeaders->AddChild(item);
							}
						}
					}
					callback->OnTotalSizeChanged();
				}

				ListViewColumnItemArranger::ListViewColumnItemArranger()
					:listView(0)
					,styleProvider(0)
					,columnItemView(0)
					,splitterDragging(false)
					,splitterLatestX(0)
				{
					columnHeaders=new GuiStackComposition;
					columnHeaders->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					columnItemViewCallback=new ColumnItemViewCallback(this);
				}

				ListViewColumnItemArranger::~ListViewColumnItemArranger()
				{
					if(!columnHeaders->GetParent())
					{
						DeleteColumnButtons();
						delete columnHeaders;
					}
				}

				void ListViewColumnItemArranger::AttachListControl(GuiListControl* value)
				{
					FixedHeightItemArranger::AttachListControl(value);
					listView=dynamic_cast<GuiListViewBase*>(value);
					if(listView)
					{
						styleProvider=listView->GetListViewStyleProvider();
						listView->GetContainerComposition()->AddChild(columnHeaders);
						columnItemView=dynamic_cast<IColumnItemView*>(listView->GetItemProvider()->RequestView(IColumnItemView::Identifier));
						if(columnItemView)
						{
							columnItemView->AttachCallback(columnItemViewCallback.Obj());
							RebuildColumns();
						}
					}
				}

				void ListViewColumnItemArranger::DetachListControl()
				{
					if(listView)
					{
						if(columnItemView)
						{
							columnItemView->DetachCallback(columnItemViewCallback.Obj());
							listView->GetItemProvider()->ReleaseView(columnItemView);
							columnItemView=0;
						}
						listView->GetContainerComposition()->RemoveChild(columnHeaders);
						styleProvider=0;
						listView=0;
					}
					FixedHeightItemArranger::DetachListControl();
				}
				
/***********************************************************************
ListViewDetailContentProvider
***********************************************************************/

				ListViewDetailContentProvider::ItemContent::ItemContent(Size minIconSize, bool fitImage, const FontProperties& font, GuiListControl::IItemProvider* _itemProvider)
					:contentComposition(0)
					,itemProvider(_itemProvider)
				{
					columnItemView=dynamic_cast<ListViewColumnItemArranger::IColumnItemView*>(itemProvider->RequestView(ListViewColumnItemArranger::IColumnItemView::Identifier));
					contentComposition=new GuiBoundsComposition;
					contentComposition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					textTable=new GuiTableComposition;
					textTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
					textTable->SetRowsAndColumns(1, 1);
					textTable->SetRowOption(0, GuiCellOption::MinSizeOption());
					textTable->SetColumnOption(0, GuiCellOption::AbsoluteOption(0));
					contentComposition->AddChild(textTable);
					{
						GuiCellComposition* cell=new GuiCellComposition;
						textTable->AddChild(cell);
						cell->SetSite(0, 0, 1, 1);

						GuiTableComposition* table=new GuiTableComposition;
						cell->AddChild(table);
						table->SetRowsAndColumns(3, 2);
						table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
						table->SetRowOption(1, GuiCellOption::MinSizeOption());
						table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
						table->SetColumnOption(0, GuiCellOption::MinSizeOption());
						table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
						table->SetAlignmentToParent(Margin(0, 0, 0, 0));
						table->SetCellPadding(2);
						{
							GuiCellComposition* cell=new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(1, 0, 1, 1);
							cell->SetPreferredMinSize(minIconSize);
							if (!fitImage)
							{
								cell->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
							}

							image=GuiImageFrameElement::Create();
							image->SetStretch(true);
							cell->SetOwnedElement(image);
						}
						{
							GuiCellComposition* cell=new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 1, 3, 1);
							cell->SetMargin(Margin(0, 0, 8, 0));

							text=GuiSolidLabelElement::Create();
							text->SetAlignments(Alignment::Left, Alignment::Center);
							text->SetFont(font);
							text->SetEllipse(true);
							cell->SetOwnedElement(text);
						}
					}
				}

				ListViewDetailContentProvider::ItemContent::~ItemContent()
				{
					if(columnItemView)
					{
						itemProvider->ReleaseView(columnItemView);
					}
				}

				compositions::GuiBoundsComposition* ListViewDetailContentProvider::ItemContent::GetContentComposition()
				{
					return contentComposition;
				}

				compositions::GuiBoundsComposition* ListViewDetailContentProvider::ItemContent::GetBackgroundDecorator()
				{
					return 0;
				}

				void ListViewDetailContentProvider::ItemContent::UpdateSubItemSize()
				{
					vint columnCount=columnItemView->GetColumnCount();
					if(columnCount>textTable->GetColumns())
					{
						columnCount=textTable->GetColumns();
					}
					for(vint i=0;i<columnCount;i++)
					{
						textTable->SetColumnOption(i, GuiCellOption::AbsoluteOption(columnItemView->GetColumnSize(i)));
					}
					textTable->UpdateCellBounds();
				}

				void ListViewDetailContentProvider::ItemContent::Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)
				{
					for(vint i=1;i<textTable->GetColumns();i++)
					{
						GuiCellComposition* cell=textTable->GetSitedCell(0, i);
						textTable->RemoveChild(cell);
						delete cell;
					}

					Ptr<GuiImageData> imageData=view->GetSmallImage(itemIndex);
					if(imageData)
					{
						image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
					}
					else
					{
						image->SetImage(0);
					}
					text->SetText(view->GetText(itemIndex));
					text->SetColor(styleProvider->GetPrimaryTextColor());

					vint columnCount=columnItemView->GetColumnCount();
					textTable->SetRowsAndColumns(1, columnCount);
					for(vint i=1;i<columnCount;i++)
					{
						GuiCellComposition* cell=new GuiCellComposition;
						textTable->AddChild(cell);
						cell->SetSite(0, i, 1, 1);
						cell->SetMargin(Margin(8, 0, 8, 0));

						GuiSolidLabelElement* subText=GuiSolidLabelElement::Create();
						subText->SetAlignments(Alignment::Left, Alignment::Center);
						subText->SetFont(text->GetFont());
						subText->SetEllipse(true);
						subText->SetText(view->GetSubItem(itemIndex, i-1));
						subText->SetColor(styleProvider->GetSecondaryTextColor());
						cell->SetOwnedElement(subText);
					}
					UpdateSubItemSize();
				}

				void ListViewDetailContentProvider::ItemContent::Uninstall()
				{
				}

				void ListViewDetailContentProvider::OnColumnChanged()
				{
					vint count=listViewItemStyleProvider->GetCreatedItemStyles().Count();
					for(vint i=0;i<count;i++)
					{
						GuiListControl::IItemStyleController* itemStyleController=listViewItemStyleProvider->GetCreatedItemStyles().Get(i);
						ItemContent* itemContent=listViewItemStyleProvider->GetItemContent<ItemContent>(itemStyleController);
						if(itemContent)
						{
							itemContent->UpdateSubItemSize();
						}
					}
				}

				ListViewDetailContentProvider::ListViewDetailContentProvider(Size _minIconSize, bool _fitImage)
					:minIconSize(_minIconSize)
					,fitImage(_fitImage)
					,itemProvider(0)
					,columnItemView(0)
					,listViewItemStyleProvider(0)
				{
				}

				ListViewDetailContentProvider::~ListViewDetailContentProvider()
				{
				}

				compositions::IGuiAxis* ListViewDetailContentProvider::CreatePreferredAxis()
				{
					return new GuiDefaultAxis;
				}

				GuiListControl::IItemArranger* ListViewDetailContentProvider::CreatePreferredArranger()
				{
					return new ListViewColumnItemArranger;
				}

				ListViewItemStyleProvider::IListViewItemContent* ListViewDetailContentProvider::CreateItemContent(const FontProperties& font)
				{
					return new ItemContent(minIconSize, fitImage, font, itemProvider);
				}

				void ListViewDetailContentProvider::AttachListControl(GuiListControl* value)
				{
					listViewItemStyleProvider=dynamic_cast<ListViewItemStyleProvider*>(value->GetStyleProvider());
					itemProvider=value->GetItemProvider();
					columnItemView=dynamic_cast<ListViewColumnItemArranger::IColumnItemView*>(itemProvider->RequestView(ListViewColumnItemArranger::IColumnItemView::Identifier));
					if(columnItemView)
					{
						columnItemView->AttachCallback(this);
					}
				}

				void ListViewDetailContentProvider::DetachListControl()
				{
					if(columnItemView)
					{
						columnItemView->DetachCallback(this);
						itemProvider->ReleaseView(columnItemView);
					}
					itemProvider=0;
					listViewItemStyleProvider=0;
				}

/***********************************************************************
ListViewSubItems
***********************************************************************/

				void ListViewSubItems::NotifyUpdateInternal(vint start, vint count, vint newCount)
				{
					owner->NotifyUpdate();
				}

/***********************************************************************
ListViewItem
***********************************************************************/

				void ListViewItem::NotifyUpdate()
				{
					if (owner)
					{
						vint index = owner->IndexOf(this);
						owner->NotifyUpdateInternal(index, 1, 1);
					}
				}

				ListViewItem::ListViewItem()
					:owner(0)
				{
					subItems.owner = this;
				}

				ListViewSubItems& ListViewItem::GetSubItems()
				{
					return subItems;
				}

				Ptr<GuiImageData> ListViewItem::GetSmallImage()
				{
					return smallImage;
				}

				void ListViewItem::SetSmallImage(Ptr<GuiImageData> value)
				{
					smallImage = value;
					NotifyUpdate();
				}

				Ptr<GuiImageData> ListViewItem::GetLargeImage()
				{
					return largeImage;
				}
				
				void ListViewItem::SetLargeImage(Ptr<GuiImageData> value)
				{
					largeImage = value;
					NotifyUpdate();
				}

				const WString& ListViewItem::GetText()
				{
					return text;
				}

				void ListViewItem::SetText(const WString& value)
				{
					text = value;
					NotifyUpdate();
				}

				description::Value ListViewItem::GetTag()
				{
					return tag;
				}

				void ListViewItem::SetTag(const description::Value& value)
				{
					tag = value;
					NotifyUpdate();
				}

/***********************************************************************
ListViewColumn
***********************************************************************/

				void ListViewColumn::NotifyUpdate()
				{
					if (owner)
					{
						vint index = owner->IndexOf(this);
						owner->NotifyUpdate(index, 1);
					}
				}

				ListViewColumn::ListViewColumn(const WString& _text, vint _size)
					:owner(0)
					,text(_text)
					,size(_size)
					,dropdownPopup(0)
					,sortingState(GuiListViewColumnHeader::NotSorted)
				{
				}

				const WString& ListViewColumn::GetText()
				{
					return text;
				}

				void ListViewColumn::SetText(const WString& value)
				{
					text = value;
					NotifyUpdate();
				}

				ItemProperty<WString> ListViewColumn::GetTextProperty()
				{
					return textProperty;
				}

				void ListViewColumn::SetTextProperty(const ItemProperty<WString>& value)
				{
					textProperty = value;
					NotifyUpdate();
				}

				vint ListViewColumn::GetSize()
				{
					return size;
				}

				void ListViewColumn::SetSize(vint value)
				{
					size = value;
					NotifyUpdate();
				}

				GuiMenu* ListViewColumn::GetDropdownPopup()
				{
					return dropdownPopup;
				}

				void ListViewColumn::SetDropdownPopup(GuiMenu* value)
				{
					dropdownPopup = value;
					NotifyUpdate();
				}

				GuiListViewColumnHeader::ColumnSortingState ListViewColumn::GetSortingState()
				{
					return sortingState;
				}

				void ListViewColumn::SetSortingState(GuiListViewColumnHeader::ColumnSortingState value)
				{
					sortingState = value;
					NotifyUpdate();
				}

/***********************************************************************
ListViewDataColumns
***********************************************************************/

				void ListViewDataColumns::NotifyUpdateInternal(vint start, vint count, vint newCount)
				{
					itemProvider->NotifyAllItemsUpdate();
				}

				ListViewDataColumns::ListViewDataColumns(IListViewItemProvider* _itemProvider)
					:itemProvider(_itemProvider)
				{
				}

				ListViewDataColumns::~ListViewDataColumns()
				{
				}

/***********************************************************************
ListViewColumns
***********************************************************************/

				void ListViewColumns::AfterInsert(vint index, const Ptr<ListViewColumn>& value)
				{
					ItemsBase<Ptr<ListViewColumn>>::AfterInsert(index, value);
					value->owner = this;
				}

				void ListViewColumns::BeforeRemove(vint index, const Ptr<ListViewColumn>& value)
				{
					value->owner = 0;
					ItemsBase<Ptr<ListViewColumn>>::BeforeRemove(index, value);
				}

				void ListViewColumns::NotifyUpdateInternal(vint start, vint count, vint newCount)
				{
					itemProvider->NotifyAllColumnsUpdate();
					itemProvider->NotifyAllItemsUpdate();
				}

				ListViewColumns::ListViewColumns(IListViewItemProvider* _itemProvider)
					:itemProvider(_itemProvider)
				{
				}

				ListViewColumns::~ListViewColumns()
				{
				}

/***********************************************************************
ListViewItemProvider
***********************************************************************/

				void ListViewItemProvider::AfterInsert(vint index, const Ptr<ListViewItem>& value)
				{
					ListProvider<Ptr<ListViewItem>>::AfterInsert(index, value);
					value->owner = this;
				}

				void ListViewItemProvider::BeforeRemove(vint index, const Ptr<ListViewItem>& value)
				{
					value->owner = 0;
					ListProvider<Ptr<ListViewItem>>::AfterInsert(index, value);
				}

				void ListViewItemProvider::NotifyAllItemsUpdate()
				{
					NotifyUpdate(0, Count());
				}

				void ListViewItemProvider::NotifyAllColumnsUpdate()
				{
					for (vint i = 0; i < columnItemViewCallbacks.Count(); i++)
					{
						columnItemViewCallbacks[i]->OnColumnChanged();
					}
				}

				Ptr<GuiImageData> ListViewItemProvider::GetSmallImage(vint itemIndex)
				{
					return Get(itemIndex)->smallImage;
				}

				Ptr<GuiImageData> ListViewItemProvider::GetLargeImage(vint itemIndex)
				{
					return Get(itemIndex)->largeImage;
				}

				WString ListViewItemProvider::GetText(vint itemIndex)
				{
					return Get(itemIndex)->text;
				}

				WString ListViewItemProvider::GetSubItem(vint itemIndex, vint index)
				{
					Ptr<ListViewItem> item=Get(itemIndex);
					if(index<0 || index>=item->GetSubItems().Count())
					{
						return L"";
					}
					else
					{
						return item->GetSubItems()[index];
					}
				}

				vint ListViewItemProvider::GetDataColumnCount()
				{
					return dataColumns.Count();
				}

				vint ListViewItemProvider::GetDataColumn(vint index)
				{
					return dataColumns[index];
				}

				bool ListViewItemProvider::AttachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)
				{
					if(columnItemViewCallbacks.Contains(value))
					{
						return false;
					}
					else
					{
						columnItemViewCallbacks.Add(value);
						return true;
					}
				}

				bool ListViewItemProvider::DetachCallback(ListViewColumnItemArranger::IColumnItemViewCallback* value)
				{
					vint index=columnItemViewCallbacks.IndexOf(value);
					if(index==-1)
					{
						return false;
					}
					else
					{
						columnItemViewCallbacks.Remove(value);
						return true;
					}
				}

				vint ListViewItemProvider::GetColumnCount()
				{
					return columns.Count();
				}

				WString ListViewItemProvider::GetColumnText(vint index)
				{
					if(index<0 || index>=columns.Count())
					{
						return L"";
					}
					else
					{
						return columns[index]->GetText();
					}
				}

				vint ListViewItemProvider::GetColumnSize(vint index)
				{
					if(index<0 || index>=columns.Count())
					{
						return 0;
					}
					else
					{
						return columns[index]->GetSize();
					}
				}

				void ListViewItemProvider::SetColumnSize(vint index, vint value)
				{
					if(index>=0 && index<columns.Count())
					{
						columns[index]->SetSize(value);
					}
				}

				GuiMenu* ListViewItemProvider::GetDropdownPopup(vint index)
				{
					if(index<0 || index>=columns.Count())
					{
						return 0;
					}
					else
					{
						return columns[index]->GetDropdownPopup();
					}
				}

				GuiListViewColumnHeader::ColumnSortingState ListViewItemProvider::GetSortingState(vint index)
				{
					if(index<0 || index>=columns.Count())
					{
						return GuiListViewColumnHeader::NotSorted;
					}
					else
					{
						return columns[index]->GetSortingState();
					}
				}

				WString ListViewItemProvider::GetTextValue(vint itemIndex)
				{
					return GetText(itemIndex);
				}

				description::Value ListViewItemProvider::GetBindingValue(vint itemIndex)
				{
					return Value::From(Get(itemIndex));
				}

				ListViewItemProvider::ListViewItemProvider()
					:columns(this)
					, dataColumns(this)
				{
				}

				ListViewItemProvider::~ListViewItemProvider()
				{
				}

				IDescriptable* ListViewItemProvider::RequestView(const WString& identifier)
				{
					if(identifier==ListViewItemStyleProvider::IListViewItemView::Identifier)
					{
						return (ListViewItemStyleProvider::IListViewItemView*)this;
					}
					else if(identifier==ListViewColumnItemArranger::IColumnItemView::Identifier)
					{
						return (ListViewColumnItemArranger::IColumnItemView*)this;
					}
					else
					{
						return 0;
					}
				}

				void ListViewItemProvider::ReleaseView(IDescriptable* view)
				{
				}

				ListViewDataColumns& ListViewItemProvider::GetDataColumns()
				{
					return dataColumns;
				}

				ListViewColumns& ListViewItemProvider::GetColumns()
				{
					return columns;
				}
			}

/***********************************************************************
GuiListView
***********************************************************************/

			GuiVirtualListView::GuiVirtualListView(IStyleProvider* _styleProvider, GuiListControl::IItemProvider* _itemProvider)
				:GuiListViewBase(_styleProvider, _itemProvider)
			{
				ChangeItemStyle(new list::ListViewBigIconContentProvider);
			}

			GuiVirtualListView::~GuiVirtualListView()
			{
			}

			bool GuiVirtualListView::ChangeItemStyle(Ptr<list::ListViewItemStyleProvider::IListViewItemContentProvider> contentProvider)
			{
				SetStyleProvider(0);
				SetArranger(0);
				SetAxis(contentProvider->CreatePreferredAxis());
				SetStyleProvider(new list::ListViewItemStyleProvider(contentProvider));
				SetArranger(contentProvider->CreatePreferredArranger());
				return true;
			}

/***********************************************************************
GuiListView
***********************************************************************/

			GuiListView::GuiListView(IStyleProvider* _styleProvider)
				:GuiVirtualListView(_styleProvider, new list::ListViewItemProvider)
			{
				items=dynamic_cast<list::ListViewItemProvider*>(itemProvider.Obj());
			}

			GuiListView::~GuiListView()
			{
			}

			list::ListViewItemProvider& GuiListView::GetItems()
			{
				return *items;
			}

			list::ListViewDataColumns& GuiListView::GetDataColumns()
			{
				return items->GetDataColumns();
			}

			list::ListViewColumns& GuiListView::GetColumns()
			{
				return items->GetColumns();
			}

			Ptr<list::ListViewItem> GuiListView::GetSelectedItem()
			{
				vint index = GetSelectedItemIndex();
				if (index == -1) return 0;
				return items->Get(index);
			}
		}
	}
}