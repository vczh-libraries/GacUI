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

	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<wchar_t>(const wchar_t& value)
	{
		return ConvertCustomTypeToJson(WString::FromChar(value));
	}

	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<VKEY>(const VKEY& value)
	{
		return ConvertCustomTypeToJson((vint)value);
	}

	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<Color>(const Color& value)
	{
		return ConvertCustomTypeToJson(value.ToString());
	}

	template<> Ptr<glr::json::JsonNode> ConvertCustomTypeToJson<Ptr<stream::MemoryStream>>(const Ptr<stream::MemoryStream>& value)
	{
		if (!value)
		{
			auto node = Ptr(new glr::json::JsonLiteral);
			node->value = glr::json::JsonLiteralValue::Null;
			return node;
		}

		stream::MemoryStream base64WStringStream;
		{
			stream::UtfGeneralEncoder<wchar_t, char8_t> utf8ToWCharEncoder;
			stream::EncoderStream utf8ToWCharStream(base64WStringStream, utf8ToWCharEncoder);
			stream::Utf8Base64Encoder binaryToBase64Utf8Encoder;
			stream::EncoderStream binaryToBase64Utf8Stream(utf8ToWCharStream, binaryToBase64Utf8Encoder);
			value->SeekFromBegin(0);
			stream::CopyStream(*value.Obj(), binaryToBase64Utf8Stream);
		}
		{
			base64WStringStream.SeekFromBegin(0);
			stream::StreamReader reader(base64WStringStream);
			return ConvertCustomTypeToJson(reader.ReadToEnd());
		}
	}

	template<> void ConvertJsonToCustomType<bool>(Ptr<glr::json::JsonNode> node, bool& value)
	{
#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType<bool>(Ptr<JsonNode>, bool&)#"
		auto jsonNode = node.Cast<glr::json::JsonLiteral>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"JSON node does not match the expected type.");
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
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"JSON node does not match the expected type.");
		CHECK_ERROR(reflection::description::TypedValueSerializerProvider<vint>::Deserialize(jsonNode->content.value, value), ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
#undef ERROR_MESSAGE_PREFIX
	}

	template<> void ConvertJsonToCustomType<float>(Ptr<glr::json::JsonNode> node, float& value)
	{
#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType<float>(Ptr<JsonNode>, float&)#"
		auto jsonNode = node.Cast<glr::json::JsonNumber>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"JSON node does not match the expected type.");
		CHECK_ERROR(reflection::description::TypedValueSerializerProvider<float>::Deserialize(jsonNode->content.value, value), ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
#undef ERROR_MESSAGE_PREFIX
	}

	template<> void ConvertJsonToCustomType<double>(Ptr<glr::json::JsonNode> node, double& value)
	{
#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType<double>(Ptr<JsonNode>, double&)#"
		auto jsonNode = node.Cast<glr::json::JsonNumber>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"JSON node does not match the expected type.");
		CHECK_ERROR(reflection::description::TypedValueSerializerProvider<double>::Deserialize(jsonNode->content.value, value), ERROR_MESSAGE_PREFIX L"Json node does not match the expected type.");
#undef ERROR_MESSAGE_PREFIX
	}

	template<> void ConvertJsonToCustomType<WString>(Ptr<glr::json::JsonNode> node, WString& value)
	{
#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType<WString>(Ptr<JsonNode>, WString&)#"
		auto jsonNode = node.Cast<glr::json::JsonString>();
		CHECK_ERROR(jsonNode, ERROR_MESSAGE_PREFIX L"JSON node does not match the expected type.");
		value = jsonNode->content.value;
#undef ERROR_MESSAGE_PREFIX
	}

	template<> void ConvertJsonToCustomType<wchar_t>(Ptr<glr::json::JsonNode> node, wchar_t& value)
	{
#define ERROR_MESSAGE_PREFIX L"presentation::remoteprotocol::ConvertJsonToCustomType<wchar_t>(Ptr<JsonNode>, wchar_t&)#"
		WString strValue;
		ConvertJsonToCustomType(node, strValue);
		CHECK_ERROR(strValue.Length() == 1, ERROR_MESSAGE_PREFIX L"Char in JSON should be a string with one char.");
		value = strValue[0];
#undef ERROR_MESSAGE_PREFIX
	}

	template<> void ConvertJsonToCustomType<VKEY>(Ptr<glr::json::JsonNode> node, VKEY& value)
	{
		vint intValue;
		ConvertJsonToCustomType(node, intValue);
		value = (VKEY)intValue;
	}

	template<> void ConvertJsonToCustomType<Color>(Ptr<glr::json::JsonNode> node, Color& value)
	{
		WString strValue;
		ConvertJsonToCustomType(node, strValue);
		value = Color::Parse(strValue);
	}

	template<> void ConvertJsonToCustomType<Ptr<stream::MemoryStream>>(Ptr<glr::json::JsonNode> node, Ptr<stream::MemoryStream>& value)
	{
		if (auto jsonLiteral = node.Cast<glr::json::JsonLiteral>())
		{
			if (jsonLiteral->value == glr::json::JsonLiteralValue::Null)
			{
				value = {};
				return;
			}
		}
		else
		{
			WString base64;
			ConvertJsonToCustomType(node, base64);

			value = Ptr(new stream::MemoryStream);

			stream::MemoryWrapperStream base64WStringStream((void*)base64.Buffer(), base64.Length() * sizeof(wchar_t));
			stream::UtfGeneralDecoder<wchar_t, char8_t> wcharToUtf8Decoder;
			stream::DecoderStream wcharToUtf8Stream(base64WStringStream, wcharToUtf8Decoder);
			stream::Utf8Base64Decoder base64Utf8ToBinaryDecoder;
			stream::DecoderStream base64Utf8ToBinaryStream(wcharToUtf8Stream, base64Utf8ToBinaryDecoder);

			stream::CopyStream(base64Utf8ToBinaryStream, *value.Obj());
		}
	}
}
