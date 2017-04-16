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
				
/***********************************************************************
DataVisualizerBase
***********************************************************************/

				DataVisualizerBase::DataVisualizerBase(Ptr<IDataVisualizer> _decoratedDataVisualizer)
					:decoratedDataVisualizer(_decoratedDataVisualizer)
				{
				}

				DataVisualizerBase::~DataVisualizerBase()
				{
					if(decoratedDataVisualizer)
					{
						GuiBoundsComposition* composition=decoratedDataVisualizer->GetBoundsComposition();
						if(composition->GetParent())
						{
							composition->GetParent()->RemoveChild(composition);
						}
						decoratedDataVisualizer=0;
					}
					if(boundsComposition)
					{
						SafeDeleteComposition(boundsComposition);
					}
				}

				IDataVisualizerFactory* DataVisualizerBase::GetFactory()
				{
					return factory;
				}

				compositions::GuiBoundsComposition* DataVisualizerBase::GetBoundsComposition()
				{
					if(!boundsComposition)
					{
						GuiBoundsComposition* decoratedComposition=0;
						if(decoratedDataVisualizer)
						{
							decoratedComposition=decoratedDataVisualizer->GetBoundsComposition();
						}
						boundsComposition=CreateBoundsCompositionInternal(decoratedComposition);
					}
					return boundsComposition;
				}

				void DataVisualizerBase::BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)
				{
					if(decoratedDataVisualizer)
					{
						decoratedDataVisualizer->BeforeVisualizeCell(dataProvider, row, column);
					}
				}

				IDataVisualizer* DataVisualizerBase::GetDecoratedDataVisualizer()
				{
					return decoratedDataVisualizer.Obj();
				}

				void DataVisualizerBase::SetSelected(bool value)
				{
					if(decoratedDataVisualizer)
					{
						decoratedDataVisualizer->SetSelected(value);
					}
				}
				
/***********************************************************************
DataEditorBase
***********************************************************************/

				DataEditorBase::DataEditorBase()
				{
				}

				DataEditorBase::~DataEditorBase()
				{
					if(boundsComposition)
					{
						SafeDeleteComposition(boundsComposition);
					}
				}

				IDataEditorFactory* DataEditorBase::GetFactory()
				{
					return factory;
				}

				compositions::GuiBoundsComposition* DataEditorBase::GetBoundsComposition()
				{
					if(!boundsComposition)
					{
						boundsComposition=CreateBoundsCompositionInternal();
					}
					return boundsComposition;
				}

				void DataEditorBase::BeforeEditCell(IDataProvider* dataProvider, vint row, vint column)
				{
				}

				void DataEditorBase::ReinstallEditor()
				{
				}
				
/***********************************************************************
ListViewMainColumnDataVisualizer
***********************************************************************/

				compositions::GuiBoundsComposition* ListViewMainColumnDataVisualizer::CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)
				{
					GuiTableComposition* table=new GuiTableComposition;
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					table->SetRowsAndColumns(3, 2);
					table->SetRowOption(0, GuiCellOption::PercentageOption(0.5));
					table->SetRowOption(1, GuiCellOption::MinSizeOption());
					table->SetRowOption(2, GuiCellOption::PercentageOption(0.5));
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());
					table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
					table->SetCellPadding(2);
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(1, 0, 1, 1);
						cell->SetPreferredMinSize(Size(16, 16));

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
						cell->SetOwnedElement(text);
					}
					return table;
				}

				ListViewMainColumnDataVisualizer::ListViewMainColumnDataVisualizer()
					:image(0)
					,text(0)
				{
				}

				void ListViewMainColumnDataVisualizer::BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)
				{
					DataVisualizerBase::BeforeVisualizeCell(dataProvider, row, column);

					Ptr<GuiImageData> imageData=dataProvider->GetRowSmallImage(row);
					if(imageData)
					{
						image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
					}
					else
					{
						image->SetImage(0);
					}

					text->SetAlignments(Alignment::Left, Alignment::Center);
					text->SetFont(font);
					text->SetColor(styleProvider->GetPrimaryTextColor());
					text->SetEllipse(true);
					text->SetText(dataProvider->GetCellText(row, column));
				}

				elements::GuiSolidLabelElement* ListViewMainColumnDataVisualizer::GetTextElement()
				{
					return text;
				}
				
/***********************************************************************
ListViewSubColumnDataVisualizer
***********************************************************************/

				compositions::GuiBoundsComposition* ListViewSubColumnDataVisualizer::CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)
				{
					GuiBoundsComposition* composition=new GuiBoundsComposition;
					composition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					composition->SetMargin(Margin(8, 0, 8, 0));

					text=GuiSolidLabelElement::Create();
					composition->SetOwnedElement(text);

					return composition;
				}

				ListViewSubColumnDataVisualizer::ListViewSubColumnDataVisualizer()
					:text(0)
				{
				}

				void ListViewSubColumnDataVisualizer::BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)
				{
					DataVisualizerBase::BeforeVisualizeCell(dataProvider, row, column);

					text->SetAlignments(Alignment::Left, Alignment::Center);
					text->SetFont(font);
					text->SetColor(styleProvider->GetSecondaryTextColor());
					text->SetEllipse(true);
					text->SetText(dataProvider->GetCellText(row, column));
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
					FontProperties font=text->GetFont();
					font.underline=true;
					text->SetFont(font);
				}

				void HyperlinkDataVisualizer::label_MouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					FontProperties font=text->GetFont();
					font.underline=false;
					text->SetFont(font);
				}

				compositions::GuiBoundsComposition* HyperlinkDataVisualizer::CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)
				{
					GuiBoundsComposition* composition=ListViewSubColumnDataVisualizer::CreateBoundsCompositionInternal(decoratedComposition);
					composition->GetEventReceiver()->mouseEnter.AttachMethod(this, &HyperlinkDataVisualizer::label_MouseEnter);
					composition->GetEventReceiver()->mouseLeave.AttachMethod(this, &HyperlinkDataVisualizer::label_MouseLeave);
					composition->SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::Hand));
					return composition;
				}

				HyperlinkDataVisualizer::HyperlinkDataVisualizer()
				{
				}

				void HyperlinkDataVisualizer::BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)
				{
					ListViewSubColumnDataVisualizer::BeforeVisualizeCell(dataProvider, row, column);
					text->SetColor(Color(0, 0, 255));
				}
				
/***********************************************************************
ImageDataVisualizer
***********************************************************************/

				compositions::GuiBoundsComposition* ImageDataVisualizer::CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)
				{
					GuiBoundsComposition* composition=new GuiBoundsComposition;
					composition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					composition->SetMargin(Margin(2, 2, 2, 2));

					image=GuiImageFrameElement::Create();
					composition->SetOwnedElement(image);
					return composition;
				}

				ImageDataVisualizer::ImageDataVisualizer()
					:image(0)
				{
				}

				void ImageDataVisualizer::BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)
				{
					DataVisualizerBase::BeforeVisualizeCell(dataProvider, row, column);

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

				compositions::GuiBoundsComposition* CellBorderDataVisualizer::CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)
				{
					GuiBoundsComposition* border1=0;
					GuiBoundsComposition* border2=0;
					{
						GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
						element->SetColor(styleProvider->GetItemSeparatorColor());

						border1=new GuiBoundsComposition;
						border1->SetOwnedElement(element);
						border1->SetAlignmentToParent(Margin(-1, 0, 0, 0));
					}
					{
						GuiSolidBorderElement* element=GuiSolidBorderElement::Create();
						element->SetColor(styleProvider->GetItemSeparatorColor());

						border2=new GuiBoundsComposition;
						border2->SetOwnedElement(element);
						border2->SetAlignmentToParent(Margin(0, -1, 0, 0));
					}
					decoratedComposition->SetAlignmentToParent(Margin(0, 0, 1, 1));

					GuiBoundsComposition* composition=new GuiBoundsComposition;
					composition->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					composition->AddChild(border1);
					composition->AddChild(border2);
					composition->AddChild(decoratedComposition);

					return composition;
				}

				CellBorderDataVisualizer::CellBorderDataVisualizer(Ptr<IDataVisualizer> decoratedDataVisualizer)
					:DataVisualizerBase(decoratedDataVisualizer)
				{
				}
				
/***********************************************************************
NotifyIconDataVisualizer
***********************************************************************/

				compositions::GuiBoundsComposition* NotifyIconDataVisualizer::CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)
				{
					GuiTableComposition* table=new GuiTableComposition;
					table->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					table->SetRowsAndColumns(1, 3);
					table->SetRowOption(0, GuiCellOption::MinSizeOption());
					table->SetColumnOption(0, GuiCellOption::MinSizeOption());
					table->SetColumnOption(1, GuiCellOption::PercentageOption(1.0));
					table->SetColumnOption(2, GuiCellOption::MinSizeOption());
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 0, 1, 1);
						cell->SetInternalMargin(Margin(2, 2, 0, 2));

						leftImage=GuiImageFrameElement::Create();
						cell->SetOwnedElement(leftImage);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 1, 1, 1);

						decoratedComposition->SetAlignmentToParent(Margin(0, 0, 0, 0));
						cell->AddChild(decoratedComposition);
					}
					{
						GuiCellComposition* cell=new GuiCellComposition;
						table->AddChild(cell);
						cell->SetSite(0, 2, 1, 1);
						cell->SetInternalMargin(Margin(0, 2, 2, 2));

						rightImage=GuiImageFrameElement::Create();
						cell->SetOwnedElement(rightImage);
					}
					return table;
				}

				NotifyIconDataVisualizer::NotifyIconDataVisualizer(Ptr<IDataVisualizer> decoratedDataVisualizer)
					:DataVisualizerBase(decoratedDataVisualizer)
					,leftImage(0)
					,rightImage(0)
				{
				}

				void NotifyIconDataVisualizer::BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)
				{
					DataVisualizerBase::BeforeVisualizeCell(dataProvider, row, column);

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
					callback->RequestSaveData();
				}

				compositions::GuiBoundsComposition* TextBoxDataEditor::CreateBoundsCompositionInternal()
				{
					return textBox->GetBoundsComposition();
				}

				TextBoxDataEditor::TextBoxDataEditor()
				{
					textBox=g::NewTextBox();
					textBox->TextChanged.AttachMethod(this, &TextBoxDataEditor::OnTextChanged);
				}

				void TextBoxDataEditor::BeforeEditCell(IDataProvider* dataProvider, vint row, vint column)
				{
					DataEditorBase::BeforeEditCell(dataProvider, row, column);
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
					callback->RequestSaveData();
				}

				compositions::GuiBoundsComposition* TextComboBoxDataEditor::CreateBoundsCompositionInternal()
				{
					return comboBox->GetBoundsComposition();
				}

				TextComboBoxDataEditor::TextComboBoxDataEditor()
				{
					textList=g::NewTextList();
					comboBox=g::NewComboBox(textList);
					comboBox->SelectedIndexChanged.AttachMethod(this, &TextComboBoxDataEditor::OnSelectedIndexChanged);
				}

				void TextComboBoxDataEditor::BeforeEditCell(IDataProvider* dataProvider, vint row, vint column)
				{
					DataEditorBase::BeforeEditCell(dataProvider, row, column);
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
					callback->RequestSaveData();
				}

				compositions::GuiBoundsComposition* DateComboBoxDataEditor::CreateBoundsCompositionInternal()
				{
					return comboBox->GetBoundsComposition();
				}

				DateComboBoxDataEditor::DateComboBoxDataEditor()
				{
					comboBox=g::NewDateComboBox();
					comboBox->SelectedDateChanged.AttachMethod(this, &DateComboBoxDataEditor::OnSelectedDateChanged);
				}

				void DateComboBoxDataEditor::BeforeEditCell(IDataProvider* dataProvider, vint row, vint column)
				{
					DataEditorBase::BeforeEditCell(dataProvider, row, column);
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