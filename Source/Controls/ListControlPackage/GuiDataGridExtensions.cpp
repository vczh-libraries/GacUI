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
						if (decoratedDataVisualizer)
						{
							visualizerTemplate->FontChanged.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)
							{
								decoratedDataVisualizer->GetTemplate()->SetFont(visualizerTemplate->GetFont());
							});
						}
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
GuiBindableDataVisualizer::Factory
***********************************************************************/

				GuiBindableDataVisualizer::Factory::Factory(TemplateProperty<GuiGridVisualizerTemplate> _templateFactory)
					:templateFactory(_templateFactory)
				{
				}

				GuiBindableDataVisualizer::Factory::~Factory()
				{
				}

				Ptr<controls::list::IDataVisualizer> GuiBindableDataVisualizer::Factory::CreateVisualizer(controls::list::IDataGridContext* dataGridContext)
				{
					auto visualizer = DataVisualizerFactory<GuiBindableDataVisualizer>::CreateVisualizer(dataGridContext).Cast<GuiBindableDataVisualizer>();
					if (visualizer)
					{
						visualizer->templateFactory = templateFactory;
					}
					return visualizer;
				}

/***********************************************************************
GuiBindableDataVisualizer::DecoratedFactory
***********************************************************************/

				GuiBindableDataVisualizer::DecoratedFactory::DecoratedFactory(TemplateProperty<GuiGridVisualizerTemplate> _templateFactory, Ptr<controls::list::IDataVisualizerFactory> _decoratedFactory)
					:DataDecoratableVisualizerFactory<GuiBindableDataVisualizer>(_decoratedFactory)
					, templateFactory(_templateFactory)
				{
				}

				GuiBindableDataVisualizer::DecoratedFactory::~DecoratedFactory()
				{
				}

				Ptr<controls::list::IDataVisualizer> GuiBindableDataVisualizer::DecoratedFactory::CreateVisualizer(controls::list::IDataGridContext* dataGridContext)
				{
					auto visualizer = DataDecoratableVisualizerFactory<GuiBindableDataVisualizer>::CreateVisualizer(dataGridContext).Cast<GuiBindableDataVisualizer>();
					if (visualizer)
					{
						visualizer->templateFactory = templateFactory;
					}
					return visualizer;
				}

/***********************************************************************
GuiBindableDataVisualizer
***********************************************************************/

				GuiTemplate* GuiBindableDataVisualizer::CreateTemplateInternal(GuiTemplate* childTemplate)
				{
					visualizerTemplate = templateFactory(dataGridContext->GetViewModelContext());
					CHECK_ERROR(visualizerTemplate, L"GuiBindableDataVisualizer::CreateTemplateInternal(GuiTemplate*)#An instance of GuiGridVisualizerTemplate is expected.");

					if (childTemplate)
					{
						childTemplate->SetAlignmentToParent(Margin(0, 0, 0, 0));
						visualizerTemplate->GetContainerComposition()->AddChild(childTemplate);
					}
					return visualizerTemplate;
				}

				GuiBindableDataVisualizer::GuiBindableDataVisualizer()
				{
				}

				GuiBindableDataVisualizer::GuiBindableDataVisualizer(Ptr<controls::list::IDataVisualizer> _decoratedVisualizer)
					:DataVisualizerBase(_decoratedVisualizer)
				{
				}

				GuiBindableDataVisualizer::~GuiBindableDataVisualizer()
				{
				}

				void GuiBindableDataVisualizer::BeforeVisualizeCell(controls::GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					DataVisualizerBase::BeforeVisualizeCell(itemProvider, row, column);
					if (auto listViewItemView = dynamic_cast<IListViewItemView*>(dataGridContext->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
					{
						auto styleProvider = dataGridContext->GetListViewStyleProvider();
						visualizerTemplate->SetPrimaryTextColor(styleProvider->GetPrimaryTextColor());
						visualizerTemplate->SetSecondaryTextColor(styleProvider->GetSecondaryTextColor());
						visualizerTemplate->SetItemSeparatorColor(styleProvider->GetItemSeparatorColor());

						visualizerTemplate->SetLargeImage(listViewItemView->GetLargeImage(row));
						visualizerTemplate->SetSmallImage(listViewItemView->GetSmallImage(row));
						visualizerTemplate->SetText(column == 0 ? listViewItemView->GetText(row) : listViewItemView->GetSubItem(row, column - 1));
					}
					if (auto dataGridView = dynamic_cast<IDataGridView*>(dataGridContext->GetItemProvider()->RequestView(IDataGridView::Identifier)))
					{
						visualizerTemplate->SetRowValue(itemProvider->GetBindingValue(row));
						visualizerTemplate->SetCellValue(dataGridView->GetBindingCellValue(row, column));
					}
				}

				void GuiBindableDataVisualizer::SetSelected(bool value)
				{
					DataVisualizerBase::SetSelected(value);
					if (visualizerTemplate)
					{
						visualizerTemplate->SetSelected(value);
					}
				}

/***********************************************************************
GuiBindableDataEditor::Factory
***********************************************************************/

				GuiBindableDataEditor::Factory::Factory(TemplateProperty<GuiGridEditorTemplate> _templateFactory)
					:templateFactory(_templateFactory)
				{
				}

				GuiBindableDataEditor::Factory::~Factory()
				{
				}

				Ptr<controls::list::IDataEditor> GuiBindableDataEditor::Factory::CreateEditor(controls::list::IDataGridContext* dataGridContext)
				{
					auto editor = DataEditorFactory<GuiBindableDataEditor>::CreateEditor(dataGridContext).Cast<GuiBindableDataEditor>();
					if (editor)
					{
						editor->templateFactory = templateFactory;

						// Invoke GuiBindableDataEditor::CreateTemplateInternal
						// so that GuiBindableDataEditor::BeforeEditCell is able to set RowValue and CellValue to the editor
						editor->GetTemplate();
					}
					return editor;
				}

/***********************************************************************
GuiBindableDataEditor
***********************************************************************/

				GuiTemplate* GuiBindableDataEditor::CreateTemplateInternal()
				{
					editorTemplate = templateFactory(dataGridContext->GetViewModelContext());
					CHECK_ERROR(editorTemplate, L"GuiBindableDataEditor::CreateTemplateInternal()#An instance of GuiGridEditorTemplate is expected.");

					editorTemplate->CellValueChanged.AttachMethod(this, &GuiBindableDataEditor::editorTemplate_CellValueChanged);
					return editorTemplate;
				}

				void GuiBindableDataEditor::editorTemplate_CellValueChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					dataGridContext->RequestSaveData();
				}

				GuiBindableDataEditor::GuiBindableDataEditor()
				{
				}

				GuiBindableDataEditor::~GuiBindableDataEditor()
				{
				}

				void GuiBindableDataEditor::BeforeEditCell(controls::GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					DataEditorBase::BeforeEditCell(itemProvider, row, column);
					if (auto listViewItemView = dynamic_cast<IListViewItemView*>(dataGridContext->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
					{
						auto styleProvider = dataGridContext->GetListViewStyleProvider();
						editorTemplate->SetPrimaryTextColor(styleProvider->GetPrimaryTextColor());
						editorTemplate->SetSecondaryTextColor(styleProvider->GetSecondaryTextColor());
						editorTemplate->SetItemSeparatorColor(styleProvider->GetItemSeparatorColor());

						editorTemplate->SetLargeImage(listViewItemView->GetLargeImage(row));
						editorTemplate->SetSmallImage(listViewItemView->GetSmallImage(row));
						editorTemplate->SetText(column == 0 ? listViewItemView->GetText(row) : listViewItemView->GetSubItem(row, column - 1));
					}
					if (auto dataGridView = dynamic_cast<IDataGridView*>(dataGridContext->GetItemProvider()->RequestView(IDataGridView::Identifier)))
					{
						editorTemplate->SetRowValue(itemProvider->GetBindingValue(row));
						editorTemplate->SetCellValue(dataGridView->GetBindingCellValue(row, column));
					}
				}

				description::Value GuiBindableDataEditor::GetEditedCellValue()
				{
					if (editorTemplate)
					{
						return editorTemplate->GetCellValue();
					}
					else
					{
						return description::Value();
					}
				}
				
/***********************************************************************
MainColumnVisualizerTemplate
***********************************************************************/

				void MainColumnVisualizerTemplate::OnTextChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetText(GetText());
				}

				void MainColumnVisualizerTemplate::OnFontChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetFont(GetFont());
				}

				void MainColumnVisualizerTemplate::OnTextColorChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetColor(GetPrimaryTextColor());
				}

				void MainColumnVisualizerTemplate::OnSmallImageChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					auto imageData = GetSmallImage();
					if (imageData)
					{
						image->SetImage(imageData->GetImage(), imageData->GetFrameIndex());
					}
					else
					{
						image->SetImage(nullptr);
					}
				}

				MainColumnVisualizerTemplate::MainColumnVisualizerTemplate()
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
						text->SetAlignments(Alignment::Left, Alignment::Center);
						text->SetEllipse(true);
						cell->SetOwnedElement(text);
					}
					table->SetAlignmentToParent(Margin(0, 0, 0, 0));

					SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					AddChild(table);

					TextChanged.AttachMethod(this, &MainColumnVisualizerTemplate::OnTextChanged);
					FontChanged.AttachMethod(this, &MainColumnVisualizerTemplate::OnFontChanged);
					PrimaryTextColorChanged.AttachMethod(this, &MainColumnVisualizerTemplate::OnTextColorChanged);
					SmallImageChanged.AttachMethod(this, &MainColumnVisualizerTemplate::OnSmallImageChanged);

					TextChanged.Execute(compositions::GuiEventArgs(this));
					FontChanged.Execute(compositions::GuiEventArgs(this));
					PrimaryTextColorChanged.Execute(compositions::GuiEventArgs(this));
					SmallImageChanged.Execute(compositions::GuiEventArgs(this));
				}

				MainColumnVisualizerTemplate::~MainColumnVisualizerTemplate()
				{
				}
				
/***********************************************************************
SubColumnVisualizerTemplate
***********************************************************************/

				void SubColumnVisualizerTemplate::OnTextChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetText(GetText());
				}

				void SubColumnVisualizerTemplate::OnFontChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetFont(GetFont());
				}

				void SubColumnVisualizerTemplate::OnTextColorChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetColor(GetSecondaryTextColor());
				}

				void SubColumnVisualizerTemplate::Initialize(bool fixTextColor)
				{
					text = GuiSolidLabelElement::Create();
					text->SetVerticalAlignment(Alignment::Center);

					SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					SetMargin(Margin(8, 0, 8, 0));
					SetOwnedElement(text);

					TextChanged.AttachMethod(this, &SubColumnVisualizerTemplate::OnTextChanged);
					FontChanged.AttachMethod(this, &SubColumnVisualizerTemplate::OnFontChanged);
					if (!fixTextColor)
					{
						SecondaryTextColorChanged.AttachMethod(this, &SubColumnVisualizerTemplate::OnTextColorChanged);
					}

					TextChanged.Execute(compositions::GuiEventArgs(this));
					FontChanged.Execute(compositions::GuiEventArgs(this));
					if (!fixTextColor)
					{
						SecondaryTextColorChanged.Execute(compositions::GuiEventArgs(this));
					}
				}

				SubColumnVisualizerTemplate::SubColumnVisualizerTemplate(bool fixTextColor)
				{
					Initialize(fixTextColor);
				}

				SubColumnVisualizerTemplate::SubColumnVisualizerTemplate()
				{
					Initialize(false);
				}

				SubColumnVisualizerTemplate::~SubColumnVisualizerTemplate()
				{
				}
				
/***********************************************************************
HyperlinkVisualizerTemplate
***********************************************************************/

				void HyperlinkVisualizerTemplate::label_MouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					FontProperties font = text->GetFont();
					font.underline = true;
					text->SetFont(font);
				}

				void HyperlinkVisualizerTemplate::label_MouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					FontProperties font = text->GetFont();
					font.underline = false;
					text->SetFont(font);
				}

				HyperlinkVisualizerTemplate::HyperlinkVisualizerTemplate()
					:SubColumnVisualizerTemplate(true)
				{
					text->SetColor(Color(0, 0, 255));
					GetEventReceiver()->mouseEnter.AttachMethod(this, &HyperlinkVisualizerTemplate::label_MouseEnter);
					GetEventReceiver()->mouseLeave.AttachMethod(this, &HyperlinkVisualizerTemplate::label_MouseLeave);
					SetAssociatedCursor(GetCurrentController()->ResourceService()->GetSystemCursor(INativeCursor::Hand));
				}

				HyperlinkVisualizerTemplate::~HyperlinkVisualizerTemplate()
				{
				}
				
/***********************************************************************
CellBorderVisualizerTemplate
***********************************************************************/

				void CellBorderVisualizerTemplate::OnItemSeparatorColorChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					border1->SetColor(GetItemSeparatorColor());
					border2->SetColor(GetItemSeparatorColor());
				}

				CellBorderVisualizerTemplate::CellBorderVisualizerTemplate()
				{
					GuiBoundsComposition* bounds1 = nullptr;
					GuiBoundsComposition* bounds2 = nullptr;
					{
						border1 = GuiSolidBorderElement::Create();

						bounds1 = new GuiBoundsComposition;
						bounds1->SetOwnedElement(border1);
						bounds1->SetAlignmentToParent(Margin(-1, 0, 0, 0));
					}
					{
						border2 = GuiSolidBorderElement::Create();

						bounds2 = new GuiBoundsComposition;
						bounds2->SetOwnedElement(border2);
						bounds2->SetAlignmentToParent(Margin(0, -1, 0, 0));
					}

					SetAlignmentToParent(Margin(0, 0, 1, 1));
					SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
					AddChild(bounds1);
					AddChild(bounds2);

					ItemSeparatorColorChanged.AttachMethod(this, &CellBorderVisualizerTemplate::OnItemSeparatorColorChanged);

					ItemSeparatorColorChanged.Execute(compositions::GuiEventArgs(this));
				}

				CellBorderVisualizerTemplate::~CellBorderVisualizerTemplate()
				{
				}
			}
		}
	}
}