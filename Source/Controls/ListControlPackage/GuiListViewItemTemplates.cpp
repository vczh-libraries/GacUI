#include "GuiListViewItemTemplates.h"
#include "../../GraphicsComposition/GuiGraphicsTableComposition.h"

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

				DefaultListViewItemTemplate::DefaultListViewItemTemplate()
				{
					SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
				}

				DefaultListViewItemTemplate::~DefaultListViewItemTemplate()
				{
				}

/***********************************************************************
BigIconListViewItemTemplate
***********************************************************************/

				void BigIconListViewItemTemplate::OnInitialize()
				{
					{
						auto table = new GuiTableComposition;
						AddChild(table);
						table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						table->SetRowsAndColumns(2, 3);
						table->SetRowOption(0, GuiCellOption::MinSizeOption());
						table->SetRowOption(1, GuiCellOption::MinSizeOption());
						table->SetColumnOption(0, GuiCellOption::PercentageOption(0.5));
						table->SetColumnOption(1, GuiCellOption::MinSizeOption());
						table->SetColumnOption(2, GuiCellOption::PercentageOption(0.5));
						table->SetAlignmentToParent(Margin(0, 0, 0, 0));
						table->SetCellPadding(5);
						{
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 1, 1, 1);
							cell->SetPreferredMinSize(Size(32, 32));

							image = GuiImageFrameElement::Create();
							image->SetStretch(true);
							cell->SetOwnedElement(Ptr(image));
						}
						{
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetMinSizeLimitation(GuiGraphicsComposition::NoLimit);
							cell->SetSite(1, 0, 1, 3);
							cell->SetPreferredMinSize(Size(64, 40));

							text = GuiSolidLabelElement::Create();
							text->SetAlignments(Alignment::Center, Alignment::Top);
							text->SetWrapLine(true);
							text->SetEllipse(true);
							cell->SetOwnedElement(Ptr(text));
						}
					}

					FontChanged.AttachMethod(this, &BigIconListViewItemTemplate::OnFontChanged);
					FontChanged.Execute(compositions::GuiEventArgs(this));
				}

				void BigIconListViewItemTemplate::OnRefresh()
				{
					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(WString::Unmanaged(IListViewItemView::Identifier))))
						{
							auto imageData = view->GetLargeImage(itemIndex);
							if (imageData)
							{
								image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
							}
							else
							{
								image->SetImage(nullptr);
							}
							text->SetText(view->GetText(itemIndex));
							text->SetColor(listView->TypedControlTemplateObject(true)->GetPrimaryTextColor());
						}
					}
				}

				void BigIconListViewItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetFont(GetFont());
				}

				BigIconListViewItemTemplate::BigIconListViewItemTemplate()
				{
				}

				BigIconListViewItemTemplate::~BigIconListViewItemTemplate()
				{
				}

/***********************************************************************
SmallIconListViewItemTemplate
***********************************************************************/

				void SmallIconListViewItemTemplate::OnInitialize()
				{
					{
						auto table = new GuiTableComposition;
						AddChild(table);
						table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						table->SetRowsAndColumns(3, 2);
						table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
						table->SetRowOption(1, GuiCellOption::MinSizeOption());
						table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
						table->SetColumnOption(0, GuiCellOption::MinSizeOption());
						table->SetColumnOption(1, GuiCellOption::MinSizeOption());
						table->SetAlignmentToParent(Margin(0, 0, 0, 0));
						table->SetCellPadding(2);
						{
							GuiCellComposition* cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(1, 0, 1, 1);
							cell->SetPreferredMinSize(Size(16, 16));

							image = GuiImageFrameElement::Create();
							image->SetStretch(true);
							cell->SetOwnedElement(Ptr(image));
						}
						{
							GuiCellComposition* cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 1, 3, 1);
							cell->SetPreferredMinSize(Size(192, 0));

							text = GuiSolidLabelElement::Create();
							text->SetAlignments(Alignment::Left, Alignment::Center);
							text->SetEllipse(true);
							cell->SetOwnedElement(Ptr(text));
						}
					}

					FontChanged.AttachMethod(this, &SmallIconListViewItemTemplate::OnFontChanged);
					FontChanged.Execute(compositions::GuiEventArgs(this));
				}

				void SmallIconListViewItemTemplate::OnRefresh()
				{
					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(WString::Unmanaged(IListViewItemView::Identifier))))
						{
							auto imageData = view->GetSmallImage(itemIndex);
							if (imageData)
							{
								image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
							}
							else
							{
								image->SetImage(nullptr);
							}
							text->SetText(view->GetText(itemIndex));
							text->SetColor(listView->TypedControlTemplateObject(true)->GetPrimaryTextColor());
						}
					}
				}

				void SmallIconListViewItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetFont(GetFont());
				}

				SmallIconListViewItemTemplate::SmallIconListViewItemTemplate()
				{
				}

				SmallIconListViewItemTemplate::~SmallIconListViewItemTemplate()
				{
				}

/***********************************************************************
ListListViewItemTemplate
***********************************************************************/

				void ListListViewItemTemplate::OnInitialize()
				{
					{
						auto table = new GuiTableComposition;
						AddChild(table);
						table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						table->SetRowsAndColumns(3, 2);
						table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
						table->SetRowOption(1, GuiCellOption::MinSizeOption());
						table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
						table->SetColumnOption(0, GuiCellOption::MinSizeOption());
						table->SetColumnOption(1, GuiCellOption::MinSizeOption());
						table->SetAlignmentToParent(Margin(0, 0, 0, 0));
						table->SetCellPadding(2);
						{
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(1, 0, 1, 1);
							cell->SetPreferredMinSize(Size(16, 16));

							image = GuiImageFrameElement::Create();
							image->SetStretch(true);
							cell->SetOwnedElement(Ptr(image));
						}
						{
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 1, 3, 1);

							auto textBounds = new GuiBoundsComposition;
							cell->AddChild(textBounds);
							textBounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
							textBounds->SetAlignmentToParent(Margin(0, 0, 16, 0));

							text = GuiSolidLabelElement::Create();
							text->SetAlignments(Alignment::Left, Alignment::Center);
							textBounds->SetOwnedElement(Ptr(text));
						}
					}

					FontChanged.AttachMethod(this, &ListListViewItemTemplate::OnFontChanged);
					FontChanged.Execute(compositions::GuiEventArgs(this));
				}

				void ListListViewItemTemplate::OnRefresh()
				{
					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(WString::Unmanaged(IListViewItemView::Identifier))))
						{
							auto imageData = view->GetSmallImage(itemIndex);
							if (imageData)
							{
								image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
							}
							else
							{
								image->SetImage(nullptr);
							}
							text->SetText(view->GetText(itemIndex));
							text->SetColor(listView->TypedControlTemplateObject(true)->GetPrimaryTextColor());
						}
					}
				}

				void ListListViewItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetFont(GetFont());
				}

				ListListViewItemTemplate::ListListViewItemTemplate()
				{
				}

				ListListViewItemTemplate::~ListListViewItemTemplate()
				{
				}

/***********************************************************************
TileListViewItemTemplate
***********************************************************************/

				elements::GuiSolidLabelElement* TileListViewItemTemplate::CreateTextElement(vint textRow)
				{
					auto cell = new GuiCellComposition;
					textTable->AddChild(cell);
					cell->SetSite(textRow + 1, 0, 1, 1);

					auto textElement = Ptr(GuiSolidLabelElement::Create());
					textElement->SetAlignments(Alignment::Left, Alignment::Center);
					textElement->SetEllipse(true);
					cell->SetOwnedElement(textElement);
					return textElement.Obj();
				}

				void TileListViewItemTemplate::ResetTextTable(vint dataColumnCount)
				{
					if (text && dataTexts.Count() == dataColumnCount) return;
					for (vint i = textTable->Children().Count() - 1; i >= 0; i--)
					{
						if (auto cell = dynamic_cast<GuiCellComposition*>(textTable->Children()[i]))
						{
							SafeDeleteComposition(cell);
						}
					}

					{
						vint textRows = dataColumnCount + 1;
						textTable->SetRowsAndColumns(textRows + 2, 1);
						textTable->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
						for (vint i = 0; i < textRows; i++)
						{
							textTable->SetRowOption(i + 1, GuiCellOption::MinSizeOption());
						}
						textTable->SetRowOption(textRows + 1, GuiCellOption::PercentageOption(0.5));
						textTable->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
					}

					text = CreateTextElement(0);
					text->SetFont(GetFont());
					{
						dataTexts.Resize(dataColumnCount);
						for (vint i = 0; i < dataColumnCount; i++)
						{
							dataTexts[i] = CreateTextElement(i + 1);
							dataTexts[i]->SetFont(GetFont());
						}
					}
				}

				void TileListViewItemTemplate::OnInitialize()
				{
					{
						auto table = new GuiTableComposition;
						AddChild(table);
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
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(1, 0, 1, 1);
							cell->SetPreferredMinSize(Size(32, 32));

							image = GuiImageFrameElement::Create();
							image->SetStretch(true);
							cell->SetOwnedElement(Ptr(image));
						}
						{
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 1, 3, 1);
							cell->SetPreferredMinSize(Size(224, 0));

							textTable = new GuiTableComposition;
							textTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
							textTable->SetCellPadding(1);
							textTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
							cell->AddChild(textTable);
						}
					}

					ResetTextTable(0);
					FontChanged.AttachMethod(this, &TileListViewItemTemplate::OnFontChanged);
					FontChanged.Execute(compositions::GuiEventArgs(this));
				}

				void TileListViewItemTemplate::OnRefresh()
				{
					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(WString::Unmanaged(IListViewItemView::Identifier))))
						{
							auto imageData = view->GetLargeImage(itemIndex);
							if (imageData)
							{
								image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
							}
							else
							{
								image->SetImage(nullptr);
							}

							vint subColumnCount = view->GetColumnCount() - 1;
							vint dataColumnCount = view->GetDataColumnCount();
							if (dataColumnCount > subColumnCount) dataColumnCount = subColumnCount;
							if (dataColumnCount < 0) dataColumnCount = 0;
							ResetTextTable(dataColumnCount);

							text->SetText(view->GetText(itemIndex));
							text->SetColor(listView->TypedControlTemplateObject(true)->GetPrimaryTextColor());
							for (vint i = 0; i < dataColumnCount; i++)
							{
								dataTexts[i]->SetText(view->GetSubItem(itemIndex, view->GetDataColumn(i)));
								dataTexts[i]->SetColor(listView->TypedControlTemplateObject(true)->GetSecondaryTextColor());
							}
						}
					}
				}

				void TileListViewItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					if (text) text->SetFont(GetFont());
					for (auto dataText : dataTexts)
					{
						dataText->SetFont(GetFont());
					}
				}

				TileListViewItemTemplate::TileListViewItemTemplate()
				{
				}

				TileListViewItemTemplate::~TileListViewItemTemplate()
				{
				}

/***********************************************************************
InformationListViewItemTemplate
***********************************************************************/

				void InformationListViewItemTemplate::ResetTextTable(vint dataColumnCount)
				{
					if (dataTexts.Count() == dataColumnCount) return;
					for (vint i = textTable->Children().Count() - 1; i >= 0; i--)
					{
						if (auto cell = dynamic_cast<GuiCellComposition*>(textTable->Children()[i]))
						{
							SafeDeleteComposition(cell);
						}
					}

					{
						textTable->SetRowsAndColumns(dataColumnCount + 2, 1);
						textTable->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
						for (vint i = 0; i < dataColumnCount; i++)
						{
							textTable->SetRowOption(i + 1, GuiCellOption::MinSizeOption());
						}
						textTable->SetRowOption(dataColumnCount + 1, GuiCellOption::PercentageOption(0.5));
						textTable->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
					}

					columnTexts.Resize(dataColumnCount);
					dataTexts.Resize(dataColumnCount);

					for (vint i = 0; i < dataColumnCount; i++)
					{
						auto cell = new GuiCellComposition;
						textTable->AddChild(cell);
						cell->SetSite(i + 1, 0, 1, 1);

						auto dataTable = new GuiTableComposition;
						dataTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						dataTable->SetRowsAndColumns(1, 2);
						dataTable->SetRowOption(0, GuiCellOption::MinSizeOption());
						dataTable->SetColumnOption(0, GuiCellOption::MinSizeOption());
						dataTable->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
						dataTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
						cell->AddChild(dataTable);
						{
							auto cell = new GuiCellComposition;
							dataTable->AddChild(cell);
							cell->SetSite(0, 0, 1, 1);

							columnTexts[i] = GuiSolidLabelElement::Create();
							columnTexts[i]->SetFont(GetFont());
							cell->SetOwnedElement(Ptr(columnTexts[i]));
						}
						{
							auto cell = new GuiCellComposition;
							dataTable->AddChild(cell);
							cell->SetSite(0, 1, 1, 1);

							dataTexts[i] = GuiSolidLabelElement::Create();
							dataTexts[i]->SetFont(GetFont());
							dataTexts[i]->SetEllipse(true);
							cell->SetOwnedElement(Ptr(dataTexts[i]));
						}
					}
				}

				void InformationListViewItemTemplate::OnInitialize()
				{
					{
						bottomLine = GuiSolidBackgroundElement::Create();
						bottomLineComposition = new GuiBoundsComposition;
						bottomLineComposition->SetOwnedElement(Ptr(bottomLine));
						bottomLineComposition->SetAlignmentToParent(Margin(8, -1, 8, 0));
						bottomLineComposition->SetPreferredMinSize(Size(0, 1));
						AddChild(bottomLineComposition);

						auto table = new GuiTableComposition;
						AddChild(table);
						table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						table->SetRowsAndColumns(3, 3);
						table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
						table->SetRowOption(1, GuiCellOption::MinSizeOption());
						table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
						table->SetColumnOption(0, GuiCellOption::MinSizeOption());
						table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
						table->SetColumnOption(2, GuiCellOption::MinSizeOption());
						table->SetAlignmentToParent(Margin(0, 0, 0, 0));
						table->SetCellPadding(4);
						{
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(1, 0, 1, 1);
							cell->SetPreferredMinSize(Size(32, 32));

							image = GuiImageFrameElement::Create();
							image->SetStretch(true);
							cell->SetOwnedElement(Ptr(image));
						}
						{
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 1, 3, 1);

							text = GuiSolidLabelElement::Create();
							text->SetEllipse(true);
							cell->SetOwnedElement(Ptr(text));
						}
						{
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 2, 3, 1);
							cell->SetPreferredMinSize(Size(224, 0));

							textTable = new GuiTableComposition;
							textTable->SetCellPadding(4);
							textTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
							textTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
							cell->AddChild(textTable);
						}
					}

					FontChanged.AttachMethod(this, &InformationListViewItemTemplate::OnFontChanged);
					FontChanged.Execute(compositions::GuiEventArgs(this));
				}

				void InformationListViewItemTemplate::OnRefresh()
				{
					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(WString::Unmanaged(IListViewItemView::Identifier))))
						{
							auto imageData = view->GetLargeImage(itemIndex);
							if (imageData)
							{
								image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
							}
							else
							{
								image->SetImage(nullptr);
							}
							text->SetText(view->GetText(itemIndex));
							text->SetColor(listView->TypedControlTemplateObject(true)->GetPrimaryTextColor());
							bottomLine->SetColor(listView->TypedControlTemplateObject(true)->GetItemSeparatorColor());

							vint subColumnCount = view->GetColumnCount() - 1;
							vint dataColumnCount = view->GetDataColumnCount();
							if (dataColumnCount > subColumnCount) dataColumnCount = subColumnCount;
							if (dataColumnCount < 0) dataColumnCount = 0;
							ResetTextTable(dataColumnCount);
							for (vint i = 0; i < dataColumnCount; i++)
							{
								{
									columnTexts[i]->SetText(view->GetColumnText(view->GetDataColumn(i) + 1) + L": ");
									columnTexts[i]->SetColor(listView->TypedControlTemplateObject(true)->GetSecondaryTextColor());
								}
								{
									dataTexts[i]->SetText(view->GetSubItem(itemIndex, view->GetDataColumn(i)));
									dataTexts[i]->SetColor(listView->TypedControlTemplateObject(true)->GetPrimaryTextColor());
								}
							}
						}
					}
				}

				void InformationListViewItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					{
						auto font = GetFont();
						font.size = (vint)(font.size * 1.2);
						text->SetFont(font);
					}

					for (auto columnText : columnTexts)
					{
						columnText->SetFont(GetFont());
					}
					for (auto dataText : dataTexts)
					{
						dataText->SetFont(GetFont());
					}
				}

				InformationListViewItemTemplate::InformationListViewItemTemplate()
				{
				}

				InformationListViewItemTemplate::~InformationListViewItemTemplate()
				{
				}

/***********************************************************************
DetailListViewItemTemplate
***********************************************************************/

				void DetailListViewItemTemplate::UpdateSubItemSize()
				{
					if (auto view = dynamic_cast<IListViewItemView*>(listControl->GetItemProvider()->RequestView(WString::Unmanaged(IListViewItemView::Identifier))))
					{
						if (columnItemView)
						{
							vint columnCount = view->GetColumnCount();
							if (columnCount > textTable->GetColumns())
							{
								columnCount = textTable->GetColumns();
							}
							for (vint i = 0; i < columnCount; i++)
							{
								textTable->SetColumnOption(i, GuiCellOption::AbsoluteOption(columnItemView->GetColumnSize(i)));
							}
						}
					}
				}

				void DetailListViewItemTemplate::ResetTextTable(vint subColumnCount)
				{
					if (subItemCells.Count() == subColumnCount) return;

					for (auto cell : subItemCells)
					{
						SafeDeleteComposition(cell);
					}
					subItemCells.Resize(subColumnCount);
					subItemTexts.Resize(subColumnCount);

					textTable->SetRowsAndColumns(1, subColumnCount + 1);
					for (vint i = 0; i < subColumnCount; i++)
					{
						auto cell = new GuiCellComposition;
						textTable->AddChild(cell);
						cell->SetSite(0, i + 1, 1, 1);

						auto textBounds = new GuiBoundsComposition;
						cell->AddChild(textBounds);
						textBounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
						textBounds->SetAlignmentToParent(Margin(8, 0, 8, 0));

						auto subText = GuiSolidLabelElement::Create();
						subText->SetAlignments(Alignment::Left, Alignment::Center);
						subText->SetFont(GetFont());
						subText->SetEllipse(true);
						textBounds->SetOwnedElement(Ptr(subText));

						subItemCells[i] = cell;
						subItemTexts[i] = subText;
					}
				}

				void DetailListViewItemTemplate::OnInitialize()
				{
					columnItemView = dynamic_cast<ListViewColumnItemArranger::IColumnItemView*>(listControl->GetItemProvider()->RequestView(WString::Unmanaged(ListViewColumnItemArranger::IColumnItemView::Identifier)));

					{
						textTable = new GuiTableComposition;
						textTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						textTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
						textTable->SetRowsAndColumns(1, 1);
						textTable->SetRowOption(0, GuiCellOption::MinSizeOption());
						textTable->SetColumnOption(0, GuiCellOption::AbsoluteOption(0));
						AddChild(textTable);
						{
							auto cell = new GuiCellComposition;
							textTable->AddChild(cell);
							cell->SetSite(0, 0, 1, 1);

							auto table = new GuiTableComposition;
							cell->AddChild(table);
							table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
							table->SetRowsAndColumns(3, 2);
							table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
							table->SetRowOption(1, GuiCellOption::MinSizeOption());
							table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
							table->SetColumnOption(0, GuiCellOption::MinSizeOption());
							table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
							table->SetAlignmentToParent(Margin(0, 0, 0, 0));
							table->SetCellPadding(2);
							{
								auto cell = new GuiCellComposition;
								table->AddChild(cell);
								cell->SetSite(1, 0, 1, 1);
								cell->SetPreferredMinSize(Size(16, 16));

								image = GuiImageFrameElement::Create();
								image->SetStretch(true);
								cell->SetOwnedElement(Ptr(image));
							}
							{
								auto cell = new GuiCellComposition;
								table->AddChild(cell);
								cell->SetSite(0, 1, 3, 1);

								auto textBounds = new GuiBoundsComposition;
								cell->AddChild(textBounds);
								textBounds->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElement);
								textBounds->SetAlignmentToParent(Margin(0, 0, 8, 0));

								text = GuiSolidLabelElement::Create();
								text->SetAlignments(Alignment::Left, Alignment::Center);
								text->SetFont(GetFont());
								text->SetEllipse(true);
								textBounds->SetOwnedElement(Ptr(text));
							}
						}
					}

					FontChanged.AttachMethod(this, &DetailListViewItemTemplate::OnFontChanged);
					FontChanged.Execute(compositions::GuiEventArgs(this));
				}

				void DetailListViewItemTemplate::OnRefresh()
				{
					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(WString::Unmanaged(IListViewItemView::Identifier))))
						{
							vint subColumnCount = view->GetColumnCount() - 1;
							if (subColumnCount < 0) subColumnCount = 0;
							ResetTextTable(subColumnCount);

							auto imageData = view->GetSmallImage(itemIndex);
							if (imageData)
							{
								image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
							}
							else
							{
								image->SetImage(0);
							}

							text->SetText(view->GetText(itemIndex));
							text->SetColor(listView->TypedControlTemplateObject(true)->GetPrimaryTextColor());

							for (vint i = 0; i < subColumnCount; i++)
							{
								subItemTexts[i]->SetText(view->GetSubItem(itemIndex, i));
								subItemTexts[i]->SetColor(listView->TypedControlTemplateObject(true)->GetSecondaryTextColor());
							}
						}
					}
					UpdateSubItemSize();
				}

				void DetailListViewItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetFont(GetFont());
					for (auto subText : subItemTexts)
					{
						subText->SetFont(GetFont());
					}
				}

				DetailListViewItemTemplate::DetailListViewItemTemplate()
				{
				}

				DetailListViewItemTemplate::~DetailListViewItemTemplate()
				{
				}
			}
		}
	}
}