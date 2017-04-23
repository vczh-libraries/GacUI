#include "GuiListViewItemTemplates.h"
#include "../Styles/GuiThemeStyleFactory.h"

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
DefaultListViewItemTemplate
***********************************************************************/

				void DefaultListViewItemTemplate::OnInitialize()
				{
					templates::GuiListItemTemplate::OnInitialize();
					SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					backgroundButton = new GuiSelectableButton(theme::GetCurrentTheme()->CreateListItemBackgroundStyle());
					backgroundButton->SetAutoSelection(false);
					backgroundButton->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					AddChild(backgroundButton->GetBoundsComposition());

					SelectedChanged.AttachMethod(this, &DefaultTextListItemTemplate::OnSelectedChanged);

					SelectedChanged.Execute(compositions::GuiEventArgs(this));
				}

				void DefaultListViewItemTemplate::OnSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					backgroundButton->SetSelected(GetSelected());
				}

				DefaultListViewItemTemplate::DefaultListViewItemTemplate()
				{
				}

				DefaultListViewItemTemplate::~DefaultListViewItemTemplate()
				{
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
					if (columnItemView)
					{
						columnItemView->DetachCallback(this);
						itemProvider->ReleaseView(columnItemView);
						columnItemView = nullptr;
					}
					itemProvider = nullptr;
					listViewItemStyleProvider = nullptr;
				}
			}
		}
	}
}