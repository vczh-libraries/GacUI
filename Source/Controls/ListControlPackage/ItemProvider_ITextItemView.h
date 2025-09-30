/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Control System

Interfaces:
***********************************************************************/

#ifndef VCZH_PRESENTATION_CONTROLS_ITEMPROVIDER_ITEXTITEMVIEW
#define VCZH_PRESENTATION_CONTROLS_ITEMPROVIDER_ITEXTITEMVIEW

#include "DataSourceImpl_IItemProvider_ItemProviderBase.h"

namespace vl::presentation::controls::list
{
	class ITextItemProviderCallback : public virtual IDescriptable, public Description<ITextItemProviderCallback>
	{
	public:
		virtual void								OnItemCheckedChanged(vint itemIndex) = 0;
	};

/***********************************************************************
ITextItemView
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

/***********************************************************************
TextItem
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

		void										NotifyUpdate(bool raiseCheckEvent);
	public:
		/// <summary>Create an empty text item.</summary>
		TextItem();
		/// <summary>Create a text item with specified text and check state.</summary>
		/// <param name="_text">The text.</param>
		/// <param name="_checked">The check state.</param>
		TextItem(const WString& _text, bool _checked=false);
		~TextItem();

		std::strong_ordering operator<=>(const TextItem& value) const { return text <=> value.text; }
		bool operator==(const TextItem& value) const { return text == value.text; }
		
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

/***********************************************************************
TextItemProvider
***********************************************************************/

	/// <summary>Item provider for <see cref="GuiVirtualTextList"/> or <see cref="GuiSelectableListControl"/>.</summary>
	class TextItemProvider
		: public ListProvider<Ptr<TextItem>>
		, public virtual ITextItemView
		, public Description<TextItemProvider>
	{
		friend class TextItem;
	protected:
		ITextItemProviderCallback*					itemProviderCallback;

		void										BeforeInsert(vint item, const Ptr<TextItem>& value)override;
		void										AfterInsert(vint item, const Ptr<TextItem>& value)override;
		void										BeforeRemove(vint item, const Ptr<TextItem>& value)override;

	public:
		// ===================== list::ITextItemView =====================

		WString										GetTextValue(vint itemIndex)override;
		description::Value							GetBindingValue(vint itemIndex)override;
		bool										GetChecked(vint itemIndex)override;
		void										SetChecked(vint itemIndex, bool value)override;

	public:
		TextItemProvider(ITextItemProviderCallback* _itemProviderCallback);
		~TextItemProvider();

		IDescriptable*								RequestView(const WString& identifier)override;
	};
}

#endif