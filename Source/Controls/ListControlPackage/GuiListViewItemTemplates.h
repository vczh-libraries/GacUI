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
				class DefaultListViewItemTemplate : public templates::GuiListItemTemplate
				{
				protected:
					GuiSelectableButton*					backgroundButton = nullptr;

					void									OnInitialize()override;
					void									OnSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					DefaultListViewItemTemplate();
					~DefaultListViewItemTemplate();
				};

				/// <summary>Big icon content provider.</summary>
				class ListViewBigIconContentProvider : public Object, public virtual ListViewItemStyleProvider::IListViewItemContentProvider, public Description<ListViewBigIconContentProvider>
				{
				protected:
					class ItemContent : public Object, public virtual ListViewItemStyleProvider::IListViewItemContent
					{
					protected:
						compositions::GuiBoundsComposition*				contentComposition;
						elements::GuiImageFrameElement*					image;
						elements::GuiSolidLabelElement*					text;

					public:
						ItemContent(Size minIconSize, bool fitImage, const FontProperties& font);
						~ItemContent();

						compositions::GuiBoundsComposition*				GetContentComposition()override;
						compositions::GuiBoundsComposition*				GetBackgroundDecorator()override;
						void											Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)override;
						void											Uninstall()override;
					};

					Size												minIconSize;
					bool												fitImage;
				public:
					/// <summary>Create the content provider.</summary>
					/// <param name="_minIconSize">The icon size.</param>
					/// <param name="_fitImage">Set to true to extend the icon size fit the image if necessary.</param>
					ListViewBigIconContentProvider(Size _minIconSize=Size(32, 32), bool _fitImage=true);
					~ListViewBigIconContentProvider();

					compositions::IGuiAxis*								CreatePreferredAxis()override;
					GuiListControl::IItemArranger*						CreatePreferredArranger()override;
					ListViewItemStyleProvider::IListViewItemContent*	CreateItemContent(const FontProperties& font)override;
					void												AttachListControl(GuiListControl* value)override;
					void												DetachListControl()override;
				};
				
				/// <summary>Small icon content provider.</summary>
				class ListViewSmallIconContentProvider : public Object, public virtual ListViewItemStyleProvider::IListViewItemContentProvider, public Description<ListViewSmallIconContentProvider>
				{
				protected:
					class ItemContent : public Object, public virtual ListViewItemStyleProvider::IListViewItemContent
					{
					protected:
						compositions::GuiBoundsComposition*				contentComposition;
						elements::GuiImageFrameElement*					image;
						elements::GuiSolidLabelElement*					text;

					public:
						ItemContent(Size minIconSize, bool fitImage, const FontProperties& font);
						~ItemContent();

						compositions::GuiBoundsComposition*				GetContentComposition()override;
						compositions::GuiBoundsComposition*				GetBackgroundDecorator()override;
						void											Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)override;
						void											Uninstall()override;
					};

					Size												minIconSize;
					bool												fitImage;
				public:
					/// <summary>Create the content provider.</summary>
					/// <param name="_minIconSize">The icon size.</param>
					/// <param name="_fitImage">Set to true to extend the icon size fit the image if necessary.</param>
					ListViewSmallIconContentProvider(Size _minIconSize=Size(16, 16), bool _fitImage=true);
					~ListViewSmallIconContentProvider();
					
					compositions::IGuiAxis*								CreatePreferredAxis()override;
					GuiListControl::IItemArranger*						CreatePreferredArranger()override;
					ListViewItemStyleProvider::IListViewItemContent*	CreateItemContent(const FontProperties& font)override;
					void												AttachListControl(GuiListControl* value)override;
					void												DetachListControl()override;
				};
				
				/// <summary>List content provider.</summary>
				class ListViewListContentProvider : public Object, public virtual ListViewItemStyleProvider::IListViewItemContentProvider, public Description<ListViewListContentProvider>
				{
				protected:
					class ItemContent : public Object, public virtual ListViewItemStyleProvider::IListViewItemContent
					{
					protected:
						compositions::GuiBoundsComposition*				contentComposition;
						elements::GuiImageFrameElement*					image;
						elements::GuiSolidLabelElement*					text;

					public:
						ItemContent(Size minIconSize, bool fitImage, const FontProperties& font);
						~ItemContent();

						compositions::GuiBoundsComposition*				GetContentComposition()override;
						compositions::GuiBoundsComposition*				GetBackgroundDecorator()override;
						void											Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)override;
						void											Uninstall()override;
					};

					Size												minIconSize;
					bool												fitImage;
				public:
					/// <summary>Create the content provider.</summary>
					/// <param name="_minIconSize">The icon size.</param>
					/// <param name="_fitImage">Set to true to extend the icon size fit the image if necessary.</param>
					ListViewListContentProvider(Size _minIconSize=Size(16, 16), bool _fitImage=true);
					~ListViewListContentProvider();
					
					compositions::IGuiAxis*								CreatePreferredAxis()override;
					GuiListControl::IItemArranger*						CreatePreferredArranger()override;
					ListViewItemStyleProvider::IListViewItemContent*	CreateItemContent(const FontProperties& font)override;
					void												AttachListControl(GuiListControl* value)override;
					void												DetachListControl()override;
				};
				
				/// <summary>Tile content provider.</summary>
				class ListViewTileContentProvider : public Object, public virtual ListViewItemStyleProvider::IListViewItemContentProvider, public Description<ListViewTileContentProvider>
				{
				protected:
					class ItemContent : public Object, public virtual ListViewItemStyleProvider::IListViewItemContent
					{
						typedef collections::Array<elements::GuiSolidLabelElement*>		DataTextElementArray;
					protected:
						compositions::GuiBoundsComposition*				contentComposition;
						elements::GuiImageFrameElement*					image;
						elements::GuiSolidLabelElement*					text;
						compositions::GuiTableComposition*				textTable;
						DataTextElementArray							dataTexts;

						void											RemoveTextElement(vint textRow);
						elements::GuiSolidLabelElement*					CreateTextElement(vint textRow, const FontProperties& font);
						void											ResetTextTable(vint textRows);
					public:
						ItemContent(Size minIconSize, bool fitImage, const FontProperties& font);
						~ItemContent();

						compositions::GuiBoundsComposition*				GetContentComposition()override;
						compositions::GuiBoundsComposition*				GetBackgroundDecorator()override;
						void											Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)override;
						void											Uninstall()override;
					};

					Size												minIconSize;
					bool												fitImage;
				public:
					/// <summary>Create the content provider.</summary>
					/// <param name="_minIconSize">The icon size.</param>
					/// <param name="_fitImage">Set to true to extend the icon size fit the image if necessary.</param>
					ListViewTileContentProvider(Size _minIconSize=Size(32, 32), bool _fitImage=true);
					~ListViewTileContentProvider();
					
					compositions::IGuiAxis*								CreatePreferredAxis()override;
					GuiListControl::IItemArranger*						CreatePreferredArranger()override;
					ListViewItemStyleProvider::IListViewItemContent*	CreateItemContent(const FontProperties& font)override;
					void												AttachListControl(GuiListControl* value)override;
					void												DetachListControl()override;
				};
				
				/// <summary>View information content provider.</summary>
				class ListViewInformationContentProvider : public Object, public virtual ListViewItemStyleProvider::IListViewItemContentProvider, public Description<ListViewInformationContentProvider>
				{
				protected:
					class ItemContent : public Object, public virtual ListViewItemStyleProvider::IListViewItemContent
					{
						typedef collections::Array<elements::GuiSolidLabelElement*>		DataTextElementArray;
					protected:
						FontProperties									baselineFont;
						compositions::GuiBoundsComposition*				contentComposition;
						elements::GuiImageFrameElement*					image;
						elements::GuiSolidLabelElement*					text;
						compositions::GuiTableComposition*				textTable;
						DataTextElementArray							dataTexts;

						elements::GuiSolidBackgroundElement*			bottomLine;
						compositions::GuiBoundsComposition*				bottomLineComposition;

					public:
						ItemContent(Size minIconSize, bool fitImage, const FontProperties& font);
						~ItemContent();

						compositions::GuiBoundsComposition*				GetContentComposition()override;
						compositions::GuiBoundsComposition*				GetBackgroundDecorator()override;
						void											Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)override;
						void											Uninstall()override;
					};

					Size												minIconSize;
					bool												fitImage;
				public:
					/// <summary>Create the content provider.</summary>
					/// <param name="_minIconSize">The icon size.</param>
					/// <param name="_fitImage">Set to true to extend the icon size fit the image if necessary.</param>
					ListViewInformationContentProvider(Size _minIconSize=Size(32, 32), bool _fitImage=true);
					~ListViewInformationContentProvider();
					
					compositions::IGuiAxis*								CreatePreferredAxis()override;
					GuiListControl::IItemArranger*						CreatePreferredArranger()override;
					ListViewItemStyleProvider::IListViewItemContent*	CreateItemContent(const FontProperties& font)override;
					void												AttachListControl(GuiListControl* value)override;
					void												DetachListControl()override;
				};
				
				/// <summary>Detail content provider.</summary>
				class ListViewDetailContentProvider
					: public Object
					, public virtual ListViewItemStyleProvider::IListViewItemContentProvider
					, protected virtual ListViewColumnItemArranger::IColumnItemViewCallback
					, public Description<ListViewDetailContentProvider>
				{
				protected:
					class ItemContent : public Object, public virtual ListViewItemStyleProvider::IListViewItemContent
					{
						typedef collections::List<elements::GuiSolidLabelElement*>		SubItemList;
					protected:
						compositions::GuiBoundsComposition*				contentComposition;
						elements::GuiImageFrameElement*					image;
						elements::GuiSolidLabelElement*					text;
						compositions::GuiTableComposition*				textTable;
						SubItemList										subItems;

						GuiListControl::IItemProvider*					itemProvider;
						ListViewColumnItemArranger::IColumnItemView*	columnItemView;

					public:
						ItemContent(Size minIconSize, bool fitImage, const FontProperties& font, GuiListControl::IItemProvider* _itemProvider);
						~ItemContent();

						compositions::GuiBoundsComposition*				GetContentComposition()override;
						compositions::GuiBoundsComposition*				GetBackgroundDecorator()override;
						void											UpdateSubItemSize();
						void											Install(GuiListViewBase::IStyleProvider* styleProvider, ListViewItemStyleProvider::IListViewItemView* view, vint itemIndex)override;
						void											Uninstall()override;
					};

					Size												minIconSize;
					bool												fitImage;
					GuiListControl::IItemProvider*						itemProvider;
					ListViewColumnItemArranger::IColumnItemView*		columnItemView;
					ListViewItemStyleProvider*							listViewItemStyleProvider;

					void												OnColumnChanged()override;
				public:
					/// <summary>Create the content provider.</summary>
					/// <param name="_minIconSize">The icon size.</param>
					/// <param name="_fitImage">Set to true to extend the icon size fit the image if necessary.</param>
					ListViewDetailContentProvider(Size _minIconSize=Size(16, 16), bool _fitImage=true);
					~ListViewDetailContentProvider();
					
					compositions::IGuiAxis*								CreatePreferredAxis()override;
					GuiListControl::IItemArranger*						CreatePreferredArranger()override;
					ListViewItemStyleProvider::IListViewItemContent*	CreateItemContent(const FontProperties& font)override;
					void												AttachListControl(GuiListControl* value)override;
					void												DetachListControl()override;
				};
			}
		}
	}
}

#endif