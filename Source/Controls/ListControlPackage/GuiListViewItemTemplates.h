/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUILISTVIEWITEMTEMPLATES
#define VCZH_PRESENTATION_CONTROLS_GUILISTVIEWITEMTEMPLATES

#include "GuiListViewControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			namespace list
			{
				class DefaultListViewItemTemplate : public PredefinedListItemTemplate<templates::GuiListItemTemplate>
				{
				public:
					DefaultListViewItemTemplate();
					~DefaultListViewItemTemplate();
				};

				class BigIconListViewItemTemplate : public DefaultListViewItemTemplate
				{
				protected:
					elements::GuiImageFrameElement*			image = nullptr;
					elements::GuiSolidLabelElement*			text = nullptr;

					void									OnInitialize()override;
					void									OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					BigIconListViewItemTemplate();
					~BigIconListViewItemTemplate();
				};

				class SmallIconListViewItemTemplate : public DefaultListViewItemTemplate
				{
				protected:
					elements::GuiImageFrameElement*			image = nullptr;
					elements::GuiSolidLabelElement*			text = nullptr;

					void									OnInitialize()override;
					void									OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					SmallIconListViewItemTemplate();
					~SmallIconListViewItemTemplate();
				};

				class ListListViewItemTemplate : public DefaultListViewItemTemplate
				{
				protected:
					elements::GuiImageFrameElement*			image = nullptr;
					elements::GuiSolidLabelElement*			text = nullptr;

					void									OnInitialize()override;
					void									OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					ListListViewItemTemplate();
					~ListListViewItemTemplate();
				};

				class TileListViewItemTemplate : public DefaultListViewItemTemplate
				{
					typedef collections::Array<elements::GuiSolidLabelElement*>		DataTextElementArray;
				protected:
					elements::GuiImageFrameElement*			image = nullptr;
					elements::GuiSolidLabelElement*			text = nullptr;
					compositions::GuiTableComposition*		textTable = nullptr;
					DataTextElementArray					dataTexts;

					elements::GuiSolidLabelElement*			CreateTextElement(vint textRow);
					void									ResetTextTable(vint textRows);
					void									OnInitialize()override;
					void									OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					TileListViewItemTemplate();
					~TileListViewItemTemplate();
				};

				class InformationListViewItemTemplate : public DefaultListViewItemTemplate
				{
					typedef collections::Array<elements::GuiSolidLabelElement*>		DataTextElementArray;
				protected:
					elements::GuiImageFrameElement*			image = nullptr;
					elements::GuiSolidLabelElement*			text = nullptr;
					compositions::GuiTableComposition*		textTable = nullptr;
					DataTextElementArray					columnTexts;
					DataTextElementArray					dataTexts;
					elements::GuiSolidBackgroundElement*	bottomLine = nullptr;
					compositions::GuiBoundsComposition*		bottomLineComposition = nullptr;

					void									OnInitialize()override;
					void									OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					InformationListViewItemTemplate();
					~InformationListViewItemTemplate();
				};

				class DetailListViewItemTemplate
					: public DefaultListViewItemTemplate
					, public virtual ListViewColumnItemArranger::IColumnItemViewCallback
				{
					typedef collections::Array<elements::GuiSolidLabelElement*>		SubItemList;
					typedef ListViewColumnItemArranger::IColumnItemView				IColumnItemView;
				protected:
					IColumnItemView*						columnItemView = nullptr;
					elements::GuiImageFrameElement*			image = nullptr;
					elements::GuiSolidLabelElement*			text = nullptr;
					compositions::GuiTableComposition*		textTable = nullptr;
					SubItemList								subItems;

					void									OnInitialize()override;
					void									OnColumnRebuilt()override;
					void									OnColumnChanged(bool needToRefreshItems)override;
					void									OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					DetailListViewItemTemplate();
					~DetailListViewItemTemplate();

					void									UpdateSubItemSize();
				};
			}
		}
	}
}

#endif