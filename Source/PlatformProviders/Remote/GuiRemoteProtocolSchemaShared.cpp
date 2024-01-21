#include "GuiRemoteProtocolSchemaShared.h"

namespace vl::presentation::remoteprotocol
{
	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<bool>(const bool& value)
	{
		auto node = Ptr(new glr::json::JsonLiteral);
		node->value = value ? glr::json::JsonLiteralValue::True : glr::json::JsonLiteralValue::False;
		return node;
	}

	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<vint>(const vint& value)
	{
		auto node = Ptr(new glr::json::JsonNumber);
		reflection::description::TypedValueSerializerProvider<vint>::Serialize(value, node->content.value);
		return node;
	}

	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<float>(const float& value)
	{
		auto node = Ptr(new glr::json::JsonNumber);
		reflection::description::TypedValueSerializerProvider<float>::Serialize(value, node->content.value);
		return node;
	}

	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<double>(const double& value)
	{
		auto node = Ptr(new glr::json::JsonNumber);
		reflection::description::TypedValueSerializerProvider<double>::Serialize(value, node->content.value);
		return node;
	}

	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<WString>(const WString& value)
	{
		auto node = Ptr(new glr::json::JsonString);
		node->content.value = value;
		return node;
	}

	template<> void ConvertJsonToCustomType<bool>(Ptr<glr::json::JsonNode> node, bool& value)
	{
	#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType<bool>(Ptr<JsonNode>, bool&)#"
		auto jsonNode = node.Cast<glr::json::JsonLiteral>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
		switch (jsonNode->value)
		{
		case glr::json::JsonLiteralValue::True: value = true; break;
		case glr::json::JsonLiteralValue::False: value = false; break;
		default: CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Unsupported json literal.");
		}
	#undef ERROR_MESSAGE_PREFIX
	}

	template<> void ConvertJsonToCustomType<vint>(Ptr<glr::json::JsonNode> node, vint& value)
	{
	#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType<vint>(Ptr<JsonNode>, vint&)#"
		auto jsonNode = node.Cast<glr::json::JsonNumber>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
		CHECK_ERROR(reflection::description::TypedValueSerializerProvider<vint>::Deserialize(jsonNode->content.value, value), ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
	#undef ERROR_MESSAGE_PREFIX
	}

	template<> void ConvertJsonToCustomType<float>(Ptr<glr::json::JsonNode> node, float& value)
	{
	#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType<float>(Ptr<JsonNode>, float&)#"
		auto jsonNode = node.Cast<glr::json::JsonNumber>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
		CHECK_ERROR(reflection::description::TypedValueSerializerProvider<float>::Deserialize(jsonNode->content.value, value), ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
	#undef ERROR_MESSAGE_PREFIX
	}

	template<> void ConvertJsonToCustomType<double>(Ptr<glr::json::JsonNode> node, double& value)
	{
	#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType<double>(Ptr<JsonNode>, double&)#"
		auto jsonNode = node.Cast<glr::json::JsonNumber>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
		CHECK_ERROR(reflection::description::TypedValueSerializerProvider<double>::Deserialize(jsonNode->content.value, value), ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
	#undef ERROR_MESSAGE_PREFIX
	}

	template<> void ConvertJsonToCustomType<WString>(Ptr<glr::json::JsonNode> node, WString& value)
	{
	#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType<WString>(Ptr<JsonNode>, WString&)#"
		auto jsonNode = node.Cast<glr::json::JsonString>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
		value = jsonNode->content.value;
	#undef ERROR_MESSAGE_PREFIX
	}
}
