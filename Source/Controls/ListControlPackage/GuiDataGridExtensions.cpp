#include "GuiDataGridExtensions.h"
#include "../Styles/GuiThemeStyleFactory.h"
#include "../GuiApplication.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			namespace list
			{
				using namespace compositions;
				using namespace elements;
				using namespace theme;
				using namespace templates;
				
/***********************************************************************
DataVisualizerBase
***********************************************************************/

				DataVisualizerBase::DataVisualizerBase(Ptr<IDataVisualizer> _decoratedDataVisualizer)
					:decoratedDataVisualizer(_decoratedDataVisualizer)
				{
				}

				DataVisualizerBase::~DataVisualizerBase()
				{
					if (decoratedDataVisualizer)
					{
						decoratedDataVisualizer->NotifyDeletedTemplate();
					}
					if (visualizerTemplate)
					{
						SafeDeleteComposition(visualizerTemplate);
					}
				}

				IDataVisualizerFactory* DataVisualizerBase::GetFactory()
				{
					return factory;
				}

				templates::GuiTemplate* DataVisualizerBase::GetTemplate()
				{
					if (!visualizerTemplate)
					{
						GuiTemplate* childTemplate = nullptr;
						if (decoratedDataVisualizer)
						{
							childTemplate = decoratedDataVisualizer->GetTemplate();
						}
						visualizerTemplate = CreateTemplateInternal(childTemplate);
					}
					return visualizerTemplate;
				}

				void DataVisualizerBase::NotifyDeletedTemplate()
				{
					visualizerTemplate = nullptr;
					if (decoratedDataVisualizer)
					{
						decoratedDataVisualizer->NotifyDeletedTemplate();
					}
				}

				void DataVisualizerBase::BeforeVisualizeCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					if (decoratedDataVisualizer)
					{
						decoratedDataVisualizer->BeforeVisualizeCell(itemProvider, row, column);
					}
				}

				IDataVisualizer* DataVisualizerBase::GetDecoratedDataVisualizer()
				{
					return decoratedDataVisualizer.Obj();
				}

				void DataVisualizerBase::SetSelected(bool value)
				{
					if (decoratedDataVisualizer)
					{
						decoratedDataVisualizer->SetSelected(value);
					}
				}
				
/***********************************************************************
DataEditorBase
***********************************************************************/

				void DataEditorBase::RequestSaveData()
				{
					dataGridContext->RequestSaveData();
				}

				DataEditorBase::DataEditorBase()
				{
				}

				DataEditorBase::~DataEditorBase()
				{
					if (editorTemplate)
					{
						SafeDeleteComposition(editorTemplate);
					}
				}

				IDataEditorFactory* DataEditorBase::GetFactory()
				{
					return factory;
				}

				templates::GuiTemplate* DataEditorBase::GetTemplate()
				{
					if (!editorTemplate)
					{
						editorTemplate = CreateTemplateInternal();
					}
					return editorTemplate;
				}

				void DataEditorBase::NotifyDeletedTemplate()
				{
					editorTemplate = nullptr;
				}

				void DataEditorBase::BeforeEditCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
				}

				void DataEditorBase::ReinstallEditor()
				{
				}
				
/***********************************************************************
ListViewMainColumnDataVisualizer
***********************************************************************/

				templates::GuiTemplate* ListViewMainColumnDataVisualizer::CreateTemplateInternal(templates::GuiTemplate* childTemplate)
				{
					GuiTableComposition* table = new GuiTableComposition;
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					table->SetRowsAndColumns(3, 2);
					table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
					table->SetRowOption(1, GuiCellOption::MinSizeOption());
					table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());
					table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
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
						cell->SetMargin(Margin(0, 0, 8, 0));

						text = GuiSolidLabelElement::Create();
						cell->SetOwnedElement(text);
					}
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));

					auto dataTemplate = new GuiTemplate;
					dataTemplate->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					dataTemplate->AddChild(table);
					dataTemplate->FontChanged.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
					{
						text->SetFont(dataTemplate->GetFont());
					});
					return dataTemplate;
				}

				ListViewMainColumnDataVisualizer::ListViewMainColumnDataVisualizer()
				{
				}

				void ListViewMainColumnDataVisualizer::BeforeVisualizeCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					DataVisualizerBase::BeforeVisualizeCell(itemProvider, row, column);

					if (auto view = dynamic_cast<IListViewItemView*>(itemProvider->RequestView(IListViewItemView::Identifier)))
					{
						auto imageData = view->GetSmallImage(row);
						if (imageData)
						{
							image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
						}
						else
						{
							image->SetImage(nullptr);
						}

						text->SetAlignments(Alignment::Left, Alignment::Center);
						text->SetColor(dataGridContext->GetListViewStyleProvider()->GetPrimaryTextColor());
						text->SetEllipse(true);
						text->SetText(column == 0 ? itemProvider->GetTextValue(row) : view->GetSubItem(row, column - 1));
					}
				}

				elements::GuiSolidLabelElement* ListViewMainColumnDataVisualizer::GetTextElement()
				{
					return text;
				}
				
/***********************************************************************
ListViewSubColumnDataVisualizer
***********************************************************************/

				templates::GuiTemplate* ListViewSubColumnDataVisualizer::CreateTemplateInternal(templates::GuiTemplate* childTemplate)
				{
					auto dataTemplate = new GuiTemplate;
					dataTemplate->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					dataTemplate->SetMargin(Margin(8, 0, 8, 0));

					text = GuiSolidLabelElement::Create();
					dataTemplate->SetOwnedElement(text);

					dataTemplate->FontChanged.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
					{
						text->SetFont(dataTemplate->GetFont());
					});
					return dataTemplate;
				}

				ListViewSubColumnDataVisualizer::ListViewSubColumnDataVisualizer()
				{
				}

				void ListViewSubColumnDataVisualizer::BeforeVisualizeCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					DataVisualizerBase::BeforeVisualizeCell(itemProvider, row, column);

					if (auto view = dynamic_cast<IListViewItemView*>(itemProvider->RequestView(IListViewItemView::Identifier)))
					{
						text->SetAlignments(Alignment::Left, Alignment::Center);
						text->SetColor(dataGridContext->GetListViewStyleProvider()->GetSecondaryTextColor());
						text->SetEllipse(true);
						text->SetText(column == 0 ? itemProvider->GetTextValue(row) : view->GetSubItem(row, column - 1));
					}
				}

				elements::GuiSolidLabelElement* ListViewSubColumnDataVisualizer::GetTextElement()
				{
					return text;
				}
				
/***********************************************************************
HyperlinkDataVisualizer
***********************************************************************/

				void HyperlinkDataVisualizer::label_MouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					FontProperties font = text->GetFont();
					font.underline = true;
					text->SetFont(font);
				}

				void HyperlinkDataVisualizer::label_MouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					FontProperties font = text->GetFont();
					font.underline = false;
					text->SetFont(font);
				}

				templates::GuiTemplate* HyperlinkDataVisualizer::CreateTemplateInternal(templates::GuiTemplate* childTemplate)
				{
					auto dataTemplate = ListViewSubColumnDataVisualizer::CreateTemplateInternal(childTemplate);
					dataTemplate->GetEventReceiver()->mouseEnter.AttachMethod(this, &HyperlinkDataVisualizer::label_MouseEnter);
					dataTemplate->GetEventReceiver()->mouseLeave.AttachMethod(this, &HyperlinkDataVisualizer::label_MouseLeave);
					dataTemplate->SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::Hand));
					return dataTemplate;
				}

				HyperlinkDataVisualizer::HyperlinkDataVisualizer()
				{
				}

				void HyperlinkDataVisualizer::BeforeVisualizeCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					ListViewSubColumnDataVisualizer::BeforeVisualizeCell(itemProvider, row, column);
					text->SetColor(Color(0, 0, 255));
				}
				
/***********************************************************************
ImageDataVisualizer
***********************************************************************/

				templates::GuiTemplate* ImageDataVisualizer::CreateTemplateInternal(templates::GuiTemplate* childTemplate)
				{
					auto dataTemplate = new GuiTemplate;
					dataTemplate->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					dataTemplate->SetMargin(Margin(2, 2, 2, 2));

					image = GuiImageFrameElement::Create();
					dataTemplate->SetOwnedElement(image);
					return dataTemplate;
				}

				ImageDataVisualizer::ImageDataVisualizer()
				{
				}

				void ImageDataVisualizer::BeforeVisualizeCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					DataVisualizerBase::BeforeVisualizeCell(itemProvider, row, column);

					image->SetImage(0, -1);
					image->SetAlignments(Alignment::Center, Alignment::Center);
					image->SetStretch(false);
					image->SetEnabled(true);
				}

				elements::GuiImageFrameElement* ImageDataVisualizer::GetImageElement()
				{
					return image;
				}
				
/***********************************************************************
CellBorderDataVisualizer
***********************************************************************/

				templates::GuiTemplate* CellBorderDataVisualizer::CreateTemplateInternal(templates::GuiTemplate* childTemplate)
				{
					GuiBoundsComposition* border1 = nullptr;
					GuiBoundsComposition* border2 = nullptr;
					{
						GuiSolidBorderElement* element = GuiSolidBorderElement::Create();
						element->SetColor(dataGridContext->GetListViewStyleProvider()->GetItemSeparatorColor());

						border1 = new GuiBoundsComposition;
						border1->SetOwnedElement(element);
						border1->SetAlignmentToParent(Margin(-1, 0, 0, 0));
					}
					{
						GuiSolidBorderElement* element = GuiSolidBorderElement::Create();
						element->SetColor(dataGridContext->GetListViewStyleProvider()->GetItemSeparatorColor());

						border2 = new GuiBoundsComposition;
						border2->SetOwnedElement(element);
						border2->SetAlignmentToParent(Margin(0, -1, 0, 0));
					}
					childTemplate->SetAlignmentToParent(Margin(0, 0, 1, 1));

					auto dataTemplate = new GuiTemplate;
					dataTemplate->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					dataTemplate->AddChild(border1);
					dataTemplate->AddChild(border2);
					dataTemplate->AddChild(childTemplate);

					return dataTemplate;
				}

				CellBorderDataVisualizer::CellBorderDataVisualizer(Ptr<IDataVisualizer> decoratedDataVisualizer)
					:DataVisualizerBase(decoratedDataVisualizer)
				{
				}
				
/***********************************************************************
NotifyIconDataVisualizer
***********************************************************************/

				templates::GuiTemplate* NotifyIconDataVisualizer::CreateTemplateInternal(templates::GuiTemplate* childTemplate)
				{
					GuiTableComposition* table = new GuiTableComposition;
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					table->SetRowsAndColumns(1, 3);
					table->SetRowOption(0, GuiCellOption::MinSizeOption());
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());
					table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
					table->SetColumnOption(2, GuiCellOption::MinSizeOption());
					{
						GuiCellComposition* cell = new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 0, 1, 1);
						cell->SetInternalMargin(Margin(2, 2, 0, 2));

						leftImage = GuiImageFrameElement::Create();
						cell->SetOwnedElement(leftImage);
					}
					{
						GuiCellComposition* cell = new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 1, 1);

						childTemplate->SetAlignmentToParent(Margin(0, 0, 0, 0));
						cell->AddChild(childTemplate);
					}
					{
						GuiCellComposition* cell = new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 2, 1, 1);
						cell->SetInternalMargin(Margin(0, 2, 2, 2));

						rightImage = GuiImageFrameElement::Create();
						cell->SetOwnedElement(rightImage);
					}
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));

					auto dataTemplate = new GuiTemplate;
					dataTemplate->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					dataTemplate->AddChild(table);
					return dataTemplate;
				}

				NotifyIconDataVisualizer::NotifyIconDataVisualizer(Ptr<IDataVisualizer> decoratedDataVisualizer)
					:DataVisualizerBase(decoratedDataVisualizer)
				{
				}

				void NotifyIconDataVisualizer::BeforeVisualizeCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					DataVisualizerBase::BeforeVisualizeCell(itemProvider, row, column);

					leftImage->SetImage(0, -1);
					leftImage->SetAlignments(Alignment::Center, Alignment::Center);
					leftImage->SetStretch(false);
					leftImage->SetEnabled(true);

					rightImage->SetImage(0, -1);
					rightImage->SetAlignments(Alignment::Center, Alignment::Center);
					rightImage->SetStretch(false);
					rightImage->SetEnabled(true);
				}

				elements::GuiImageFrameElement* NotifyIconDataVisualizer::GetLeftImageElement()
				{
					return leftImage;
				}

				elements::GuiImageFrameElement* NotifyIconDataVisualizer::GetRightImageElement()
				{
					return rightImage;
				}
				
/***********************************************************************
TextBoxDataEditor
***********************************************************************/

				void TextBoxDataEditor::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					RequestSaveData();
				}

				templates::GuiTemplate* TextBoxDataEditor::CreateTemplateInternal()
				{
					textBox = g::NewTextBox();
					textBox->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					textBox->TextChanged.AttachMethod(this, &TextBoxDataEditor::OnTextChanged);

					auto dataTemplate = new GuiTemplate;
					dataTemplate->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					dataTemplate->AddChild(textBox->GetBoundsComposition());
					dataTemplate->FontChanged.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
					{
						textBox->SetFont(dataTemplate->GetFont());
					});
					return dataTemplate;
				}

				TextBoxDataEditor::TextBoxDataEditor()
				{
				}

				void TextBoxDataEditor::BeforeEditCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					DataEditorBase::BeforeEditCell(itemProvider, row, column);
					textBox->SetText(L"");
					GetApplication()->InvokeInMainThread([this]()
					{
						textBox->SetFocus();
					});
				}

				GuiSinglelineTextBox* TextBoxDataEditor::GetTextBox()
				{
					return textBox;
				}
				
/***********************************************************************
TextComboBoxDataEditor
***********************************************************************/

				void TextComboBoxDataEditor::OnSelectedIndexChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					RequestSaveData();
				}

				templates::GuiTemplate* TextComboBoxDataEditor::CreateTemplateInternal()
				{
					textList = g::NewTextList();
					comboBox = g::NewComboBox(textList);
					comboBox->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					comboBox->SelectedIndexChanged.AttachMethod(this, &TextComboBoxDataEditor::OnSelectedIndexChanged);

					auto dataTemplate = new GuiTemplate;
					dataTemplate->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					dataTemplate->AddChild(comboBox->GetBoundsComposition());
					dataTemplate->FontChanged.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
					{
						comboBox->SetFont(dataTemplate->GetFont());
					});
					return dataTemplate;
				}

				TextComboBoxDataEditor::TextComboBoxDataEditor()
				{
				}

				void TextComboBoxDataEditor::BeforeEditCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					DataEditorBase::BeforeEditCell(itemProvider, row, column);
					textList->GetItems().Clear();
				}

				GuiComboBoxListControl* TextComboBoxDataEditor::GetComboBoxControl()
				{
					return comboBox;
				}

				GuiTextList* TextComboBoxDataEditor::GetTextListControl()
				{
					return textList;
				}
				
/***********************************************************************
DateComboBoxDataEditor
***********************************************************************/

				void DateComboBoxDataEditor::OnSelectedDateChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					RequestSaveData();
				}

				templates::GuiTemplate* DateComboBoxDataEditor::CreateTemplateInternal()
				{
					comboBox = g::NewDateComboBox();
					comboBox->GetBoundsComposition()->SetAlignmentToParent(Margin(0, 0, 0, 0));
					comboBox->SelectedDateChanged.AttachMethod(this, &DateComboBoxDataEditor::OnSelectedDateChanged);

					auto dataTemplate = new GuiTemplate;
					dataTemplate->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					dataTemplate->AddChild(comboBox->GetBoundsComposition());
					dataTemplate->FontChanged.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
					{
						comboBox->SetFont(dataTemplate->GetFont());
					});
					return dataTemplate;
				}

				DateComboBoxDataEditor::DateComboBoxDataEditor()
				{
				}

				void DateComboBoxDataEditor::BeforeEditCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					DataEditorBase::BeforeEditCell(itemProvider, row, column);
					comboBox->SetSelectedDate(DateTime::LocalTime());
				}

				GuiDateComboBox* DateComboBoxDataEditor::GetComboBoxControl()
				{
					return comboBox;
				}

				GuiDatePicker* DateComboBoxDataEditor::GetDatePickerControl()
				{
					return comboBox->GetDatePicker();
				}
			}
		}
	}
}