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
#include "../../GraphicsElement/GuiGraphicsElement.h"

namespace vl::presentation::remoteprotocol
{
	template<typename T>
	struct JsonNameHelper;

	template<typename TKey, typename TValue, TKey TValue::* Field>
	struct ArrayMap
	{
		using KK = typename KeyType<TKey>::Type;
		collections::Dictionary<TKey, TValue>			map;

		auto&& Keys() const								{ return map.Keys(); }
		auto&& Values() const							{ return map.Values(); }
		vint Count() const								{ return map.Count(); }
		const TValue& Get(const KK& key) const			{ return map.Get(key); }
		const TValue& operator[](const KK& key) const	{ return map[key]; }

		bool Add(const TValue& value)					{ return map.Add(value.*Field, value); }
		bool Remove(const KK& key)						{ return map.Remove(key); }
		bool Clear()									{ return map.Clear(); }
	};

	template<typename T>
	struct JsonHelper
	{
		static Ptr<glr::json::JsonNode> ToJson(const T& value);
		static void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, T& value);
	};

	template<typename T>
	Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const T& value)
	{
		return JsonHelper<T>::ToJson(value);
	}

	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<bool>(const bool& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<vint>(const vint& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<float>(const float& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<double>(const double& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<WString>(const WString& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<wchar_t>(const wchar_t& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<VKEY>(const VKEY& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<Color>(const Color& value);
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<Ptr<stream::MemoryStream>>(const Ptr<stream::MemoryStream>& value);

	template<typename T>
	void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, T& value)
	{
		return JsonHelper<T>::FromJson(node, value);
	}

	template<> void ConvertJsonToCustomType<bool>(Ptr<glr::json::JsonNode> node, bool& value);
	template<> void ConvertJsonToCustomType<vint>(Ptr<glr::json::JsonNode> node, vint& value);
	template<> void ConvertJsonToCustomType<float>(Ptr<glr::json::JsonNode> node, float& value);
	template<> void ConvertJsonToCustomType<double>(Ptr<glr::json::JsonNode> node, double& value);
	template<> void ConvertJsonToCustomType<WString>(Ptr<glr::json::JsonNode> node, WString& value);
	template<> void ConvertJsonToCustomType<wchar_t>(Ptr<glr::json::JsonNode> node, wchar_t& value);
	template<> void ConvertJsonToCustomType<VKEY>(Ptr<glr::json::JsonNode> node, VKEY& value);
	template<> void ConvertJsonToCustomType<Color>(Ptr<glr::json::JsonNode> node, Color& value);
	template<> void ConvertJsonToCustomType<Ptr<stream::MemoryStream>>(Ptr<glr::json::JsonNode> node, Ptr<stream::MemoryStream>& value);

	template<typename T>
	void ConvertCustomTypeToJsonField(Ptr<glr::json::JsonObject> node, const wchar_t* name, const T& value)
	{
		auto field = Ptr(new glr::json::JsonObjectField);
		field->name.value = WString::Unmanaged(name);
		field->value = ConvertCustomTypeToJson(value);
		node->fields.Add(field);
	}

	template<typename T>
	struct JsonHelper<Nullable<T>>
	{
		static Ptr<glr::json::JsonNode> ToJson(const Nullable<T>& value)
		{
			if (!value)
			{
				auto node = Ptr(new glr::json::JsonLiteral);
				node->value = glr::json::JsonLiteralValue::Null;
				return node;
			}
			else
			{
				return ConvertCustomTypeToJson(value.Value());
			}
		}

		static void FromJson(Ptr<glr::json::JsonNode> node, Nullable<T>& value)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::ConvertJsonToCustomType<T>(Ptr<JsonNode>, Ptr<List<T>>&)#"
			if (auto jsonLiteral = node.Cast<glr::json::JsonLiteral>())
			{
				if (jsonLiteral->value == glr::json::JsonLiteralValue::Null)
				{
					value.Reset();
					return;
				}
				else
				{
					CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
				}
			}
			else
			{
				T item;
				ConvertJsonToCustomType(node, item);
				value = item;
			}
#undef ERROR_MESSAGE_PREFIX
		}
	};

	template<typename T>
	struct JsonHelper<Ptr<T>>
	{
		static Ptr<glr::json::JsonNode> ToJson(const Ptr<T>& value)
		{
			if (!value)
			{
				auto node = Ptr(new glr::json::JsonLiteral);
				node->value = glr::json::JsonLiteralValue::Null;
				return node;
			}
			else
			{
				return ConvertCustomTypeToJson(*value.Obj());
			}
		}

		static void FromJson(Ptr<glr::json::JsonNode> node, Ptr<T>& value)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::ConvertJsonToCustomType<T>(Ptr<JsonNode>, Ptr<T>&)#"
			if (auto jsonLiteral = node.Cast<glr::json::JsonLiteral>())
			{
				if (jsonLiteral->value == glr::json::JsonLiteralValue::Null)
				{
					value = nullptr;
					return;
				}
				else
				{
					CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
				}
			}
			else
			{
				value = Ptr(new T);
				ConvertJsonToCustomType(node, *value.Obj());
			}
#undef ERROR_MESSAGE_PREFIX
		}
	};

	template<typename T>
	struct JsonHelper<collections::List<T>>
	{
		static Ptr<glr::json::JsonNode> ToJson(const collections::List<T>& value)
		{
			auto node = Ptr(new glr::json::JsonArray);
			for (auto&& item : value)
			{
				node->items.Add(ConvertCustomTypeToJson(item));
			}
			return node;
		}

		static void FromJson(Ptr<glr::json::JsonNode> node, collections::List<T>& value)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::ConvertJsonToCustomType<T>(Ptr<JsonNode>, List<T>&)#"
			if (auto jsonArray = node.Cast<glr::json::JsonArray>())
			{
				for (auto jsonItem : jsonArray->items)
				{
					T item;
					ConvertJsonToCustomType(jsonItem, item);
					value.Add(std::move(item));
				}
				return;
			}
			CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
#undef ERROR_MESSAGE_PREFIX
		}
	};

	template<typename TKey, typename TValue, TKey TValue::* Field>
	struct JsonHelper<ArrayMap<TKey, TValue, Field>>
	{
		static Ptr<glr::json::JsonNode> ToJson(const ArrayMap<TKey, TValue, Field>& value)
		{
			auto&& values = const_cast<collections::List<TValue>&>(value.map.Values());
			return ConvertCustomTypeToJson(values);
		}

		static void FromJson(Ptr<glr::json::JsonNode> node, ArrayMap<TKey, TValue, Field>& value)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::ConvertJsonToCustomType<T>(Ptr<JsonNode>, ArrayMap<TKey, TValue, Field>&)#"
			CHECK_FAIL(L"Not Implemented!");
#undef ERROR_MESSAGE_PREFIX
		}
	};

	template<typename TKey, typename TValue>
	struct JsonHelper<collections::Dictionary<TKey, TValue>>
	{
		static Ptr<glr::json::JsonNode> ToJson(const collections::Dictionary<TKey, TValue>& value)
		{
			auto node = Ptr(new glr::json::JsonArray);
			for (auto [key, value] : value)
			{
				auto pairNode = Ptr(new glr::json::JsonArray);
				pairNode->items.Add(ConvertCustomTypeToJson(key));
				pairNode->items.Add(ConvertCustomTypeToJson(value));
				node->items.Add(pairNode);
			}
			return node;
		}

		static void FromJson(Ptr<glr::json::JsonNode> node, collections::Dictionary<TKey, TValue>& value)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::ConvertJsonToCustomType<T>(Ptr<JsonNode>, Dictionary<TKey, TValue>&)#"
			CHECK_FAIL(L"Not Implemented!");
#undef ERROR_MESSAGE_PREFIX
		}
	};

	template<typename ...Ts>
	struct JsonHelper<Variant<Ts...>>
	{
		static Ptr<glr::json::JsonNode> ToJson(const Variant<Ts...>& value)
		{
			auto node = Ptr(new glr::json::JsonArray);
			value.Apply([&node]<typename T>(const T& element)
			{
				node->items.Add(ConvertCustomTypeToJson(WString::Unmanaged(JsonNameHelper<T>::Name)));
				node->items.Add(ConvertCustomTypeToJson(element));
			});
			return node;
		}

		static void FromJson(Ptr<glr::json::JsonNode> node, Variant<Ts...>& value)
		{
#define ERROR_MESSAGE_PREFIX L"vl::presentation::remoteprotocol::ConvertJsonToCustomType<T>(Ptr<JsonNode>, Variant<Ts...>&)#"
			CHECK_FAIL(L"Not Implemented!");
#undef ERROR_MESSAGE_PREFIX
		}
	};
}

#endif
