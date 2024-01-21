#include "GuiRemoteProtocolSchemaShared.h"

namespace vl::presentation::remoteprotocol
{
	Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const bool& value)
	{
		auto node = Ptr(new glr::json::JsonLiteral);
		node->value = value ? glr::json::JsonLiteralValue::True : glr::json::JsonLiteralValue::False;
		return node;
	}

	Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const vint& value)
	{
		auto node = Ptr(new glr::json::JsonNumber);
		reflection::description::TypedValueSerializerProvider<vint>::Serialize(value, node->content.value);
		return node;
	}

	Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const float& value)
	{
		auto node = Ptr(new glr::json::JsonNumber);
		reflection::description::TypedValueSerializerProvider<float>::Serialize(value, node->content.value);
		return node;
	}

	Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const double& value)
	{
		auto node = Ptr(new glr::json::JsonNumber);
		reflection::description::TypedValueSerializerProvider<double>::Serialize(value, node->content.value);
		return node;
	}

	Ptr<glr::json::JsonNode> ConvertCustomTypeToJson(const WString& value)
	{
		auto node = Ptr(new glr::json::JsonString);
		node->content.value = value;
		return node;
	}

	void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, bool& value)
	{
	#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType(Ptr<JsonNode>, bool&)#"
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

	void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, vint& value)
	{
	#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType(Ptr<JsonNode>, vint&)#"
		auto jsonNode = node.Cast<glr::json::JsonNumber>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
		CHECK_ERROR(reflection::description::TypedValueSerializerProvider<vint>::Deserialize(jsonNode->content.value, value), ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
	#undef ERROR_MESSAGE_PREFIX
	}

	void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, float& value)
	{
	#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType(Ptr<JsonNode>, float&)#"
		auto jsonNode = node.Cast<glr::json::JsonNumber>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
		CHECK_ERROR(reflection::description::TypedValueSerializerProvider<float>::Deserialize(jsonNode->content.value, value), ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
	#undef ERROR_MESSAGE_PREFIX
	}

	void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, double& value)
	{
	#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType(Ptr<JsonNode>, double&)#"
		auto jsonNode = node.Cast<glr::json::JsonNumber>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
		CHECK_ERROR(reflection::description::TypedValueSerializerProvider<double>::Deserialize(jsonNode->content.value, value), ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
	#undef ERROR_MESSAGE_PREFIX
	}

	void ConvertJsonToCustomType(Ptr<glr::json::JsonNode> node, WString& value)
	{
	#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType(Ptr<JsonNode>, WString&)#"
		auto jsonNode = node.Cast<glr::json::JsonString>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
		value = jsonNode->content.value;
	#undef ERROR_MESSAGE_PREFIX
	}
}
