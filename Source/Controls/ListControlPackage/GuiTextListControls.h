/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_GUITEXTLISTCONTROLS
#define VCZH_PRESENTATION_CONTROLS_GUITEXTLISTCONTROLS

#include "GuiListControls.h"

namespace vl
{
	namespace presentation
	{
		namespace controls
		{
			class GuiVirtualTextList;
			class GuiTextList;

			namespace list
			{

/***********************************************************************
DefaultTextListItemTemplate
***********************************************************************/

				/// <summary>The required <see cref="GuiListControl::IItemProvider"/> view for <see cref="GuiVirtualTextList"/>.</summary>
				class ITextItemView : public virtual IDescriptable, public Description<ITextItemView>
				{
				public:
					/// <summary>The identifier for this view.</summary>
					static const wchar_t* const				Identifier;

					/// <summary>Get the check state of an item.</summary>
					/// <returns>The check state of an item.</returns>
					/// <param name="itemIndex">The index of an item.</param>
					virtual bool							GetChecked(vint itemIndex) = 0;
					/// <summary>Set the check state of an item without invoving any UI action.</summary>
					/// <param name="itemIndex">The index of an item.</param>
					/// <param name="value">The new check state.</param>
					virtual void							SetChecked(vint itemIndex, bool value) = 0;
				};

				class DefaultTextListItemTemplate : public templates::GuiTextListItemTemplate
				{
				protected:
					using BulletStyle = templates::GuiControlTemplate;

					GuiSelectableButton*					bulletButton = nullptr;
					elements::GuiSolidLabelElement*			textElement = nullptr;
					bool									supressEdit = false;

					virtual TemplateProperty<BulletStyle>	CreateBulletStyle();
					void									OnInitialize()override;
					void									OnFontChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void									OnTextChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void									OnTextColorChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void									OnCheckedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
					void									OnBulletSelectedChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
				public:
					DefaultTextListItemTemplate();
					~DefaultTextListItemTemplate();
				};

				class DefaultCheckTextListItemTemplate : public DefaultTextListItemTemplate
				{
				protected:
					TemplateProperty<BulletStyle>			CreateBulletStyle()override;
				public:
				};

				class DefaultRadioTextListItemTemplate : public DefaultTextListItemTemplate
				{
				protected:
					TemplateProperty<BulletStyle>			CreateBulletStyle()override;
				public:
				};

/***********************************************************************
TextItemProvider
***********************************************************************/

				class TextItemProvider;

				/// <summary>Text item. This is the item data structure for [T:vl.presentation.controls.list.TextItemProvider].</summary>
				class TextItem : public Object, public Description<TextItem>
				{
					friend class TextItemProvider;
				protected:
					TextItemProvider*							owner;
					WString										text;
					bool										checked;

				public:
					/// <summary>Create an empty text item.</summary>
					TextItem();
					/// <summary>Create a text item with specified text and check state.</summary>
					/// <param name="_text">The text.</param>
					/// <param name="_checked">The check state.</param>
					TextItem(const WString& _text, bool _checked=false);
					~TextItem();

					bool										operator==(const TextItem& value)const;
					bool										operator!=(const TextItem& value)const;
					
					/// <summary>Get the text of this item.</summary>
					/// <returns>The text of this item.</returns>
					const WString&								GetText();
					/// <summary>Set the text of this item.</summary>
					/// <param name="value">The text of this item.</param>
					void										SetText(const WString& value);

					/// <summary>Get the check state of this item.</summary>
					/// <returns>The check state of this item.</returns>
					bool										GetChecked();
					/// <summary>Set the check state of this item.</summary>
					/// <param name="value">The check state of this item.</param>
					void										SetChecked(bool value);
				};

				/// <summary>Item provider for <see cref="GuiVirtualTextList"/> or <see cref="GuiSelectableListControl"/>.</summary>
				class TextItemProvider
					: public ListProvider<Ptr<TextItem>>
					, protected ITextItemView
					, public Description<TextItemProvider>
				{
					friend class TextItem;
					friend class vl::presentation::controls::GuiTextList;
				protected:
					GuiTextList*								listControl;

					void										AfterInsert(vint item, const Ptr<TextItem>& value)override;
					void										BeforeRemove(vint item, const Ptr<TextItem>& value)override;

					WString										GetTextValue(vint itemIndex)override;
					description::Value							GetBindingValue(vint itemIndex)override;
					bool										GetChecked(vint itemIndex)override;
					void										SetChecked(vint itemIndex, bool value)override;
				public:
					TextItemProvider();
					~TextItemProvider();

					IDescriptable*								RequestView(const WString& identifier)override;
				};
			}

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

				void													OnStyleInstalled(vint itemIndex, ItemStyle* style)override;
				void													OnItemTemplateChanged(compositions::GuiGraphicsComposition* sender, compositions::GuiEventArgs& arguments);
			public:
				/// <summary>Create a Text list control in virtual mode.</summary>
				/// <param name="themeName">The theme name for retriving a default control template.</param>
				/// <param name="_itemProvider">The item provider for this control.</param>
				GuiVirtualTextList(theme::ThemeName themeName, GuiListControl::IItemProvider* _itemProvider);
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
			class GuiTextList : public GuiVirtualTextList, public Description<GuiTextList>
			{
			protected:
				list::TextItemProvider*									items;
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