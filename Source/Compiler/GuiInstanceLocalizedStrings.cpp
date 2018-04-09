#include "GuiInstanceLocalizedStrings.h"
#include "GuiInstanceLoader.h"

namespace vl
{
	namespace presentation
	{
		using namespace collections;
		using namespace parsing::xml;

/***********************************************************************
GuiInstanceLocalizedStrings
***********************************************************************/

		Ptr<GuiInstanceLocalizedStrings> GuiInstanceLocalizedStrings::LoadFromXml(Ptr<GuiResourceItem> resource, Ptr<parsing::xml::XmlDocument> xml, GuiResourceError::List& errors)
		{
			auto ls = MakePtr<GuiInstanceLocalizedStrings>();

			if (xml->rootElement->name.value!=L"LocalizedStrings")
			{
				errors.Add(GuiResourceError({ { resource },xml->rootElement->codeRange.start }, L"The root element of localized strings should be \"LocalizedStrings\"."));
				return nullptr;
			}
			ls->tagPosition = { {resource},xml->rootElement->name.codeRange.start };

			auto attDefaultLocale = XmlGetAttribute(xml->rootElement, L"DefaultLocale");
			if (!attDefaultLocale)
			{
				errors.Add(GuiResourceError({ { resource },xml->rootElement->codeRange.start }, L"Missing attribute \"DefaultLocale\" in \"LocalizedStrings\"."));
				return nullptr;
			}
			ls->defaultLocale = attDefaultLocale->value.value;

			SortedList<WString> existingLocales;
			FOREACH(Ptr<XmlElement>, xmlStrings, XmlGetElements(xml->rootElement))
			{
				if (xmlStrings->name.value != L"Strings")
				{
					errors.Add(GuiResourceError({ { resource },xmlStrings->codeRange.start }, L"Unknown element \"" + xmlStrings->name.value + L"\", it should be \"Strings\"."));
					continue;
				}

				auto attLocales = XmlGetAttribute(xmlStrings, L"Locales");
				if (!attLocales)
				{
					errors.Add(GuiResourceError({ { resource },xmlStrings->codeRange.start }, L"Missing attribute \"Locales\" in \"Strings\"."));
				}
				else
				{
					auto strings = MakePtr<GuiInstanceLocalizedStrings::Strings>();
					ls->strings.Add(strings);
					ls->tagPosition = { { resource },xmlStrings->name.codeRange.start };
					SplitBySemicolon(attLocales->value.value, strings->locales);

					FOREACH(WString, locale, strings->locales)
					{
						if (!existingLocales.Contains(locale))
						{
							existingLocales.Add(locale);
						}
						else
						{
							errors.Add(GuiResourceError({ { resource },attLocales->codeRange.start }, L"Locale \"" + locale + L"\" already exists."));
						}
					}

					FOREACH(Ptr<XmlElement>, xmlString, XmlGetElements(xmlStrings))
					{
						if (xmlString->name.value != L"String")
						{
							errors.Add(GuiResourceError({ { resource },xmlString->codeRange.start }, L"Unknown element \"" + xmlString->name.value + L"\", it should be \"String\"."));
							continue;
						}

						auto attName = XmlGetAttribute(xmlString, L"Name");
						auto attText = XmlGetAttribute(xmlString, L"Text");

						if (!attName)
						{
							errors.Add(GuiResourceError({ { resource },xmlString->codeRange.start }, L"Missing attribute \"Name\" in \"String\"."));
						}
						if (!attText)
						{
							errors.Add(GuiResourceError({ { resource },xmlString->codeRange.start }, L"Missing attribute \"Text\" in \"String\"."));
						}

						if (attName && attText)
						{
							if (strings->items.Keys().Contains(attName->value.value))
							{
								errors.Add(GuiResourceError({ { resource },xmlString->codeRange.start }, L"String \"" + attName->value.value + L"\" already exists."));
							}
							else
							{
								auto item = MakePtr<GuiInstanceLocalizedStrings::StringItem>();
								item->name = attName->value.value;
								item->text = attText->value.value;
								item->textPosition = { {resource},attText->value.codeRange.start };
								item->textPosition.column += 1;
								strings->items.Add(item->name, item);
							}
						}
					}
				}
			}

			if (!existingLocales.Contains(ls->defaultLocale))
			{
				errors.Add(GuiResourceError({ { resource },xml->rootElement->codeRange.start }, L"Strings for the default locale \"" + ls->defaultLocale + L"\" is not defined."));
			}

			return ls;
		}

		Ptr<parsing::xml::XmlElement> GuiInstanceLocalizedStrings::SaveToXml()
		{
			auto xml = MakePtr<XmlElement>();
			xml->name.value = L"LocalizedStrings";
			{
				auto att = MakePtr<XmlAttribute>();
				att->name.value = L"DefaultLocale";
				att->value.value = defaultLocale;
				xml->attributes.Add(att);
			}

			FOREACH(Ptr<GuiInstanceLocalizedStrings::Strings>, lss, strings)
			{
				auto xmlStrings = MakePtr<XmlElement>();
				xml->subNodes.Add(xmlStrings);
				xmlStrings->name.value = L"Strings";
				{
					auto att = MakePtr<XmlAttribute>();
					att->name.value = L"Strings";
					att->value.value = From(lss->locales).Aggregate(WString(L""), [](const WString& a, const WString& b)
					{
						return a == L"" ? b : a + L";" + b;
					});
					xmlStrings->attributes.Add(att);
				}

				FOREACH(Ptr<GuiInstanceLocalizedStrings::StringItem>, lssi, lss->items.Values())
				{
					auto xmlString = MakePtr<XmlElement>();
					xmlStrings->subNodes.Add(xmlString);
					{
						auto att = MakePtr<XmlAttribute>();
						att->name.value = L"Name";
						att->value.value = lssi->name;
						xmlString->attributes.Add(att);
					}
					{
						auto att = MakePtr<XmlAttribute>();
						att->name.value = L"Text";
						att->value.value = lssi->text;
						xmlString->attributes.Add(att);
					}
				}
			}

			return xml;
		}
	}
}