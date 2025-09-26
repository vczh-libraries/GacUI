/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITEXTLISTCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUITEXTLISTCONTROLS

#include "ItemProvider_ITextItemView.h"
#include "ItemTemplate_ITextItemView.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{

/***********************************************************************
GuiVirtualTextList
***********************************************************************/

			enum class TextListView
			{
				Text,
				Check,
				Radio,
				Unknown,
			};

			/// <summary>Text list control in virtual mode.</summary>
			class GuiVirtualTextList : public GuiSelectableListControl, public Description<GuiVirtualTextList>
			{
				GUI_SPECIFY_CONTROL_TEMPLATE_TYPE(TextListTemplate, GuiSelectableListControl)
			protected:
				TextListView											view = TextListView::Unknown;

				void													OnStyleInstalled(vint itemIndex, ItemStyle* style, bool refreshPropertiesOnly)override;
				void													OnItemTemplateChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				void													OnKeyDown(compositions::GuiGraphicsComposition* sender, compositions::GuiKeyEventArgs& arguments);
			public:
				/// <summary>Create a Text list control in virtual mode.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiVirtualTextList(theme::ThemeName themeName, list::IItemProvider* _itemProvider);
				~GuiVirtualTextList();

				/// <summary>Item checked changed event.</summary>
				compositions::GuiItemNotifyEvent						ItemChecked;

				/// <summary>Get the current view.</summary>
				/// <returns>The current view. After [M:vl.presentation.controls.GuiListControl.SetItemTemplate] is called, the current view is reset to Unknown.</returns>
				TextListView											GetView();
				/// <summary>Set the current view.</summary>
				/// <param name="_view">The current view.</param>
				void													SetView(TextListView _view);
			};

/***********************************************************************
GuiTextList
***********************************************************************/
			
			/// <summary>Text list control.</summary>
			class GuiTextList
				: public GuiVirtualTextList
				, protected virtual list::ITextItemProviderCallback
				, public Description<GuiTextList>
			{
			protected:
				list::TextItemProvider*									items;

				void													OnItemCheckedChanged(vint itemIndex) override;
			public:
				/// <summary>Create a Text list control.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				GuiTextList(theme::ThemeName themeName);
				~GuiTextList();

				/// <summary>Get all text items.</summary>
				/// <returns>All text items.</returns>
				list::TextItemProvider&									GetItems();

				/// <summary>Get the selected item.</summary>
				/// <returns>Returns the selected item. If there are multiple selected items, or there is no selected item, null will be returned.</returns>
				Ptr<list::TextItem>										GetSelectedItem();
			};
		}
	}
}

#endif