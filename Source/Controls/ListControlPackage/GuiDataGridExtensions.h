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
					FontProperties										font;
					GuiListViewBase::IStyleProvider*					styleProvider = nullptr;
					description::Value									viewModelContext;
					compositions::GuiBoundsComposition*					boundsComposition = nullptr;
					Ptr<IDataVisualizer>								decoratedDataVisualizer;

					virtual compositions::GuiBoundsComposition*			CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)=0;
				public:
					/// <summary>Create the data visualizer.</summary>
					/// <param name="_decoratedDataVisualizer">The decorated data visualizer inside the current data visualizer.</param>
					DataVisualizerBase(Ptr<IDataVisualizer> _decoratedDataVisualizer = nullptr);
					~DataVisualizerBase();

					IDataVisualizerFactory*								GetFactory()override;
					compositions::GuiBoundsComposition*					GetBoundsComposition()override;
					void												BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)override;
					IDataVisualizer*									GetDecoratedDataVisualizer()override;
					void												SetSelected(bool value)override;
				};
				
				template<typename TVisualizer>
				class DataVisualizerFactory : public Object, public virtual IDataVisualizerFactory, public Description<DataVisualizerFactory<TVisualizer>>
				{
				public:
					Ptr<IDataVisualizer> CreateVisualizer(const FontProperties& font, GuiListViewBase::IStyleProvider* styleProvider, const description::Value& viewModelContext)override
					{
						DataVisualizerBase* dataVisualizer = new TVisualizer;
						dataVisualizer->factory = this;
						dataVisualizer->font = font;
						dataVisualizer->styleProvider = styleProvider;
						dataVisualizer->viewModelContext = viewModelContext;
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

					Ptr<IDataVisualizer> CreateVisualizer(const FontProperties& font, GuiListViewBase::IStyleProvider* styleProvider, const description::Value& viewModelContext)override
					{
						Ptr<IDataVisualizer> decoratedDataVisualizer = decoratedFactory->CreateVisualizer(font, styleProvider, viewModelContext);
						DataVisualizerBase* dataVisualizer = new TVisualizer(decoratedDataVisualizer);
						dataVisualizer->factory = this;
						dataVisualizer->font = font;
						dataVisualizer->styleProvider = styleProvider;
						dataVisualizer->viewModelContext = viewModelContext;
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
					IDataEditorCallback*								callback = nullptr;
					description::Value									viewModelContext;
					compositions::GuiBoundsComposition*					boundsComposition = nullptr;

					virtual compositions::GuiBoundsComposition*			CreateBoundsCompositionInternal()=0;
				public:
					/// <summary>Create the data editor.</summary>
					DataEditorBase();
					~DataEditorBase();

					IDataEditorFactory*									GetFactory()override;
					compositions::GuiBoundsComposition*					GetBoundsComposition()override;
					void												BeforeEditCell(IDataProvider* dataProvider, vint row, vint column)override;
					void												ReinstallEditor()override;
				};
				
				template<typename TEditor>
				class DataEditorFactory : public Object, public virtual IDataEditorFactory, public Description<DataEditorFactory<TEditor>>
				{
				public:
					Ptr<IDataEditor> CreateEditor(IDataEditorCallback* callback, const description::Value& viewModelContext)override
					{
						DataEditorBase* dataEditor = new TEditor;
						dataEditor->factory = this;
						dataEditor->callback = callback;
						dataEditor->viewModelContext = viewModelContext;
						return dataEditor;
					}
				};

/***********************************************************************
Visualizer Extensions
***********************************************************************/

				/// <summary>Data visualizer that displays an image and a text. Use ListViewMainColumnDataVisualizer::Factory as the factory class.</summary>
				class ListViewMainColumnDataVisualizer : public DataVisualizerBase
				{
				public:
					typedef DataVisualizerFactory<ListViewMainColumnDataVisualizer>			Factory;
				protected:
					elements::GuiImageFrameElement*						image;
					elements::GuiSolidLabelElement*						text;

					compositions::GuiBoundsComposition*					CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)override;
				public:
					/// <summary>Create the data visualizer.</summary>
					ListViewMainColumnDataVisualizer();

					void												BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)override;

					/// <summary>Get the internal text element.</summary>
					/// <returns>The text element.</returns>
					elements::GuiSolidLabelElement*						GetTextElement();
				};
				
				/// <summary>Data visualizer that displays a text. Use ListViewSubColumnDataVisualizer::Factory as the factory class.</summary>
				class ListViewSubColumnDataVisualizer : public DataVisualizerBase
				{
				public:
					typedef DataVisualizerFactory<ListViewSubColumnDataVisualizer>			Factory;
				protected:
					elements::GuiSolidLabelElement*						text;

					compositions::GuiBoundsComposition*					CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)override;
				public:
					/// <summary>Create the data visualizer.</summary>
					ListViewSubColumnDataVisualizer();

					void												BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)override;

					/// <summary>Get the internal text element.</summary>
					/// <returns>The text element.</returns>
					elements::GuiSolidLabelElement*						GetTextElement();
				};

				/// <summary>Data visualizer that displays a hyperlink. Use HyperlinkDataVisualizer::Factory as the factory class.</summary>
				class HyperlinkDataVisualizer : public ListViewSubColumnDataVisualizer
				{
				public:
					typedef DataVisualizerFactory<HyperlinkDataVisualizer>					Factory;
				protected:

					void												label_MouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												label_MouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					compositions::GuiBoundsComposition*					CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)override;
				public:
					HyperlinkDataVisualizer();

					void												BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)override;
				};

				/// <summary>Data visualizer that displays am image. Use ImageDataVisualizer::Factory as the factory class.</summary>
				class ImageDataVisualizer : public DataVisualizerBase
				{
				public:
					typedef DataVisualizerFactory<ImageDataVisualizer>						Factory;
				protected:
					elements::GuiImageFrameElement*						image;

					compositions::GuiBoundsComposition*					CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)override;
				public:
					/// <summary>Create the data visualizer.</summary>
					ImageDataVisualizer();

					void												BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)override;

					/// <summary>Get the internal image element.</summary>
					/// <returns>The image element.</returns>
					elements::GuiImageFrameElement*						GetImageElement();
				};
				
				/// <summary>Data visualizer that display a cell border that around another data visualizer. Use CellBorderDataVisualizer::Factory as the factory class.</summary>
				class CellBorderDataVisualizer : public DataVisualizerBase
				{
				public:
					typedef DataDecoratableVisualizerFactory<CellBorderDataVisualizer>		Factory;
				protected:

					compositions::GuiBoundsComposition*					CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)override;
				public:
					/// <summary>Create the data visualizer.</summary>
					/// <param name="decoratedDataVisualizer">The decorated data visualizer.</param>
					CellBorderDataVisualizer(Ptr<IDataVisualizer> decoratedDataVisualizer);
				};

				/// <summary>Data visualizer that display two icons (both optional) that beside another data visualizer. Use NotifyIconDataVisualizer::Factory as the factory class.</summary>
				class NotifyIconDataVisualizer : public DataVisualizerBase
				{
				public:
					typedef DataDecoratableVisualizerFactory<NotifyIconDataVisualizer>		Factory;
				protected:
					elements::GuiImageFrameElement*						leftImage;
					elements::GuiImageFrameElement*						rightImage;

					compositions::GuiBoundsComposition*					CreateBoundsCompositionInternal(compositions::GuiBoundsComposition* decoratedComposition)override;
				public:
					/// <summary>Create the data visualizer.</summary>
					/// <param name="decoratedDataVisualizer">The decorated data visualizer.</param>
					NotifyIconDataVisualizer(Ptr<IDataVisualizer> decoratedDataVisualizer);

					void												BeforeVisualizeCell(IDataProvider* dataProvider, vint row, vint column)override;

					/// <summary>Get the internal left image element.</summary>
					/// <returns>The image element.</returns>
					elements::GuiImageFrameElement*						GetLeftImageElement();
					/// <summary>Get the internal right image element.</summary>
					/// <returns>The image element.</returns>
					elements::GuiImageFrameElement*						GetRightImageElement();
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
					GuiSinglelineTextBox*								textBox;

					void												OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					compositions::GuiBoundsComposition*					CreateBoundsCompositionInternal()override;
				public:
					/// <summary>Create the data editor.</summary>
					TextBoxDataEditor();

					void												BeforeEditCell(IDataProvider* dataProvider, vint row, vint column)override;

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
					GuiComboBoxListControl*								comboBox;
					GuiTextList*										textList;
					
					void												OnSelectedIndexChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					compositions::GuiBoundsComposition*					CreateBoundsCompositionInternal()override;
				public:
					/// <summary>Create the data editor.</summary>
					TextComboBoxDataEditor();

					void												BeforeEditCell(IDataProvider* dataProvider, vint row, vint column)override;

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
					GuiDateComboBox*									comboBox;
					
					void												OnSelectedDateChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					compositions::GuiBoundsComposition*					CreateBoundsCompositionInternal()override;
				public:
					/// <summary>Create the data editor.</summary>
					DateComboBoxDataEditor();

					void												BeforeEditCell(IDataProvider* dataProvider, vint row, vint column)override;

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