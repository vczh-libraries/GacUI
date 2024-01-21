/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
GacUI::Remote Window

Interfaces:
  IGuiRemoteProtocol

***********************************************************************/

#ifndef VCZH_PRESENTATION_GUIREMOTECONTROLLER_REMOTEPROTOCOLSCHEMASHARED
#define VCZH_PRESENTATION_GUIREMOTECONTROLLER_REMOTEPROTOCOLSCHEMASHARED

#include <VlppGlrParser.h>
#include "../../NativeWindow/GuiNativeWindow.h"

namespace vl::presentation::remoteprotocol
{
	template<typename T>
	Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const Ptr<collections::List<T>>& value)
	{
		if (!value)
		{
			auto node = Ptr(new glr::json::JsonLiteral);
			node->value = glr::json::JsonLiteralValue::Null;
			return node;
		}
		else
		{
			auto node = Ptr(new glr::json::JsonArray);
			for (auto&& item : *value.Obj())
			{
				node->items.Add(ConvertCustomTypeToJson(item));
			}
			return node;
		}
	}

	template<typename T>
	void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, Ptr<collections::List<T>>& value)
	{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::ConvertJsonToCustomType<T>(Ptr<JsonNode>, Ptr<List<T>>&)#"
		if (auto jsonLiteral = node.Cast<glr::json::JsonLiteral>())
		{
			if (jsonLiteral->value != glr::json::JsonLiteralValue::Null)
			{
				value = {};
				return;
			}
		}
		else if (auto jsonArray = node.Cast<glr::json::JsonArray>())
		{
			value = Ptr(new collections::List<T>);
			for (auto jsonItem : jsonArray->items)
			{
				T item;
				ConvertJsonToCustomType(jsonItem, item);
				value->Add(std::move(item));
			}
			return;
		}
		CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
#undef ERROR_MESSAGE_PREFIX
	}

	template<typename T>
	void ConvertCustomTypeToJsonField(Ptr<glr::json::JsonObject> node, const wchar_t* name, const T& value)
	{
		auto field = Ptr(new glr::json::JsonObjectField);
		field->name.value = WString::Unmanaged(name);
		field->value = ConvertCustomTypeToJson(value);
		node->fields.Add(field);
	}

	extern Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const bool& value);
	extern Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const vint& value);
	extern Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const float& value);
	extern Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const double& value);
	extern Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const WString& value);

	extern void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, bool& value);
	extern void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, vint& value);
	extern void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, float& value);
	extern void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, double& value);
	extern void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, WString& value);
}

#endif
