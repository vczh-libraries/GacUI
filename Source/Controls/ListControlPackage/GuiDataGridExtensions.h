/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUIDATAEXTENSIONS
#define VCZH_PRESENTATION_CONTROLS_GUIDATAEXTENSIONS

#include "GuiDataGridInterfaces.h"

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

				class DataVisualizerFactory;
				class DataEditorFactory;
				
				/// <summary>Base class for all data visualizers.</summary>
				class DataVisualizerBase : public Object, public virtual IDataVisualizer
				{
					friend class DataVisualizerFactory;
					using ItemTemplate = templates::GuiGridVisualizerTemplate;
				protected:
					DataVisualizerFactory*								factory = nullptr;
					IDataGridContext*									dataGridContext = nullptr;
					templates::GuiGridVisualizerTemplate*				visualizerTemplate = nullptr;

				public:
					/// <summary>Create the data visualizer.</summary>
					DataVisualizerBase();
					~DataVisualizerBase();

					IDataVisualizerFactory*								GetFactory()override;
					templates::GuiGridVisualizerTemplate*				GetTemplate()override;
					void												NotifyDeletedTemplate()override;
					void												BeforeVisualizeCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)override;
					void												SetSelected(bool value)override;
				};
				
				class DataVisualizerFactory : public Object, public virtual IDataVisualizerFactory, public Description<DataVisualizerFactory>
				{
					using ItemTemplate = templates::GuiGridVisualizerTemplate;
				protected:
					TemplateProperty<ItemTemplate>						templateFactory;
					Ptr<DataVisualizerFactory>							decoratedFactory;

					ItemTemplate*										CreateItemTemplate(controls::list::IDataGridContext* dataGridContext);
				public:
					DataVisualizerFactory(TemplateProperty<ItemTemplate> _templateFactory, Ptr<DataVisualizerFactory> _decoratedFactory = nullptr);
					~DataVisualizerFactory();

					Ptr<IDataVisualizer>								CreateVisualizer(IDataGridContext* dataGridContext)override;
				};
				
				/// <summary>Base class for all data editors.</summary>
				class DataEditorBase : public Object, public virtual IDataEditor
				{
					friend class DataEditorFactory;
					using ItemTemplate = templates::GuiGridEditorTemplate;
				protected:
					IDataEditorFactory*									factory = nullptr;
					IDataGridContext*									dataGridContext = nullptr;
					templates::GuiGridEditorTemplate*					editorTemplate = nullptr;

					void												OnCellValueChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					/// <summary>Create the data editor.</summary>
					DataEditorBase();
					~DataEditorBase();

					IDataEditorFactory*									GetFactory()override;
					templates::GuiGridEditorTemplate*					GetTemplate()override;
					void												NotifyDeletedTemplate()override;
					void												BeforeEditCell(GuiListControl::IItemProvider* itemProvider, vint row, vint column)override;
					bool												GetCellValueSaved()override;
				};
				
				class DataEditorFactory : public Object, public virtual IDataEditorFactory, public Description<DataEditorFactory>
				{
					using ItemTemplate = templates::GuiGridEditorTemplate;
				protected:
					TemplateProperty<ItemTemplate>						templateFactory;

				public:
					DataEditorFactory(TemplateProperty<ItemTemplate> _templateFactory);
					~DataEditorFactory();

					Ptr<IDataEditor>									CreateEditor(IDataGridContext* dataGridContext)override;
				};

/***********************************************************************
Visualizer Extensions
***********************************************************************/

				class MainColumnVisualizerTemplate : public templates::GuiGridVisualizerTemplate, public Description<MainColumnVisualizerTemplate>
				{
				protected:
					elements::GuiImageFrameElement*						image = nullptr;
					elements::GuiSolidLabelElement*						text = nullptr;

					void												OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnSmallImageChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					MainColumnVisualizerTemplate();
					~MainColumnVisualizerTemplate();
				};

				class SubColumnVisualizerTemplate : public templates::GuiGridVisualizerTemplate, public Description<SubColumnVisualizerTemplate>
				{
				protected:
					elements::GuiSolidLabelElement*						text = nullptr;

					void												OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												Initialize(bool fixTextColor);

					SubColumnVisualizerTemplate(bool fixTextColor);
				public:
					SubColumnVisualizerTemplate();
					~SubColumnVisualizerTemplate();
				};

				class HyperlinkVisualizerTemplate : public SubColumnVisualizerTemplate, public Description<HyperlinkVisualizerTemplate>
				{
				protected:
					void												label_MouseEnter(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void												label_MouseLeave(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

				public:
					HyperlinkVisualizerTemplate();
					~HyperlinkVisualizerTemplate();
				};

				class FocusRectangleVisualizerTemplate : public templates::GuiGridVisualizerTemplate, public Description<FocusRectangleVisualizerTemplate>
				{
				protected:
					compositions::GuiBoundsComposition*					focusComposition = nullptr;

					void												OnSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

				public:
					FocusRectangleVisualizerTemplate();
					~FocusRectangleVisualizerTemplate();
				};

				class CellBorderVisualizerTemplate : public templates::GuiGridVisualizerTemplate, public Description<CellBorderVisualizerTemplate>
				{
				protected:
					elements::GuiSolidBorderElement*					border1 = nullptr;
					elements::GuiSolidBorderElement*					border2 = nullptr;

					void												OnItemSeparatorColorChanged(GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);

				public:
					CellBorderVisualizerTemplate();
					~CellBorderVisualizerTemplate();
				};
			}
		}
	}
}

#endif