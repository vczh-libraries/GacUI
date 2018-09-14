#include "GuiDataGridExtensions.h"
#include "../../GraphicsComposition/GuiGraphicsTableComposition.h"

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

				DataVisualizerBase::DataVisualizerBase()
				{
				}

				DataVisualizerBase::~DataVisualizerBase()
				{
					if (visualizerTemplate)
					{
						SafeDeleteComposition(visualizerTemplate);
					}
				}

				IDataVisualizerFactory* DataVisualizerBase::GetFactory()
				{
					return factory;
				}

				templates::GuiGridVisualizerTemplate* DataVisualizerBase::GetTemplate()
				{
					return visualizerTemplate;
				}

				void DataVisualizerBase::NotifyDeletedTemplate()
				{
					visualizerTemplate = nullptr;
				}

				void DataVisualizerBase::BeforeVisualizeCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					if (auto listViewItemView = dynamic_cast<IListViewItemView*>(dataGridContext->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
					{
						auto style = dataGridContext->GetListViewControlTemplate();
						visualizerTemplate->SetPrimaryTextColor(style->GetPrimaryTextColor());
						visualizerTemplate->SetSecondaryTextColor(style->GetSecondaryTextColor());
						visualizerTemplate->SetItemSeparatorColor(style->GetItemSeparatorColor());

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

				void DataVisualizerBase::SetSelected(bool value)
				{
					if (visualizerTemplate)
					{
						visualizerTemplate->SetSelected(value);
					}
				}

/***********************************************************************
DataVisualizerFactory
***********************************************************************/

				DataVisualizerFactory::ItemTemplate* DataVisualizerFactory::CreateItemTemplate(controls::list::IDataGridContext* dataGridContext)
				{
					ItemTemplate* itemTemplate = templateFactory({});
					CHECK_ERROR(itemTemplate, L"DataVisualizerFactory::CreateItemTemplate(IDataGridContext*)#An instance of GuiGridEditorTemplate is expected.");
					if (decoratedFactory)
					{
						auto childTemplate = decoratedFactory->CreateItemTemplate(dataGridContext);
						childTemplate->SetAlignmentToParent(Margin(0, 0, 0, 0));
						itemTemplate->GetContainerComposition()->AddChild(childTemplate);

#define FORWARD_EVENT(NAME)\
						itemTemplate->NAME##Changed.AttachLambda([=](GuiGraphicsComposition* sender, GuiEventArgs& arguments)\
						{\
							childTemplate->Set##NAME(itemTemplate->Get##NAME());\
						});\

#define FORWARD_EVENT_IMPL(CLASS, TYPE, NAME, VALUE) FORWARD_EVENT(NAME)

						GuiTemplate_PROPERTIES(FORWARD_EVENT_IMPL)
						GuiControlTemplate_PROPERTIES(FORWARD_EVENT_IMPL)
						GuiGridCellTemplate_PROPERTIES(FORWARD_EVENT_IMPL)
						GuiGridVisualizerTemplate_PROPERTIES(FORWARD_EVENT_IMPL)

#undef FORWARD_EVENT_IMPL
#undef FORWARD_EVENT
					}
					return itemTemplate;
				}

				DataVisualizerFactory::DataVisualizerFactory(TemplateProperty<ItemTemplate> _templateFactory, Ptr<DataVisualizerFactory> _decoratedFactory)
					:templateFactory(_templateFactory)
					, decoratedFactory(_decoratedFactory)
				{
				}

				DataVisualizerFactory::~DataVisualizerFactory()
				{
				}

				Ptr<controls::list::IDataVisualizer> DataVisualizerFactory::CreateVisualizer(controls::list::IDataGridContext* dataGridContext)
				{
					auto dataVisualizer = MakePtr<DataVisualizerBase>();
					dataVisualizer->factory = this;
					dataVisualizer->dataGridContext = dataGridContext;
					dataVisualizer->visualizerTemplate = CreateItemTemplate(dataGridContext);

					return dataVisualizer;
				}
				
/***********************************************************************
DataEditorBase
***********************************************************************/

				void DataEditorBase::OnCellValueChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
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

				templates::GuiGridEditorTemplate* DataEditorBase::GetTemplate()
				{
					return editorTemplate;
				}

				void DataEditorBase::NotifyDeletedTemplate()
				{
					editorTemplate = nullptr;
				}

				void DataEditorBase::BeforeEditCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)
				{
					if (auto listViewItemView = dynamic_cast<IListViewItemView*>(dataGridContext->GetItemProvider()->RequestView(IListViewItemView::Identifier)))
					{
						auto style = dataGridContext->GetListViewControlTemplate();
						editorTemplate->SetPrimaryTextColor(style->GetPrimaryTextColor());
						editorTemplate->SetSecondaryTextColor(style->GetSecondaryTextColor());
						editorTemplate->SetItemSeparatorColor(style->GetItemSeparatorColor());

						editorTemplate->SetLargeImage(listViewItemView->GetLargeImage(row));
						editorTemplate->SetSmallImage(listViewItemView->GetSmallImage(row));
						editorTemplate->SetText(column == 0 ? listViewItemView->GetText(row) : listViewItemView->GetSubItem(row, column - 1));
					}
					if (auto dataGridView = dynamic_cast<IDataGridView*>(dataGridContext->GetItemProvider()->RequestView(IDataGridView::Identifier)))
					{
						editorTemplate->SetRowValue(itemProvider->GetBindingValue(row));
						editorTemplate->SetCellValue(dataGridView->GetBindingCellValue(row, column));
					}
					editorTemplate->CellValueChanged.AttachMethod(this, &DataEditorBase::OnCellValueChanged);
				}

				bool DataEditorBase::GetCellValueSaved()
				{
					if (editorTemplate)
					{
						return editorTemplate->GetCellValueSaved();
					}
					return true;
				}

/***********************************************************************
DataEditorFactory
***********************************************************************/

				DataEditorFactory::DataEditorFactory(TemplateProperty<GuiGridEditorTemplate> _templateFactory)
					:templateFactory(_templateFactory)
				{
				}

				DataEditorFactory::~DataEditorFactory()
				{
				}

				Ptr<IDataEditor> DataEditorFactory::CreateEditor(controls::list::IDataGridContext* dataGridContext)
				{
					auto editor = MakePtr<DataEditorBase>();
					editor->factory = this;
					editor->dataGridContext = dataGridContext;

					ItemTemplate* itemTemplate = templateFactory({});
					CHECK_ERROR(itemTemplate, L"DataEditorFactory::CreateEditor(IDataGridContext*)#An instance of GuiGridEditorTemplate is expected.");
					editor->editorTemplate = itemTemplate;
					return editor;
				}
				
/***********************************************************************
MainColumnVisualizerTemplate
***********************************************************************/

				void MainColumnVisualizerTemplate::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetText(GetText());
				}

				void MainColumnVisualizerTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetFont(GetFont());
				}

				void MainColumnVisualizerTemplate::OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetColor(GetPrimaryTextColor());
				}

				void MainColumnVisualizerTemplate::OnSmallImageChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
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

				void SubColumnVisualizerTemplate::OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetText(GetText());
				}

				void SubColumnVisualizerTemplate::OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					text->SetFont(GetFont());
				}

				void SubColumnVisualizerTemplate::OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
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
					text->SetEllipse(true);
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

				void FocusRectangleVisualizerTemplate::OnSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments)
				{
					focusComposition->SetVisible(GetSelected());
				}

				FocusRectangleVisualizerTemplate::FocusRectangleVisualizerTemplate()
				{
					SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					focusComposition = new GuiBoundsComposition();
					{
						auto focus = GuiFocusRectangleElement::Create();
						focusComposition->SetOwnedElement(focus);
						focusComposition->SetAlignmentToParent(Margin(1, 1, 1, 1));
					}
					auto container = new GuiBoundsComposition();
					{
						container->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						container->SetAlignmentToParent(Margin(2, 2, 2, 2));
					}

					AddChild(focusComposition);
					AddChild(container);
					SetContainerComposition(container);

					SelectedChanged.AttachMethod(this, &FocusRectangleVisualizerTemplate::OnSelectedChanged);
					SelectedChanged.Execute(compositions::GuiEventArgs(this));
				}

				FocusRectangleVisualizerTemplate::~FocusRectangleVisualizerTemplate()
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
					SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);

					auto bounds1 = new GuiBoundsComposition;
					{
						border1 = GuiSolidBorderElement::Create();
						bounds1->SetOwnedElement(border1);
						bounds1->SetAlignmentToParent(Margin(-1, 0, 0, 0));
					}
					auto bounds2 = new GuiBoundsComposition;
					{
						border2 = GuiSolidBorderElement::Create();
						bounds2->SetOwnedElement(border2);
						bounds2->SetAlignmentToParent(Margin(0, -1, 0, 0));
					}
					auto container = new GuiBoundsComposition();
					{
						container->SetMinSizeLimitation(GuiGraphicsComposition::LimitToElementAndChildren);
						container->SetAlignmentToParent(Margin(0, 0, 1, 1));
					}

					AddChild(bounds1);
					AddChild(bounds2);
					AddChild(container);
					SetContainerComposition(container);

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