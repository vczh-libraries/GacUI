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
					DefaultListViewItemTemplate::OnInitialize();
					{
						auto table = new GuiTableComposition;
						AddChild(table);
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
							cell->SetOwnedElement(image);
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
							cell->SetOwnedElement(text);
						}
					}

					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
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
							text->SetColor(listView->GetControlTemplateObject(true)->GetPrimaryTextColor());
						}
					}

					FontChanged.AttachMethod(this, &BigIconListViewItemTemplate::OnFontChanged);

					FontChanged.Execute(compositions::GuiEventArgs(this));
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
					DefaultListViewItemTemplate::OnInitialize();
					{
						auto table = new GuiTableComposition;
						AddChild(table);
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
							cell->SetOwnedElement(image);
						}
						{
							GuiCellComposition* cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 1, 3, 1);
							cell->SetPreferredMinSize(Size(192, 0));

							text = GuiSolidLabelElement::Create();
							text->SetAlignments(Alignment::Left, Alignment::Center);
							text->SetEllipse(true);
							cell->SetOwnedElement(text);
						}
					}

					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
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
							text->SetColor(listView->GetControlTemplateObject(true)->GetPrimaryTextColor());
						}
					}

					FontChanged.AttachMethod(this, &SmallIconListViewItemTemplate::OnFontChanged);

					FontChanged.Execute(compositions::GuiEventArgs(this));
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
					DefaultListViewItemTemplate::OnInitialize();
					{
						auto table = new GuiTableComposition;
						AddChild(table);
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
							cell->SetOwnedElement(image);
						}
						{
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 1, 3, 1);
							cell->SetMargin(Margin(0, 0, 16, 0));

							text = GuiSolidLabelElement::Create();
							text->SetAlignments(Alignment::Left, Alignment::Center);
							cell->SetOwnedElement(text);
						}
					}

					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
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
							text->SetColor(listView->GetControlTemplateObject(true)->GetPrimaryTextColor());
						}
					}

					FontChanged.AttachMethod(this, &ListListViewItemTemplate::OnFontChanged);

					FontChanged.Execute(compositions::GuiEventArgs(this));
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

					auto textElement = GuiSolidLabelElement::Create();
					textElement->SetAlignments(Alignment::Left, Alignment::Center);
					textElement->SetEllipse(true);
					cell->SetOwnedElement(textElement);
					return textElement;
				}

				void TileListViewItemTemplate::ResetTextTable(vint textRows)
				{
					textTable->SetRowsAndColumns(textRows + 2, 1);
					textTable->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
					for (vint i = 0; i<textRows; i++)
					{
						textTable->SetRowOption(i + 1, GuiCellOption::MinSizeOption());
					}
					textTable->SetRowOption(textRows + 1, GuiCellOption::PercentageOption(0.5));
					textTable->SetColumnOption(0, GuiCellOption::PercentageOption(1.0));
				}

				void TileListViewItemTemplate::OnInitialize()
				{
					DefaultListViewItemTemplate::OnInitialize();
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
							cell->SetOwnedElement(image);
						}
						{
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 1, 3, 1);
							cell->SetPreferredMinSize(Size(224, 0));

							textTable = new GuiTableComposition;
							textTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
							textTable->SetCellPadding(1);
							ResetTextTable(1);
							textTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
							cell->AddChild(textTable);
							{
								text = CreateTextElement(0);
							}
						}
					}

					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
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
							text->SetColor(listView->GetControlTemplateObject(true)->GetPrimaryTextColor());

							vint dataColumnCount = view->GetDataColumnCount();
							ResetTextTable(dataColumnCount + 1);
							dataTexts.Resize(dataColumnCount);
							for (vint i = 0; i < dataColumnCount; i++)
							{
								dataTexts[i] = CreateTextElement(i + 1);
								dataTexts[i]->SetText(view->GetSubItem(itemIndex, view->GetDataColumn(i)));
								dataTexts[i]->SetColor(listView->GetControlTemplateObject(true)->GetSecondaryTextColor());
							}
						}
					}

					FontChanged.AttachMethod(this, &TileListViewItemTemplate::OnFontChanged);

					FontChanged.Execute(compositions::GuiEventArgs(this));
				}

				void TileListViewItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetFont(GetFont());
					if (auto view = dynamic_cast<IListViewItemView*>(listControl->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
					{
						vint dataColumnCount = view->GetDataColumnCount();
						for (vint i = 0; i < dataColumnCount; i++)
						{
							dataTexts[i]->SetFont(GetFont());
						}
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

				void InformationListViewItemTemplate::OnInitialize()
				{
					DefaultListViewItemTemplate::OnInitialize();
					{
						bottomLine = GuiSolidBackgroundElement::Create();
						bottomLineComposition = new GuiBoundsComposition;
						bottomLineComposition->SetOwnedElement(bottomLine);
						bottomLineComposition->SetAlignmentToParent(Margin(8, -1, 8, 0));
						bottomLineComposition->SetPreferredMinSize(Size(0, 1));
						AddChild(bottomLineComposition);

						auto table = new GuiTableComposition;
						AddChild(table);
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
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(1, 0, 1, 1);
							cell->SetPreferredMinSize(Size(32, 32));

							image = GuiImageFrameElement::Create();
							image->SetStretch(true);
							cell->SetOwnedElement(image);
						}
						{
							auto cell = new GuiCellComposition;
							table->AddChild(cell);
							cell->SetSite(0, 1, 3, 1);

							text = GuiSolidLabelElement::Create();
							text->SetEllipse(true);
							cell->SetOwnedElement(text);
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

					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
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
							text->SetColor(listView->GetControlTemplateObject(true)->GetPrimaryTextColor());
							bottomLine->SetColor(listView->GetControlTemplateObject(true)->GetItemSeparatorColor());

							vint dataColumnCount = view->GetDataColumnCount();
							columnTexts.Resize(dataColumnCount);
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
								auto cell = new GuiCellComposition;
								textTable->AddChild(cell);
								cell->SetSite(i + 1, 0, 1, 1);

								auto dataTable = new GuiTableComposition;
								dataTable->SetRowsAndColumns(1, 2);
								dataTable->SetRowOption(0, GuiCellOption::MinSizeOption());
								dataTable->SetColumnOption(0, GuiCellOption::MinSizeOption());
								dataTable->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
								dataTable->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
								dataTable->SetAlignmentToParent(Margin(0, 0, 0, 0));
								cell->AddChild(dataTable);
								{
									auto cell = new GuiCellComposition;
									dataTable->AddChild(cell);
									cell->SetSite(0, 0, 1, 1);

									columnTexts[i] = GuiSolidLabelElement::Create();
									columnTexts[i]->SetText(view->GetColumnText(view->GetDataColumn(i) + 1) + L": ");
									columnTexts[i]->SetColor(listView->GetControlTemplateObject(true)->GetSecondaryTextColor());
									cell->SetOwnedElement(columnTexts[i]);
								}
								{
									auto cell = new GuiCellComposition;
									dataTable->AddChild(cell);
									cell->SetSite(0, 1, 1, 1);

									dataTexts[i]= GuiSolidLabelElement::Create();
									dataTexts[i]->SetEllipse(true);
									dataTexts[i]->SetText(view->GetSubItem(itemIndex, view->GetDataColumn(i)));
									dataTexts[i]->SetColor(listView->GetControlTemplateObject(true)->GetPrimaryTextColor());
									cell->SetOwnedElement(dataTexts[i]);
								}
							}
						}
					}

					FontChanged.AttachMethod(this, &InformationListViewItemTemplate::OnFontChanged);

					FontChanged.Execute(compositions::GuiEventArgs(this));
				}

				void InformationListViewItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					{
						auto font = GetFont();
						font.size = (vint)(font.size * 1.2);
						text->SetFont(font);
					}
					if (auto view = dynamic_cast<IListViewItemView*>(listControl->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
					{
						vint dataColumnCount = view->GetDataColumnCount();
						for (vint i = 0; i < dataColumnCount; i++)
						{
							columnTexts[i]->SetFont(GetFont());
							dataTexts[i]->SetFont(GetFont());
						}
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

				void DetailListViewItemTemplate::OnInitialize()
				{
					DefaultListViewItemTemplate::OnInitialize();
					columnItemView = dynamic_cast<ListViewColumnItemArranger::IColumnItemView*>(listControl->GetItemProvider()->RequestView(ListViewColumnItemArranger::IColumnItemView::Identifier));

					{
						textTable = new GuiTableComposition;
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
								cell->SetOwnedElement(image);
							}
							{
								auto cell = new GuiCellComposition;
								table->AddChild(cell);
								cell->SetSite(0, 1, 3, 1);
								cell->SetMargin(Margin(0, 0, 8, 0));

								text = GuiSolidLabelElement::Create();
								text->SetAlignments(Alignment::Left, Alignment::Center);
								text->SetEllipse(true);
								cell->SetOwnedElement(text);
							}
						}
					}

					if (auto listView = dynamic_cast<GuiVirtualListView*>(listControl))
					{
						auto itemIndex = GetIndex();
						if (auto view = dynamic_cast<IListViewItemView*>(listView->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
						{
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
							text->SetColor(listView->GetControlTemplateObject(true)->GetPrimaryTextColor());

							vint columnCount = view->GetColumnCount() - 1;
							subItems.Resize(columnCount);
							textTable->SetRowsAndColumns(1, columnCount + 1);
							for (vint i = 0; i < columnCount; i++)
							{
								auto cell = new GuiCellComposition;
								textTable->AddChild(cell);
								cell->SetSite(0, i + 1, 1, 1);
								cell->SetMargin(Margin(8, 0, 8, 0));

								subItems[i] = GuiSolidLabelElement::Create();
								subItems[i]->SetAlignments(Alignment::Left, Alignment::Center);
								subItems[i]->SetFont(text->GetFont());
								subItems[i]->SetEllipse(true);
								subItems[i]->SetText(view->GetSubItem(itemIndex, i));
								subItems[i]->SetColor(listView->GetControlTemplateObject(true)->GetSecondaryTextColor());
								cell->SetOwnedElement(subItems[i]);
							}
							OnColumnChanged();
						}
					}

					FontChanged.AttachMethod(this, &DetailListViewItemTemplate::OnFontChanged);

					FontChanged.Execute(compositions::GuiEventArgs(this));
				}

				void DetailListViewItemTemplate::OnColumnChanged()
				{
					if (auto view = dynamic_cast<IListViewItemView*>(listControl->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
					{
						if (columnItemView)
						{
							vint columnCount = view->GetColumnCount();
							if (columnCount>textTable->GetColumns())
							{
								columnCount = textTable->GetColumns();
							}
							for (vint i = 0; i<columnCount; i++)
							{
								textTable->SetColumnOption(i, GuiCellOption::AbsoluteOption(columnItemView->GetColumnSize(i)));
							}
							textTable->UpdateCellBounds();
						}
					}
				}

				void DetailListViewItemTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetFont(GetFont());
					if (auto view = dynamic_cast<IListViewItemView*>(listControl->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
					{
						vint columnCount = view->GetColumnCount() - 1;
						for (vint i = 0; i < columnCount; i++)
						{
							subItems[i]->SetFont(GetFont());
						}
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