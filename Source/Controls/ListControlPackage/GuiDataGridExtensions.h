/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDATAEXTENSIONS
#define VCZH_PRESENTATION_CONTROLS_GUIDATAEXTENSIONS

#include "GuiDataGridInterfaces.h"
#include "GuiComboControls.h"
#include "GuiTextListControls.h"
#include "../GuiDateTimeControls.h"
#include "../TextEditorPackage/GuiTextControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			namespace list
			{

/***********************************************************************
Extension Bases
***********************************************************************/
				
				/// <summary>Base class for all data visualizers.</summary>
				class DataVisualizerBase : public Object, public virtual IDataVisualizer
				{
					template<typename T>
					friend class DataVisualizerFactory;
					template<typename T>
					friend class DataDecoratableVisualizerFactory;
				protected:
					IDataVisualizerFactory*								factory = nullptr;
					IDataGridContext*									dataGridContext = nullptr;
					templates::GuiTemplate*								visualizerTemplate = nullptr;
					Ptr<IDataVisualizer>								decoratedDataVisualizer;

					virtual templates::GuiTemplate*						CreateTemplateInternal(templates::GuiTemplate* childTemplate) = 0;
				public:
					/// <summary>Create the data visualizer.</summary>
					/// <param name="_decoratedDataVisualizer">The decorated data visualizer inside the current data visualizer.</param>
					DataVisualizerBase(Ptr<IDataVisualizer> _decoratedDataVisualizer = nullptr);
					~DataVisualizerBase();

					IDataVisualizerFactory*								GetFactory()override;
					templates::GuiTemplate*								GetTemplate()override;
					void												NotifyDeletedTemplate()override;
					void												BeforeVisualizeCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)override;
					IDataVisualizer*									GetDecoratedDataVisualizer()override;
					void												SetSelected(bool value)override;
				};
				
				template<typename TVisualizer>
				class DataVisualizerFactory : public Object, public virtual IDataVisualizerFactory, public Description<DataVisualizerFactory<TVisualizer>>
				{
				public:
					Ptr<IDataVisualizer> CreateVisualizer(IDataGridContext* dataGridContext)override
					{
						DataVisualizerBase* dataVisualizer = new TVisualizer;
						dataVisualizer->factory = this;
						dataVisualizer->dataGridContext = dataGridContext;
						return dataVisualizer;
					}
				};
				
				template<typename TVisualizer>
				class DataDecoratableVisualizerFactory : public Object, public virtual IDataVisualizerFactory, public Description<DataDecoratableVisualizerFactory<TVisualizer>>
				{
				protected:
					Ptr<IDataVisualizerFactory>							decoratedFactory;
				public:
					DataDecoratableVisualizerFactory(Ptr<IDataVisualizerFactory> _decoratedFactory)
						:decoratedFactory(_decoratedFactory)
					{
					}

					Ptr<IDataVisualizer> CreateVisualizer(IDataGridContext* dataGridContext)override
					{
						Ptr<IDataVisualizer> decoratedDataVisualizer = decoratedFactory->CreateVisualizer(dataGridContext);
						DataVisualizerBase* dataVisualizer = new TVisualizer(decoratedDataVisualizer);
						dataVisualizer->factory = this;
						dataVisualizer->dataGridContext = dataGridContext;
						return dataVisualizer;
					}
				};
				
				/// <summary>Base class for all data editors.</summary>
				class DataEditorBase : public Object, public virtual IDataEditor
				{
					template<typename T>
					friend class DataEditorFactory;
				protected:
					IDataEditorFactory*									factory = nullptr;
					IDataGridContext*									dataGridContext = nullptr;
					templates::GuiTemplate*								editorTemplate = nullptr;

					virtual templates::GuiTemplate*						CreateTemplateInternal() = 0;
					void												RequestSaveData();
				public:
					/// <summary>Create the data editor.</summary>
					DataEditorBase();
					~DataEditorBase();

					IDataEditorFactory*									GetFactory()override;
					templates::GuiTemplate*								GetTemplate()override;
					void												NotifyDeletedTemplate()override;
					void												BeforeEditCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)override;
					void												ReinstallEditor()override;
				};
				
				template<typename TEditor>
				class DataEditorFactory : public Object, public virtual IDataEditorFactory, public Description<DataEditorFactory<TEditor>>
				{
				public:
					Ptr<IDataEditor> CreateEditor(IDataGridContext* dataGridContext)override
					{
						DataEditorBase* dataEditor = new TEditor;
						dataEditor->factory = this;
						dataEditor->dataGridContext = dataGridContext;
						return dataEditor;
					}
				};

/***********************************************************************
GuiBindableDataVisualizer
***********************************************************************/

				/// <summary>Data visualizer object for [T:vl.presentation.controls.GuiBindableDataGrid].</summary>
				class GuiBindableDataVisualizer : public controls::list::DataVisualizerBase, public Description<GuiBindableDataVisualizer>
				{
					using GuiGridVisualizerTemplate = templates::GuiGridVisualizerTemplate;
				public:
					class Factory : public controls::list::DataVisualizerFactory<GuiBindableDataVisualizer>
					{
					protected:
						TemplateProperty<GuiGridVisualizerTemplate>		templateFactory;

					public:
						Factory(TemplateProperty<GuiGridVisualizerTemplate> _templateFactory);
						~Factory();

						Ptr<controls::list::IDataVisualizer>			CreateVisualizer(controls::list::IDataGridContext* dataGridContext)override;
					};

					class DecoratedFactory : public controls::list::DataDecoratableVisualizerFactory<GuiBindableDataVisualizer>
					{
					protected:
						TemplateProperty<GuiGridVisualizerTemplate>		templateFactory;

					public:
						DecoratedFactory(TemplateProperty<GuiGridVisualizerTemplate> _templateFactory, Ptr<controls::list::IDataVisualizerFactory> _decoratedFactory);
						~DecoratedFactory();

						Ptr<controls::list::IDataVisualizer>			CreateVisualizer(controls::list::IDataGridContext* dataGridContext)override;
					};

				protected:
					TemplateProperty<GuiGridVisualizerTemplate>			templateFactory;
					GuiGridVisualizerTemplate*							visualizerTemplate = nullptr;

					templates::GuiTemplate*								CreateTemplateInternal(templates::GuiTemplate* childTemplate)override;
				public:
					GuiBindableDataVisualizer();
					GuiBindableDataVisualizer(Ptr<controls::list::IDataVisualizer> _decoratedVisualizer);
					~GuiBindableDataVisualizer();

					void												BeforeVisualizeCell(controls::GuiListControl::IItemProvider* itemProvider, vint row, vint column)override;
					void												SetSelected(bool value)override;
				};

/***********************************************************************
GuiBindableDataEditor
***********************************************************************/

				class GuiBindableDataEditor : public controls::list::DataEditorBase, public Description<GuiBindableDataEditor>
				{
					using GuiGridEditorTemplate = templates::GuiGridEditorTemplate;
				public:
					class Factory : public controls::list::DataEditorFactory<GuiBindableDataEditor>
					{
					protected:
						TemplateProperty<GuiGridEditorTemplate>			templateFactory;

					public:
						Factory(TemplateProperty<GuiGridEditorTemplate> _templateFactory);
						~Factory();

						Ptr<controls::list::IDataEditor>				CreateEditor(controls::list::IDataGridContext* dataGridContext)override;
					};

				protected:
					TemplateProperty<GuiGridEditorTemplate>				templateFactory;
					GuiGridEditorTemplate*								editorTemplate = nullptr;

					templates::GuiTemplate*								CreateTemplateInternal()override;
					void												editorTemplate_CellValueChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					GuiBindableDataEditor();
					~GuiBindableDataEditor();

					void												BeforeEditCell(controls::GuiListControl::IItemProvider* itemProvider, vint row, vint column)override;
					description::Value									GetEditedCellValue();
				};

/***********************************************************************
Visualizer Extensions
***********************************************************************/

				class MainColumnVisualizerTemplate : public templates::GuiGridVisualizerTemplate
				{
				protected:
					elements::GuiImageFrameElement*						image = nullptr;
					elements::GuiSolidLabelElement*						text = nullptr;

					void												OnTextChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnFontChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnTextColorChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnSmallImageChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					MainColumnVisualizerTemplate();
					~MainColumnVisualizerTemplate();
				};

				class SubColumnVisualizerTemplate : public templates::GuiGridVisualizerTemplate
				{
				protected:
					elements::GuiSolidLabelElement*						text = nullptr;

					void												OnTextChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnFontChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnTextColorChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												Initialize(bool fixTextColor);

					SubColumnVisualizerTemplate(bool fixTextColor);
				public:
					SubColumnVisualizerTemplate();
					~SubColumnVisualizerTemplate();
				};

				class HyperlinkVisualizerTemplate : public SubColumnVisualizerTemplate
				{
				protected:
					void												label_MouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												label_MouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

				public:
					HyperlinkVisualizerTemplate();
					~HyperlinkVisualizerTemplate();
				};

				class CellBorderVisualizerTemplate : public templates::GuiGridVisualizerTemplate
				{
				protected:
					elements::GuiSolidBorderElement*					border1 = nullptr;
					elements::GuiSolidBorderElement*					border2 = nullptr;

					void												OnItemSeparatorColorChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

				public:
					CellBorderVisualizerTemplate();
					~CellBorderVisualizerTemplate();
				};

/***********************************************************************
Editor Extensions
***********************************************************************/
				
				/// <summary>Data editor that displays a text box. Use TextBoxDataEditor::Factory as the factory class.</summary>
				class TextBoxDataEditor : public DataEditorBase
				{
				public:
					typedef DataEditorFactory<TextBoxDataEditor>							Factory;
				protected:
					GuiSinglelineTextBox*								textBox = nullptr;

					void												OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					templates::GuiTemplate*								CreateTemplateInternal()override;
				public:
					/// <summary>Create the data editor.</summary>
					TextBoxDataEditor();

					void												BeforeEditCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)override;

					/// <summary>Get the <see cref="GuiSinglelineTextBox"/> editor control.</summary>
					/// <returns>The control.</returns>
					GuiSinglelineTextBox*								GetTextBox();
				};
				
				/// <summary>Data editor that displays a text combo box. Use TextComboBoxDataEditor::Factory as the factory class.</summary>
				class TextComboBoxDataEditor : public DataEditorBase
				{
				public:
					typedef DataEditorFactory<TextComboBoxDataEditor>						Factory;
				protected:
					GuiComboBoxListControl*								comboBox = nullptr;
					GuiTextList*										textList = nullptr;
					
					void												OnSelectedIndexChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					templates::GuiTemplate*								CreateTemplateInternal()override;
				public:
					/// <summary>Create the data editor.</summary>
					TextComboBoxDataEditor();

					void												BeforeEditCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)override;

					/// <summary>Get the <see cref="GuiComboBoxListControl"/> editor control.</summary>
					/// <returns>The control.</returns>
					GuiComboBoxListControl*								GetComboBoxControl();

					/// <summary>Get the <see cref="GuiTextList"/> editor control.</summary>
					/// <returns>The control.</returns>
					GuiTextList*										GetTextListControl();
				};
				
				/// <summary>Data editor that displays a date combo box. Use DateComboBoxDataEditor::Factory as the factory class.</summary>
				class DateComboBoxDataEditor : public DataEditorBase
				{
				public:
					typedef DataEditorFactory<DateComboBoxDataEditor>						Factory;
				protected:
					GuiDateComboBox*									comboBox = nullptr;
					
					void												OnSelectedDateChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					templates::GuiTemplate*								CreateTemplateInternal()override;
				public:
					/// <summary>Create the data editor.</summary>
					DateComboBoxDataEditor();

					void												BeforeEditCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)override;

					/// <summary>Get the <see cref="GuiDateComboBox"/> editor control.</summary>
					/// <returns>The control.</returns>
					GuiDateComboBox*									GetComboBoxControl();

					/// <summary>Get the <see cref="GuiDatePicker"/> editor control.</summary>
					/// <returns>The control.</returns>
					GuiDatePicker*										GetDatePickerControl();
				};
			}
		}
	}
}

#endif