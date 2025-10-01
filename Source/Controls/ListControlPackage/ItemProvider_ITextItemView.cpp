#include "ItemProvider_ITextItemView.h"

namespace vl::presentation::controls::list
{
	using namespace collections;
	using namespace reflection::description;

	const wchar_t* const ITextItemView::Identifier = L"vl::presentation::controls::list::ITextItemView";

/***********************************************************************
TextItem
***********************************************************************/

	void TextItem::NotifyUpdate(bool raiseCheckEvent)
	{
		if (owner)
		{
			vint index = owner->IndexOf(this);
			owner->InvokeOnItemModified(index, 1, 1, false);

			if (raiseCheckEvent)
			{
				owner->itemProviderCallback->OnItemCheckedChanged(index);
			}
		}
	}

	TextItem::TextItem()
		:owner(0)
		, checked(false)
	{
	}

	TextItem::TextItem(const WString& _text, bool _checked)
		:owner(0)
		, text(_text)
		, checked(_checked)
	{
	}

	TextItem::~TextItem()
	{
	}

	const WString& TextItem::GetText()
	{
		return text;
	}

	void TextItem::SetText(const WString& value)
	{
		if (text != value)
		{
			text = value;
			NotifyUpdate(false);
		}
	}

	bool TextItem::GetChecked()
	{
		return checked;
	}

	void TextItem::SetChecked(bool value)
	{
		if (checked != value)
		{
			checked = value;
			NotifyUpdate(true);
		}
	}

/***********************************************************************
TextItemProvider
***********************************************************************/

	void TextItemProvider::BeforeInsert(vint item, const Ptr<TextItem>& value)
	{
		// Check if this item is already in the provider
		if (value->owner)
		{
			throw ArgumentException(L"The TextItem is already belog to a TextItemProvider.", L"vl::presentation::controls::list::TextItemProvider::BeforeInsert", L"value");
		}
	}

	void TextItemProvider::AfterInsert(vint item, const Ptr<TextItem>& value)
	{
		ListProvider<Ptr<TextItem>>::AfterInsert(item, value);
		value->owner = this;
	}

	void TextItemProvider::BeforeRemove(vint item, const Ptr<TextItem>& value)
	{
		value->owner = 0;
		ListProvider<Ptr<TextItem>>::BeforeRemove(item, value);
	}

	WString TextItemProvider::GetTextValue(vint itemIndex)
	{
		return Get(itemIndex)->GetText();
	}

	description::Value TextItemProvider::GetBindingValue(vint itemIndex)
	{
		return Value::From(Get(itemIndex));
	}

	bool TextItemProvider::GetChecked(vint itemIndex)
	{
		return Get(itemIndex)->GetChecked();
	}

	void TextItemProvider::SetChecked(vint itemIndex, bool value)
	{
		return Get(itemIndex)->SetChecked(value);
	}

	TextItemProvider::TextItemProvider(ITextItemProviderCallback* _itemProviderCallback)
		:itemProviderCallback(_itemProviderCallback)
	{
	}

	TextItemProvider::~TextItemProvider()
	{
	}

	IDescriptable* TextItemProvider::RequestView(const WString& identifier)
	{
		if (identifier == ITextItemView::Identifier)
		{
			return (ITextItemView*)this;
		}
		else
		{
			return nullptr;
		}
	}
}